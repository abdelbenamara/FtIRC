/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ircserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/14 22:40:21 by abenamar          #+#    #+#             */
/*   Updated: 2024/08/11 14:32:46 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <csignal>
#include <exception>
#include <iostream>
#include "Server.hpp"

void noop(int /* signum */) throw() { return; }

int main(int argc, char *argv[])
{
	Server *server;

	if (argc != 3)
	{
		std::cerr << "Usage: ircserv port password" << std::endl;

		return (2);
	}

	try
	{
		server = (Server::Builder())
					 .withNumericServ(argv[1])
					 .withPassword(argv[2])
					 .build();
	}
	catch (std::exception const &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;

		return (1);
	}

	try
	{
		if (std::signal(SIGINT, ::noop) == SIG_ERR)
			throw std::runtime_error(std::string("std::runtime_error: std::signal (SIGINT): ") + strerror(errno));

		if (std::signal(SIGQUIT, ::noop) == SIG_ERR)
			throw std::runtime_error(std::string("std::runtime_error: std::signal (SIGQUIT): ") + strerror(errno));

		std::cout << "Info: IRC server listening on port: " << server->port() << std::endl;

		server->run();
	}
	catch (std::exception const &e)
	{
		delete server;

		std::cerr << "Error: " << e.what() << std::endl;

		return (1);
	}

	return (0);
}
