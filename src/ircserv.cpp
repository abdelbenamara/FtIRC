/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ircserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/14 22:40:21 by abenamar          #+#    #+#             */
/*   Updated: 2024/11/02 16:59:56 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <csignal>
#include <exception>
#include <iostream>
#include "Server.hpp"

static void noop(int) { return; }

int main(int argc, char *argv[])
{
	irc::Server *server;

	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;

		return (2);
	}

	try
	{
		server = &irc::Server::getInstance(argv[1], argv[2]);

		if (std::signal(SIGINT, ::noop) == SIG_ERR)
			throw irc::RuntimeErrno("std::signal (SIGINT)");
		else if (std::signal(SIGQUIT, ::noop) == SIG_ERR)
			throw irc::RuntimeErrno("std::signal (SIGQUIT)");

		std::cout << "Info: IRC server listening on port " << server->getPort() << std::endl;

		while (true)
			server->poll();
	}
	catch (std::exception const &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;

		return (1);
	}

	return (0);
}
