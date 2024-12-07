/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ircserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/14 22:40:21 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/07 06:29:37 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <csignal>
#include <exception>
#include <iostream>

#include "Client.hpp"
#include "GameBot.hpp"
#include "Server.hpp"
#include "utils.hpp"

static irc::utils::s_istringmap<int>::type const SIGNALS(
	irc::utils::to_istringmap<int>(
		(char const *[]){
			"SIGTERM",
			"SIGINT",
			"SIGQUIT",
			"SIGHUP",
			"SIGCHLD"},
		(int[]){
			SIGTERM,
			SIGINT,
			SIGQUIT,
			SIGHUP,
			SIGCHLD},
		5));

static void noop(int) { return; }

static void handle(irc::utils::s_istringmap<int>::type const &signals)
{
	irc::utils::s_istringmap<int>::type::const_iterator cit;

	for (cit = signals.begin(); cit != signals.end(); ++cit)
		if (std::signal(cit->second, ::noop) == SIG_ERR)
			throw std::runtime_error(
				irc::utils::strerrno("std::signal: " + cit->first));

	return;
}

int main(int argc, char *argv[])
{
	irc::Client *bot(NULL);

	if (argc < 3)
	{
		std::cerr << "Usage: " << argv[0] << " <port> <password>"
				  << " [<configuration file>]" << std::endl;

		return (2);
	}

	try
	{
		if (argc > 3)
			irc::Server::instance(argv[1], argv[2], argv[3]);
		else
			irc::Server::instance(argv[1], argv[2]);

		std::cout << "Info: IRC server properties: "
				  << irc::utils::to_string(
						 irc::Server::instance().getProperties(),
						 " ")
				  << std::endl;
		std::cout << "Info: IRC server listening on ["
				  << irc::utils::get_haddr(irc::Server::instance().getSocket())
				  << "]:"
				  << irc::utils::get_hport(irc::Server::instance().getSocket())
				  << std::endl;

		bot = new irc::GameBot(irc::utils::connect_socket(
			irc::Server::instance().getSocket()));

		irc::Server::instance().addBot("GameBot", "#jan-ken-pon", bot);
		::handle(SIGNALS);

		while (true)
			irc::Server::instance().poll();
	}
	catch (std::exception const &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}

	delete bot;

	std::cout << "Warning: IRC server shutting down" << std::endl;

	return (0);
}
