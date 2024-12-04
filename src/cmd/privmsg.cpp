/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   privmsg.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/03 23:50:09 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/04 03:32:52 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.hpp"

void irc::Command::privmsg(Message const &message, Client &client)
{
    std::size_t cap(irc::Server::instance().getSizeProperty(PRP_MAXTARGETS));
    std::vector<std::string> targets;
    std::string text;
    Message::Builder builder;
    std::vector<std::string>::const_iterator cit;
    Server::t_clients::iterator userit;
    Server::t_channels::iterator chanit;

    if (message.getParameters().empty())
        return (Server::instance().produce(ERR_NORECIPIENT,
                                           client,
                                           "No recipient given (" +
                                               message.getCommand() + ')'));
    else if (!client.isRegistered())
        return (
            Command::reply(ERR_NOTREGISTERED, client, message.getCommand()));

    targets = utils::split(message.getParameters().at(0), ',');
    text = utils::to_string(std::vector<std::string>(
                                ++message.getParameters().begin(),
                                message.getParameters().end()),
                            " ");

    targets.erase(std::remove(targets.begin(), targets.end(), std::string()),
                  targets.end());
    builder
        .withPrefix(client.str())
        .withCommand(message.getCommand());

    for (cit = targets.begin(); cap > 0 && cit != targets.end(); --cap, ++cit)
    {
        if (text.empty())
            Server::instance().produce(ERR_NOTEXTTOSEND,
                                       client,
                                       "No text to send");
        else if (Server::instance()
                     .getTextProperty(PRP_CHANTYPES)
                     .find(cit->at(0)) == std::string::npos)
        {
            userit = Server::instance().getUser(*cit);

            if (userit == Server::instance().getClients().end())
                Command::reply(ERR_NOSUCHNICK, client, *cit);
            else
                Server::instance().produce(
                    userit->second,
                    builder
                        .withParameter(userit->second.getNickname())
                        .addParameter(text)
                        .build());
        }
        else
        {
            chanit = Server::instance().getChannel(*cit);

            if (chanit == Server::instance().getChannels().end())
                Command::reply(ERR_NOSUCHCHANNEL, client, *cit);
            else
                chanit->second.publish(builder
                                           .withParameter(chanit->first)
                                           .addParameter(text)
                                           .build(),
                                       &client);
        }
    }

    return;
}
