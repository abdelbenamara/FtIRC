/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ircserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/14 22:40:21 by abenamar          #+#    #+#             */
/*   Updated: 2024/10/14 21:38:13 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/socket.h>
#include <sys/types.h>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <exception>
#include <iostream>
#include <stdexcept>
#include "Client.hpp"
#include "Command.hpp"
#include "Message.hpp"
#include "RuntimeErrno.hpp"
#include "Server.hpp"

static void noop(int /* signum */) throw() { return; }

static void debug(Message const &message)
{
	std::cout << "Debug: Client: [prefix=" << message.getPrefix() << ", command=" << message.getCommand() << ", parameters=";

	for (std::vector<std::string>::const_iterator p = message.getParameters().begin(); p != message.getParameters().end(); ++p)
	{
		std::cout << *p;

		if (p + 1 != message.getParameters().end())
			std::cout << ',';
	}

	std::cout << "] " << message.getInput();
}

static void run(Server *const server) throw(std::runtime_error)
{
	std::map<int, Client *const>::iterator it;
	Client *client;
	int nfds, connfd;

	while (true)
	{
		nfds = server->waitForEvents();

		try
		{
			for (int i = 0; i < nfds; ++i)
			{
				connfd = server->getEventSocket(i);

				if (connfd == server->getSocket())
					server->addClient();
				else if (!server->getClient(connfd)->updateInput())
					server->removeClient(connfd);
			}

			for (it = server->getClients().begin(); it != server->getClients().end();)
			{
				client = it->second;

				while (client->hasMessage())
				{
					::debug(client->message());

					if (Command::APPLY.find(client->message().getCommand()) == Command::APPLY.end())
					{
						if (send(client->getSocket(), ("421 " + client->message().getCommand() + " :Unknown command\r\n").c_str(), client->message().getCommand().length() + 23, 0) == -1)
							throw RuntimeErrno("send");
					}
					else
						Command::APPLY.at(client->message().getCommand())(*client, *server);

					client->removeMessage();

					if (client->isGone())
						break;
				}

				if (client->isGone())
					server->removeClient(it++);
				else
					++it;
			}
		}
		catch (std::exception const &e)
		{
			std::cerr << "Error: " << e.what() << std::endl;
		}
	}

	return;
}

int main(int argc, char *argv[])
{
	Server *server = NULL;

	if (argc != 3)
	{
		std::cerr << "Usage: ircserv port password" << std::endl;

		return (2);
	}

	try
	{
		server = Server::Builder()
					 .withNumericServ(argv[1])
					 .withPassword(argv[2])
					 .build();

		if (std::signal(SIGINT, ::noop) == SIG_ERR)
			throw RuntimeErrno("std::signal (SIGINT)");

		if (std::signal(SIGQUIT, ::noop) == SIG_ERR)
			throw RuntimeErrno("std::signal (SIGQUIT)");

		std::cout << "Info: IRC server listening on port: " << server->port() << std::endl;

		::run(server);
	}
	catch (std::exception const &e)
	{
		delete server;

		std::cerr << "Error: " << e.what() << std::endl;

		return (1);
	}

	return (0);
}
