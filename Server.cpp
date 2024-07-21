/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejankovs <ejankovs@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/21 18:13:17 by ejankovs          #+#    #+#             */
/*   Updated: 2024/07/21 20:19:51 by ejankovs         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server() throw() : _port("0"), _password(""), _sockfd(-1) {}

Server::Server(const char* port, std::string password) throw() : _port(port), _password(password), _sockfd(-1) {}


Server::Server(Server const& src) throw() : _port(src._port), _password(src._password), _channels(src._channels), _clients(src._clients), _sockfd(src._sockfd) {}

Server::~Server() throw() { stop(); }

Server& Server::operator=(Server const& rhs) throw() {
    if (this != &rhs) {
        _port = rhs._port;
        _password = rhs._password;
        _channels = rhs._channels;
        _clients = rhs._clients;
        _sockfd = rhs._sockfd;
    }
    return *this;
}

// we'll need to verify in client
int	Server::getSockFd() const { return this->_sockfd; }

const char* Server::GetAddrException::what() const throw()
{
	return ("Error: getaddrinfo");
}

const char* Server::SocketErrorException::what() const throw()
{
	return ("Error: socket");
}

int Server::start() throw(Server::GetAddrException)
{
	int n;
	addrinfo hints;
	addrinfo *addr, *info;

	hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV | AI_V4MAPPED | AI_ADDRCONFIG;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_addrlen = 0;
	hints.ai_addr = NULL;
	hints.ai_canonname = NULL;
	hints.ai_next = NULL;

	if (getaddrinfo(NULL, this->_port, &hints, &addr) != 0)
		throw GetAddrException();

	for (info = addr; info != NULL; info = info->ai_next)
	{
		this->_sockfd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);

		if (this->_sockfd == -1)
			continue;

		if (fcntl(this->_sockfd, F_SETFL, O_NONBLOCK) == -1)
		{
			std::cerr << "Error: fcntl: " << strerror(errno) << std::endl;

			if (close(this->_sockfd) == -1)
				std::cerr << "Error: close: " << strerror(errno) << std::endl;

			freeaddrinfo(addr);
			// for the time being the same exception is throw just to get out of the function
			throw GetAddrException();
		}

		if (bind(this->_sockfd, info->ai_addr, info->ai_addrlen) == 0)
			break;

		if (close(this->_sockfd) == -1)
		{
			std::cerr << "Error: close: " << strerror(errno) << std::endl;

			freeaddrinfo(addr);

			throw GetAddrException();
		}
	}

	if (this->_sockfd == -1)
	{
		std::cerr << "Error: socket: " << strerror(errno) << std::endl;

		freeaddrinfo(addr);

		throw GetAddrException();
	}

	if (info == NULL)
	{
		std::cerr << "Error: bind: " << strerror(errno) << std::endl;

		freeaddrinfo(addr);

		throw GetAddrException();
	}

	if (getsockname(this->_sockfd, info->ai_addr, &info->ai_addrlen) == -1)
	{
		std::cerr << "Error: getsockname: " << strerror(errno) << std::endl;

		if (close(this->_sockfd) == -1)
			std::cerr << "Error: close: " << strerror(errno) << std::endl;

		freeaddrinfo(addr);

		throw GetAddrException();
	}

	std::cout << "Info: IRC server initialized with port: ";

	if (info->ai_family == AF_INET)
		std::cout << ntohs(reinterpret_cast<sockaddr_in *>(info->ai_addr)->sin_port);
	else
		std::cout << ntohs(reinterpret_cast<sockaddr_in6 *>(info->ai_addr)->sin6_port);

	std::cout << std::endl;

	freeaddrinfo(addr);
}

void Server::stop() throw()
{
	if (close(this->_sockfd) == -1)
	{
		std::cerr << "Error: close: " << strerror(errno) << std::endl;
		
		throw SocketErrorException();
	}
}

void Server::addChannel(const Channel& channel) throw() {
    if (_channels.size() >= MAX_CHANNELS) {
        std::cerr << "Error: Maximum number of channels reached" << std::endl;
        return;
    }
    _channels.push_back(channel);
}

void Server::removeChannel(const Channel& channel) throw() {
    _channels.erase(std::remove(_channels.begin(), _channels.end(), channel), _channels.end());
}

void Server::addClient(const User& client) throw() {
    if (_clients.size() >= MAX_CLIENTS) {
        std::cerr << "Error: Maximum number of clients reached" << std::endl;
        return;
    }
    _clients.push_back(client);
}

void Server::removeClient(const User& client) throw() {
    _clients.erase(std::remove(_clients.begin(), _clients.end(), client), _clients.end());
}