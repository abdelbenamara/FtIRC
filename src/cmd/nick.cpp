/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   nick.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/01 20:17:58 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/07 18:44:40 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.hpp"

void irc::Command::nick(Message const &message, Client &client)
{
    bool const registered(client.isRegistered());
    std::string nick;
    Server::t_clients::iterator it;
    Message::Builder builder;

    builder.withCommand(message.getCommand());

    if (message.getParameters().empty() ||
        message.getParameters().at(0).empty())
        return (
            Command::reply(ERR_NONICKNAMEGIVEN, client, message.getCommand()));

    nick = message.getParameters().at(0);

    if (client.getNickname() == nick)
        return;

    it = Server::instance().getClient(nick);

    if (it != Server::instance().getClients().end())
    {
        if (it->second.isRegistered())
            return (Command::reply(ERR_NICKNAMEINUSE, client, nick));

        Server::instance().produce(it->second,
                                   builder
                                       .withPrefix(it->second.str())
                                       .withParameter(it->second.getUsername())
                                       .build());
        Command::reply(ERR_NICKNAMEINUSE, it->second, nick);
        it->second.clearNickname();
    }

    builder.withPrefix(client.str());

    try
    {
        client.setNickname(nick);
    }
    catch (std::exception const &)
    {
        Command::reply(ERR_ERRONEUSNICKNAME, client, nick);

        throw;
    }

    if (registered)
    {
        Server::instance().produce(client,
                                   builder
                                       .withParameter(client.getNickname())
                                       .build());
        client.publish(builder.build());
    }
    else if (client.isRegistered())
        Server::instance().challengeRegistration(client);

    return;
}
