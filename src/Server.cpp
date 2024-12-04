/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 12:37:05 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/04 03:31:46 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

irc::Server &irc::Server::instance(std::string const &numericserv,
								   std::string const &password,
								   std::string const &filename)
{
	static Server singleton(numericserv, password, filename);

	return (singleton);
}

irc::Server::Server(std::string const &numericserv,
					std::string const &password,
					std::string const &filename)
	: AServer(numericserv, password, filename),
	  epollfd(::epoll_create1(0)),
	  start(::time(NULL)),
	  events(NULL),
	  buffer(NULL),
	  clients(),
	  hosts(&utils::i_string_less),
	  overflows(),
	  inputs(),
	  outputs(),
	  channels(&utils::i_string_less)
{
	epoll_event hints;

	hints.events = EPOLLIN;
	hints.data.fd = this->getSocket().first;

	try
	{
		if (this->epollfd == -1)
			throw std::runtime_error(utils::strerrno("epoll_create1"));
		else if (::fcntl(hints.data.fd, F_SETFL, O_NONBLOCK) == -1)
			throw std::runtime_error(
				utils::strerrno("fcntl: F_SETFL: O_NONBLOCK"));
		else if (::listen(hints.data.fd, SOMAXCONN) == -1)
			throw std::runtime_error(utils::strerrno("listen"));
		else if (::epoll_ctl(this->epollfd,
							 EPOLL_CTL_ADD,
							 hints.data.fd,
							 &hints) == -1)
			throw std::runtime_error(
				utils::strerrno("epoll_ctl: EPOLL_CTL_ADD)"));
		else if (std::signal(SIGPIPE, SIG_IGN) == SIG_ERR)
			throw std::runtime_error(utils::strerrno("std::signal: SIGPIPE"));

		this->events = new epoll_event[this->getIntegerProperty(PRP_MAXEVENTS)];
		this->buffer = new char[this->getSizeProperty(PRP_LINELEN)];
	}
	catch (std::exception const &)
	{
		delete[] this->buffer;
		delete[] this->events;

		::close(this->epollfd);

		throw;
	}

	return;
}

irc::Server::~Server(void) throw()
{
	Server::t_clients::reverse_iterator rit;

	for (rit = this->clients.rbegin(); rit != this->clients.rend();)
	{
		try
		{
			this->removeClient(rit->second, "Server shutting down");
		}
		catch (std::exception const &e)
		{
			std::cerr << "Error: " << rit->second.str()
					  << ": " << e.what() << std::endl;

			::close(rit->first);
			this->clients.erase(rit->first);
		}
	}

	delete[] this->buffer;
	delete[] this->events;

	::close(this->epollfd);

	return;
}

std::map<int, irc::Client> const &irc::Server::getClients(
	void) const throw() { return (this->clients); }

irc::Server::t_channels const &irc::Server::getChannels(
	void) const throw() { return (this->channels); }

irc::Server::t_clients::iterator irc::Server::getClient(
	std::string const &nick) throw()
{
	Server::t_clients::iterator it;

	for (it = this->clients.begin(); it != this->clients.end(); ++it)
		if (!utils::i_string_less(it->second.getNickname(), nick) &&
			!utils::i_string_less(nick, it->second.getNickname()))
			break;

	return (it);
}

irc::Server::t_clients::iterator irc::Server::getUser(
	std::string const &nick) throw()
{
	Server::t_clients::iterator it(this->getClient(nick));

	if (it->second.isRegistered())
		return (it);

	return (this->clients.end());
}

irc::Server::t_channels::iterator irc::Server::getChannel(
	std::string const &name) throw() { return (this->channels.find(name)); }

irc::Server::t_channels::iterator irc::Server::addChannel(
	std::string const &name)
{
	return (this->channels.insert(std::make_pair(name, Channel(name))).first);
}

void irc::Server::removeChannel(std::string const &name)
{
	Server::t_channels::iterator it(this->channels.find(name));

	if (it == this->channels.end())
		throw std::out_of_range(name + ": unknown channel");

	return (this->channels.erase(it));
}

void irc::Server::produce(Client const &client, Message const &message)
{
	Server::t_outputs::iterator it(
		this->outputs.find(client.getSocket().first));
	std::string buf;

	if (it == this->outputs.end())
		throw std::out_of_range(utils::to_string(client.getSocket().first) +
								", " + client.str() + ": unknown client");

	return (it->second.push(message));
}

void irc::Server::produce(std::string const &command,
						  Client const &client,
						  std::string const &comment)
{
	return (this->produce(client,
						  Message::Builder()
							  .withPrefix(this->getTextProperty(PRP_SERVERNAME))
							  .withCommand(command)
							  .withParameter(client.getNickname())
							  .addParameter(comment)
							  .build()));
}

void irc::Server::produceSupportList(Client const &client)
{
	std::set<std::string> const tokens(this->getSupport());
	std::set<std::string>::const_iterator cit(tokens.begin());
	Message::Builder builder;

	builder
		.withPrefix(this->getTextProperty(PRP_SERVERNAME))
		.withCommand(RPL_ISUPPORT);

	while (cit != tokens.end())
	{
		builder.withParameter(client.getNickname());

		for (int i = 0; cit != tokens.end() && i < 13; ++cit, ++i)
			builder.addParameter(*cit);

		this->produce(client,
					  builder
						  .addParameter("are supported by this server")
						  .build());
	}

	return;
}

void irc::Server::challengeRegistration(Client &client)
{
	Message::Builder builder;

	if (!client.isRegistered())
		throw std::runtime_error(
			"registration must be complete to be challenged");
	else if (client.getPassword() != this->getTextProperty(PRP_SERVERPASS))
	{
		Command::reply(ERR_PASSWDMISMATCH, client, CMD_PASS);

		return (this->removeClient(client, "Access denied by configuration"));
	}

	this->produce(RPL_WELCOME,
				  client,
				  "Welcome to the " + this->getTextProperty(PRP_NETWORK) +
					  " Network, " + client.str());
	this->produce(
		RPL_YOURHOST,
		client,
		"Your host is " + this->getTextProperty(PRP_SERVERNAME) +
			", running version " + this->getTextProperty(PRP_VERSION));
	::strftime(this->buffer,
			   this->getSizeProperty(PRP_LINELEN),
			   "%H:%M:%S %b %d %Y %Z",
			   ::gmtime(&this->start));
	this->produce(RPL_CREATED,
				  client,
				  std::string("This server was created ") + this->buffer);
	this->produce(
		client,
		builder
			.withPrefix(this->getTextProperty(PRP_SERVERNAME))
			.withCommand(RPL_MYINFO)
			.withParameter(client.getNickname())
			.addParameter(this->getTextProperty(PRP_SERVERNAME))
			.addParameter(this->getTextProperty(PRP_VERSION))
			.addParameter(this->getTextProperty(PRP_USERMODESINFO))
			.addParameter(this->getTextProperty(PRP_CHANMODESINFO))
			.addParameter(this->getTextProperty(PRP_CHANMODESPARAMINFO))
			.build());
	this->produceSupportList(client);
	Command::apply(builder
					   .withoutPrefix()
					   .withCommand(CMD_MOTD)
					   .withParameter(this->getTextProperty(PRP_SERVERNAME))
					   .build(),
				   client);

	if (!client.getModes().empty())
		Command::apply(builder
						   .withCommand(CMD_MODE)
						   .withParameter(client.getNickname())
						   .build(),
					   client);

	return;
}

void irc::Server::removeClient(Client &client, std::string const &comment)
{
	Message::Builder builder;
	Server::t_channels::iterator chanit;
	Client::t_channels::const_iterator cit(client.getChannels().begin());
	AServer::t_sizes::iterator hit(
		this->hosts.find(utils::get_haddr(client.getSocket())));

	client.publish(builder
					   .withPrefix(client.str())
					   .withCommand(CMD_QUIT)
					   .withParameter(comment)
					   .build());

	for (; cit != client.getChannels().end();)
	{
		chanit = this->channels.find((*cit)->getName());
		++cit;

		chanit->second.removeMember(client);

		if (chanit->second.getMembers().empty())
			this->channels.erase(chanit);
	}

	this->produce(
		client,
		builder
			.withoutPrefix()
			.withCommand(CMD_ERROR)
			.withParameter(
				"Closing link: (" + client.userId() + ") [" + comment + ']')
			.build());

	try
	{
		this->write(client);
	}
	catch (std::exception const &)
	{
		// Any error (e.g. brken pipe) is irrelevant
		// Note: client socket must be closed anyway
	}

	if (::epoll_ctl(this->epollfd,
					EPOLL_CTL_DEL,
					client.getSocket().first,
					this->events) == -1)
		throw std::runtime_error(utils::strerrno("epoll_ctl: EPOLL_CTL_DEL"));
	else if (::close(client.getSocket().first) == -1)
		throw std::runtime_error(utils::strerrno("close"));

	this->outputs.erase(client.getSocket().first);
	this->inputs.erase(client.getSocket().first);
	this->overflows.erase(client.getSocket().first);

	if (!(--(hit->second)))
		this->hosts.erase(hit);

	this->clients.erase(client.getSocket().first);

	return;
}

void irc::Server::poll(void)
{
	int const nfds(::epoll_wait(this->epollfd,
								this->events,
								this->getIntegerProperty(PRP_MAXEVENTS),
								this->getIntegerProperty(PRP_POLLTIMEOUT)));
	Server::t_clients::iterator it;

	if (nfds == -1)
		throw std::runtime_error(utils::strerrno("epoll_wait"));

	for (int i = 0; i < nfds; ++i)
	{
		if (this->events[i].data.fd == this->getSocket().first)
		{
			this->addClient();

			continue;
		}

		it = this->clients.find(this->events[i].data.fd);

		if (it == this->clients.end())
			continue;

		try
		{
			this->read(it->second);
		}
		catch (std::exception const &e)
		{
			std::cerr << "Error: " << it->second.str()
					  << ": " << e.what() << std::endl;

			this->removeClient(it->second, "Connection reset");
		}

		for (it = this->clients.begin(); it != this->clients.end();)
		{
			try
			{
				this->write((it++)->second);
			}
			catch (std::exception const &e)
			{
				std::cerr << "Error: " << (--it)->second.str()
						  << ": " << e.what() << std::endl;

				this->removeClient((it++)->second, "Connection lost");
			}
		}
	}

	return;
}

void irc::Server::addClient(void)
{
	socklen_t addrlen(sizeof(sockaddr_storage));
	sockaddr_storage addr;
	epoll_event hints;
	utils::t_sockinfo si;
	AServer::t_sizes::iterator hit;

	try
	{
		hints.events = EPOLLIN | EPOLLOUT | EPOLLET;
		hints.data.fd = ::accept(this->getSocket().first,
								 reinterpret_cast<sockaddr *>(&addr),
								 &addrlen);

		if (hints.data.fd == -1)
			throw std::runtime_error(utils::strerrno("accept"));
		else if (::fcntl(hints.data.fd, F_SETFL, O_NONBLOCK) == -1)
			throw std::runtime_error(
				utils::strerrno("fcntl: F_SETFL: O_NONBLOCK"));
		else if (::epoll_ctl(this->epollfd,
							 EPOLL_CTL_ADD,
							 hints.data.fd,
							 &hints) == -1)
			throw std::runtime_error(
				utils::strerrno("epoll_ctl: EPOLL_CTL_ADD"));
	}
	catch (std::exception const &)
	{
		::epoll_ctl(this->epollfd, EPOLL_CTL_DEL, hints.data.fd, &hints);
		::close(hints.data.fd);

		throw;
	}

	si.first = hints.data.fd;
	si.second = addr;
	hit = this->hosts.insert(std::make_pair(utils::get_haddr(si), 0)).first;

	this->clients.insert(std::make_pair(si.first, Client(si)));
	this->overflows.insert(std::make_pair(si.first, false));
	this->inputs.insert(std::make_pair(si.first, std::string()));
	this->inputs.find(si.first)
		->second.reserve(this->getSizeProperty(PRP_LINELEN));
	this->outputs.insert(std::make_pair(si.first, std::queue<Message>()));

	if (this->hosts.size() > this->getSizeProperty(PRP_GLOBALMAX) ||
		++(hit->second) > this->getSizeProperty(PRP_LOCALMAX))
		return (this->removeClient(
			this->clients.find(si.first)->second,
			"No more connections allowed from your host"));

	return (this->produce(CMD_NOTICE,
						  this->clients.find(si.first)->second,
						  "*** Your IP address (" + hit->first +
							  ") is used for your netwide unique identifier"));
}

void irc::Server::addMessage(Client &client, std::size_t const &crlfpos)
{
	static std::size_t const maxpos(
		this->getSizeProperty(PRP_LINELEN) - Message::CRLF.length());
	std::string *input(&this->inputs.find(client.getSocket().first)->second);
	bool *overflow(&this->overflows.find(client.getSocket().first)->second);

	if (crlfpos == std::string::npos)
	{
		if (input->length() == this->getSizeProperty(PRP_LINELEN))
			*overflow = true;

		if (*overflow)
			input->clear();

		return;
	}

	if (*overflow || crlfpos > maxpos)
		this->produce(ERR_INPUTTOOLONG, client, "Input line was too long");
	else if (crlfpos != input->find_first_not_of(' '))
		client.produce(
			Message::from(input->substr(0, crlfpos) + Message::CRLF));

	*overflow = false;

	return (this->addMessage(
		client,
		input->erase(0, input->find_first_not_of(Message::CRLF, crlfpos))
			.find_first_of(Message::CRLF)));
}

bool irc::Server::consume(Client &client)
{
	std::string *input(&this->inputs.find(client.getSocket().first)->second);
	ssize_t incap, nread;

	do
	{
		incap = this->getSizeProperty(PRP_LINELEN) - input->length();
		nread = ::recv(client.getSocket().first, this->buffer, incap, 0);

		if (!nread)
			return (false);
		else if (nread == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;

			throw std::runtime_error(utils::strerrno("recv"));
		}

		input->append(this->buffer, nread);
		this->addMessage(
			client,
			input->find_first_of(Message::CRLF, input->length() - nread));
	} while (incap == nread);

	return (true);
}

void irc::Server::read(Client &client)
{
	int const connfd(client.getSocket().first);

	if (!this->consume(client))
		return (this->removeClient(client, "Client exited"));

	while (this->clients.find(connfd) != this->clients.end() &&
		   !client.getMessages().empty())
	{
#ifndef NDEBUG
		std::cout << "Debug: #" << connfd << " --> "
				  << client.getMessages().front() << std::endl;
#endif

		try
		{
			Command::apply(client.consume(), client);
		}
		catch (std::exception const &e)
		{
			std::cerr << "Error: " << client.str()
					  << ": " << e.what() << std::endl;
		}
	}

	return;
}

void irc::Server::write(Client &client)
{
	Server::t_outputs::iterator it(
		this->outputs.find(client.getSocket().first));
	std::string buf;

	while (!it->second.empty())
	{
		buf = it->second.front().str();

		if (::send(it->first, buf.c_str(), buf.length(), 0) == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;

			throw std::runtime_error(utils::strerrno("send"));
		}

#ifndef NDEBUG
		std::cout << "Debug: #" << it->first << " <-- "
				  << it->second.front() << std::endl;
#endif

		it->second.pop();
	}

	return;
}
