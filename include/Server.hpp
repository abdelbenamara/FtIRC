/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 12:33:05 by abenamar          #+#    #+#             */
/*   Updated: 2024/11/28 13:10:06 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <csignal>
#include <ctime>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "AServer.hpp"
#include "Channel.hpp"
#include "Client.hpp"
#include "Command.hpp"
#include "Message.hpp"
#include "utils.hpp"

namespace irc
{
	class Channel;
	class Client;
	class Message;

	class Server : public AServer
	{
	public:
		typedef std::map<int, Client> t_clients;
		typedef utils::s_istringmap<Channel>::type t_channels;

		static Server &instance(std::string const &numericserv = SRV_SERVERPORT,
								std::string const &password = SRV_SERVERPASS,
								std::string const &filename = SRV_CONFIGFILE);

		virtual ~Server(void) throw();

		t_clients const &getClients(void) const throw();
		t_channels const &getChannels(void) const throw();

		t_clients::iterator getClient(std::string const &nick) throw();
		t_clients::iterator getUser(std::string const &nick) throw();
		t_channels::iterator getChannel(std::string const &name) throw();

		t_channels::iterator addChannel(std::string const &name);
		void removeChannel(std::string const &name);
		void produce(Client const &client, Message const &message);
		void produce(std::string const &command,
					 Client const &client,
					 std::string const &comment);
		void produceSupportList(Client const &client);
		void challengeRegistration(Client &client);
		void removeClient(Client &client, std::string const &comment);
		void poll(void);

	private:
		typedef std::map<int, std::queue<Message> > t_outputs;

		int const epollfd;
		time_t const start;

		epoll_event *events;
		char *buffer;
		t_clients clients;
		AServer::t_sizes hosts;
		std::map<int, bool> overflows;
		std::map<int, std::string> inputs;
		t_outputs outputs;
		t_channels channels;

		Server(std::string const &numericserv,
			   std::string const &password,
			   std::string const &filename);

		Server(void);					   /* = delete (C++11) */
		Server(Server const &);			   /* = delete (C++11) */
		Server &operator=(Server const &); /* = delete (C++11) */

		void addClient(void);
		void addMessage(Client &client, std::size_t const &crlfpos);
		bool consume(Client &client);
		void read(Client &client);
		void write(Client &client);
	};
} // namespace irc

#endif
