/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   invite.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/01 20:21:16 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/02 20:16:31 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.hpp"

void irc::Command::invite(Message const &message, Client &client)
{
    Client::t_channels::const_iterator cit;
    Server::t_clients::iterator userit;
    Server::t_channels::const_iterator chancit;
    Message::Builder builder;

    if (!client.isRegistered())
        return (
            Command::reply(ERR_NOTREGISTERED, client, message.getCommand()));
    else if (message.getParameters().empty())
    {
        for (cit = client.getInvites().begin();
             cit != client.getInvites().end();
             ++cit)
            Server::instance().produce(RPL_INVITELIST,
                                       client,
                                       (*cit)->getName());

        return (Server::instance().produce(RPL_ENDOFINVITELIST,
                                           client,
                                           "End of INVITE list"));
    }

    userit = Server::instance().getUser(message.getParameters().at(0));

    if (userit == Server::instance().getClients().end())
        return (Command::reply(ERR_NOSUCHNICK,
                               client,
                               message.getParameters().at(0)));
    else if (message.getParameters().size() == 1)
        return (
            Command::reply(ERR_NEEDMOREPARAMS, client, message.getCommand()));

    chancit = Server::instance().getChannel(message.getParameters().at(1));

    if (chancit == Server::instance().getChannels().end())
        return (Command::reply(ERR_NOSUCHCHANNEL,
                               client,
                               message.getParameters().at(1)));
    else if (chancit->second.getMembers().find(&client) ==
             chancit->second.getMembers().end())
        return (Command::reply(ERR_NOTONCHANNEL, client, chancit->first));
    else if (chancit->second.getMembers().find(&(userit->second)) !=
             chancit->second.getMembers().end())
        return (Server::instance().produce(
            client,
            builder
                .withPrefix(Server::instance().getTextProperty(PRP_SERVERNAME))
                .withCommand(ERR_USERONCHANNEL)
                .withParameter(client.getNickname())
                .addParameter(userit->second.getNickname())
                .addParameter(chancit->first)
                .addParameter("is already on channel")
                .build()));
    else if (chancit->second.getModes().find(CHAN_MODE_i) !=
                 chancit->second.getModes().end() &&
             chancit->second.getOperators().find(&client) ==
                 chancit->second.getOperators().end())
        return (Command::reply(ERR_CHANOPRIVSNEEDED, client, chancit->first));

    userit->second.addInvite(chancit->second);
    Server::instance().produce(userit->second,
                               builder
                                   .withPrefix(client.str())
                                   .withCommand(message.getCommand())
                                   .withParameter(userit->second.getNickname())
                                   .addParameter(chancit->first)
                                   .build());

    return (Server::instance().produce(
        client,
        builder
            .withPrefix(Server::instance().getTextProperty(PRP_SERVERNAME))
            .withCommand(RPL_INVITING)
            .withParameter(client.getNickname())
            .addParameter(userit->second.getNickname())
            .addParameter(chancit->first)
            .build()));
}
