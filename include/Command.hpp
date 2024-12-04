/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/21 21:05:45 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/04 02:12:35 by abenamar         ###   ########.fr       */
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
#define CMD_ERROR "ERROR"
#define CMD_PING "PING"
#define CMD_PONG "PONG"
#define CMD_OPER "OPER"
#define CMD_QUIT "QUIT"
#define CMD_MOTD "MOTD"
#define CMD_VERSION "VERSION"
#define CMD_NAMES "NAMES"
#define CMD_JOIN "JOIN"
#define CMD_TOPIC "TOPIC"
#define CMD_INVITE "INVITE"
#define CMD_KICK "KICK"
#define CMD_MODE "MODE"
#define CMD_PRIVMSG "PRIVMSG"
#define CMD_NOTICE "NOTICE"
#define CMD_WALLOPS "WALLOPS"
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
#define RPL_UMODEIS "221"
#define RPL_CHANNELMODEIS "324"
#define RPL_CREATIONTIME "329"
#define RPL_NOTOPIC "331"
#define RPL_TOPIC "332"
#define RPL_TOPICWHOTIME "333"
#define RPL_INVITELIST "336"
#define RPL_ENDOFINVITELIST "337"
#define RPL_INVITING "341"
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

#define ERR_NOSUCHNICK "401"
#define ERR_NOSUCHSERVER "402"
#define ERR_NOSUCHCHANNEL "403"
#define ERR_TOOMANYCHANNELS "405"
#define ERR_TOOMANYTARGETS "407"
#define ERR_NOORIGIN "409"
#define ERR_NORECIPIENT "411"
#define ERR_NOTEXTTOSEND "412"
#define ERR_INPUTTOOLONG "417"
#define ERR_UNKNOWNCOMMAND "421"
#define ERR_NOMOTD "422"
#define ERR_NONICKNAMEGIVEN "431"
#define ERR_ERRONEUSNICKNAME "432"
#define ERR_NICKNAMEINUSE "433"
#define ERR_USERNOTINCHANNEL "441"
#define ERR_NOTONCHANNEL "442"
#define ERR_USERONCHANNEL "443"
#define ERR_SUMMONDISABLED "445"
#define ERR_USERSDISABLED "446"
#define ERR_NOTREGISTERED "451"
#define ERR_NEEDMOREPARAMS "461"
#define ERR_ALREADYREGISTRED "462"
#define ERR_PASSWDMISMATCH "464"
#define ERR_KEYSET "467"
#define ERR_CHANNELISFULL "471"
#define ERR_UNKNOWNMODE "472"
#define ERR_INVITEONLYCHAN "473"
#define ERR_BADCHANNELKEY "475"
#define ERR_BADCHANMASK "476"
#define ERR_NOPRIVILEGES "481"
#define ERR_CHANOPRIVSNEEDED "482"
#define ERR_UMODEUNKNOWNFLAG "501"
#define ERR_USERSDONTMATCH "502"
#define ERR_INVALIDKEY "525"
#define ERR_INVALIDMODEPARAM "696"

namespace irc
{
    class Client;
    class Message;
    class Server;

    class Command
    {
    public:
        static void reply(std::string const &error,
                          Client const &client,
                          std::string const &argument);
        static void apply(Message const &message, Client &client);

        virtual ~Command(void) throw();

    private:
        typedef void (*t_cmd)(Message const &, Client &);

        static utils::s_istringmap<std::string>::type const ERRORS;
        static utils::s_istringmap<t_cmd>::type const COMMANDS;

        static void pass(Message const &message, Client &client);
        static void nick(Message const &message, Client &client);
        static void user(Message const &message, Client &client);
        static void ping(Message const &message, Client &client);
        static void oper(Message const &message, Client &client);
        static void quit(Message const &message, Client &client);
        static void motd(Message const &message, Client &client);
        static void version(Message const &message, Client &client);
        static void names(Message const &message, Client &client);
        static void join(Message const &message, Client &client);
        static void topic(Message const &message, Client &client);
        static void invite(Message const &message, Client &client);
        static void kick(Message const &message, Client &client);
        static void mode(Message const &message, Client &client);
        static void privmsg(Message const &message, Client &client);
        static void notice(Message const &message, Client &client);
        // static void wallops(Message const &message, Client &client);

        Command(void);                       /* = delete (C++11) */
        Command(Command const &);            /* = delete (C++11) */
        Command &operator=(Command const &); /* = delete (C++11) */
    };
} // namespace irc

#endif
