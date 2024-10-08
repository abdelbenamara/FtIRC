/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejankovs <ejankovs@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/11 20:21:22 by abenamar          #+#    #+#             */
/*   Updated: 2024/10/08 20:09:51 by ejankovs         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __CLIENT_HPP__
#define __CLIENT_HPP__

#include <queue>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "Message.hpp"
#include "RuntimeErrno.hpp"

class Client
{
public:
	Client(int const connfd);
	virtual ~Client(void) throw();

	int getSocket(void) const throw();
	bool updateInput(void);
	bool hasMessage(void) const throw();
	Message const &message(void);
	void removeMessage(void) throw(std::runtime_error);
	bool isAuthorized(void) const throw();
	void setAuthorized(bool const isAuthorized) throw();

	void setNickname(std::string nick) throw();
	std::string getNickname() throw();
	void setUsername(std::string username) throw();
    void setMode(std::string mode) throw();
    void setRealname(std::string realname) throw();
	bool isRegistered() throw();
    void setRegistered(bool state) throw();

private:
	int const connfd;
	bool isMessageTooLong, authorized;
	std::string input, nickname, username, realname, mode;
	std::queue<Message> messages;
	bool isRegister;
	

	Client(void);
	Client(Client const & /* src */);

	Client &operator=(Client const & /* rhs */) throw();

	void addMessage(std::size_t const crlfpos);
};

#endif
