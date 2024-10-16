/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejankovs <ejankovs@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/11 20:21:22 by abenamar          #+#    #+#             */
/*   Updated: 2024/10/16 20:35:15 by ejankovs         ###   ########.fr       */
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
#include <map>
//#include "Channel.hpp"
#include "Message.hpp"
#include "RuntimeErrno.hpp"

class Channel;

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
	virtual void identify(std::string const &username);
	std::string const &getNickname(void) const throw();
	void setNickname(std::string const &nickname) throw();
	virtual bool isRegistered(void) throw();
	virtual bool isGone(void) throw();
	virtual void quit(void) throw();
	std::map<int, Channel *const> &getChannels(void) throw();

private:
	int const connfd;
	bool isMessageTooLong, authorized, identified, gone;
	std::string input, nickname, username, host;
	std::queue<Message> messages;
	std::map<std::string, Channel *const> channels;

	Client(void);
	Client(Client const &src);

	Client &operator=(Client const & /* rhs */) throw();

	void cleanupInput(std::size_t const &crlfpos);
	void addMessage(std::size_t const &crlfpos);
};

#endif
