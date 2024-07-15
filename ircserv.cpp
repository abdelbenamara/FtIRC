/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ircserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/14 22:40:21 by abenamar          #+#    #+#             */
/*   Updated: 2024/07/15 11:04:44 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ircserv.hpp"

int setup(char const *port)
{
	std::stringstream buffer;
	int n, sockfd;
	addrinfo hints;
	addrinfo *addr, *info;

	buffer << port;
	buffer >> n;

	if (buffer.fail() || !buffer.eof() || n < 0 || n > 65535)
	{
		std::cerr << "Error: invalid port `" << port << '\'' << std::endl;

		return (-1);
	}

	hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV | AI_V4MAPPED | AI_ADDRCONFIG;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_addrlen = 0;
	hints.ai_addr = NULL;
	hints.ai_canonname = NULL;
	hints.ai_next = NULL;

	if (getaddrinfo(NULL, port, &hints, &addr) != 0)
	{
		std::cerr << "Error: getaddrinfo" << std::endl;

		return (-1);
	}

	for (info = addr; info != NULL; info = info->ai_next)
	{
		sockfd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);

		if (sockfd == -1)
			continue;

		if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1)
		{
			std::cerr << "Error: fcntl: " << strerror(errno) << std::endl;

			if (close(sockfd) == -1)
				std::cerr << "Error: close: " << strerror(errno) << std::endl;

			freeaddrinfo(addr);

			return (-1);
		}

		if (bind(sockfd, info->ai_addr, info->ai_addrlen) == 0)
			break;

		if (close(sockfd) == -1)
		{
			std::cerr << "Error: close: " << strerror(errno) << std::endl;

			freeaddrinfo(addr);

			return (-1);
		}
	}

	if (sockfd == -1)
	{
		std::cerr << "Error: socket: " << strerror(errno) << std::endl;

		freeaddrinfo(addr);

		return (-1);
	}

	if (info == NULL)
	{
		std::cerr << "Error: bind: " << strerror(errno) << std::endl;

		freeaddrinfo(addr);

		return (-1);
	}

	if (getsockname(sockfd, info->ai_addr, &info->ai_addrlen) == -1)
	{
		std::cerr << "Error: getsockname: " << strerror(errno) << std::endl;

		if (close(sockfd) == -1)
			std::cerr << "Error: close: " << strerror(errno) << std::endl;

		freeaddrinfo(addr);

		return (-1);
	}

	std::cout << "Info: IRC server initialized with port: ";

	if (info->ai_family == AF_INET)
		std::cout << ntohs(reinterpret_cast<sockaddr_in *>(info->ai_addr)->sin_port);
	else
		std::cout << ntohs(reinterpret_cast<sockaddr_in6 *>(info->ai_addr)->sin6_port);

	std::cout << std::endl;

	freeaddrinfo(addr);

	return (sockfd);
}

int main(int argc, char *argv[])
{
	int sockfd;

	if (argc != 3)
	{
		std::cerr << "Usage: ircserv port password" << std::endl;

		return (2);
	}

	sockfd = ::setup(argv[1]);

	if (sockfd == -1)
		return (1);

	if (close(sockfd) == -1)
	{
		std::cerr << "Error: close: " << strerror(errno) << std::endl;

		return (1);
	}

	return (0);
}
