/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ircserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/14 22:40:21 by abenamar          #+#    #+#             */
/*   Updated: 2024/08/07 22:20:40 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ircserv.hpp"

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
		server = new Server(argv[1], argv[2], MAX_EVENTS, BUF_SIZE);
	}
	catch (std::exception const &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;

		return (1);
	}

	try
	{
		if (std::signal(SIGINT, ServerUtils::interrupt) == SIG_ERR)
			throw std::runtime_error(std::string("std::runtime_error: std::signal (SIGINT): ") + strerror(errno));

		if (std::signal(SIGQUIT, ServerUtils::interrupt) == SIG_ERR)
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
