/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ircserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/14 22:40:21 by abenamar          #+#    #+#             */
/*   Updated: 2024/08/26 14:10:16 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <csignal>
#include <exception>
#include <iostream>
#include <stdexcept>
#include "Client.hpp"
#include "Message.hpp"
#include "RuntimeErrno.hpp"
#include "Server.hpp"

static void noop(int /* signum */) throw() { return; }

static void run(Server *const server) throw(std::runtime_error)
{
	std::map<int, Client *const>::const_iterator it;
	int nfds, fd;

	while (true)
	{
		nfds = server->waitForEvents();

		try
		{
			for (int i = 0; i < nfds; ++i)
			{
				fd = server->getEventSocket(i);

				if (fd == server->getSocket())
					server->addClient();
				else if (!server->getClient(fd)->appendInput())
					server->removeClient(fd);
			}

			for (it = server->getClientsBegin(); it != server->getClientsEnd(); ++it)
			{
				while (it->second->hasMessage())
				{
					std::cout << "Info: Client: " << it->second->message().getInput();

					it->second->dropMessage();
				}
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
