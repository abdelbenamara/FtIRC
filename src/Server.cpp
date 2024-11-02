/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 12:37:05 by abenamar          #+#    #+#             */
/*   Updated: 2024/11/02 21:09:23 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

std::size_t const irc::Server::MAX_CLIENTS = std::min<std::size_t>(SRV_MAX_CLIENTS, SOMAXCONN), irc::Server::MAX_CHANNELS = SRV_MAX_CHANNELS;

std::size_t const irc::Server::PASS_MAX_LEN = Message::MAX_CHARS - std::string(CMD_PASS).length() - 2, irc::Server::MAX_EVENTS = SRV_MAX_EVENTS;

epoll_event irc::Server::events[irc::Server::MAX_EVENTS];

char irc::Server::buffer[MSG_LINE_LEN];

int irc::Server::epollfd = epoll_create1(0), irc::Server::sockfd = -1;

irc::Server &irc::Server::getInstance(std::string const &numericserv, std::string const &password)
{
	static Server instance(numericserv, password);

	return (instance);
}

void irc::Server::produce(Client const &client, Message const &message)
{
	std::string buf;

	try
	{
		buf = message.str();

		if (send(client.getSocket(), buf.c_str(), buf.length(), 0) == -1)
			throw RuntimeErrno("send");

#ifndef NDEBUG
		std::cout << "Debug: Server --> Client #" << client.getSocket() << ": " << message << std::endl;
#endif
	}
	catch (std::exception const &e)
	{
		throw std::runtime_error("irc::Server::produce: " + std::string(e.what()));
	}

	return;
}

void irc::Server::produce(Client const &client, std::string const &command, std::string const &comment)
{
	try
	{
		return (irc::Server::produce(client, Message::Builder()
												 .withPrefix(SRV_NAME)
												 .withCommand(command)
												 .addParameter(client.getNickname())
												 .addParameter(comment)
												 .build()));
	}
	catch (std::exception const &e)
	{
		throw std::runtime_error("irc::Server::produce: " + std::string(e.what()));
	}
}

int irc::Server::initPort(std::string const &numericserv)
{
	std::istringstream check(numericserv);
	in_port_t port;
	addrinfo hints, *info;
	int eai;
	sockaddr_storage addr;

	try
	{
		check >> port;

		if (!check.eof())
			throw std::invalid_argument("std::invalid_argument: " + numericserv + ": numericserv must be a port number between 1 and 65535, or 0 to request a system-allocated (dynamic) port");

		hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV | AI_V4MAPPED | AI_ADDRCONFIG;
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_addrlen = sizeof(sockaddr_storage);
		hints.ai_addr = NULL;
		hints.ai_canonname = NULL;
		hints.ai_next = NULL;
		eai = getaddrinfo(NULL, numericserv.c_str(), &hints, &hints.ai_next);

		if (eai)
			throw std::runtime_error("std::runtime_error: getaddrinfo: " + std::string(gai_strerror(eai)) + (eai == EAI_SYSTEM ? ": " + std::string(strerror(errno)) : ""));

		for (info = hints.ai_next; info != NULL; info = info->ai_next)
		{
			irc::Server::sockfd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);

			if (irc::Server::sockfd == -1)
				continue;
			else if (!bind(irc::Server::sockfd, info->ai_addr, info->ai_addrlen))
				break;

			close(irc::Server::sockfd);
		}

		freeaddrinfo(hints.ai_next);

		if (info == NULL)
			throw RuntimeErrno("socket / bind");
		else if (getsockname(irc::Server::sockfd, reinterpret_cast<sockaddr *>(&addr), &hints.ai_addrlen) == -1)
			throw RuntimeErrno("getsockname");
		else if (addr.ss_family == AF_INET)
			port = reinterpret_cast<sockaddr_in *>(&addr)->sin_port;
		else
			port = reinterpret_cast<sockaddr_in6 *>(&addr)->sin6_port;
	}
	catch (std::exception const &e)
	{
		throw std::runtime_error("irc::Server::initPort: " + std::string(e.what()));
	}

	return (ntohs(port));
}

irc::Server::Server(std::string const &numericserv, std::string const &password)
try : port(irc::Server::initPort(numericserv)),
	password(password),
	clients(),
	buffers(),
	overflows(),
	channels()
{
	epoll_event hints;

	if (irc::Server::epollfd == -1)
		throw RuntimeErrno("epoll_create1");
	else if (this->password.length() > irc::Server::PASS_MAX_LEN)
		throw std::length_error("std::length_error: " + utils::to_string(this->password.length()) + ": password must not have more than " + utils::to_string(irc::Server::PASS_MAX_LEN) + " characters");
	else if (this->password.find_first_of("\0\r\n", 0, 3) != std::string::npos)
		throw std::domain_error("std::domain_error: password must have any character except: NUL, CR, LF");
	else if (fcntl(irc::Server::sockfd, F_SETFL, O_NONBLOCK) == -1)
		throw RuntimeErrno("fcntl");
	else if (listen(irc::Server::sockfd, irc::Server::MAX_CLIENTS) == -1)
		throw RuntimeErrno("listen");

	hints.events = EPOLLIN;
	hints.data.fd = irc::Server::sockfd;

	if (epoll_ctl(irc::Server::epollfd, EPOLL_CTL_ADD, hints.data.fd, &hints) == -1)
		throw RuntimeErrno("epoll_ctl");

	return;
}
catch (std::exception const &e)
{
	close(irc::Server::sockfd);
	close(irc::Server::epollfd);

	throw std::runtime_error("irc::Server::Server: " + std::string(e.what()));
}

irc::Server::~Server(void) throw()
{
	for (std::map<int, Client>::const_iterator cit = this->clients.begin(), tmp = cit; cit != this->clients.end(); tmp = cit++)
	{
		try
		{
			this->removeClient(tmp->second, "Server shutting down");
		}
		catch (std::exception const &e)
		{
			close(tmp->second.getSocket());

			std::cerr << "Error: irc::Server::~Server: " << e.what() << std::endl;
		}
	}

	close(irc::Server::sockfd);
	close(irc::Server::epollfd);

	return;
}

in_port_t const &irc::Server::getPort(void) const throw() { return (this->port); }

std::map<int, irc::Client> const &irc::Server::getClients(void) const throw() { return (this->clients); }

std::map<std::string, irc::Channel> &irc::Server::getChannels(void) throw() { return (this->channels); }

void irc::Server::challengeRegistration(Client const &client)
{
	try
	{
		if (!client.isRegistered())
			throw std::runtime_error("std::runtime_error: client must be registered to complete its registration with this server");
		else if (client.getPassword() != this->password)
			return (this->removeClient(client, "Access denied by configuration"));

		irc::Server::produce(client, RPL_WELCOME, "Welcome to the Internet Relay Network " + client.str());
	}
	catch (std::exception const &e)
	{
		throw std::runtime_error("irc::Server::challengeRegistration: " + std::string(e.what()));
	}

	return;
}

void irc::Server::removeClient(Client const &client, std::string const &comment)
{
	try
	{
		if (this->clients.find(client.getSocket()) == this->clients.end())
			throw std::runtime_error("std::runtime_error: unknown client " + client.str());

		irc::Server::produce(client, Message::Builder()
										 .withCommand(CMD_ERROR)
										 .addParameter("Closing link: (" + client.userId() + ") [" + comment + ']')
										 .build());

		if (epoll_ctl(irc::Server::epollfd, EPOLL_CTL_DEL, client.getSocket(), irc::Server::events) == -1)
			throw RuntimeErrno("epoll_ctl");
		else if (close(client.getSocket()) == -1)
			throw RuntimeErrno("close");

		this->overflows.erase(client.getSocket());
		this->buffers.erase(client.getSocket());
		this->clients.erase(client.getSocket());
	}
	catch (std::exception const &e)
	{
		throw std::runtime_error("irc::Server::removeClient: " + std::string(e.what()));
	}

	return;
}

void irc::Server::poll(void)
{
	int n, fd;
	std::map<int, Client>::iterator it;

	try
	{
		n = epoll_wait(irc::Server::epollfd, irc::Server::events, irc::Server::MAX_EVENTS, -1);

		if (n == -1)
			throw RuntimeErrno("epoll_wait");

		for (int i = 0; i < n; ++i)
		{
			fd = irc::Server::events[i].data.fd;

			if (fd == irc::Server::sockfd)
			{
				this->addClient();

				continue;
			}

			it = this->clients.find(fd);

			if (it == this->clients.end())
				continue;
			else if (!this->consumeBuffer(it->second))
			{
				this->removeClient(it->second, "Client exited");

				continue;
			}

			while (this->clients.find(fd) != this->clients.end() && !it->second.getMessages().empty())
			{
#ifndef NDEBUG
				std::cout << "Debug: Client #" << fd << " --> Server: " << it->second.getMessages().front() << std::endl;
#endif

				try
				{
					Command::apply(it->second.consume(), it->second);
				}
				catch (std::exception const &e)
				{
					std::cerr << "Error: " << e.what() << std::endl;
				}
			}
		}
	}
	catch (std::exception const &e)
	{
		throw std::runtime_error("irc::Server::poll: " + std::string(e.what()));
	}

	return;
}

void irc::Server::addClient(void)
{
	static char buf[INET6_ADDRSTRLEN];
	socklen_t addrlen = sizeof(sockaddr_storage);
	sockaddr_storage addr;
	epoll_event hints;
	void *ip;
	Client *client;

	try
	{
		hints.events = EPOLLIN | EPOLLOUT | EPOLLET;
		hints.data.fd = accept(irc::Server::sockfd, reinterpret_cast<sockaddr *>(&addr), &addrlen);

		if (hints.data.fd == -1)
			throw RuntimeErrno("accept");
		else if (fcntl(hints.data.fd, F_SETFL, O_NONBLOCK) == -1)
			throw RuntimeErrno("fcntl");
		else if (epoll_ctl(irc::Server::epollfd, EPOLL_CTL_ADD, hints.data.fd, &hints) == -1)
			throw RuntimeErrno("epoll_ctl");
		else if (addr.ss_family == AF_INET)
			ip = &reinterpret_cast<sockaddr_in *>(&addr)->sin_addr;
		else
			ip = &reinterpret_cast<sockaddr_in6 *>(&addr)->sin6_addr;

		client = &this->clients.insert(std::make_pair(hints.data.fd, Client(hints.data.fd, inet_ntop(addr.ss_family, ip, buf, INET6_ADDRSTRLEN)))).first->second;

		this->buffers.insert(std::make_pair(client->getSocket(), ""));
		this->buffers.find(client->getSocket())->second.reserve(Message::MAX_LEN);
		this->overflows.insert(std::make_pair(client->getSocket(), false));
		irc::Server::produce(*client, CMD_NOTICE, "*** Your IP address (" + client->getHostaddr() + ") is used for your netwide unique identifier.");
	}
	catch (std::exception const &e)
	{
		epoll_ctl(irc::Server::epollfd, EPOLL_CTL_DEL, hints.data.fd, irc::Server::events);
		close(hints.data.fd);

		throw std::runtime_error("irc::Server::addClient: " + std::string(e.what()));
	}

	return;
}

void irc::Server::extractMessage(Client &client, std::size_t const &crlfpos)
{
	std::string *buffer, input;
	bool *overflow;

	try
	{
		buffer = &this->buffers.find(client.getSocket())->second;
		overflow = &this->overflows.find(client.getSocket())->second;

		if (crlfpos == std::string::npos)
		{
			if (buffer->length() == Message::MAX_LEN)
				*overflow = true;

			if (*overflow)
				buffer->clear();

			return;
		}

		input = buffer->substr(0, crlfpos) + Message::CRLF;

		buffer->erase(0, buffer->find_first_not_of(Message::CRLF, crlfpos));

		if (!*overflow && 0 < crlfpos && crlfpos <= Message::MAX_CHARS)
			client.produce(Message::from(input));

		*overflow = false;
	}
	catch (std::exception const &e)
	{
		throw std::runtime_error("irc::Server::extractMessage: " + std::string(e.what()));
	}

	return (this->extractMessage(client, buffer->find_first_of(Message::CRLF)));
}

bool irc::Server::consumeBuffer(Client &client)
{
	std::string *buffer;
	ssize_t incap, nread;

	try
	{
		do
		{
			buffer = &this->buffers.find(client.getSocket())->second;
			incap = Message::MAX_LEN - buffer->length();
			nread = recv(client.getSocket(), irc::Server::buffer, incap, 0);

			if (!nread)
				return (false);
			else if (nread == -1)
			{
				if (errno == EAGAIN || errno == EWOULDBLOCK)
					break;

				throw RuntimeErrno("recv");
			}

			buffer->append(irc::Server::buffer, nread);
			this->extractMessage(client, buffer->find_first_of(Message::CRLF, buffer->length() - nread));
		} while (incap == nread);
	}
	catch (std::exception const &e)
	{
		throw std::runtime_error("irc::Server::consumeBuffer: " + std::string(e.what()));
	}

	return (true);
}
