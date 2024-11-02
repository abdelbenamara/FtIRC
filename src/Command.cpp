/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/21 21:21:33 by abenamar          #+#    #+#             */
/*   Updated: 2024/11/02 15:09:24 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.hpp"

std::map<std::string, std::string> const irc::Command::ERRORS = utils::arrays_to_map<std::string>(
    (char const *[]){
        ERR_UNKNOWNCOMMAND,
        ERR_NONICKNAMEGIVEN,
        ERR_ERRONEUSNICKNAME,
        ERR_NICKNAMEINUSE,
        ERR_SUMMONDISABLED,
        ERR_USERSDISABLED,
        ERR_NOTREGISTERED,
        ERR_NEEDMOREPARAMS,
        ERR_ALREADYREGISTRED,
        ERR_BADCHANMASK,
        ERR_NOOPERHOST},
    (char const *[]){
        "Unknown command",
        "No nickname given",
        "Erroneous nickname",
        "Nickname is already in use",
        "SUMMON has been disabled",
        "USERS has been disabled",
        "You have not registered",
        "Not enough parameters",
        "Unauthorized command (already registered)",
        "Invalid channel name"
        "Invalid oper credentials"},
    11);

std::map<std::string, irc::Command::t_command> const irc::Command::COMMANDS = utils::arrays_to_map<irc::Command::t_command>(
    (char const *[]){
        CMD_PASS,
        CMD_NICK,
        CMD_USER,
        CMD_OPER,
        CMD_QUIT,
        CMD_JOIN},
    (irc::Command::t_command[]){
        &irc::Command::pass,
        &irc::Command::nick,
        &irc::Command::user,
        &irc::Command::oper,
        &irc::Command::quit,
        &irc::Command::join},
    6);

void irc::Command::reply(std::string const &error, Client const &client, std::string const &argument)
{
    std::map<std::string, std::string>::const_iterator cit = irc::Command::ERRORS.find(error);

    if (cit == irc::Command::ERRORS.end())
        throw std::out_of_range("irc::Command::reply: std::out_of_range: " + error + ": unknown error reply");

    return (Server::produce(client, Message::Builder()
                                        .withPrefix(SRV_NAME)
                                        .withCommand(error)
                                        .addParameter(client.getNickname())
                                        .addParameter(argument)
                                        .addParameter(cit->second)
                                        .build()));
}

void irc::Command::apply(Message const &message, Client &client)
{
    std::set<std::string> prefixes;
    std::map<std::string, irc::Command::t_command>::const_iterator cit;

    try
    {
        if (!message.getPrefix().empty())
        {
            prefixes.insert(client.getNickname());
            prefixes.insert(client.getNickname() + '@' + client.getHostaddr());
            prefixes.insert(client.str());

            if (prefixes.find(message.getPrefix()) == prefixes.end())
                throw std::runtime_error("std::runtime_error: message from client must not have a prefix of another user than himself");
        }

        cit = irc::Command::COMMANDS.find(message.getCommand());

        if (cit == irc::Command::COMMANDS.end())
        {
            Command::reply(ERR_UNKNOWNCOMMAND, client, message.getCommand());

            throw std::out_of_range("std::out_of_range: " + message.getCommand() + ": unknown command");
        }
    }
    catch (std::exception const &e)
    {
        throw std::runtime_error("irc::Command::apply: " + std::string(e.what()));
    }

    return (cit->second(message, client));
}

irc::Command::~Command(void) throw() { return; }

void irc::Command::pass(Message const &message, Client &client)
{
    try
    {
        if (message.getParameters().empty())
            return (irc::Command::reply(ERR_NEEDMOREPARAMS, client, message.getCommand()));
        else if (client.isRegistered())
            return (irc::Command::reply(ERR_ALREADYREGISTRED, client, message.getCommand()));

        client.setPassword(message.getParameters().at(0));
    }
    catch (std::exception const &e)
    {
        throw std::runtime_error("irc::Command::pass: " + std::string(e.what()));
    }

    return;
}

void irc::Command::nick(Message const &message, Client &client)
{
    std::string nickname;
    std::map<int, Client>::const_iterator cit;
    bool registered;

    try
    {
        if (message.getParameters().empty())
            return (irc::Command::reply(ERR_NONICKNAMEGIVEN, client, message.getCommand()));

        nickname = message.getParameters().at(0);

        for (cit = Server::getInstance().getClients().begin(); cit != Server::getInstance().getClients().end(); ++cit)
        {
            if (cit->second.getNickname() != nickname)
                continue;

            if (cit->second.isRegistered())
                return (irc::Command::reply(ERR_NICKNAMEINUSE, client, nickname));

            irc::Command::reply(ERR_NICKNAMEINUSE, cit->second, nickname);

            break;
        }

        registered = client.isRegistered();

        try
        {
            client.setNickname(nickname);
        }
        catch (std::exception const &)
        {
            irc::Command::reply(ERR_ERRONEUSNICKNAME, client, nickname);

            throw;
        }

        if (!registered && client.isRegistered())
            Server::getInstance().challengeRegistration(client);
    }
    catch (std::exception const &e)
    {
        throw std::runtime_error("irc::Command::nick: " + std::string(e.what()));
    }

    return;
}

void irc::Command::user(Message const &message, Client &client)
{
    unsigned long mode;

    try
    {
        if (message.getParameters().size() < 4)
            return (irc::Command::reply(ERR_NEEDMOREPARAMS, client, message.getCommand()));
        else if (client.isRegistered())
            return (irc::Command::reply(ERR_ALREADYREGISTRED, client, message.getCommand()));

        client.setRealname(message.getParameters().at(3));
        client.removeMode(USR_MODE_W);
        client.removeMode(USR_MODE_I);

        if (std::istringstream(message.getParameters().at(1)) >> mode)
        {
            if (mode & (1 << 2))
                client.addMode(USR_MODE_W);
            else if (mode & (1 << 3))
                client.addMode(USR_MODE_I);
        }

        client.setUsername(message.getParameters().at(0));

        if (client.isRegistered())
            Server::getInstance().challengeRegistration(client);
    }
    catch (std::exception const &e)
    {
        throw std::runtime_error("irc::Command::user: " + std::string(e.what()));
    }

    return;
}

void irc::Command::oper(Message const &message, Client &client)
{
    try
    {
        if (message.getParameters().size() < 2)
            return (irc::Command::reply(ERR_NEEDMOREPARAMS, client, message.getCommand()));
        else if (!client.isRegistered())
            return (irc::Command::reply(ERR_NOTREGISTERED, client, message.getCommand()));
        else if (message.getParameters().at(0).compare(SRV_OPER_NAME) || message.getParameters().at(1).compare(SRV_OPER_PASSWORD))
            return (irc::Command::reply(ERR_NOOPERHOST, client, message.getCommand()));

        client.addMode(USR_MODE_O);
        Server::produce(client, CMD_MODE, "+o");
        Server::produce(client, RPL_YOUREOPER, "You are now an IRC operator");
    }
    catch (std::exception const &e)
    {
        throw std::runtime_error("irc::Command::oper: " + std::string(e.what()));
    }

    return;
}

void irc::Command::quit(Message const &message, Client &client)
{
    try
    {
        Server::getInstance().removeClient(client, message.getParameters().empty() ? "Client exited" : "Quit: " + utils::sequence_to_string(message.getParameters(), " "));
    }
    catch (std::exception const &e)
    {
        throw std::runtime_error("irc::Command::quit: " + std::string(e.what()));
    }

    return;
}

void irc::Command::join(Message const &message, Client &client)
{
    irc::Server *server;
    std::size_t cur, cpos, kpos;
    std::string names, keys, buf;
    irc::Channel *channel;

    try
    {
        if (message.getParameters().size() < 1)
            return (irc::Command::reply(ERR_NEEDMOREPARAMS, client, message.getCommand()));
        else if (!client.isRegistered())
            return (irc::Command::reply(ERR_NOTREGISTERED, client, message.getCommand()));

        server = &irc::Server::getInstance();
        cpos = 0;
        kpos = 0;
        names = message.getParameters().at(0);

        if (message.getParameters().size() > 1)
            keys = message.getParameters().at(1);

        while (cpos < names.length())
        {
            cur = names.find(',', cpos);
            buf = names.substr(cpos, cur - cpos);
            cpos = cur + 1;

            if (server->getChannels().find(buf) == server->getChannels().end())
            {
                try
                {
                    channel = &server->getChannels().insert(std::make_pair(buf, irc::Channel(buf))).first->second;
                }
                catch (std::exception const &)
                {
                    irc::Command::reply(ERR_BADCHANMASK, client, buf);

                    continue;
                }

                channel->addMember(client);
                channel->addOperator(client);

                continue;
            }

            if (kpos < keys.length())
            {
                cur = keys.find(',', kpos);
                buf = keys.substr(kpos, cur - kpos);
                kpos = cur + 1;
            }

            if (!channel->getKey().empty() && buf != channel->getKey())
            {
                irc::Command::reply(ERR_BADCHANNELKEY, client, channel->getName());

                continue;
            }

            Server::produce(client, Message::Builder()
                                        .withPrefix(client.str())
                                        .withCommand(message.getCommand())
                                        .addParameter(channel->getName())
                                        .build());
        }
    }
    catch (std::exception const &e)
    {
        throw std::runtime_error("irc::Command::join: " + std::string(e.what()));
    }

    return;
}

void irc::Command::summon(Message const &message, Client &client) { return (irc::Command::reply(ERR_SUMMONDISABLED, client, message.getCommand())); }

void irc::Command::users(Message const &message, Client &client) { return (irc::Command::reply(ERR_USERSDISABLED, client, message.getCommand())); }
