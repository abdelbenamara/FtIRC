/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   version.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/01 20:27:50 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/01 21:14:50 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.hpp"

void irc::Command::version(Message const &message, Client &client)
{
    if (!message.getParameters().empty() &&
        message.getParameters().at(0) !=
            Server::instance().getTextProperty(PRP_SERVERNAME))
        return (Command::reply(ERR_NOSUCHSERVER,
                               client,
                               message.getParameters().at(0)));

    Server::instance().produce(
        client,
        Message::Builder()
            .withPrefix(Server::instance().getTextProperty(PRP_SERVERNAME))
            .withCommand(RPL_VERSION)
            .withParameter(client.getNickname())
            .addParameter(Server::instance().getTextProperty(PRP_VERSION))
            .addParameter(
                Server::instance().getTextProperty(PRP_SERVERNAME))
            .addParameter("42 ft_irc project")
            .build());

    return (Server::instance().produceSupportList(client));
}
