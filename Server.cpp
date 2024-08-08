/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 12:37:05 by abenamar          #+#    #+#             */
/*   Updated: 2024/08/08 02:25:45 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(void) throw() : sockfd(-1), epfd(-1), maxevents(-1), bufsize(0), password(NULL), events(NULL), msgbuf(NULL), messages() { return; }

Server::Server(char const *const numericserv, char const *const password, int const maxevents, int const bufsize) throw(std::invalid_argument, std::runtime_error)
try : sockfd(ServerUtils::initSocket(numericserv)), epfd(epoll_create1(0)), maxevents(maxevents), bufsize(bufsize), password(password), events(ServerUtils::initArray<epoll_event>(maxevents, "maxevents")), msgbuf(ServerUtils::initArray<char>(bufsize, "bufsize")), messages()
{
	if (this->epfd == -1)
		throw std::runtime_error(std::string("std::runtime_error: epoll_create1: ") + strerror(errno));

	return;
}
catch (std::exception const &e)
{
	std::string what("Server::Server: ");

	what += e.what();

	if (what.find("ipv4") == std::string::npos)
	{
		if (what.find("epoll_create1") == std::string::npos)
		{
			if (what.find("maxevents") == std::string::npos)
				delete[] this->events;

			close(this->epfd);
		}

		close(this->sockfd);
	}

	throw std::runtime_error(what);
}

Server::Server(Server const & /* src */) throw() : sockfd(-1), epfd(-1), maxevents(-1), bufsize(0), password(NULL), events(NULL), msgbuf(NULL), messages() { return; }

Server::~Server(void) throw()
{
	delete[] this->msgbuf;
	delete[] this->events;

	for (std::map<int, std::string>::iterator it = this->messages.begin(); it != this->messages.end(); ++it)
		close(it->first);

	close(this->epfd);
	close(this->sockfd);

	return;
}

Server &Server::operator=(Server const & /* rhs */) throw() { return (*this); }

in_port_t Server::port(void) const throw(std::runtime_error)
{
	sockaddr_storage addr;
	socklen_t addrlen = sizeof(addr);

	if (getsockname(this->sockfd, reinterpret_cast<sockaddr *>(&addr), &addrlen) == -1)
		throw std::runtime_error(std::string("Server::port: std::runtime_error: getsockname: ") + strerror(errno));

	if (addr.ss_family == AF_INET)
		return ntohs(reinterpret_cast<sockaddr_in *>(&addr)->sin_port);

	return ntohs(reinterpret_cast<sockaddr_in6 *>(&addr)->sin6_port);
}

void Server::run(void) throw(std::runtime_error)
{
	epoll_event hints;
	int nfds, connfd, nread;
	sockaddr_storage addr;
	socklen_t addrlen;

	hints.events = EPOLLIN;
	hints.data.fd = this->sockfd;

	if (epoll_ctl(this->epfd, EPOLL_CTL_ADD, this->sockfd, &hints) == -1)
		throw std::runtime_error(std::string("Server::run: std::runtime_error: epoll_ctl (add ipv4 listening socket): ") + strerror(errno));

	while (true)
	{
		nfds = epoll_wait(this->epfd, this->events, this->maxevents, -1);

		if (nfds == -1)
			throw std::runtime_error(std::string("Server::run: std::runtime_error: epoll_wait: ") + strerror(errno));

		for (int i = 0; i < nfds; ++i)
		{
			if (this->events[i].data.fd == this->sockfd)
			{
				addrlen = sizeof(addr);
				connfd = accept(this->events[i].data.fd, reinterpret_cast<sockaddr *>(&addr), &addrlen);

				if (connfd == -1)
					throw std::runtime_error(std::string("Server::run: std::runtime_error: accept: ") + strerror(errno));

				if (fcntl(connfd, F_SETFL, O_NONBLOCK) == -1)
					throw std::runtime_error(std::string("Server::run: std::runtime_error: fcntl: ") + strerror(errno));

				hints.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
				hints.data.fd = connfd;

				if (epoll_ctl(this->epfd, EPOLL_CTL_ADD, connfd, &hints) == -1)
					throw std::runtime_error(std::string("Server::run: std::runtime_error: epoll_ctl (add connection socket): ") + strerror(errno));

				messages[this->events[i].data.fd] = "";
			}
			else
			{
				nread = recv(this->events[i].data.fd, this->msgbuf, this->bufsize, MSG_DONTWAIT);

				if (!nread)
				{
					if (epoll_ctl(this->epfd, EPOLL_CTL_DEL, this->events[i].data.fd, &hints) == -1)
						throw std::runtime_error(std::string("Server::run: std::runtime_error: epoll_ctl (remove connection socket): ") + strerror(errno));

					messages.erase(this->events[i].data.fd);

					if (close(this->events[i].data.fd) == -1)
						throw std::runtime_error(std::string("Server::run: std::runtime_error: close: ") + strerror(errno));

					continue;
				}

				if (nread == -1)
				{
					if (errno == EAGAIN && errno == EWOULDBLOCK)
						continue;

					throw std::runtime_error(std::string("Server::run: std::runtime_error: recv: ") + strerror(errno));
				}

				if (this->msgbuf[nread - 1] != '\0')
					this->msgbuf[nread] = '\0';

				messages[this->events[i].data.fd] += this->msgbuf;

				if (*messages[this->events[i].data.fd].rbegin() == '\n')
				{
					std::clog << messages[this->events[i].data.fd];

					messages[this->events[i].data.fd].clear();
				}
			}
		}
	}

	return;
}
