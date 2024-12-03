/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   quit.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/01 20:18:37 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/01 21:02:54 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.hpp"

void irc::Command::quit(Message const &message, Client &client)
{
    std::string comment("Client exited");

    if (!message.getParameters().empty())
        comment = "Quit: " + utils::to_string(message.getParameters(), " ");

    return (Server::instance().removeClient(client, comment));
}
