/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/01 20:18:12 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/01 21:00:58 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.hpp"

void irc::Command::ping(Message const &message, Client &client)
{
    std::string target(Server::instance().getTextProperty(PRP_SERVERNAME));

    if (message.getParameters().empty())
        return (
            Command::reply(ERR_NEEDMOREPARAMS, client, message.getCommand()));
    else if (message.getParameters().at(0).empty())
        return (Command::reply(ERR_NOORIGIN, client, message.getCommand()));

    if (message.getParameters().size() > 1)
        target = message.getParameters().at(1);

    return (Server::instance().produce(
        client,
        Message::Builder()
            .withPrefix(Server::instance().getTextProperty(PRP_SERVERNAME))
            .withCommand(CMD_PONG)
            .withParameter(target)
            .addParameter(message.getParameters().at(0))
            .build()));
}
