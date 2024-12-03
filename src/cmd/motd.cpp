/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   motd.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/01 20:18:44 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/01 21:14:36 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.hpp"

void irc::Command::motd(Message const &message, Client &client)
{
    std::ifstream ifs;
    std::string buf;

    if (!message.getParameters().empty() &&
        message.getParameters().at(0) !=
            Server::instance().getTextProperty(PRP_SERVERNAME))
        return (Command::reply(ERR_NOSUCHSERVER,
                               client,
                               message.getParameters().at(0)));

    ifs.open(Server::instance().getTextProperty(PRP_MOTDFILE).c_str());

    if (!ifs.is_open())
        return (Server::instance().produce(ERR_NOMOTD,
                                           client,
                                           "MOTD File is missing"));

    Server::instance().produce(
        RPL_MOTDSTART,
        client,
        Server::instance().getTextProperty(PRP_SERVERNAME) +
            " message of the day");

    while (std::getline(ifs, buf))
        Server::instance().produce(
            RPL_MOTD,
            client,
            buf.substr(0, Server::instance().getSizeProperty(PRP_MOTDLINELEN)));

    Server::instance().produce(RPL_ENDOFMOTD,
                               client,
                               "End of message of the day");

    return (ifs.close());
}
