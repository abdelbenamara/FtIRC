/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/21 21:05:45 by abenamar          #+#    #+#             */
/*   Updated: 2024/11/09 16:12:21 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __COMMAND_HPP__
#define __COMMAND_HPP__

#include <algorithm>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include "Channel.hpp"
#include "Client.hpp"
#include "Message.hpp"
#include "Server.hpp"
#include "utils.hpp"

/* ************************************************************************** */
/*                                                                            */
/*                                 commands                                   */
/*                                                                            */
/* ************************************************************************** */

#define CMD_PASS "PASS"
#define CMD_NICK "NICK"
#define CMD_USER "USER"
#define CMD_OPER "OPER"
#define CMD_QUIT "QUIT"
#define CMD_SQUIT "SQUIT"
#define CMD_PRIVMSG "PRIVMSG"
#define CMD_NOTICE "NOTICE"
#define CMD_MOTD "MOTD"
#define CMD_VERSION "VERSION"
#define CMD_JOIN "JOIN"
#define CMD_MODE "MODE"
#define CMD_TOPIC "TOPIC"
#define CMD_INVITE "INVITE"
#define CMD_KICK "KICK"
#define CMD_ERROR "ERROR"
#define CMD_SUMMON "SUMMON"
#define CMD_USERS "USERS"

/* ************************************************************************** */
/*                                                                            */
/*                              numeric replies                               */
/*                                                                            */
/* ************************************************************************** */

#define RPL_WELCOME "001"
#define RPL_YOURHOST "002"
#define RPL_CREATED "003"
#define RPL_MYINFO "004"
#define RPL_ISUPPORT "005"
#define RPL_VERSION "351"
#define RPL_NAMREPLY "353"
#define RPL_ENDOFNAMES "366"
#define RPL_MOTD "372"
#define RPL_MOTDSTART "375"
#define RPL_ENDOFMOTD "376"
#define RPL_YOUREOPER "381"

/* ************************************************************************** */
/*                                                                            */
/*                                   errors                                   */
/*                                                                            */
/* ************************************************************************** */

#define ERR_NOSUCHSERVER "402"
#define ERR_INPUTTOOLONG "417"
#define ERR_UNKNOWNCOMMAND "421"
#define ERR_NOMOTD "422"
#define ERR_NONICKNAMEGIVEN "431"
#define ERR_ERRONEUSNICKNAME "432"
#define ERR_NICKNAMEINUSE "433"
#define ERR_SUMMONDISABLED "445"
#define ERR_USERSDISABLED "446"
#define ERR_NOTREGISTERED "451"
#define ERR_NEEDMOREPARAMS "461"
#define ERR_ALREADYREGISTRED "462"
#define ERR_PASSWDMISMATCH "464"
#define ERR_BADCHANNELKEY "475"
#define ERR_BADCHANMASK "476"

namespace irc
{
    class Client;

    class Command
    {
    public:
        static void apply(Message const &message, Client &client);
        static void reply(std::string const &error, Client const &client, std::string const &argument);

        virtual ~Command(void) throw();

    private:
        typedef void (*t_command)(Message const &, Client &);

        static std::map<std::string, std::string, utils::t_istringcomp> const ERRORS;
        static std::map<std::string, t_command, utils::t_istringcomp> const COMMANDS;

        static void pass(Message const &message, Client &client);
        static void nick(Message const &message, Client &client);
        static void user(Message const &message, Client &client);
        static void oper(Message const &message, Client &client);
        static void quit(Message const &message, Client &client);
        // static void squit(Message const &message, Client &client);
        // static void notice(Message const &message, Client &client);
        // static void privmsg(Message const &message, Client &client);
        static void motd(Message const &message, Client &client);
        static void version(Message const &message, Client &client);
        static void join(Message const &message, Client &client);
        // static void mode(Message const &message, Client &client);
        static void topic(Message const &message, Client &client);
        // static void invite(Message const &message, Client &client);
        // static void kick(Message const &message, Client &client);
        static void summon(Message const &message, Client &client);
        static void users(Message const &message, Client &client);

        Command(void);                       /* = delete (C++11) */
        Command(Command const &);            /* = delete (C++11) */
        Command &operator=(Command const &); /* = delete (C++11) */
    };
} // namespace irc

#endif
