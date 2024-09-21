/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/21 21:21:33 by abenamar          #+#    #+#             */
/*   Updated: 2024/09/21 22:20:34 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.hpp"

std::map<std::string, void (*)(Client &, Server const &)> const Command::APPLY = Command::createApplyMap();

std::map<std::string, void (*)(Client &, Server const &)> Command::createApplyMap(void)
{
    std::map<std::string, void (*)(Client &, Server const &)> map;

    map["PASS"] = &Command::pass;
    map["NICK"] = &Command::nick;
    map["USER"] = &Command::pass;

    return (map);
}

void Command::pass(Client &client, Server const &server)
{
    Message const &message = client.message();
    ssize_t nwrite;

    try
    {
        if (message.getParameters().empty())
            nwrite = send(client.getSocket(), "461 PASS :Not enough parameters\r\n", 33, 0);
        else if (client.isAuthorized())
            nwrite = send(client.getSocket(), "462 :Unauthorized command (already registered)\r\n", 48, 0);
        else if (message.getParameters().at(0) != server.getPassword())
            nwrite = send(client.getSocket(), "464 :Password incorrect\r\n", 25, 0);

        if (nwrite == -1)
            throw RuntimeErrno("send");

        client.setAuthorized(true);
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error("Command::pass: " + std::string(e.what()));
    }

    return;
}

void Command::nick(Client &client, Server const &server)
{
    (void)client;
    (void)server;
    // TODO

    return;
}

void Command::user(Client &client, Server const &server)
{
    (void)client;
    (void)server;
    // TODO

    return;
}
