/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   kick.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/01 20:21:21 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/01 21:41:41 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.hpp"

void irc::Command::kick(Message const &message, Client &client)
{
    std::string comment(client.getNickname());
    Server::t_channels::iterator chanit;
    Channel::t_clients::const_iterator mcit, ocit;
    std::vector<std::string> users;
    std::vector<std::string>::const_iterator cit;
    Server::t_clients::iterator userit;
    Message::Builder builder;

    if (message.getParameters().size() < 2)
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

    mcit = chanit->second.getMembers().find(&client);
    ocit = chanit->second.getOperators().find(&client);
    users = utils::split(message.getParameters().at(1), ',');

    if (message.getParameters().size() > 2)
        comment = message.getParameters()
                      .at(2)
                      .substr(0,
                              Server::instance().getSizeProperty(PRP_KICKLEN));

    users.erase(std::remove(users.begin(), users.end(), std::string()),
                users.end());

    for (cit = users.begin(); cit != users.end(); ++cit)
    {
        userit = Server::instance().getUser(*cit);

        if (userit == Server::instance().getClients().end())
            Command::reply(ERR_NOSUCHNICK, client, *cit);
        else if (mcit == chanit->second.getMembers().end())
            Command::reply(ERR_NOTONCHANNEL, client, chanit->first);
        else if (chanit->second.getMembers().find(&(userit->second)) ==
                 chanit->second.getMembers().end())
            Server::instance().produce(
                client,
                builder
                    .withPrefix(
                        Server::instance().getTextProperty(PRP_SERVERNAME))
                    .withCommand(ERR_USERNOTINCHANNEL)
                    .withParameter(client.getNickname())
                    .addParameter(userit->second.getNickname())
                    .addParameter(chanit->first)
                    .addParameter("They are not on that channel")
                    .build());
        else if (ocit == chanit->second.getOperators().end())
            Command::reply(ERR_CHANOPRIVSNEEDED, client, chanit->first);
        else
        {
            chanit->second.removeMember(userit->second);
            Server::instance().produce(
                userit->second,
                builder
                    .withPrefix(client.str())
                    .withCommand(message.getCommand())
                    .withParameter(chanit->first)
                    .addParameter(userit->second.getNickname())
                    .addParameter(comment)
                    .build());

            if (chanit->second.getMembers().empty())
                Server::instance().removeChannel(chanit->second.getName());
            else
                chanit->second.publish(builder.build());
        }
    }

    return;
}
