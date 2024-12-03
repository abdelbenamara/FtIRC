/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   user.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/01 20:18:04 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/02 20:17:26 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.hpp"

void irc::Command::user(Message const &message, Client &client)
{
    std::size_t mode;

    if (message.getParameters().size() < 4 ||
        message.getParameters().at(3).empty())
        return (
            Command::reply(ERR_NEEDMOREPARAMS, client, message.getCommand()));
    else if (client.isRegistered())
        return (
            Command::reply(ERR_ALREADYREGISTRED, client, message.getCommand()));

    client.setRealname(message.getParameters().at(3));
    client.removeMode(USR_MODE_w);
    client.removeMode(USR_MODE_i);

    if (std::istringstream(message.getParameters().at(1)) >> mode)
    {
        if (mode & (1 << 2))
            client.addMode(USR_MODE_w);

        if (mode & (1 << 3))
            client.addMode(USR_MODE_i);
    }

    client.setUsername(message.getParameters().at(0));

    if (client.isRegistered())
        Server::instance().challengeRegistration(client);

    return;
}
