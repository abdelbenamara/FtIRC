/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ircserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/14 22:40:21 by abenamar          #+#    #+#             */
/*   Updated: 2024/09/21 22:23:45 by abenamar         ###   ########.fr       */
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

static void run(Server *const server) throw(std::runtime_error)
{
	std::map<int, Client *const>::const_iterator it;
	Message const *message;
	int nfds, n;

	while (true)
	{
		nfds = server->waitForEvents();

		try
		{
			for (int i = 0; i < nfds; ++i)
			{
				n = server->getEventSocket(i);

				if (n == server->getSocket())
					server->addClient();
				else if (!server->getClient(n)->updateInput())
					server->removeClient(n);
			}

			for (it = server->getClientsBegin(); it != server->getClientsEnd(); ++it)
			{
				while (it->second->hasMessage())
				{
					message = &it->second->message();

					std::cout << "Info: Client: [prefix=" << message->getPrefix() << ", command=" << message->getCommand() << ", parameters=";

					for (std::vector<std::string>::const_iterator p = message->getParameters().begin(); p != message->getParameters().end(); ++p)
					{
						std::cout << *p;

						if (p + 1 != message->getParameters().end())
							std::cout << ',';
					}

					std::cout << "] " << message->getInput();

					if (Command::APPLY.find(message->getCommand()) == Command::APPLY.end())
					{
						n = send(it->second->getSocket(), ("421 " + message->getCommand() + " :Unknown command\r\n").c_str(), message->getCommand().length() + 23, 0);

						if (n == -1)
							throw RuntimeErrno("::run", "send");
					}
					else
						Command::APPLY.at(message->getCommand());

					it->second->removeMessage();
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
