/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 12:37:05 by abenamar          #+#    #+#             */
/*   Updated: 2024/11/09 16:42:34 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

std::map<std::string, irc::Channel, irc::utils::t_istringcomp> irc::Server::channels(&utils::istringcomp);

int irc::Server::epollfd = -1, irc::Server::sockfd = -1;

epoll_event *irc::Server::events = NULL;

char *irc::Server::buffer = NULL;

irc::Server &irc::Server::getInstance(std::string const &numericserv, std::string const &password)
try
{
	static Server instance(numericserv, password);

	return (instance);
}
catch (std::exception const &e)
{
	throw std::runtime_error("irc::Server::getInstance: " + std::string(e.what()));
}

int irc::Server::initPort(std::string const &numericserv)
try
{
	std::istringstream check(numericserv);
	in_port_t port;
	addrinfo hints, *info;
	int eai;
	sockaddr_storage addr;

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
		throw utils::RuntimeErrno("socket, bind");
	else if (getsockname(Server::sockfd, reinterpret_cast<sockaddr *>(&addr), &hints.ai_addrlen) == -1)
		throw utils::RuntimeErrno("getsockname");
	else if (addr.ss_family == AF_INET)
		port = reinterpret_cast<sockaddr_in *>(&addr)->sin_port;
	else
		port = reinterpret_cast<sockaddr_in6 *>(&addr)->sin6_port;

	return (ntohs(port));
}
catch (std::exception const &e)
{
	throw std::runtime_error("irc::Server::initPort: " + std::string(e.what()));
}

irc::Server::Server(std::string const &numericserv, std::string const &password)
try : start(time(NULL)),
	port(Server::initPort(numericserv)),
	password(password),
	clients(),
	overflows(),
	inputs(),
	outputs()
{
	size_t const max = Config::getInstance().getSize(PRP_LINELEN) - std::string(CMD_PASS).length() - Message::CRLF.length() - 2;
	epoll_event hints;

	if (this->password.length() > max)
		throw std::length_error("std::length_error: " + utils::to_string(this->password.length()) + ": password must not have more than " + utils::to_string(max) + " characters");
	else if (this->password.find_first_of("\0\r\n", 0, 3) != std::string::npos)
		throw std::domain_error("std::domain_error: password must have any character except: NUL, CR, LF");
	else if (fcntl(Server::sockfd, F_SETFL, O_NONBLOCK) == -1)
		throw utils::RuntimeErrno("fcntl (F_SETFL)");
	else if (listen(Server::sockfd, std::min<std::size_t>(Config::getInstance().getSize(PRP_MAXCLIENTS), SOMAXCONN)) == -1)
		throw utils::RuntimeErrno("listen");

	hints.events = EPOLLIN;
	hints.data.fd = Server::sockfd;
	Server::epollfd = epoll_create1(0);

	if (Server::epollfd == -1)
		throw utils::RuntimeErrno("epoll_create1");
	else if (epoll_ctl(Server::epollfd, EPOLL_CTL_ADD, hints.data.fd, &hints) == -1)
		throw utils::RuntimeErrno("epoll_ctl (EPOLL_CTL_ADD)");
	else if (std::signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		throw utils::RuntimeErrno("std::signal (SIGPIPE)");

	Server::events = new epoll_event[Config::getInstance().getSize(PRP_MAXEVENTS)];
	Server::buffer = new char[Config::getInstance().getSize(PRP_LINELEN)];

	return;
}
catch (std::exception const &e)
{
	delete[] Server::buffer;
	delete[] Server::events;

	close(Server::sockfd);
	close(Server::epollfd);

	throw std::runtime_error("irc::Server::Server: " + std::string(e.what()));
}

irc::Server::~Server(void) throw()
{
	for (std::map<int, Client>::iterator it = this->clients.begin(); it != this->clients.end(); it = this->clients.begin())
	{
		try
		{
			this->removeClient(it->second, "Server shutting down");
		}
		catch (std::exception const &e)
		{
			close(it->first);
			this->clients.erase(it->first);

			std::cerr << "Error: irc::Server::~Server: " << e.what() << std::endl;
		}
	}

	delete[] Server::buffer;
	delete[] Server::events;

	close(Server::sockfd);
	close(Server::epollfd);

	return;
}

in_port_t const &irc::Server::getPort(void) const throw() { return (this->port); }

std::map<int, irc::Client> const &irc::Server::getClients(void) const throw() { return (this->clients); }

void irc::Server::flush(void)
try
{
	for (std::map<int, Client>::const_iterator cit = this->clients.begin(); cit != this->clients.end(); ++cit)
		this->write(cit->second);

	return;
}
catch (std::exception const &e)
{
	throw std::runtime_error("irc::Server::flush: " + std::string(e.what()));
}

void irc::Server::produce(Client const &client, Message const &message)
try
{
	std::map<int, std::queue<Message> >::iterator it = this->outputs.find(client.getSocket());
	std::string buf;

	if (it == this->outputs.end())
		throw std::out_of_range("std::out_of_range: " + utils::to_string(client.getSocket()) + ": unknown client " + client.str());

	it->second.push(message);

	return;
}
catch (std::exception const &e)
{
	throw std::runtime_error("irc::Server::produce: " + std::string(e.what()));
}

void irc::Server::produce(std::string const &command, Client const &client, std::string const &comment)
{
	return (this->produce(client, Message::Builder()
									  .withPrefix(Config::getInstance().getText(PRP_SERVERNAME))
									  .withCommand(command)
									  .addParameter(client.getNickname())
									  .addParameter(comment)
									  .build()));
}

void irc::Server::support(Client const &client)
{
	this->produce(client, Message::Builder()
							  .withPrefix(Config::getInstance().getText(PRP_SERVERNAME))
							  .withCommand(RPL_ISUPPORT)
							  .addParameter(client.getNickname())
							  .addParameter(PRP_CASEMAPPING "=" + Config::getInstance().getText(PRP_CASEMAPPING))
							  .addParameter(PRP_CHANLIMIT "=" + Config::getInstance().getText(PRP_CHANTYPES) + ':' + utils::to_string(Config::getInstance().getSize(PRP_CHANLIMIT)))
							  .addParameter(PRP_CHANMODES "=" + Config::getInstance().getText(PRP_CHANMODES))
							  .addParameter(PRP_CHANNELLEN "=" + utils::to_string(Config::getInstance().getSize(PRP_CHANNELLEN)))
							  .addParameter(PRP_CHANTYPES "=" + Config::getInstance().getText(PRP_CHANTYPES))
							  .addParameter(PRP_KEYLEN "=" + utils::to_string(Config::getInstance().getSize(PRP_KEYLEN)))
							  .addParameter(PRP_KICKLEN "=" + utils::to_string(Config::getInstance().getSize(PRP_KICKLEN)))
							  .addParameter(PRP_LINELEN "=" + utils::to_string(Config::getInstance().getSize(PRP_LINELEN)))
							  .addParameter(PRP_MAXTARGETS "=" + utils::to_string(Config::getInstance().getSize(PRP_MAXTARGETS)))
							  .addParameter(PRP_MODES "=" + utils::to_string(Config::getInstance().getSize(PRP_MODES)))
							  .addParameter(PRP_NETWORK "=" + Config::getInstance().getText(PRP_NETWORK))
							  .addParameter(PRP_NICKLEN "=" + utils::to_string(Config::getInstance().getSize(PRP_NICKLEN)))
							  .addParameter(PRP_PREFIX "=" + Config::getInstance().getText(PRP_PREFIX))
							  .addParameter("are supported by this server")
							  .build());
	this->produce(client, Message::Builder()
							  .withPrefix(Config::getInstance().getText(PRP_SERVERNAME))
							  .withCommand(RPL_ISUPPORT)
							  .addParameter(client.getNickname())
							  .addParameter(PRP_TOPICLEN "=" + utils::to_string(Config::getInstance().getSize(PRP_TOPICLEN)))
							  .addParameter(PRP_USERLEN "=" + utils::to_string(Config::getInstance().getSize(PRP_USERLEN)))
							  .addParameter(PRP_USERMODES "=" + Config::getInstance().getText(PRP_USERMODES))
							  .addParameter("are supported by this server")
							  .build());

	return;
}

void irc::Server::challengeRegistration(Client &client)
try
{
	if (!client.isRegistered())
		throw std::runtime_error("std::runtime_error: client must be registered to complete its registration with this server");
	else if (client.getPassword() != this->password)
	{
		Command::reply(ERR_PASSWDMISMATCH, client, CMD_PASS);

		return (this->removeClient(client, "Access denied by configuration"));
	}

	this->produce(RPL_WELCOME, client, "Welcome to the " + Config::getInstance().getText(PRP_NETWORK) + " Network, " + client.str());
	this->produce(RPL_YOURHOST, client, "Your host is " + Config::getInstance().getText(PRP_SERVERNAME) + ", running version " + Config::getInstance().getText(PRP_VERSION));

	strftime(Server::buffer, Config::getInstance().getSize(PRP_LINELEN), "%H:%M:%S %b %d %Y %Z", gmtime(&this->start));
	std::cout << "Debug: " << Server::buffer << std::endl;

	this->produce(RPL_CREATED, client, std::string("This server was created ") + Server::buffer);
	this->produce(client, Message::Builder()
							  .withPrefix(Config::getInstance().getText(PRP_SERVERNAME))
							  .withCommand(RPL_MYINFO)
							  .addParameter(client.getNickname())
							  .addParameter(Config::getInstance().getText(PRP_SERVERNAME))
							  .addParameter(Config::getInstance().getText(PRP_VERSION))
							  .addParameter(Config::getInstance().getText(PRP_USERMODESINFO))
							  .addParameter(Config::getInstance().getText(PRP_CHANMODESINFO))
							  .addParameter(Config::getInstance().getText(PRP_CHANMODESPARAMINFO))
							  .build());
	this->support(client);
	Command::apply(Message::Builder().withCommand(CMD_MOTD).build(), client);

	if (!client.getModes().empty())
		this->produce(client, Message::Builder()
								  .withPrefix(client.str())
								  .withCommand(CMD_MODE)
								  .addParameter(client.getNickname())
								  .addParameter(std::string(1, '+') + utils::sequence_to_string(client.getModes(), ""))
								  .build());

	return;
}
catch (std::exception const &e)
{
	throw std::runtime_error("irc::Server::challengeRegistration: " + std::string(e.what()));
}

void irc::Server::removeClient(Client &client, std::string const &comment)
try
{
	Message message(Message::Builder().withPrefix(client.str()).withCommand(CMD_QUIT).addParameter(comment).build());
	std::map<std::string, Channel, utils::t_istringcomp>::iterator it;

	if (this->clients.find(client.getSocket()) == this->clients.end())
		throw std::out_of_range("std::out_of_range: " + utils::to_string(client.getSocket()) + ": unknown client " + client.str());

	for (std::map<int, Client>::const_iterator cit = this->clients.begin(); cit != this->clients.end(); ++cit)
	{
		if (cit->second == client)
			continue;
		else if (std::find_first_of(client.getChannels().begin(), client.getChannels().end(), cit->second.getChannels().begin(), cit->second.getChannels().end()) != client.getChannels().end())
			this->produce(cit->second, message);
	}

	for (std::set<std::string, utils::t_istringcomp>::const_iterator ccit = client.getChannels().begin(); ccit != client.getChannels().end();)
	{
		it = Server::channels.find(*ccit++);

		if (it != Server::channels.end())
		{
			it->second.removeMember(client);

			if (it->second.getMembers().empty())
				Server::channels.erase(it);
		}
	}

	this->produce(client, Message::Builder()
							  .withCommand(CMD_ERROR)
							  .addParameter("Closing link: (" + client.userId() + ") [" + comment + ']')
							  .build());
	this->write(client);

	if (epoll_ctl(Server::epollfd, EPOLL_CTL_DEL, client.getSocket(), Server::events) == -1)
		throw utils::RuntimeErrno("epoll_ctl (EPOLL_CTL_DEL)");
	else if (close(client.getSocket()) == -1)
		throw utils::RuntimeErrno("close");

	this->outputs.erase(client.getSocket());
	this->inputs.erase(client.getSocket());
	this->overflows.erase(client.getSocket());
	this->clients.erase(client.getSocket());

	return;
}
catch (std::exception const &e)
{
	throw std::runtime_error("irc::Server::removeClient: " + std::string(e.what()));
}

void irc::Server::poll(void)
try
{
	int n, fd;
	std::map<int, Client>::iterator it;

	n = epoll_wait(Server::epollfd, Server::events, Config::getInstance().getSize(PRP_MAXEVENTS), -1);

	if (n == -1)
		throw utils::RuntimeErrno("epoll_wait");

	for (int i = 0; i < n; ++i)
	{
		fd = Server::events[i].data.fd;

		if (fd == Server::sockfd)
			this->addClient();

		it = this->clients.find(fd);

		if (it == this->clients.end())
			continue;

		this->write(it->second);
		this->read(it->second);
	}

	this->flush();

	return;
}
catch (std::exception const &e)
{
	throw std::runtime_error("irc::Server::poll: " + std::string(e.what()));
}

void irc::Server::write(Client const &client)
try
{
	std::map<int, std::queue<Message> >::iterator it = this->outputs.find(client.getSocket());
	std::string buf;

	while (!it->second.empty())
	{
		buf = it->second.front().str();

		if (send(client.getSocket(), buf.c_str(), buf.length(), 0) == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;

			throw utils::RuntimeErrno("send");
		}

#ifndef NDEBUG
		std::cout << "Debug: Server --> Client #" << client.getSocket() << ": " << it->second.front() << std::endl;
#endif

		it->second.pop();
	}

	return;
}
catch (std::exception const &e)
{
	throw std::runtime_error("irc::Server::write: " + std::string(e.what()));
}

void irc::Server::addClient(void)
try
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
		hints.data.fd = accept(Server::sockfd, reinterpret_cast<sockaddr *>(&addr), &addrlen);

		if (hints.data.fd == -1)
			throw utils::RuntimeErrno("accept");
		else if (fcntl(hints.data.fd, F_SETFL, O_NONBLOCK) == -1)
			throw utils::RuntimeErrno("fcntl (F_SETFL)");
		else if (epoll_ctl(Server::epollfd, EPOLL_CTL_ADD, hints.data.fd, &hints) == -1)
			throw utils::RuntimeErrno("epoll_ctl (EPOLL_CTL_ADD)");
		else if (addr.ss_family == AF_INET)
			ip = &reinterpret_cast<sockaddr_in *>(&addr)->sin_addr;
		else
			ip = &reinterpret_cast<sockaddr_in6 *>(&addr)->sin6_addr;

		client = &this->clients.insert(std::make_pair(hints.data.fd, Client(hints.data.fd, inet_ntop(addr.ss_family, ip, buf, INET6_ADDRSTRLEN)))).first->second;

		this->overflows.insert(std::make_pair(client->getSocket(), false));
		this->inputs.insert(std::make_pair(client->getSocket(), ""));
		this->inputs.find(client->getSocket())->second.reserve(Config::getInstance().getSize(PRP_LINELEN));
		this->outputs.insert(std::make_pair(client->getSocket(), std::queue<Message>()));
		this->produce(CMD_NOTICE, *client, "*** Your IP address (" + client->getHostaddr() + ") is used for your netwide unique identifier.");
	}
	catch (std::exception const &)
	{
		this->outputs.erase(hints.data.fd);
		this->inputs.erase(hints.data.fd);
		this->overflows.erase(hints.data.fd);
		this->clients.erase(hints.data.fd);
		epoll_ctl(Server::epollfd, EPOLL_CTL_DEL, hints.data.fd, &hints);
		close(hints.data.fd);

		throw;
	}

	return;
}
catch (std::exception const &e)
{
	throw std::runtime_error("irc::Server::addClient: " + std::string(e.what()));
}

void irc::Server::extractMessage(Client &client, std::size_t const &crlfpos)
try
{
	std::string *buffer, input;
	bool *overflow;

	buffer = &this->inputs.find(client.getSocket())->second;
	overflow = &this->overflows.find(client.getSocket())->second;

	if (crlfpos == std::string::npos)
	{
		if (buffer->length() == Config::getInstance().getSize(PRP_LINELEN))
			*overflow = true;

		if (*overflow)
			buffer->clear();

		return;
	}

	input = buffer->substr(0, crlfpos) + Message::CRLF;

	buffer->erase(0, buffer->find_first_not_of(Message::CRLF, crlfpos));

	if (*overflow)
		Command::reply(ERR_INPUTTOOLONG, client, "(?)");
	else if (0 < crlfpos && crlfpos <= Config::getInstance().getSize(PRP_LINELEN) - Message::CRLF.length())
		client.produce(Message::from(input));

	*overflow = false;

	return (this->extractMessage(client, buffer->find_first_of(Message::CRLF)));
}
catch (std::exception const &e)
{
	throw std::runtime_error("irc::Server::extractMessage: " + std::string(e.what()));
}

bool irc::Server::consumeBuffer(Client &client)
try
{
	std::string *buffer;
	ssize_t incap, nread;

	do
	{
		buffer = &this->inputs.find(client.getSocket())->second;
		incap = Config::getInstance().getSize(PRP_LINELEN) - buffer->length();
		nread = recv(client.getSocket(), Server::buffer, incap, 0);

		if (!nread)
			return (false);
		else if (nread == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;

			throw utils::RuntimeErrno("recv");
		}

		buffer->append(Server::buffer, nread);
		this->extractMessage(client, buffer->find_first_of(Message::CRLF, buffer->length() - nread));
	} while (incap == nread);

	return (true);
}
catch (std::exception const &e)
{
	throw std::runtime_error("irc::Server::consumeBuffer: " + std::string(e.what()));
}

void irc::Server::read(Client &client)
try
{
	int const fd = client.getSocket();

	if (!this->consumeBuffer(client))
	{
		this->removeClient(client, "Client exited");

		return;
	}

	while (this->clients.find(fd) != this->clients.end() && !client.getMessages().empty())
	{
#ifndef NDEBUG
		std::cout << "Debug: Client #" << client.getSocket() << " --> Server: " << client.getMessages().front() << std::endl;
#endif

		try
		{
			Command::apply(client.consume(), client);
		}
		catch (std::exception const &e)
		{
			std::cerr << "Error: " << e.what() << std::endl;
		}
	}

	return;
}
catch (std::exception const &e)
{
	throw std::runtime_error("irc::Server::read: " + std::string(e.what()));
}
