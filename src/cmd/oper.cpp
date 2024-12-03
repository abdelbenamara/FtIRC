/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   oper.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/01 20:18:30 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/02 20:17:17 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.hpp"

void irc::Command::oper(Message const &message, Client &client)
{
    if (message.getParameters().size() < 2)
        return (
            Command::reply(ERR_NEEDMOREPARAMS, client, message.getCommand()));
    else if (!client.isRegistered())
        return (
            Command::reply(ERR_NOTREGISTERED, client, message.getCommand()));
    else if (message.getParameters().at(0) !=
                 Server::instance().getTextProperty(PRP_OPERNAME) ||
             message.getParameters().at(1) !=
                 Server::instance().getTextProperty(PRP_OPERPASS))
        return (
            Command::reply(ERR_PASSWDMISMATCH, client, message.getCommand()));

    client.addMode(USR_MODE_o);
    Server::instance().produce(CMD_MODE,
                               client,
                               std::string(1, '+') + USR_MODE_o);

    return (Server::instance().produce(RPL_YOUREOPER,
                                       client,
                                       "You are now an IRC operator"));
}
