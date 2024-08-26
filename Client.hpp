/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/11 20:21:22 by abenamar          #+#    #+#             */
/*   Updated: 2024/08/26 12:35:53 by abenamar         ###   ########.fr       */
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

	bool appendInput(char *const buffer);
	bool hasMessage(void) const throw();
	Message const &message(void);
	void dropMessage(void) throw();

private:
	int const connfd;
	bool isMessageTooLong;
	std::string input, *password;
	std::queue<Message> messages;

	Client(void);
	Client(Client const & /* src */);

	Client &operator=(Client const & /* rhs */) throw();

	void extractMessage(std::size_t const crlfpos);
};

#endif
