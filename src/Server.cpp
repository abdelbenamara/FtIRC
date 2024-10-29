/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 12:37:05 by abenamar          #+#    #+#             */
/*   Updated: 2024/10/29 18:38:55 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

namespace irc
{
	unsigned int const Server::MAX_CLIENTS = std::min<unsigned int>(SRV_MAX_CLIENTS, SOMAXCONN);

	std::size_t const Server::PASS_MAX_LEN = Message::MAX_CHARS - std::string(CMD_PASS).length() - 2;

	unsigned int const Server::MAX_EVENTS = SRV_MAX_EVENTS;

	epoll_event Server::events[Server::MAX_EVENTS];

	char Server::buffer[MSG_SIZE];

	int Server::epollfd = epoll_create1(0), Server::sockfd = -1;

	Server &Server::getInstance(std::string const &numericserv, std::string const &password)
	{
		static Server instance(numericserv, password);

		return (instance);
	}

	void Server::produce(Client const &client, Message const &message)
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
			throw std::runtime_error("Server::produce: " + std::string(e.what()));
		}

		return;
	}

	void Server::produce(Client const &client, std::string const &command, std::string const &comment)
	{
		try
		{
			return (Server::produce(client, Message::Builder()
												.withPrefix(SRV_NAME)
												.withCommand(command)
												.addParameter(client.getNickname())
												.addParameter(comment)
												.build()));
		}
		catch (std::exception const &e)
		{
			throw std::runtime_error("Server::produce: " + std::string(e.what()));
		}
	}

	int Server::initServerPort(std::string const &numericserv)
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
				Server::sockfd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);

				if (Server::sockfd == -1)
					continue;
				else if (!bind(Server::sockfd, info->ai_addr, info->ai_addrlen))
					break;

				close(Server::sockfd);
			}

			freeaddrinfo(hints.ai_next);

			if (info == NULL)
				throw RuntimeErrno("socket / bind");
			else if (getsockname(Server::sockfd, reinterpret_cast<sockaddr *>(&addr), &hints.ai_addrlen) == -1)
				throw RuntimeErrno("getsockname");
			else if (addr.ss_family == AF_INET)
				port = reinterpret_cast<sockaddr_in *>(&addr)->sin_port;
			else
				port = reinterpret_cast<sockaddr_in6 *>(&addr)->sin6_port;
		}
		catch (std::exception const &e)
		{
			throw std::runtime_error("Server::initServerPort: " + std::string(e.what()));
		}

		return (ntohs(port));
	}

	Server::Server(std::string const &numericserv, std::string const &password)
	try : port(Server::initServerPort(numericserv)),
		password(password),
		clients(),
		buffers(),
		overflows()
	{
		std::ostringstream err;
		epoll_event hints;

		if (Server::epollfd == -1)
			throw RuntimeErrno("epoll_create1");
		else if (this->password.length() > Server::PASS_MAX_LEN)
			throw std::length_error(reinterpret_cast<std::ostringstream &>(err << "std::length_error: " << this->password.length() << ": password must not have more than " << Server::PASS_MAX_LEN << " characters").str());
		else if (this->password.find_first_of("\0\r\n", 0, 3) != std::string::npos)
			throw std::domain_error("std::domain_error: password must have any character except: NUL, CR, LF");
		else if (fcntl(Server::sockfd, F_SETFL, O_NONBLOCK) == -1)
			throw RuntimeErrno("fcntl");
		else if (listen(Server::sockfd, Server::MAX_CLIENTS) == -1)
			throw RuntimeErrno("listen");

		hints.events = EPOLLIN;
		hints.data.fd = Server::sockfd;

		if (epoll_ctl(Server::epollfd, EPOLL_CTL_ADD, hints.data.fd, &hints) == -1)
			throw RuntimeErrno("epoll_ctl");

		return;
	}
	catch (std::exception const &e)
	{
		close(Server::sockfd);
		close(Server::epollfd);

		throw std::runtime_error("Server::Server: " + std::string(e.what()));
	}

	Server::~Server(void) throw()
	{
		for (std::map<int, Client>::iterator it = this->clients.begin(); it != this->clients.end(); ++it)
		{
			try
			{
				Server::produce(it->second, Message::Builder()
												.withCommand(CMD_ERROR)
												.addParameter("Closing link: (" + it->second.userId() + ") [Server shutting down]")
												.build());
			}
			catch (std::exception const &e)
			{
				std::cerr << "Error: Server::~Server: " << e.what() << std::endl;
			}

			close(it->first);
		}

		close(Server::sockfd);
		close(Server::epollfd);

		return;
	}

	in_port_t const &Server::getPort(void) const throw() { return (this->port); }

	std::map<int, Client> const &Server::getClients(void) const throw() { return (this->clients); }

	void Server::completeRegistration(Client const &client)
	{
		try
		{
			if (!client.isRegistered())
				throw std::runtime_error("std::runtime_error: client must be registered to complete its registration with this server");
			else if (client.getPassword() != this->password)
			{
				Server::produce(client, Message::Builder()
											.withCommand(CMD_ERROR)
											.addParameter("Closing link: (" + client.userId() + ") [Access denied by configuration]")
											.build());
				this->removeClient(client);
			}
			else
			{
				Server::produce(client, RPL_WELCOME, "Welcome to the Internet Relay Network " + client.str());
			}
		}
		catch (std::exception const &e)
		{
			throw std::runtime_error("Server::completeRegistration: " + std::string(e.what()));
		}

		return;
	}

	void Server::poll(void)
	{
		int n, fd;
		Client *client;
		std::string command;

		try
		{
			n = epoll_wait(Server::epollfd, Server::events, Server::MAX_EVENTS, -1);

			if (n == -1)
				throw RuntimeErrno("epoll_wait");

			for (int i = 0; i < n; ++i)
			{
				fd = Server::events[i].data.fd;

				if (fd == Server::sockfd)
					this->addClient();
				if (this->clients.find(fd) == this->clients.end())
					continue;
				else
				{
					client = &this->clients.find(fd)->second;

					if (!this->consumeBuffer(*client))
					{
						this->removeClient(*client);

						continue;
					}

					while (this->clients.find(fd) != this->clients.end() && !client->getMessages().empty())
					{
#ifndef NDEBUG
						std::cout << "Debug: Client #" << fd << " --> Server: " << client->getMessages().front() << std::endl;
#endif

						command = client->getMessages().front().getCommand();

						try
						{
							Command::apply(command)(client->consume(), *client);
						}
						catch (Command::Unknown const &)
						{
							Command::reply(ERR_UNKNOWNCOMMAND, *client, client->consume().getCommand());
						}
						catch (std::exception const &e)
						{
							std::cerr << "Error: " << e.what() << std::endl;
						}
					}
				}
			}
		}
		catch (std::exception const &e)
		{
			throw std::runtime_error("Server::poll: " + std::string(e.what()));
		}

		return;
	}

	void Server::addClient(void)
	{
		static char buf[INET6_ADDRSTRLEN];
		socklen_t addrlen = sizeof(sockaddr_storage);
		sockaddr_storage addr;
		epoll_event hints;
		void *ip;
		Client *client;

		try
		{
			hints.events = EPOLLIN | EPOLLET;
			hints.data.fd = accept(Server::sockfd, reinterpret_cast<sockaddr *>(&addr), &addrlen);

			if (hints.data.fd == -1)
				throw RuntimeErrno("accept");
			else if (fcntl(hints.data.fd, F_SETFL, O_NONBLOCK) == -1)
				throw RuntimeErrno("fcntl");
			else if (epoll_ctl(Server::epollfd, EPOLL_CTL_ADD, hints.data.fd, &hints) == -1)
				throw RuntimeErrno("epoll_ctl");
			else if (addr.ss_family == AF_INET)
				ip = &reinterpret_cast<sockaddr_in *>(&addr)->sin_addr;
			else
				ip = &reinterpret_cast<sockaddr_in6 *>(&addr)->sin6_addr;

			this->clients.insert(std::make_pair(hints.data.fd, Client(hints.data.fd, inet_ntop(addr.ss_family, ip, buf, INET6_ADDRSTRLEN))));

			client = &this->clients.find(hints.data.fd)->second;

			this->buffers.insert(std::make_pair(client->getSocket(), ""));
			this->buffers.find(client->getSocket())->second.reserve(Message::MAX_LEN);
			this->overflows.insert(std::make_pair(client->getSocket(), false));
			Server::produce(*client, CMD_NOTICE, "*** Your IP address (" + client->getHostaddr() + ") is used for your netwide unique identifier.");
		}
		catch (std::exception const &e)
		{
			epoll_ctl(Server::epollfd, EPOLL_CTL_DEL, hints.data.fd, Server::events);
			close(hints.data.fd);

			throw std::runtime_error("Server::addClient: " + std::string(e.what()));
		}

		return;
	}

	void Server::removeClient(Client const &client)
	{
		try
		{
			if (this->clients.find(client.getSocket()) == this->clients.end())
				throw std::runtime_error("std::runtime_error: unknown client " + client.str());
			else if (epoll_ctl(Server::epollfd, EPOLL_CTL_DEL, client.getSocket(), Server::events) == -1)
				throw RuntimeErrno("epoll_ctl");
			else if (close(client.getSocket()) == -1)
				throw RuntimeErrno("close");

			this->overflows.erase(client.getSocket());
			this->buffers.erase(client.getSocket());
			this->clients.erase(client.getSocket());
		}
		catch (std::exception const &e)
		{
			throw std::runtime_error("Server::removeClient: " + std::string(e.what()));
		}

		return;
	}

	void Server::parseMessage(Client &client, std::size_t const &crlfpos)
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
				client.produce(Message::parse(input));

			*overflow = false;
		}
		catch (std::exception const &e)
		{
			throw std::runtime_error("Server::parseMessage: " + std::string(e.what()));
		}

		return (this->parseMessage(client, buffer->find_first_of(Message::CRLF)));
	}

	bool Server::consumeBuffer(Client &client)
	{
		std::string *buffer;
		ssize_t incap, nread;

		try
		{
			do
			{
				buffer = &this->buffers.find(client.getSocket())->second;
				incap = Message::MAX_LEN - buffer->length();
				nread = recv(client.getSocket(), Server::buffer, incap, 0);

				if (!nread)
					return (false);
				else if (nread == -1)
				{
					if (errno == EAGAIN || errno == EWOULDBLOCK)
						break;

					throw RuntimeErrno("recv");
				}

				buffer->append(Server::buffer, nread);
				this->parseMessage(client, buffer->find_first_of(Message::CRLF, buffer->length() - nread));
			} while (incap == nread);
		}
		catch (std::exception const &e)
		{
			throw std::runtime_error("Server::consumeBuffer: " + std::string(e.what()));
		}

		return (true);
	}
} // namespace irc
