/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejankovs <ejankovs@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 12:37:05 by abenamar          #+#    #+#             */
/*   Updated: 2024/10/15 22:14:50 by ejankovs         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

int const Server::MAXEVENTS = 16;

Server::Server(void) : epollfd(-1), sockfd(-1), password(), events(NULL) { return; }

Server::Server(int const &epollfd, int const &sockfd, std::string const &password) : epollfd(epollfd), sockfd(sockfd), password(password), events(new epoll_event[Server::MAXEVENTS]), nfds(0), clients() { return; }

Server::Server(Server const & /* src */) : epollfd(-1), sockfd(-1), password(), events(NULL) { return; }

Server::~Server(void) throw()
{
	delete[] this->events;

	for (std::map<int, Client *const>::iterator it = this->clients.begin(); it != this->clients.end(); ++it)
		delete it->second;

	close(this->sockfd);
	close(this->epollfd);

	return;
}

Server &Server::operator=(Server const & /* rhs */) throw() { return (*this); }

in_port_t Server::port(void) const
{
	sockaddr_storage addr;
	socklen_t addrlen = sizeof(addr);

	if (getsockname(this->sockfd, reinterpret_cast<sockaddr *>(&addr), &addrlen) == -1)
		throw RuntimeErrno("Server::port", "getsockname");

	if (addr.ss_family == AF_INET)
		return (ntohs(reinterpret_cast<sockaddr_in *>(&addr)->sin_port));

	return (ntohs(reinterpret_cast<sockaddr_in6 *>(&addr)->sin6_port));
}

int Server::waitForEvents(void)
{
	this->nfds = epoll_wait(this->epollfd, this->events, Server::MAXEVENTS, -1);

	if (this->nfds == -1)
		throw RuntimeErrno("Server::waitForEvents", "epoll_wait");

	return (this->nfds);
}

int Server::getEventSocket(int const &pos) const
{
	std::ostringstream err;

	if (pos < 0 || this->nfds <= pos)
	{
		err << "Server::getEventSocket:: std::out_of_range: `" << pos << "': parameter must be between 0 and the value returned by `Server::waitForEvents' excluded";

		throw std::out_of_range(err.str());
	}

	return (this->events[pos].data.fd);
}

int Server::getSocket(void) const throw() { return (this->sockfd); }

std::string const &Server::getPassword(void) const throw() { return (this->password); }

Client *const &Server::getClient(int const &connfd)
{
	std::ostringstream err;

	if (this->clients.find(connfd) == this->clients.end())
	{
		err << "Server::getClient: std::invalid_argument: `" << connfd << "': parameter must be a value returned by `Server::getEventSocket'";

		throw std::invalid_argument(err.str());
	}

	return (this->clients[connfd]);
}

std::map<int, Client *const> &Server::getClients(void) throw() { return (this->clients); }

void Server::addClient(void)
{
	epoll_event hints;
	sockaddr_storage addr;
	socklen_t addrlen = sizeof(addr);

	hints.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
	hints.data.fd = accept(this->sockfd, reinterpret_cast<sockaddr *>(&addr), &addrlen);

	try
	{
		if (hints.data.fd == -1)
			throw RuntimeErrno("accept");

		if (fcntl(hints.data.fd, F_SETFL, O_NONBLOCK) == -1)
			throw RuntimeErrno("fcntl");

		if (epoll_ctl(this->epollfd, EPOLL_CTL_ADD, hints.data.fd, &hints) == -1)
			throw RuntimeErrno("epoll_ctl");

		this->clients.insert(std::make_pair(hints.data.fd, new Client(hints.data.fd)));
	}
	catch (std::exception const &e)
	{
		epoll_ctl(this->epollfd, EPOLL_CTL_DEL, hints.data.fd, this->events);
		close(hints.data.fd);

		throw std::runtime_error("Server::addClient: " + std::string(e.what()));
	}

	return;
}

void Server::setClient(int const &connfd, Client *const client)
{
	this->clients.insert(std::make_pair(connfd, client));

	return;
}

void Server::removeClient(std::map<int, Client *const>::iterator it)
{
	Client *const client = it->second;

	if (epoll_ctl(this->epollfd, EPOLL_CTL_DEL, client->getSocket(), this->events) == -1)
		throw RuntimeErrno("Server::removeClient", "epoll_ctl");

	this->clients.erase(it);

	delete client;

	return;
}

void Server::removeClient(int const &connfd)
{
	std::ostringstream err;
	std::map<int, Client *const>::iterator it = this->clients.find(connfd);

	if (it == this->clients.end())
	{
		err << "Server::removeClient: std::invalid_argument: `" << connfd << "': parameter must be a value returned by `Server::getEventSocket'";

		throw std::invalid_argument(err.str());
	}

	return this->removeClient(it);
}

int Server::getConnfd(std::string clientName) throw()
{
    for (std::map<int, Client *const>::const_iterator it = this->clients.begin(); it != this->clients.end(); ++it) {
        if (it->second->getNickname() == clientName) {
            return it->first;
        }
    }
	return -1;
}