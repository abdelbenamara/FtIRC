/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 12:33:05 by abenamar          #+#    #+#             */
/*   Updated: 2024/11/02 14:39:17 by abenamar         ###   ########.fr       */
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

#include "Channel.hpp"
#include "Client.hpp"
#include "Command.hpp"
#include "Message.hpp"
#include "RuntimeErrno.hpp"

#define SRV_NAME "ft.irc.local"
#define SRV_MAX_CLIENTS ((std::size_t)SOMAXCONN)
#define SRV_MAX_CHANNELS ((std::size_t)20)
#define SRV_MAX_EVENTS ((std::size_t)16)
#define SRV_OPER_NAME "root"
#define SRV_OPER_PASSWORD "12345"

namespace irc
{
	class Channel;
	class Client;

	class Server
	{
	public:
		static std::size_t const MAX_CLIENTS, MAX_CHANNELS;

		static Server &getInstance(std::string const &numericserv = "6667", std::string const &password = "");
		static void produce(Client const &client, Message const &message);
		static void produce(Client const &client, std::string const &command, std::string const &comment);

		virtual ~Server(void) throw();

		in_port_t const &getPort(void) const throw();
		std::map<int, Client> const &getClients(void) const throw();

		std::map<std::string, Channel> &getChannels(void) throw();
		void challengeRegistration(Client const &client);
		void removeClient(Client const &client, std::string const &comment);
		void poll(void);

	private:
		static std::size_t const PASS_MAX_LEN, MAX_EVENTS;

		static epoll_event events[];
		static char buffer[];
		static int epollfd, sockfd;

		static int initPort(std::string const &numericserv);

		in_port_t const port;
		std::string const password;

		std::map<int, Client> clients;
		std::map<int, std::string> buffers;
		std::map<int, bool> overflows;
		std::map<std::string, Channel> channels;

		Server(std::string const &numericserv, std::string const &password);

		Server(void);					   /* = delete (C++11) */
		Server(Server const &);			   /* = delete (C++11) */
		Server &operator=(Server const &); /* = delete (C++11) */

		void addClient(void);
		void extractMessage(Client &client, std::size_t const &crlfpos);
		bool consumeBuffer(Client &client);
	};
} // namespace irc

#endif
