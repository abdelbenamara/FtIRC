/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pass.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/01 20:17:47 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/01 20:26:45 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.hpp"

void irc::Command::pass(Message const &message, Client &client)
{
    if (message.getParameters().empty())
        return (
            Command::reply(ERR_NEEDMOREPARAMS, client, message.getCommand()));
    else if (client.isRegistered())
        return (
            Command::reply(ERR_ALREADYREGISTRED, client, message.getCommand()));

    return (client.setPassword(message.getParameters().at(0)));
}
