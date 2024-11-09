/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ircserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/14 22:40:21 by abenamar          #+#    #+#             */
/*   Updated: 2024/11/09 14:26:38 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <csignal>
#include <exception>
#include <iostream>
#include "Config.hpp"
#include "Server.hpp"
#include "utils.hpp"

static void noop(int) { return; }

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;

		return (2);
	}

	try
	{
		try
		{
			std::cout << "Info: IRC server configured from " << irc::Config::getInstance().getText(PRP_CONFIGFILE) << std::endl;
		}
		catch (std::exception const &e)
		{
			std::cerr << "Error: " << e.what() << std::endl;
			std::cout << "Warn: IRC server configured from default properties" << std::endl;
		}

		std::cout << "Info: IRC server listening on port " << irc::Server::getInstance(argv[1], argv[2]).getPort() << std::endl;

		if (std::signal(SIGINT, ::noop) == SIG_ERR)
			throw irc::utils::RuntimeErrno("std::signal (SIGINT)");
		else if (std::signal(SIGQUIT, ::noop) == SIG_ERR)
			throw irc::utils::RuntimeErrno("std::signal (SIGQUIT)");

		while (true)
			irc::Server::getInstance().poll();
	}
	catch (std::exception const &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;

		return (1);
	}

	return (0);
}
