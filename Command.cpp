/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/21 21:21:33 by abenamar          #+#    #+#             */
/*   Updated: 2024/10/29 11:36:07 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.hpp"

std::map<std::string, std::string> const Command::ERRORS = Command::initErrors();

std::map<std::string, Command::t_command> const Command::COMMANDS = Command::initCommands();

Command::Unknown::Unknown(std::string const &what_arg) : std::out_of_range(what_arg) { return; }

void Command::reply(std::string const &error, Client const &client, std::string const &parameter)
{
    std::map<std::string, std::string>::const_iterator cit = Command::ERRORS.find(error);

    if (cit == Command::ERRORS.end())
        throw Command::Unknown("Command::reply: Command::Unknown: std::out_of_range: " + error + ": unknown error reply");

    return (Server::produce(client, Message::Builder()
                                        .withPrefix(SRV_NAME)
                                        .withCommand(error)
                                        .addParameter(client.getNickname())
                                        .addParameter(parameter)
                                        .addParameter(cit->second)
                                        .build()));
}

Command::t_command Command::apply(std::string const &command)
{
    std::map<std::string, Command::t_command>::const_iterator cit = Command::COMMANDS.find(command);

    if (cit == Command::COMMANDS.end())
        throw Command::Unknown("Command::apply: Command::Unknown: std::out_of_range: " + command + ": unknown command");

    return (cit->second);
}

Command::~Command(void) throw() { return; }

std::map<std::string, std::string> Command::initErrors(void)
{
    std::map<std::string, std::string> map;

    map.insert(std::make_pair(ERR_UNKNOWNCOMMAND, "Unknown command"));
    map.insert(std::make_pair(ERR_NONICKNAMEGIVEN, "No nickname given"));
    map.insert(std::make_pair(ERR_ERRONEUSNICKNAME, "Erroneous nickname"));
    map.insert(std::make_pair(ERR_NICKNAMEINUSE, "Nickname is already in use"));
    map.insert(std::make_pair(ERR_NOTREGISTERED, "You have not registered"));
    map.insert(std::make_pair(ERR_NEEDMOREPARAMS, "Not enough parameters"));
    map.insert(std::make_pair(ERR_ALREADYREGISTRED, "Unauthorized command (already registered)"));
    map.insert(std::make_pair(ERR_NOOPERHOST, "Invalid oper credentials"));

    return (map);
}

std::map<std::string, Command::t_command> Command::initCommands(void)
{
    std::map<std::string, Command::t_command> map;

    map.insert(std::make_pair(CMD_PASS, &Command::pass));
    map.insert(std::make_pair(CMD_NICK, &Command::nick));
    map.insert(std::make_pair(CMD_USER, &Command::user));
    map.insert(std::make_pair(CMD_OPER, &Command::oper));
    map.insert(std::make_pair(CMD_QUIT, &Command::quit));
    // map.insert(std::make_pair(CMD_SQUIT, &Command::squit));
    // map.insert(std::make_pair(CMD_JOIN, &Command::join));
    // map.insert(std::make_pair(CMD_NOTICE, &Command::notice));
    // map.insert(std::make_pair(CMD_PRIVMSG, &Command::privmsg));

    return (map);
}

void Command::pass(Message const &message, Client &client)
{
    try
    {
        if (message.getParameters().empty())
            Command::reply(ERR_NEEDMOREPARAMS, client, message.getCommand());
        else if (client.isRegistered())
            Command::reply(ERR_ALREADYREGISTRED, client, message.getCommand());
        else
            client.setPassword(message.getParameters().at(0));
    }
    catch (std::exception const &e)
    {
        throw std::runtime_error("Command::pass: " + std::string(e.what()));
    }

    return;
}

void Command::nick(Message const &message, Client &client)
{
    std::string nickname;
    std::map<int, Client> const *clients;
    std::map<int, Client>::const_iterator cit;
    bool registered;

    try
    {
        if (message.getParameters().empty())
            Command::reply(ERR_NONICKNAMEGIVEN, client, message.getCommand());
        else
        {
            nickname = message.getParameters().at(0);
            clients = &Server::getInstance().getClients();

            for (cit = clients->begin(); cit != clients->end(); ++cit)
            {
                if (cit->second.getNickname() == nickname)
                {
                    if (cit->second.isRegistered())
                    {
                        Command::reply(ERR_NICKNAMEINUSE, client, nickname);

                        break;
                    }

                    Command::reply(ERR_NICKNAMEINUSE, cit->second, nickname);
                }
            }

            if (cit == clients->end())
            {
                registered = client.isRegistered();

                try
                {
                    client.setNickname(nickname);
                }
                catch (std::exception const &)
                {
                    Command::reply(ERR_ERRONEUSNICKNAME, client, nickname);

                    throw;
                }

                if (!registered && client.isRegistered())
                    Server::getInstance().completeRegistration(client);
            }
        }
    }
    catch (std::exception const &e)
    {
        throw std::runtime_error("Command::nick: " + std::string(e.what()));
    }

    return;
}

void Command::user(Message const &message, Client &client)
{
    unsigned long mode;

    try
    {
        if (message.getParameters().size() < 4)
            Command::reply(ERR_NEEDMOREPARAMS, client, message.getCommand());
        else if (client.isRegistered())
            Command::reply(ERR_ALREADYREGISTRED, client, message.getCommand());
        else
        {
            client.removeMode(USR_MODE_W);
            client.removeMode(USR_MODE_I);

            if (std::istringstream(message.getParameters().at(1)) >> mode)
            {
                if (mode & (1 << 2))
                    client.addMode(USR_MODE_W);
                else if (mode & (1 << 3))
                    client.addMode(USR_MODE_I);
            }

            client.setRealname(message.getParameters().at(3));
            client.setUsername(message.getParameters().at(0));

            if (client.isRegistered())
                Server::getInstance().completeRegistration(client);
        }
    }
    catch (std::exception const &e)
    {
        throw std::runtime_error("Command::user: " + std::string(e.what()));
    }

    return;
}

void Command::oper(Message const &message, Client &client)
{
    if (message.getParameters().size() < 2)
        Command::reply(ERR_NEEDMOREPARAMS, client, message.getCommand());
    else if (!client.isRegistered())
        Command::reply(ERR_NOTREGISTERED, client, message.getCommand());
    else if (message.getParameters().at(0).compare(SRV_OPER_NAME) || message.getParameters().at(1).compare(SRV_OPER_PASSWORD))
        Command::reply(ERR_NOOPERHOST, client, message.getCommand());
    else
    {
        client.addMode(USR_MODE_O);

        Server::produce(client, CMD_MODE, "+o");
        Server::produce(client, RPL_YOUREOPER, "You are now an IRC operator");
    }
}

void Command::quit(Message const &message, Client &client)
{
    std::ostringstream o;

    try
    {
        if (message.getParameters().empty())
            o << "Client exited";
        else
        {
            o << "Quit:";

            for (std::vector<std::string>::const_iterator cit = message.getParameters().begin(); cit != message.getParameters().end(); ++cit)
                o << ' ' << *cit;
        }

        Server::produce(client, Message::Builder()
                                    .withCommand(CMD_ERROR)
                                    .addParameter("Closing link: (" + client.userId() + ") [" + o.str() + ']')
                                    .build());
        Server::getInstance().removeClient(client);
    }
    catch (std::exception const &e)
    {
        throw std::runtime_error("Command::quit: " + std::string(e.what()));
    }

    return;
}

// void Command::notice(Message const &message, Client &client)
// {
//     try
//     {
//         if (message.getParameters().empty() || message.getParameters().at(1).empty())
//             return;

//         std::string mask = message.getParameters().at(0);

//         if (mask[0] == '#')
//         {
//             std::size_t idx = mask.find_last_of(".");
//             if (idx != std::string::npos && mask.substr(idx, mask.length()).find("*") == std::string::npos)
//             {
//                 // TO DO
//             }
//         }
//         else
//         {
//             int target_fd = server.getclient.getSocket()(mask);
//             if (target_fd >= 0)
//                 Server::produce(target_fd, client.getNickname(), CMD_NOTICE).addParameter(mask).addParameter(message.getParameters().at(1));
//         }
//     }
//     catch (std::exception const &e)
//     {
//         throw std::runtime_error("Command::notice: " + std::string(e.what()));
//     }

//     return;
// }

// void Command::privmsg(Message const &message, Client &client)
// {
//     // ERR_TOOMANYTARGETS Not done
//     // reste a faire le parsing sur les wildcards + envoyer message sur un channel
//     try
//     {
//         // ou faire une condition ou l'on commence par le message
//         if (message.getParameters().empty())
//             Server::produce(client, Message::Builder()
//                                         .withCommand("411", "No recipient given (PRIVMSG)");
//         else if (server.getclient.getSocket()(message.getParameters().at(0)) < 0)
//             Server::produce(client, Message::Builder()
//                                         .withCommand("401")
//                                         .addParameter(message.getParameters().at(0))
//                                         .addParameter("No such nick/channel");
//         else if (message.getParameters().size() < 2 || message.getParameters().at(1).empty())
//             // || message.getParameters().at(1)[0] != ':') j'aimerais pouvoir check ca, possible de remettre les : (?)
//             Server::produce(client, Message::Builder()
//                                         .withCommand("412", " :No text to server.write");
//         // host mask (#<mask>), pas de server mask, ici 1 seul serveur
//         std::string mask = message.getParameters().at(0);
//         if (message.getParameters().at(0)[0] == '#')
//         {
//                 std::size_t idx = mask.find_last_of(".");
//                 // ERR_NOTOPLEVEL, manque le .com
//                 if (idx == std::string::npos)
//                 Server::produce(client, Message::Builder()
//                                         .withCommand("413")
//                                         .addParameter(mask)
//                                         .addParameter("No toplevel domain specified");
//             // ERR_WILDTOPLEVEL, un * apres le dernier point
//             else if (mask.substr(idx, mask.length()).find("*") != std::string::npos)
//                 Server::produce(client, Message::Builder()
//                                         .withCommand("414")
//                                         .addParameter(mask)
//                                         .addParameter("Wildcard in toplevel domain");
//                 // TODO: envoyer au bon utilisateur selon son host mask
//         }
//         else
//             // encore une fois, j'utilise getNickname mais on devrait utiliser <nick>!<user>@<host>
//             // la ligne est un peu longue aussi
//             Server::produce(server.getclient.getSocket()(message.getParameters().at(0)), client.getNickname(), CMD_PRIVMSG)
//                                         .addParameter(message.getParameters().at(0) + ' ' + message.getParameters().at(1));
//     }
//     catch (std::exception const &e)
//     {
//         throw std::runtime_error("Command::privmsg: " + std::string(e.what()));
//     }

//     return;
// }

// void Command::join(Message const &message, Client & client)
// {
//     std::string nick = client.getNickname();

//     try {
//         if (message.getParameters().empty())
// 		{
//             server.write(client.getSocket(), Message::Builder().withCommand("461 " + nick + " JOIN :Not enough parameters");
// 			return ;
// 		}

// 		// faire une boucle pour tous les channels avec , en separateur
//         std::string channelName = message.getParameters().at(0);
//         std::string key = message.getParameters().size() > 1 ? message.getParameters().at(1) : "";

//         if (client.getChannels().size() >= 20000)
//             server.write(client.getSocket(), Message::Builder().withCommand("405 " + nick + " " + channelName).addParameter("You have joined too many channels");
//         else if (!isValidChannelName(channelName)) {
//             server.write(client.getSocket(), Message::Builder().withCommand("476 " + nick + " " + channelName).addParameter("Bad Channel Mask");
//             return;
//         }

//         Channel *channel = server.findOrCreateChannel(channelName);

//         if (channel->isInviteOnly() && !channel->isUserInvited(&client)) {
//             server.write(client.getSocket(), Message::Builder().withCommand("473 " + nick + " " + channelName).addParameter("Cannot join channel (+i)");
//             return;
//         }

//         if (channel->hasKey() && channel->getKey() != key) {
//             server.write(client.getSocket(), Message::Builder().withCommand("475 " + nick + " " + channelName).addParameter("Cannot join channel (+k)");
//             return;
//         }

//         if (channel->isFull())
// 		{
//             server.write(client.getSocket(), Message::Builder().withCommand("471 " + nick + " " + channelName).addParameter("Cannot join channel (+l)");
//             return;
// 		}

//         channel->addUser(&client);
//         client.addChannel(channel);

//         channel->broadcast(":" + nick + " JOIN " + channelName);

//         if (!channel->getTopic().empty())
//             server.write(client.getSocket(), Message::Builder().withCommand("332 " + nick + " " + channelName).addParameter(channel->getTopic());

//     }
// 	catch (std::exception const &e)
// 	{
//         throw std::runtime_error("Command::join: " + std::string(e.what()));
//     }

// 	return ;
// }
