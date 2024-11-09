/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/21 21:21:33 by abenamar          #+#    #+#             */
/*   Updated: 2024/11/09 16:49:37 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.hpp"

std::map<std::string, std::string, irc::utils::t_istringcomp> const irc::Command::ERRORS = utils::arrays_to_imap<std::string>(
    (char const *[]){
        ERR_NOSUCHSERVER,
        ERR_INPUTTOOLONG,
        ERR_UNKNOWNCOMMAND,
        ERR_NOMOTD,
        ERR_NONICKNAMEGIVEN,
        ERR_ERRONEUSNICKNAME,
        ERR_NICKNAMEINUSE,
        ERR_SUMMONDISABLED,
        ERR_USERSDISABLED,
        ERR_NOTREGISTERED,
        ERR_NEEDMOREPARAMS,
        ERR_ALREADYREGISTRED,
        ERR_PASSWDMISMATCH,
        ERR_BADCHANMASK},
    (char const *[]){
        "No such server",
        "Input line was too long",
        "Unknown command",
        "MOTD File is missing",
        "No nickname given",
        "Erroneous nickname",
        "Nickname is already in use",
        "SUMMON has been disabled",
        "USERS has been disabled",
        "You have not registered",
        "Not enough parameters",
        "Unauthorized command (already registered)",
        "Password incorrect",
        "Invalid channel name"},
    14);

std::map<std::string, irc::Command::t_command, irc::utils::t_istringcomp> const irc::Command::COMMANDS = utils::arrays_to_imap<irc::Command::t_command>(
    (char const *[]){
        CMD_PASS,
        CMD_NICK,
        CMD_USER,
        CMD_OPER,
        CMD_QUIT,
        CMD_MOTD,
        CMD_VERSION,
        CMD_JOIN,
        CMD_TOPIC},
    (irc::Command::t_command[]){
        &irc::Command::pass,
        &irc::Command::nick,
        &irc::Command::user,
        &irc::Command::oper,
        &irc::Command::quit,
        &irc::Command::motd,
        &irc::Command::version,
        &irc::Command::join,
        &irc::Command::topic},
    9);

void irc::Command::reply(std::string const &error, Client const &client, std::string const &argument)
try
{
    std::map<std::string, std::string, utils::t_istringcomp>::const_iterator cit = Command::ERRORS.find(error);

    if (cit == Command::ERRORS.end())
        throw std::out_of_range("std::out_of_range: " + error + ": unknown error reply");

    Server::getInstance().produce(client, Message::Builder()
                                              .withPrefix(Config::getInstance().getText(PRP_SERVERNAME))
                                              .withCommand(error)
                                              .addParameter(client.getNickname())
                                              .addParameter(argument)
                                              .addParameter(cit->second)
                                              .build());

    return (Server::getInstance().flush());
}
catch (std::exception const &e)
{
    throw std::runtime_error("irc::Command::reply: " + std::string(e.what()));
}

void irc::Command::apply(Message const &message, Client &client)
try
{
    std::set<std::string> prefixes;
    std::map<std::string, Command::t_command, utils::t_istringcomp>::const_iterator cit;

    if (!message.getPrefix().empty())
    {
        prefixes.insert(client.getNickname());
        prefixes.insert(client.getNickname() + '@' + client.getHostaddr());
        prefixes.insert(client.str());

        if (prefixes.find(message.getPrefix()) == prefixes.end())
            throw std::runtime_error("std::runtime_error: message from client must not have a prefix of another user than himself");
    }

    cit = Command::COMMANDS.find(message.getCommand());

    if (cit == Command::COMMANDS.end())
    {
        Command::reply(ERR_UNKNOWNCOMMAND, client, message.getCommand());

        throw std::out_of_range("std::out_of_range: " + message.getCommand() + ": unknown command");
    }

    cit->second(message, client);

    return (Server::getInstance().flush());
}
catch (std::exception const &e)
{
    throw std::runtime_error("irc::Command::apply: " + std::string(e.what()));
}

irc::Command::~Command(void) throw() { return; }

void irc::Command::pass(Message const &message, Client &client)
try
{
    if (message.getParameters().empty())
        return (Command::reply(ERR_NEEDMOREPARAMS, client, message.getCommand()));
    else if (client.isRegistered())
        return (Command::reply(ERR_ALREADYREGISTRED, client, message.getCommand()));

    return (client.setPassword(message.getParameters().at(0)));
}
catch (std::exception const &e)
{
    throw std::runtime_error("irc::Command::pass: " + std::string(e.what()));
}

void irc::Command::nick(Message const &message, Client &client)
try
{
    std::string nickname;
    std::map<int, Client>::const_iterator cit;
    bool registered;

    if (message.getParameters().empty())
        return (Command::reply(ERR_NONICKNAMEGIVEN, client, message.getCommand()));

    nickname = message.getParameters().at(0);

    for (cit = Server::getInstance().getClients().begin(); cit != Server::getInstance().getClients().end(); ++cit)
    {
        if (cit->second.getNickname() != nickname)
            continue;

        if (cit->second.isRegistered())
            return (Command::reply(ERR_NICKNAMEINUSE, client, nickname));

        Command::reply(ERR_NICKNAMEINUSE, cit->second, nickname);

        break;
    }

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
        Server::getInstance().challengeRegistration(client);

    return;
}
catch (std::exception const &e)
{
    throw std::runtime_error("irc::Command::nick: " + std::string(e.what()));
}

void irc::Command::user(Message const &message, Client &client)
try
{
    unsigned long mode;

    if (message.getParameters().size() < 4)
        return (Command::reply(ERR_NEEDMOREPARAMS, client, message.getCommand()));
    else if (client.isRegistered())
        return (Command::reply(ERR_ALREADYREGISTRED, client, message.getCommand()));

    client.setRealname(message.getParameters().at(3));
    client.removeMode(Config::getInstance().getText(PRP_USERMODESINFO)[2]);
    client.removeMode(Config::getInstance().getText(PRP_USERMODESINFO)[0]);

    if (std::istringstream(message.getParameters().at(1)) >> mode)
    {
        if (mode & (1 << 2))
            client.addMode(Config::getInstance().getText(PRP_USERMODESINFO)[2]);
        else if (mode & (1 << 3))
            client.addMode(Config::getInstance().getText(PRP_USERMODESINFO)[0]);
    }

    client.setUsername(message.getParameters().at(0));

    if (client.isRegistered())
        Server::getInstance().challengeRegistration(client);

    return;
}
catch (std::exception const &e)
{
    throw std::runtime_error("irc::Command::user: " + std::string(e.what()));
}

void irc::Command::oper(Message const &message, Client &client)
try
{
    irc::Server *server = &Server::getInstance();

    if (message.getParameters().size() < 2)
        return (Command::reply(ERR_NEEDMOREPARAMS, client, message.getCommand()));
    else if (!client.isRegistered())
        return (Command::reply(ERR_NOTREGISTERED, client, message.getCommand()));
    else if (message.getParameters().at(0) != Config::getInstance().getText(PRP_OPERNAME) || message.getParameters().at(1) != Config::getInstance().getText(PRP_OPERPASS))
        return (Command::reply(ERR_PASSWDMISMATCH, client, message.getCommand()));

    client.addMode(Config::getInstance().getText(PRP_USERMODESINFO)[1]);
    server->produce(CMD_MODE, client, std::string(1, '+') + Config::getInstance().getText(PRP_USERMODESINFO)[1]);

    return (server->produce(RPL_YOUREOPER, client, "You are now an IRC operator"));
}
catch (std::exception const &e)
{
    throw std::runtime_error("irc::Command::oper: " + std::string(e.what()));
}

void irc::Command::quit(Message const &message, Client &client)
try
{
    return (Server::getInstance().removeClient(client, message.getParameters().empty() ? "Client exited" : "Quit: " + utils::sequence_to_string(message.getParameters(), " ")));
}
catch (std::exception const &e)
{
    throw std::runtime_error("irc::Command::quit: " + std::string(e.what()));
}

void irc::Command::motd(Message const &message, Client &client)
try
{
    std::ifstream ifs;
    std::string line;

    if (!message.getParameters().empty() && message.getParameters().at(0) != Config::getInstance().getText(PRP_SERVERNAME))
        return (Command::reply(ERR_NOSUCHSERVER, client, message.getParameters().at(0)));

    ifs.open(Config::getInstance().getText(PRP_MOTDFILE).c_str());

    if (!ifs.is_open())
        return (Command::reply(ERR_NOMOTD, client, Config::getInstance().getText(PRP_SERVERNAME)));

    Server::getInstance().produce(RPL_MOTDSTART, client, Config::getInstance().getText(PRP_SERVERNAME) + " message of the day");

    while (std::getline(ifs, line))
        Server::getInstance().produce(RPL_MOTD, client, line.substr(0, 80));

    Server::getInstance().produce(RPL_ENDOFMOTD, client, "End of message of the day");

    return (ifs.close());
}
catch (std::exception const &e)
{
    throw std::runtime_error("irc::Command::motd: " + std::string(e.what()));
}

void irc::Command::version(Message const &message, Client &client)
try
{
    if (!message.getParameters().empty() && message.getParameters().at(0) != Config::getInstance().getText(PRP_SERVERNAME))
        return (Command::reply(ERR_NOSUCHSERVER, client, message.getParameters().at(0)));

    Server::getInstance().produce(client, Message::Builder()
                                              .withPrefix(Config::getInstance().getText(PRP_SERVERNAME))
                                              .withCommand(RPL_VERSION)
                                              .addParameter(Config::getInstance().getText(PRP_VERSION))
                                              .addParameter(Config::getInstance().getText(PRP_SERVERNAME))
                                              .addParameter("42 ft_irc project")
                                              .build());

    return (Server::getInstance().support(client));
}
catch (std::exception const &e)
{
    throw std::runtime_error("irc::Command::version: " + std::string(e.what()));
}

void irc::Command::join(Message const &message, Client &client)
try
{
    std::size_t cur, cpos, kpos, max;
    std::string names, keys, namebuf, keybuf;
    std::map<std::string, Channel, utils::t_istringcomp>::iterator it;
    Message::Builder builder;

    if (message.getParameters().size() < 1)
        return (Command::reply(ERR_NEEDMOREPARAMS, client, message.getCommand()));
    else if (!client.isRegistered())
        return (Command::reply(ERR_NOTREGISTERED, client, message.getCommand()));

    cpos = 0;
    kpos = 0;
    names = message.getParameters().at(0);

    if (message.getParameters().size() > 1)
        keys = message.getParameters().at(1);

    while (cpos != std::string::npos)
    {
        cur = names.find(',', cpos);
        namebuf = names.substr(cpos, cur - cpos);
        cpos = names.find_first_not_of(',', cur);
        it = Server::channels.find(namebuf);

        if (kpos != std::string::npos)
        {
            cur = keys.find(',', kpos);
            keybuf = keys.substr(kpos, cur - kpos);
            kpos = keys.find_first_not_of(',', cur);
        }

        if (it == Server::channels.end())
        {
            try
            {
                it = Server::channels.insert(std::make_pair(namebuf, Channel(namebuf, client))).first;
            }
            catch (std::exception const &)
            {
                Command::reply(ERR_BADCHANMASK, client, namebuf);

                continue;
            }
        }
        else
        {
            if (!it->second.getKey().empty() && keybuf != it->second.getKey())
            {
                Command::reply(ERR_BADCHANNELKEY, client, it->second.getName());

                continue;
            }

            it->second.addMember(client);
        }

        keybuf.clear();
        builder.withPrefix(client.str()).withCommand(message.getCommand()).withParameters(std::vector<std::string>(1, it->second.getName()));
        Server::getInstance().produce(client, builder.build());
        it->second.publish(client, builder.build());

        max = Config::getInstance().getSize(PRP_LINELEN) - Config::getInstance().getText(PRP_SERVERNAME).length() - std::string(RPL_NAMREPLY).length() - 2 * Config::getInstance().getSize(PRP_NICKLEN) - Message::CRLF.length() - 4;
        namebuf = it->second.users();

        while (!namebuf.empty())
        {
            cur = std::min(namebuf.length(), namebuf.find(' ', max));

            Server::getInstance().produce(client, Message::Builder()
                                                      .withPrefix(Config::getInstance().getText(PRP_SERVERNAME))
                                                      .withCommand(RPL_NAMREPLY)
                                                      .addParameter(client.getNickname())
                                                      .addParameter("=")
                                                      .addParameter(it->second.getName())
                                                      .addParameter(namebuf.substr(0, cur))
                                                      .build());

            namebuf = namebuf.substr(cur);
        }

        Server::getInstance().produce(client, Message::Builder()
                                                  .withPrefix(Config::getInstance().getText(PRP_SERVERNAME))
                                                  .withCommand(RPL_ENDOFNAMES)
                                                  .addParameter(client.getNickname())
                                                  .addParameter(it->second.getName())
                                                  .addParameter("End of NAMES list")
                                                  .build());
    }

    return;
}
catch (std::exception const &e)
{
    throw std::runtime_error("irc::Command::join: " + std::string(e.what()));
}

void irc::Command::topic(Message const &message, Client &client)
{
    try
    {
        (void)message;
        (void)client;
    }
    catch (std::exception const &e)
    {
        throw std::runtime_error("irc::Command::topic: " + std::string(e.what()));
    }

    return;
}

void irc::Command::summon(Message const &message, Client &client) { return (Command::reply(ERR_SUMMONDISABLED, client, message.getCommand())); }

void irc::Command::users(Message const &message, Client &client) { return (Command::reply(ERR_USERSDISABLED, client, message.getCommand())); }
