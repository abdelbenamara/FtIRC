/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 12:33:05 by abenamar          #+#    #+#             */
/*   Updated: 2024/11/09 15:22:17 by abenamar         ###   ########.fr       */
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
#include <csignal>
#include <ctime>
#include <iostream>
#include <limits>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "Channel.hpp"
#include "Client.hpp"
#include "Command.hpp"
#include "Config.hpp"
#include "Message.hpp"
#include "utils.hpp"

namespace irc
{
	class Channel;
	class Client;

	class Server
	{
	public:
		static std::map<std::string, Channel, utils::t_istringcomp> channels;

		static Server &getInstance(std::string const &numericserv = SRV_SERVERPORT, std::string const &password = SRV_SERVERPASS);

		virtual ~Server(void) throw();

		in_port_t const &getPort(void) const throw();
		std::map<int, Client> const &getClients(void) const throw();

		void flush(void);
		void produce(Client const &client, Message const &message);
		void produce(std::string const &command, Client const &client, std::string const &comment);
		void support(Client const &client);
		void challengeRegistration(Client &client);
		void removeClient(Client &client, std::string const &comment);
		void poll(void);

	private:
		static int epollfd, sockfd;
		static epoll_event *events;
		static char *buffer;

		static int initPort(std::string const &numericserv);

		time_t const start;
		in_port_t const port;
		std::string const password;

		std::map<int, Client> clients;
		std::map<int, bool> overflows;
		std::map<int, std::string> inputs;
		std::map<int, std::queue<Message> > outputs;

		Server(std::string const &numericserv, std::string const &password);

		Server(void);					   /* = delete (C++11) */
		Server(Server const &);			   /* = delete (C++11) */
		Server &operator=(Server const &); /* = delete (C++11) */

		void write(Client const &client);
		void addClient(void);
		void extractMessage(Client &client, std::size_t const &crlfpos);
		bool consumeBuffer(Client &client);
		void read(Client &client);
	};
} // namespace irc

#endif
