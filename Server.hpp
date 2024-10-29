/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 12:33:05 by abenamar          #+#    #+#             */
/*   Updated: 2024/10/29 11:30:41 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
#include <iostream>
#include <limits>
#include <map>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include "Client.hpp"
#include "Command.hpp"
#include "Message.hpp"
#include "RuntimeErrno.hpp"

#define SRV_MAX_EVENTS 16
#define SRV_NAME "ft_irc"
#define SRV_OPER_NAME "root"
#define SRV_OPER_PASSWORD "12345"

class Server
{
public:
	static Server &getInstance(std::string const &numericserv = "6667", std::string const &password = "");
	static void produce(Client const &client, Message const &message);
	static void produce(Client const &client, std::string const &command, std::string const &comment);

	virtual ~Server(void) throw();

	in_port_t const &getPort(void) const throw();
	std::map<int, Client> const &getClients(void) const throw();

	void completeRegistration(Client const &client);
	void poll(void);
	void removeClient(Client const &client);

private:
	static unsigned int const MAX_EVENTS;
	static std::size_t const PASS_MAX_LEN;

	static epoll_event EVENTS[];
	static char BUFFER[];
	static int sockfd;

	static int initServerPort(std::string const &numericserv);

	int const epollfd;
	in_port_t const port;
	std::string const password;

	std::map<int, Client> clients;
	std::map<int, std::string> buffers;
	std::map<int, bool> overflows;

	Server(std::string const &numericserv, std::string const &password);

	Server(void);					   /* = delete (C++11) */
	Server(Server const &);			   /* = delete (C++11) */
	Server &operator=(Server const &); /* = delete (C++11) */

	void addClient(void);
	void parseMessage(Client &client, std::size_t const &crlfpos);
	bool consumeBuffer(Client &client);
};

#endif
