/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejankovs <ejankovs@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/21 21:05:45 by abenamar          #+#    #+#             */
/*   Updated: 2024/10/09 20:16:38 by ejankovs         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __COMMAND_HPP__
#define __COMMAND_HPP__

#include <sys/socket.h>
#include <sys/types.h>
#include <map>
#include <string>
#include "Client.hpp"
#include "Message.hpp"
#include "RuntimeErrno.hpp"
#include "Server.hpp"
#include <iostream>

class Command
{
public:
    virtual ~Command(void) throw();

    static std::map<std::string, void (*)(Client &, Server const &)> const APPLY;

private:
    Command(void);
    Command(Command const & /* src */);

    Command &operator=(Command const & /* rhs */) throw();

    static std::map<std::string, void (*)(Client &, Server const &)> createApplyMap(void);
    static void pass(Client &client, Server const &server);
    static void nick(Client &client, Server const &server);
    static void user(Client &client, Server const &server);

	// static void quit(Client &client, Server const &server);
	// static void join(Client &client, Server const &server);
	// static void mode(Client &client, Server const &server);
	// static void topic(Client &client, Server const &server);
	// static void invite(Client &client, Server const &server);
	// static void kick(Client &client, Server const &server);
	// static void privmsg(Client &client, Server const &server);
};

#endif
