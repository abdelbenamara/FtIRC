/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 12:33:05 by abenamar          #+#    #+#             */
/*   Updated: 2024/08/08 02:24:57 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __SERV_HPP__
#define __SERV_HPP__

#include <cerrno>
#include <cstring>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "ServerUtils.hpp"

class Server
{
public:
	Server(char const *const numericserv, char const *const password, int const maxevents, int const bufsize) throw(std::invalid_argument, std::runtime_error);
	virtual ~Server(void) throw();

	in_port_t port(void) const throw(std::runtime_error);
	void run(void) throw(std::runtime_error);

private:
	int const sockfd, epfd, maxevents, bufsize;
	char const *const password;
	epoll_event *const events;
	char *const msgbuf;

	std::map<int, std::string> messages;

	Server(void) throw();
	Server(Server const & /* src */) throw();

	Server &operator=(Server const & /* rhs */) throw();
};

#endif
