/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/11 20:21:22 by abenamar          #+#    #+#             */
/*   Updated: 2024/10/14 20:42:04 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __CLIENT_HPP__
#define __CLIENT_HPP__

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <queue>
#include <stdexcept>
#include <string>
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
	void removeMessage(void);
	bool isAuthorized(void) const throw();
	void setAuthorized(bool const &isAuthorized) throw();
	virtual bool isIdentified(void) throw();
	virtual void identify(void) throw();
	std::string const &getNickname(void) const throw();
	void setNickname(std::string const &nickname) throw();
	virtual bool isRegistered(void) throw();	
	virtual bool isGone(void) throw();
	virtual void quit(void) throw();

private:
	int const connfd;
	bool isMessageTooLong, authorized, identified, gone;
	std::string input, nickname;
	std::queue<Message> messages;

	Client(void);
	Client(Client const &src);

	Client &operator=(Client const & /* rhs */) throw();

	void addMessage(std::size_t const &crlfpos);
};

#endif
