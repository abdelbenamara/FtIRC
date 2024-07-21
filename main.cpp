/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejankovs <ejankovs@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/21 18:22:55 by ejankovs          #+#    #+#             */
/*   Updated: 2024/07/21 19:06:14 by ejankovs         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ircserv.hpp"
#include "Server.hpp"

// maybe check if the password is too long
int passwordCheck(char const *password)
{
	return (0);
}

int main(int argc, char *argv[])
{
	int sockfd;

	if (argc != 3)
	{
		std::cerr << "Usage: ircserv port password" << std::endl;

		return (2);
	}

	std::stringstream buffer;
	int n;
	char const *port;
	port = argv[1];
	int socketfd;

	buffer << port;
	buffer >> n;

	if (buffer.fail() || !buffer.eof() || n < 0 || n > 65535)
	{
		std::cerr << "Error: invalid port `" << port << '\'' << std::endl;
		return (-1);
	}
	
	Server ircServer(port, argv[2]);
	
	// to pass to client
	socketfd = ircServer.start();
	ircServer.stop();

	if (sockfd == -1)
		return (1);

	if (close(sockfd) == -1)
	{
		std::cerr << "Error: close: " << strerror(errno) << std::endl;

		return (1);
	}

	return (0);
}