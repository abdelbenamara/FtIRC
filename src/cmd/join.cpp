/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   join.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/01 20:21:04 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/02 20:16:43 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.hpp"

void irc::Command::join(Message const &message, Client &client)
{
    std::vector<std::string> names, keys;
    std::vector<std::string>::const_iterator ncit, kcit;
    Server::t_channels::iterator chanit;
    Message::Builder builder;

    if (message.getParameters().empty())
        return (
            Command::reply(ERR_NEEDMOREPARAMS, client, message.getCommand()));
    else if (!client.isRegistered())
        return (
            Command::reply(ERR_NOTREGISTERED, client, message.getCommand()));

    names = utils::split(message.getParameters().at(0), ',');

    if (message.getParameters().size() > 1)
        keys = utils::split(message.getParameters().at(1), ',');

    keys.resize(names.size());

    for (ncit = names.begin(), kcit = keys.begin();
         ncit != names.end();
         ++ncit, ++kcit)
    {
        if (ncit->empty())
            continue;

        chanit = Server::instance().getChannel(*ncit);

        if (client.getChannels().size() ==
            Server::instance().getSizeProperty(PRP_CHANLIMIT))
        {
            if (chanit == Server::instance().getChannels().end())
                Command::reply(ERR_NOSUCHCHANNEL, client, *ncit);
            else
                Command::reply(ERR_TOOMANYCHANNELS, client, *ncit);

            continue;
        }
        else if (chanit == Server::instance().getChannels().end())
        {
            try
            {
                chanit = Server::instance().addChannel(*ncit);
            }
            catch (std::exception const &)
            {
                Command::reply(ERR_BADCHANMASK, client, *ncit);

                continue;
            }
        }

        if (*kcit != chanit->second.getKey())
            Command::reply(ERR_BADCHANNELKEY, client, chanit->second.getName());
        else if (chanit->second.getModes().find(CHAN_MODE_i) !=
                     chanit->second.getModes().end() &&
                 client.getInvites().find(&(chanit->second)) ==
                     client.getInvites().end())
            Command::reply(ERR_INVITEONLYCHAN,
                           client,
                           chanit->second.getName());
        else if (chanit->second.getModes().find(CHAN_MODE_l) !=
                     chanit->second.getModes().end() &&
                 chanit->second.getMembers().size() >=
                     chanit->second.getLimit())
            Command::reply(ERR_CHANNELISFULL,
                           client,
                           chanit->second.getName());
        else
        {
            chanit->second.addMember(client);
            chanit->second.publish(builder
                                       .withPrefix(client.str())
                                       .withCommand(message.getCommand())
                                       .withParameter(chanit->first)
                                       .build());

            if (!chanit->second.getTopic().empty())
                Command::apply(builder
                                   .withoutPrefix()
                                   .withCommand(CMD_TOPIC)
                                   .build(),
                               client);

            Command::apply(builder
                               .withoutPrefix()
                               .withCommand(CMD_NAMES)
                               .build(),
                           client);
        }
    }

    return;
}
