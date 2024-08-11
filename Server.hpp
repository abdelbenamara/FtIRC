/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 12:33:05 by abenamar          #+#    #+#             */
/*   Updated: 2024/08/11 14:31:38 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __SERV_HPP__
#define __SERV_HPP__

#include <cerrno>
#include <cstring>
#include <istream>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

class Server
{
public:
	class Builder
	{
	public:
		Builder(void) throw();
		virtual ~Builder(void) throw();

		Builder &withNumericServ(char const *const numericserv) throw();
		Builder &withPassword(char const *const password) throw();
		Builder &withMaxEvents(int const maxevents) throw();
		Builder &withBufSize(int const bufsize) throw();
		Server *build(void) throw(std::invalid_argument, std::runtime_error);

	private:
		int epfd, sockfd, maxevents, bufsize;
		char const *numericserv, *password;
		epoll_event *events;
		char *msgbuf;

		Builder(Builder const & /* src */) throw();

		Builder &operator=(Builder const & /* rhs */) throw();

		static char const *gai_strerror(int errcode) throw();
		static int initSocket(char const *const numericserv) throw(std::invalid_argument, std::runtime_error);

		template <typename T>
		static T *initArray(int const size, char const *const type) throw(std::invalid_argument, std::runtime_error)
		{
			std::ostringstream err;
			T *tmp;

			if (size < 1)
			{
				err << "Server::Builder::build: Server::Builder::initArray: std::invalid_argument: `" << size << "': " << type << " parameter must be a strictly positive integer";

				throw std::invalid_argument(err.str());
			}

			try
			{
				tmp = new T[size + 1];
			}
			catch (std::exception const &e)
			{
				err << "Server::Builder::build: Server::Builder::initArray: std::runtime_error: std::bad_alloc: `" << size << "': try with a smaller integer for " << type << " parameter";

				throw std::runtime_error(err.str());
			}

			return (tmp);
		}

		void clear(void) throw();
	};

	virtual ~Server(void) throw();

	in_port_t port(void) const throw(std::runtime_error);
	void run(void) throw(std::runtime_error);

private:
	int const epfd, sockfd, maxevents, bufsize;
	char const *const password;
	epoll_event *const events;
	char *const msgbuf;

	std::map<int, std::string> messages;

	Server(void) throw();
	Server(int const epfd, int const sockfd, int const maxevents, int const bufsize, char const *const password, epoll_event *const events, char *const msgbuf) throw();
	Server(Server const & /* src */) throw();

	Server &operator=(Server const & /* rhs */) throw();
};

#endif
