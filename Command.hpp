/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejankovs <ejankovs@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/21 21:05:45 by abenamar          #+#    #+#             */
/*   Updated: 2024/10/14 19:42:36 by ejankovs         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __COMMAND_HPP__
#define __COMMAND_HPP__

#include <sys/socket.h>
#include <sys/types.h>
#include <algorithm>
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

    static std::map<std::string, void (*)(Client &, Server &)> const APPLY;

private:
    Command(void);
    Command(Command const & /* src */);

    Command &operator=(Command const & /* rhs */) throw();

    static std::map<std::string, void (*)(Client &, Server &)> createApplyMap(void);
    static void pass(Client &client, Server &server);
    static void nick(Client &client, Server &server);
    static void user(Client &client, Server &server);
    static void quit(Client &client, Server &server);
    static void privmsg(Client &client, Server &server);
    // static void join(Client &client, Server &server);
    // static void mode(Client &client, Server &server);
    // static void topic(Client &client, Server &server);
    // static void invite(Client &client, Server &server);
    // static void kick(Client &client, Server &server);
};

#endif
