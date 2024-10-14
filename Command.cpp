/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/21 21:21:33 by abenamar          #+#    #+#             */
/*   Updated: 2024/10/14 15:49:13 by abenamar         ###   ########.fr       */
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
    std::map<int, Client *const>::const_iterator it = server.getClientsBegin();

    try
    {
        if (message.getParameters().empty())
            nwrite = send(client.getSocket(), "431 :No nickname given\r\n", 24, 0);
        else if (std::find_if(nickname.begin(), nickname.end(), Message::isNotInNicknameFormat) != nickname.end())
            nwrite = send(client.getSocket(), ("432 " + nickname + " :Erroneous nickname\r\n").c_str(), nickname.length() + 26, 0);
        else
            for (; it != server.getClientsEnd(); ++it)
            {
                if (it->second->getNickname() == nickname)
                {
                    nwrite = send(client.getSocket(), ("433 " + nickname + " :Nickname is already in use\r\n").c_str(), nickname.length() + 34, 0);

                    break;
                }
            }

        if (it == server.getClientsEnd())
            client.setNickname(nickname);

        if (client.isRegistered())
            nwrite = send(client.getSocket(), ("001 " + client.getNickname() + " :Welcome to the ft_irc Network, " + client.getNickname() + "\r\n").c_str(), client.getNickname().length() * 2 + 39, 0);
        else if (!client.isAuthorized() && client.isIdentified())
            nwrite = send(client.getSocket(), "464 :Password incorrect\r\n", 25, 0);

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
            client.identify();

        if (client.isRegistered())
            nwrite = send(client.getSocket(), ("001 " + client.getNickname() + " :Welcome to the ft_irc Network, " + client.getNickname() + "\r\n").c_str(), client.getNickname().length() * 2 + 39, 0);
        else if (!client.isAuthorized() && !client.getNickname().empty())
            nwrite = send(client.getSocket(), "464 :Password incorrect\r\n", 25, 0);

        if (nwrite == -1)
            throw RuntimeErrno("send");
    }
    catch (std::exception const &e)
    {
        throw std::runtime_error("Command::user: " + std::string(e.what()));
    }

    return;
}
