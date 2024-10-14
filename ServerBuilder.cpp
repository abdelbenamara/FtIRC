/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerBuilder.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/12 13:15:25 by abenamar          #+#    #+#             */
/*   Updated: 2024/10/13 20:53:56 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Builder::Builder(void) : epollfd(-1), sockfd(-1), numericserv("6667"), password() { return; }

Server::Builder::Builder(Server::Builder const & /* src */) { return; }

Server::Builder &Server::Builder::operator=(Server::Builder const & /* rhs */) throw() { return (*this); }

Server::Builder::~Builder(void) throw() { return; }

Server::Builder &Server::Builder::withNumericServ(std::string const &numericserv)
{
	std::istringstream check(numericserv);
	int port;

	check >> port;

	if (check.fail() || !check.eof())
		throw std::invalid_argument("Server::Builder::withNumericServ: std::invalid_argument: `" + numericserv + "': parameter must represent a numeric port number");

	if (port < 0 || std::numeric_limits<in_port_t>::max() < port)
		throw std::out_of_range("Server::Builder::withNumericServ: std::out_of_range: `" + numericserv + "': port number must be between 1 and 65535, or 0 to request a system-allocated (dynamic) port");

	this->numericserv = numericserv;

	return (*this);
}

Server::Builder &Server::Builder::withPassword(std::string const &password)
{
	this->password = password;

	return (*this);
}

Server *Server::Builder::build(void)
{
	addrinfo hints, *info;
	epoll_event event;
	Server *server;

	try
	{
		this->epollfd = epoll_create1(0);

		if (this->epollfd == -1)
			throw RuntimeErrno("epoll_create1");

		hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV | AI_V4MAPPED | AI_ADDRCONFIG;
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_addrlen = 0;
		hints.ai_addr = NULL;
		hints.ai_canonname = NULL;
		hints.ai_next = NULL;
		this->sockfd = getaddrinfo(NULL, this->numericserv.c_str(), &hints, &hints.ai_next);

		if (this->sockfd)
			throw std::runtime_error("std::runtime_error: getaddrinfo: " + Server::Builder::gai_strerror(this->sockfd));

		for (info = hints.ai_next; info != NULL; info = info->ai_next)
		{
			this->sockfd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);

			if (this->sockfd == -1)
				continue;

			if (!bind(this->sockfd, info->ai_addr, info->ai_addrlen))
				break;

			close(this->sockfd);
		}

		freeaddrinfo(hints.ai_next);

		if (info == NULL)
			throw RuntimeErrno("socket, bind");

		if (fcntl(this->sockfd, F_SETFL, O_NONBLOCK) == -1)
			throw RuntimeErrno("fcntl");

		if (listen(this->sockfd, SOMAXCONN) == -1)
			throw RuntimeErrno("listen");

		event.events = EPOLLIN;
		event.data.fd = this->sockfd;

		if (epoll_ctl(this->epollfd, EPOLL_CTL_ADD, event.data.fd, &event) == -1)
			throw RuntimeErrno("epoll_ctl");

		server = new Server(this->epollfd, this->sockfd, this->password);
	}
	catch (std::exception const &e)
	{
		this->clear();

		throw std::runtime_error("Server::Builder::build: " + std::string(e.what()));
	}

	return (server);
}

std::string const Server::Builder::gai_strerror(int const &errcode)
{
	switch (errcode)
	{
	case EAI_ADDRFAMILY:
		return ("EAI_ADDRFAMILY: Address family for hostname not supported");
	case EAI_AGAIN:
		return ("EAI_AGAIN: Temporary failure in name resolution");
	case EAI_BADFLAGS:
		return ("EAI_BADFLAGS: Bad value for ai_flags");
	case EAI_FAIL:
		return ("EAI_FAIL: Non-recoverable failure in name resolution");
	case EAI_FAMILY:
		return ("EAI_FAMILY: ai_family not supported");
	case EAI_MEMORY:
		return ("EAI_MEMORY: Memory allocation failure");
	case EAI_NODATA:
		return ("EAI_NODATA: No address associated with hostname");
	case EAI_NONAME:
		return ("EAI_NONAME: Name or service not known");
	case EAI_SERVICE:
		return ("EAI_SERVICE: Servname not supported for ai_socktype");
	case EAI_SOCKTYPE:
		return ("EAI_SOCKTYPE: ai_socktype not supported");
	case EAI_SYSTEM:
		return ("EAI_SYSTEM: System error" + std::string(strerror(errno)));
	default:
		break;
	}

	return ("Unknown error");
}

void Server::Builder::clear(void) throw()
{
	close(this->sockfd);
	close(this->epollfd);

	return;
}
