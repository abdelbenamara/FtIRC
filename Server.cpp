/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 12:37:05 by abenamar          #+#    #+#             */
/*   Updated: 2024/08/11 14:34:11 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Builder::Builder(void) throw() : epfd(-1), sockfd(-1), maxevents(16), bufsize(512), numericserv("6667"), password(""), events(NULL), msgbuf(NULL) { return; }

Server::Builder::Builder(Server::Builder const & /* src */) throw() : epfd(-1) { return; }

Server::Builder &Server::Builder::operator=(Server::Builder const & /* rhs */) throw() { return (*this); }

Server::Builder::~Builder(void) throw() { return; }

Server::Builder &Server::Builder::withNumericServ(char const *const numericserv) throw()
{
	this->numericserv = numericserv;

	return (*this);
}

Server::Builder &Server::Builder::withPassword(char const *const password) throw()
{
	this->password = password;

	return (*this);
}

Server::Builder &Server::Builder::withMaxEvents(int const maxevents) throw()
{
	this->maxevents = maxevents;

	return (*this);
}

Server::Builder &Server::Builder::withBufSize(int const bufsize) throw()
{
	this->bufsize = bufsize;

	return (*this);
}

Server *Server::Builder::build(void) throw(std::invalid_argument, std::runtime_error)
{
	try
	{
		this->epfd = epoll_create1(0);

		if (this->epfd == -1)
			throw std::runtime_error(std::string("Server::Builder::build: std::runtime_error: epoll_create1: ") + strerror(errno));

		this->sockfd = Server::Builder::initSocket(this->numericserv);
		this->events = Server::Builder::initArray<epoll_event>(maxevents, "maxevents");
		this->msgbuf = Server::Builder::initArray<char>(bufsize, "bufsize");
	}
	catch (std::exception const &e)
	{
		this->clear();

		throw;
	}

	return new Server(this->epfd, this->sockfd, this->maxevents, this->bufsize, this->password, this->events, this->msgbuf);
}

char const *Server::Builder::gai_strerror(int errcode) throw()
{
	switch (errcode)
	{
	case EAI_ADDRFAMILY:
		return "EAI_ADDRFAMILY: Address family for hostname not supported";
	case EAI_AGAIN:
		return "EAI_AGAIN: Temporary failure in name resolution";
	case EAI_BADFLAGS:
		return "EAI_BADFLAGS: Bad value for ai_flags";
	case EAI_FAIL:
		return "EAI_FAIL: Non-recoverable failure in name resolution";
	case EAI_FAMILY:
		return "EAI_FAMILY: ai_family not supported";
	case EAI_MEMORY:
		return "EAI_MEMORY: Memory allocation failure";
	case EAI_NODATA:
		return "EAI_NODATA: No address associated with hostname";
	case EAI_NONAME:
		return "EAI_NONAME: Name or service not known";
	case EAI_SERVICE:
		return "EAI_SERVICE: Servname not supported for ai_socktype";
	case EAI_SOCKTYPE:
		return "EAI_SOCKTYPE: ai_socktype not supported";
	case EAI_SYSTEM:
		return std::string("EAI_SYSTEM: System error: ").append(strerror(errno)).c_str();
	default:
		break;
	}

	return "Unknown error";
}

int Server::Builder::initSocket(char const *const numericserv) throw(std::invalid_argument, std::runtime_error)
{
	int tmp;
	std::istream const &check = std::istringstream(numericserv) >> tmp;
	addrinfo hints, *info;

	if (check.fail() || !check.eof() || tmp < 0 || tmp > std::numeric_limits<in_port_t>::max())
		throw std::invalid_argument(std::string("Server::Builder::build: Server::Builder::initSocket: std::invalid_argument: `") + numericserv + "': port number must be between 1 and 65535, or it can be 0 to use a random available port");

	hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV | AI_V4MAPPED | AI_ADDRCONFIG;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_addrlen = 0;
	hints.ai_addr = NULL;
	hints.ai_canonname = NULL;
	hints.ai_next = NULL;
	tmp = getaddrinfo(NULL, numericserv, &hints, &hints.ai_next);

	if (tmp)
		throw std::runtime_error(std::string("Server::Builder::build: Server::Builder::initSocket: std::runtime_error: getaddrinfo") + Server::Builder::gai_strerror(tmp));

	for (info = hints.ai_next; info != NULL; info = info->ai_next)
	{
		tmp = socket(info->ai_family, info->ai_socktype, info->ai_protocol);

		if (tmp == -1)
			continue;

		if (!bind(tmp, info->ai_addr, info->ai_addrlen))
			break;

		close(tmp);
	}

	freeaddrinfo(hints.ai_next);

	if (info == NULL)
		throw std::runtime_error(std::string("Server::Builder::build: Server::Builder::initSocket: std::runtime_error: socket, bind: ") + strerror(errno));

	if (fcntl(tmp, F_SETFL, O_NONBLOCK) == -1)
		throw std::runtime_error(std::string("Server::Builder::build: Server::Builder::initSocket: std::runtime_error: fcntl: ") + strerror(errno));

	if (listen(tmp, SOMAXCONN) == -1)
		throw std::runtime_error(std::string("Server::Builder::build: Server::Builder::initSocket: std::runtime_error: listen: ") + strerror(errno));

	return (tmp);
}

void Server::Builder::clear(void) throw()
{
	delete[] this->msgbuf;
	delete[] this->events;

	close(this->sockfd);
	close(this->epfd);

	return;
}

Server::Server(void) throw() : epfd(-1), sockfd(-1), maxevents(-1), bufsize(0), password(NULL), events(NULL), msgbuf(NULL), messages() { return; }

Server::Server(int const epfd, int const sockfd, int const maxevents, int const bufsize, char const *const password, epoll_event *const events, char *const msgbuf) throw() : epfd(epfd), sockfd(sockfd), maxevents(maxevents), bufsize(bufsize), password(password), events(events), msgbuf(msgbuf), messages() { return; }

Server::Server(Server const & /* src */) throw() : epfd(-1), sockfd(-1), maxevents(-1), bufsize(0), password(NULL), events(NULL), msgbuf(NULL), messages() { return; }

Server::~Server(void) throw()
{
	delete[] this->msgbuf;
	delete[] this->events;

	for (std::map<int, std::string>::iterator it = this->messages.begin(); it != this->messages.end(); ++it)
		close(it->first);

	close(this->sockfd);
	close(this->epfd);

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
		throw std::runtime_error(std::string("Server::run: std::runtime_error: epoll_ctl (add listening socket): ") + strerror(errno));

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

				if (nread == -1)
				{
					if (errno == EAGAIN && errno == EWOULDBLOCK)
						continue;

					throw std::runtime_error(std::string("Server::run: std::runtime_error: recv: ") + strerror(errno));
				}

				if (!nread)
				{
					if (epoll_ctl(this->epfd, EPOLL_CTL_DEL, this->events[i].data.fd, &hints) == -1)
						throw std::runtime_error(std::string("Server::run: std::runtime_error: epoll_ctl (remove connection socket): ") + strerror(errno));

					messages.erase(this->events[i].data.fd);

					if (close(this->events[i].data.fd) == -1)
						throw std::runtime_error(std::string("Server::run: std::runtime_error: close: ") + strerror(errno));

					continue;
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
