/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   names.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/01 20:33:38 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/01 21:28:07 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.hpp"

void irc::Command::names(Message const &message, Client &client)
{
    std::vector<std::string> names(1, "*"), params;
    Message::Builder builder;
    std::vector<std::string>::const_iterator cit;
    Server::t_channels::iterator chanit;
    std::size_t cur, pos;
    std::string buf;

    if (!client.isRegistered())
        return (
            Command::reply(ERR_NOTREGISTERED, client, message.getCommand()));
    else if (message.getParameters().size() > 1 &&
             message.getParameters().at(1) !=
                 Server::instance().getTextProperty(PRP_SERVERNAME))
        return (Command::reply(ERR_NOSUCHSERVER,
                               client,
                               message.getParameters().at(1)));

    if (!message.getParameters().empty())
        names = utils::split(message.getParameters().at(0), ',');

    names.erase(std::remove(names.begin(), names.end(), std::string()),
                names.end());
    builder.withPrefix(Server::instance().getTextProperty(PRP_SERVERNAME));

    for (cit = names.begin(); cit != names.end(); ++cit)
    {
        chanit = Server::instance().getChannel(*cit);

        if (chanit != Server::instance().getChannels().end())
        {
            builder
                .withCommand(RPL_NAMREPLY)
                .withParameter(client.getNickname())
                .addParameter("=")
                .addParameter(chanit->first);

            params = builder.getParameters();
            cur = Server::instance().getSizeProperty(PRP_LINELEN) -
                  builder.build().str().length() - 2;
            buf = chanit->second.namesList(client);

            while (!buf.empty())
            {
                pos = buf.find_last_of(' ', cur);

                Server::instance().produce(client,
                                           builder
                                               .withParameters(params)
                                               .addParameter(buf.substr(0, pos))
                                               .build());
                buf.erase(0, pos + 1);
            }
        }

        builder
            .withCommand(RPL_ENDOFNAMES)
            .withParameter(client.getNickname());

        if (chanit == Server::instance().getChannels().end())
            builder.addParameter(*cit);
        else
            builder.addParameter(chanit->first);

        Server::instance().produce(client,
                                   builder
                                       .addParameter("End of NAMES list")
                                       .build());
    }

    return;
}
