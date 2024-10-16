/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejankovs <ejankovs@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/21 21:21:33 by abenamar          #+#    #+#             */
/*   Updated: 2024/10/16 20:32:25 by ejankovs         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.hpp"

std::map<std::string, void (*)(Client &, Server &)> const Command::APPLY = Command::createApplyMap();

std::map<std::string, void (*)(Client &, Server &)> Command::createApplyMap(void)
{
    std::map<std::string, void (*)(Client &, Server &)> map;

    map["PASS"] = &Command::pass;
    map["NICK"] = &Command::nick;
    map["USER"] = &Command::user;
    map["QUIT"] = &Command::quit;
    map["PRIVMSG"] = &Command::privmsg;
	map["NOTICE"] = &Command::notice;
    map["JOIN"] = &Command::join;

    return (map);
}

Command::~Command(void) throw() { return; }

Command &Command::operator=(Command const & /* rhs */) throw() { return (*this); }

void Command::pass(Client &client, Server &server)
{
    Message const &message = client.message();
    ssize_t nwrite = 0;

    try
    {
        if (message.getParameters().empty())
            nwrite = send(client.getSocket(), "461 PASS :Not enough parameters\r\n", 33, 0);
        else if (message.getParameters().at(0) == server.getPassword())
            client.setAuthorized(true);

        if (nwrite == -1)
            throw RuntimeErrno("send");
    }
    catch (std::exception const &e)
    {
        throw std::runtime_error("Command::pass: " + std::string(e.what()));
    }

    return;
}

void Command::nick(Client &client, Server &server)
{
    Message const &message = client.message();
    std::string const &nickname = message.getParameters().at(0);
    ssize_t nwrite = 0;
    std::map<int, Client *const>::iterator it = server.getClients().begin();

    try
    {
        if (message.getParameters().empty())
            nwrite = send(client.getSocket(), "431 :No nickname given\r\n", 24, 0);
        else if (client.isRegistered())
            nwrite = send(client.getSocket(), "462 :Unauthorized command (already registered)\r\n", 48, 0);
        else if (std::find_if(nickname.begin(), nickname.end(), Message::isNotInNicknameFormat) != nickname.end())
            nwrite = send(client.getSocket(), ("432 " + nickname + " :Erroneous nickname\r\n").c_str(), nickname.length() + 26, 0);
        else
            for (; it != server.getClients().end(); ++it)
            {
                if (it->second->getNickname() == nickname)
                {
                    nwrite = send(client.getSocket(), ("433 " + nickname + " :Nickname is already in use\r\n").c_str(), nickname.length() + 34, 0);

                    break;
                }
            }

        if (it == server.getClients().end())
        {
            client.setNickname(nickname);

            if (client.isRegistered())
                nwrite = send(client.getSocket(), ("001 " + client.getNickname() + " :Welcome to the ft_irc Network, " + client.getNickname() + "\r\n").c_str(), client.getNickname().length() * 2 + 39, 0);
            else if (!client.isAuthorized() && client.isIdentified())
                nwrite = send(client.getSocket(), "464 :Password incorrect\r\n", 25, 0);
        }

        if (nwrite == -1)
            throw RuntimeErrno("send");
    }
    catch (std::exception const &e)
    {
        throw std::runtime_error("Command::nick: " + std::string(e.what()));
    }

    return;
}

void Command::user(Client &client, Server &server)
{
    Message const &message = client.message();
    ssize_t nwrite = 0;

    (void)server;

    try
    {
        if (message.getParameters().size() < 4)
            nwrite = send(client.getSocket(), "461 USER :Not enough parameters\r\n", 33, 0);
        else if (client.isRegistered())
            nwrite = send(client.getSocket(), "462 :Unauthorized command (already registered)\r\n", 48, 0);
        else
        {
            client.identify(message.getParameters().at(0));

            if (client.isRegistered())
                nwrite = send(client.getSocket(), ("001 " + client.getNickname() + " :Welcome to the ft_irc Network, " + client.getNickname() + "\r\n").c_str(), client.getNickname().length() * 2 + 39, 0);
            else if (!client.isAuthorized() && !client.getNickname().empty())
                nwrite = send(client.getSocket(), "464 :Password incorrect\r\n", 25, 0);
        }

        if (nwrite == -1)
            throw RuntimeErrno("send");
    }
    catch (std::exception const &e)
    {
        throw std::runtime_error("Command::user: " + std::string(e.what()));
    }

    return;
}

void Command::quit(Client &client, Server &server)
{
    (void)server;

    client.quit();

    return;
}

void Command::privmsg(Client &client, Server &server)
{
    Message const &message = client.message();
    ssize_t nwrite = 0;

    (void)server;

    // ERR_TOOMANYTARGETS Not done
    // reste a faire le parsing sur les wildcards + envoyer message sur un channel
    try
    {
        // ou faire une condition ou l'on commence par le message
        if (message.getParameters().empty())
            nwrite = send(client.getSocket(), "411 :No recipient given (PRIVMSG)\r\n", 35, 0);
        else if (server.getConnfd(message.getParameters().at(0)) < 0)
            nwrite = send(client.getSocket(), ("401 " + message.getParameters().at(0) + " :No such nick/channel\r\n").c_str(), message.getParameters().at(0).length() + 27, 0);
        else if (message.getParameters().size() < 2 || message.getParameters().at(1).empty())
            // || message.getParameters().at(1)[0] != ':') j'aimerais pouvoir check ca, possible de remettre les : (?)
            nwrite = send(client.getSocket(), "412 :No text to send\r\n", 22, 0);
        // host mask (#<mask>), pas de server mask, ici 1 seul serveur
        std::string mask = message.getParameters().at(0);
        if (message.getParameters().at(0)[0] == '#')
        {
            std::size_t idx = mask.find_last_of(".");
            // ERR_NOTOPLEVEL, manque le .com
            if (idx == std::string::npos)
                nwrite = send(client.getSocket(), ("413 " + mask + " :No toplevel domain specified\r\n").c_str(), mask.length() + 36, 0);
            // ERR_WILDTOPLEVEL, un * apres le dernier point
            else if (mask.substr(idx, mask.length()).find("*") != std::string::npos)
                nwrite = send(client.getSocket(), ("414 " + mask + " :Wildcard in toplevel domain\r\n").c_str(), mask.length() + 35, 0);
            // TODO: envoyer au bon utilisateur selon son host mask
        }
        else
            // encore une fois, j'utilise getNickname mais on devrait utiliser <nick>!<user>@<host>
            // la ligne est un peu longue aussi
            nwrite = send(server.getConnfd(message.getParameters().at(0)), (":" + client.getNickname() + " PRIVMSG " + message.getParameters().at(0) + " " + message.getParameters().at(1)).c_str(), 11 + client.getNickname().length() + message.getParameters().at(0).length() + message.getParameters().at(1).length(), 0);

        if (nwrite == -1)
            throw RuntimeErrno("send");
    }
    catch (std::exception const &e)
    {
        throw std::runtime_error("Command::privmsg: " + std::string(e.what()));
    }

    return;
}

void Command::notice(Client &client, Server &server)
{
    Message const &message = client.message();
    ssize_t nwrite = 0;

    (void)server;

    try
    {
        if (message.getParameters().empty() || message.getParameters().at(1).empty())
            return;

        std::string mask = message.getParameters().at(0);

        if (mask[0] == '#') {
            std::size_t idx = mask.find_last_of(".");
            if (idx != std::string::npos && mask.substr(idx, mask.length()).find("*") == std::string::npos) {
				// TO DO
            }
        }
		else
		{
            int target_fd = server.getConnfd(mask);
            if (target_fd >= 0)
                nwrite = send(target_fd, (":" + client.getNickname() + " NOTICE " + mask + " :" + message.getParameters().at(1) + "\r\n").c_str(), client.getNickname().length() + mask.length() + message.getParameters().at(1).length() + 12, 0);
        }

        if (nwrite == -1)
            throw RuntimeErrno("send");
    }
    catch (std::exception const &e)
    {
        throw std::runtime_error("Command::notice: " + std::string(e.what()));
    }

    return;
}

void Command::join(Client &client, Server &server)
{
    Message const &message = client.message();
    std::string nick = client.getNickname();
    ssize_t nwrite = 0;

    try {
        if (message.getParameters().empty())
		{
            nwrite = send(client.getSocket(), ("461 " + nick + " JOIN :Not enough parameters\r\n").c_str(), 39 + nick.length(), 0);
			return ;
		}

		// faire une boucle pour tous les channels avec , en separateur
        std::string channelName = message.getParameters().at(0);
        std::string key = message.getParameters().size() > 1 ? message.getParameters().at(1) : "";

        if (client.getChannels().size() >= 20000)
            nwrite = send(client.getSocket(), ("405 " + nick + " " + channelName + " :You have joined too many channels\r\n").c_str(), 55 + nick.length() + channelName.length(), 0);
        else if (!isValidChannelName(channelName)) {
            nwrite = send(client.getSocket(), ("476 " + nick + " " + channelName + " :Bad Channel Mask\r\n").c_str(), 40 + nick.length() + channelName.length(), 0);
            return;
        }

        Channel *channel = server.findOrCreateChannel(channelName);

        if (channel->isInviteOnly() && !channel->isUserInvited(&client)) {
            nwrite = send(client.getSocket(), ("473 " + nick + " " + channelName + " :Cannot join channel (+i)\r\n").c_str(), 50 + nick.length() + channelName.length(), 0);
            return;
        }

        if (channel->hasKey() && channel->getKey() != key) {
            nwrite = send(client.getSocket(), ("475 " + nick + " " + channelName + " :Cannot join channel (+k)\r\n").c_str(), 50 + nick.length() + channelName.length(), 0);
            return;
        }

        if (channel->isFull())
		{
            nwrite = send(client.getSocket(), ("471 " + nick + " " + channelName + " :Cannot join channel (+l)\r\n").c_str(), 50 + nick.length() + channelName.length(), 0);
            return;
		}
		
        channel->addUser(&client);
        client.addChannel(channel);

        channel->broadcast(":" + nick + " JOIN " + channelName + "\r\n");

        if (!channel->getTopic().empty())
            nwrite = send(client.getSocket(), ("332 " + nick + " " + channelName + " :" + channel->getTopic() + "\r\n").c_str(), 35 + nick.length() + channelName.length() + channel->getTopic().length(), 0);

        if (nwrite == -1)
            throw RuntimeErrno("send");

    }
	catch (std::exception const &e)
	{
        throw std::runtime_error("Command::join: " + std::string(e.what()));
    }

	return ;
}
