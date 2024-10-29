/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/21 21:05:45 by abenamar          #+#    #+#             */
/*   Updated: 2024/10/29 11:34:22 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __COMMAND_HPP__
#define __COMMAND_HPP__

#include <algorithm>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include "Client.hpp"
#include "Message.hpp"
#include "Server.hpp"

#define CMD_PASS "PASS"
#define CMD_NICK "NICK"
#define CMD_USER "USER"
#define CMD_OPER "OPER"
#define CMD_QUIT "QUIT"
#define CMD_SQUIT "SQUIT"
#define CMD_NOTICE "NOTICE"
#define CMD_PRIVMSG "PRIVMSG"
#define CMD_JOIN "JOIN"
#define CMD_MODE "MODE"
#define CMD_TOPIC "TOPIC"
#define CMD_INVITE "INVITE"
#define CMD_KICK "KICK"
#define CMD_ERROR "ERROR"

#define RPL_WELCOME "001"
#define RPL_YOUREOPER "381"

#define ERR_UNKNOWNCOMMAND "421"
#define ERR_NONICKNAMEGIVEN "431"
#define ERR_ERRONEUSNICKNAME "432"
#define ERR_NICKNAMEINUSE "433"
#define ERR_NOTREGISTERED "451"
#define ERR_NEEDMOREPARAMS "461"
#define ERR_ALREADYREGISTRED "462"
#define ERR_NOOPERHOST "491"

class Server;

class Command
{
public:
    class Unknown : public std::out_of_range
    {
    public:
        explicit Unknown(std::string const &what_arg);
    };

    typedef void (*t_command)(Message const &, Client &);

    static void reply(std::string const &error, Client const &client, std::string const &parameter);
    static t_command apply(std::string const &command);

    virtual ~Command(void) throw();

private:
    static std::map<std::string, std::string> const ERRORS;
    static std::map<std::string, t_command> const COMMANDS;

    static std::map<std::string, std::string> initErrors(void);
    static std::map<std::string, t_command> initCommands(void);
    static void pass(Message const &message, Client &client);
    static void nick(Message const &message, Client &client);
    static void user(Message const &message, Client &client);
    static void oper(Message const &message, Client &client);
    static void quit(Message const &message, Client &client);
    // static void squit(Message const &message, Client &client);
    // static void join(Message const &message, Client &client);
    // static void notice(Message const &message, Client &client);
    // static void privmsg(Message const &message, Client &client);
    // static void mode(Message const &message, Client &client);
    // static void topic(Message const &message, Client &client);
    // static void invite(Message const &message, Client &client);
    // static void kick(Message const &message, Client &client);

    Command(void);                               /* = delete (C++11) */
    Command(Command const &);                    /* = delete (C++11) */
    Command &operator=(Command const &) throw(); /* = delete (C++11) */
};

#endif
