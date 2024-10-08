/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejankovs <ejankovs@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/21 21:21:33 by abenamar          #+#    #+#             */
/*   Updated: 2024/10/08 20:18:07 by ejankovs         ###   ########.fr       */
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
    ssize_t nwrite = 0;
	std::cout << "bruh" << std::endl;

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
    Message const &message = client.message();
    ssize_t nwrite = 0;

    try
    {
        if (message.getParameters().empty())
            nwrite = send(client.getSocket(), "431 :No nickname given\r\n", 24, 0);
        else if (server.isNicknameInUse(message.getParameters().at(0)))
		{
			std::string errorMessage = "433 " + message.getParameters().at(0) + " :Nickname is already in use\r\n";
            nwrite = send(client.getSocket(), errorMessage.c_str(), errorMessage.length(), 0);
		}
        else
        {
            client.setNickname(message.getParameters().at(0));
			
			std::string nickMessage = "NICK :" + client.getNickname() + "\r\n";
            nwrite = send(client.getSocket(), nickMessage.c_str(), nickMessage.length(), 0);
        }

        if (nwrite == -1)
            throw RuntimeErrno("send");
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error("Command::nick: " + std::string(e.what()));
    }

    return;
}

void Command::user(Client &client, Server const &server)
{
	(void)server;
    Message const &message = client.message();
    ssize_t nwrite = 0;

    try
    {
        if (message.getParameters().size() < 4)
            nwrite = send(client.getSocket(), "461 USER :Not enough parameters\r\n", 35, 0);
        else if (client.isRegistered())
        {
            nwrite = send(client.getSocket(), "462 :You may not reregister\r\n", 29, 0);
        }
        else
        {
            client.setUsername(message.getParameters().at(0));
            client.setMode(message.getParameters().at(1));
            client.setRealname(message.getParameters().at(3));

            client.setRegistered(true);
        }

        if (nwrite == -1)
            throw RuntimeErrno("send");
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error("Command::user: " + std::string(e.what()));
    }

	return ;
}
