/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejankovs <ejankovs@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 12:33:05 by abenamar          #+#    #+#             */
/*   Updated: 2024/10/08 19:18:23 by ejankovs         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <unistd.h>
#include "Client.hpp"
#include "Message.hpp"
#include "RuntimeErrno.hpp"

class Server
{
public:
	class Builder
	{
	public:
		Builder(void);
		virtual ~Builder(void) throw();

		Builder &withNumericServ(std::string const &numericserv);
		Builder &withPassword(std::string const &password);
		Server *build(void);

	private:
		int epollfd, sockfd;
		std::string numericserv, password;

		Builder(Builder const & /* src */);

		Builder &operator=(Builder const & /* rhs */) throw();

		static std::string const gai_strerror(int errcode);

		void clear(void) throw();
	};

	virtual ~Server(void) throw();

	in_port_t port(void) const;
	int waitForEvents(void);
	int getEventSocket(int const &pos) const;
	int getSocket(void) const throw();
	std::string const &getPassword(void) const throw();
	Client *const &getClient(int const &connfd);
	void addClient(void);
	void removeClient(int const &connfd);
	std::map<int, Client *const>::const_iterator getClientsBegin(void) const throw();
	std::map<int, Client *const>::const_iterator getClientsEnd(void) const throw();

	bool isNicknameInUse(std::string nick) const throw();

private:
	static int const MAXEVENTS;

	int const epollfd, sockfd;
	std::string const password;
	epoll_event *const events;

	int nfds;
	std::map<int, Client *const> clients;

	Server(void);
	Server(int const epollfd, int const sockfd, std::string const &password);
	Server(Server const & /* src */);

	Server &operator=(Server const & /* rhs */) throw();
};

#endif
