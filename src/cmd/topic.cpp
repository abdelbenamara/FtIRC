/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   topic.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/01 20:21:10 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/02 20:16:54 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.hpp"

void irc::Command::topic(Message const &message, Client &client)
{
    Server::t_channels::iterator chanit;
    Message::Builder builder;

    if (message.getParameters().empty())
        return (
            Command::reply(ERR_NEEDMOREPARAMS, client, message.getCommand()));
    else if (!client.isRegistered())
        return (
            Command::reply(ERR_NOTREGISTERED, client, message.getCommand()));

    chanit = Server::instance().getChannel(message.getParameters().at(0));

    if (chanit == Server::instance().getChannels().end())
        return (Command::reply(ERR_NOSUCHCHANNEL,
                               client,
                               message.getParameters().at(0)));
    else if (chanit->second.getMembers().find(&client) ==
             chanit->second.getMembers().end())
        return (Command::reply(ERR_NOTONCHANNEL, client, chanit->first));

    builder.withPrefix(Server::instance().getTextProperty(PRP_SERVERNAME));

    if (message.getParameters().size() > 1)
    {
        if (chanit->second.getModes().find(CHAN_MODE_t) !=
                chanit->second.getModes().end() &&
            chanit->second.getOperators().find(&client) ==
                chanit->second.getOperators().end())
            return (
                Command::reply(ERR_CHANOPRIVSNEEDED, client, chanit->first));
        else if (chanit->second.getTopic() == message.getParameters().at(1))
            return;

        chanit->second.setTopic(message.getParameters().at(1));
        chanit->second.setTopicWhoTime(client);

        return (chanit->second.publish(
            builder
                .withPrefix(client.str())
                .withCommand(message.getCommand())
                .withParameter(chanit->first)
                .addParameter(chanit->second.getTopic())
                .build()));
    }
    else if (chanit->second.getTopic().empty())
        return (Server::instance().produce(
            client,
            builder
                .withCommand(RPL_NOTOPIC)
                .withParameter(client.getNickname())
                .addParameter(chanit->first)
                .addParameter("No topic is set")
                .build()));

    Server::instance().produce(client,
                               builder
                                   .withCommand(RPL_TOPIC)
                                   .withParameter(client.getNickname())
                                   .addParameter(chanit->first)
                                   .addParameter(chanit->second.getTopic())
                                   .build());

    return (Server::instance().produce(
        client,
        builder
            .withCommand(RPL_TOPICWHOTIME)
            .withParameter(client.getNickname())
            .addParameter(chanit->first)
            .addParameter(chanit->second.getTopicWhoTime().first->str())
            .addParameter(
                utils::to_string(chanit->second.getTopicWhoTime().second))
            .build()));
}
