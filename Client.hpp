/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/11 20:21:22 by abenamar          #+#    #+#             */
/*   Updated: 2024/09/21 21:43:30 by abenamar         ###   ########.fr       */
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
	void removeMessage(void) throw();
	bool isAuthorized(void) const throw();
	void setAuthorized(bool const isAuthorized) throw();

private:
	int const connfd;
	bool isMessageTooLong, authorized;
	std::string input, nickname, username;
	std::queue<Message> messages;

	Client(void);
	Client(Client const & /* src */);

	Client &operator=(Client const & /* rhs */) throw();

	void addMessage(std::size_t const crlfpos);
};

#endif
