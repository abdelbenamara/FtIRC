/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/21 21:21:33 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/07 18:52:41 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.hpp"

static void summon(irc::Message const &message, irc::Client &client)
{
    return (
        irc::Command::reply(ERR_SUMMONDISABLED, client, message.getCommand()));
}

static void users(irc::Message const &message, irc::Client &client)
{
    return (
        irc::Command::reply(ERR_USERSDISABLED, client, message.getCommand()));
}

irc::utils::s_istringmap<std::string>::type const
    irc::Command::ERRORS(utils::to_istringmap<std::string>(
        (char const *[]){
            ERR_NOSUCHNICK,
            ERR_NOSUCHSERVER,
            ERR_NOSUCHCHANNEL,
            ERR_TOOMANYCHANNELS,
            ERR_NOORIGIN,
            ERR_UNKNOWNCOMMAND,
            ERR_NONICKNAMEGIVEN,
            ERR_ERRONEUSNICKNAME,
            ERR_NICKNAMEINUSE,
            ERR_NOTONCHANNEL,
            ERR_SUMMONDISABLED,
            ERR_USERSDISABLED,
            ERR_NOTREGISTERED,
            ERR_NEEDMOREPARAMS,
            ERR_ALREADYREGISTRED,
            ERR_PASSWDMISMATCH,
            ERR_KEYSET,
            ERR_CHANNELISFULL,
            ERR_UNKNOWNMODE,
            ERR_INVITEONLYCHAN,
            ERR_BADCHANNELKEY,
            ERR_BADCHANMASK,
            ERR_CHANOPRIVSNEEDED,
            ERR_UMODEUNKNOWNFLAG,
            ERR_INVALIDKEY},
        (char const *[]){
            "No such nick",
            "No such server",
            "No such channel",
            "You have joined too many channels",
            "No origin specified",
            "Unknown command",
            "No nickname given",
            "Erroneous nickname",
            "Nickname is already in use",
            "You are not on that channel",
            "SUMMON has been disabled",
            "USERS has been disabled",
            "You have not registered",
            "Not enough parameters",
            "Unauthorized command (already registered)",
            "Password incorrect",
            "Channel key already set",
            "Cannot join channel (+l)",
            "is not a recognised channel mode",
            "Cannot join channel (+i)",
            "Cannot join channel (+k)",
            "Bad Channel Mask",
            "You are not channel operator",
            "is not a recognised user mode",
            "Key is not well-formed"},
        25));

irc::utils::s_istringmap<irc::Command::t_cmd>::type const
    irc::Command::COMMANDS(utils::to_istringmap<irc::Command::t_cmd>(
        (char const *[]){
            CMD_PASS,
            CMD_NICK,
            CMD_USER,
            CMD_PING,
            CMD_OPER,
            CMD_QUIT,
            CMD_LUSERS,
            CMD_MOTD,
            CMD_VERSION,
            CMD_NAMES,
            CMD_JOIN,
            CMD_TOPIC,
            CMD_INVITE,
            CMD_KICK,
            CMD_MODE,
            CMD_PRIVMSG,
            CMD_NOTICE,
            CMD_WHO,
            CMD_WALLOPS,
            CMD_SUMMON,
            CMD_USERS},
        (irc::Command::t_cmd[]){
            &irc::Command::pass,
            &irc::Command::nick,
            &irc::Command::user,
            &irc::Command::ping,
            &irc::Command::oper,
            &irc::Command::quit,
            &irc::Command::lusers,
            &irc::Command::motd,
            &irc::Command::version,
            &irc::Command::names,
            &irc::Command::join,
            &irc::Command::topic,
            &irc::Command::invite,
            &irc::Command::kick,
            &irc::Command::mode,
            &irc::Command::privmsg,
            &irc::Command::notice,
            &irc::Command::who,
            &irc::Command::wallops,
            &::summon,
            &::users},
        21));

irc::Command::~Command(void) throw() { return; }

void irc::Command::reply(std::string const &error,
                         Client const &client,
                         std::string const &argument)
{
    utils::s_istringmap<std::string>::type::const_iterator cit(
        Command::ERRORS.find(error));

    if (cit == Command::ERRORS.end())
        throw std::out_of_range(error + ": unknown error reply");

    return (Server::instance().produce(
        client,
        Message::Builder()
            .withPrefix(Server::instance().getTextProperty(PRP_SERVERNAME))
            .withCommand(error)
            .withParameter(client.getNickname())
            .addParameter(argument)
            .addParameter(cit->second)
            .build()));
}

void irc::Command::apply(Message const &message, Client &client)
{
    utils::s_istringmap<irc::Command::t_cmd>::type::const_iterator cit(
        Command::COMMANDS.find(message.getCommand()));

    if (cit == Command::COMMANDS.end())
    {
        Command::reply(ERR_UNKNOWNCOMMAND, client, message.getCommand());

        throw std::out_of_range(message.getCommand() + ": unknown command");
    }

    return (cit->second(message, client));
}
