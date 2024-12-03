/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AServer.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/07 02:18:22 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/01 15:31:52 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __ASERVER_HPP__
#define __ASERVER_HPP__

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "utils.hpp"

/* ************************************************************************** */
/*                                                                            */
/*                                  settings                                  */
/*                                                                            */
/* ************************************************************************** */

#define PRP_CONFIGFILE "CONFIGFILE"
#define PRP_VERSION "VERSION"

#define SRV_CONFIGFILE "ircserv.conf"
#define SRV_VERSION "ft-irc-7.1"

/* ****************************** support list ****************************** */

#define PRP_CASEMAPPING "CASEMAPPING"
#define PRP_CHANTYPES "CHANTYPES"
#define PRP_CHANMODESINFO "CHANMODESINFO"
#define PRP_CHANMODESPARAMINFO "CHANMODESPARAMINFO"
#define PRP_CHANMODES "CHANMODES"
#define PRP_PREFIX "PREFIX"
#define PRP_USERMODESINFO "USERMODESINFO"
#define PRP_USERMODES "USERMODES"

#define SRV_CASEMAPPING "ascii"
#define SRV_CHANTYPES "#&"
#define SRV_CHANMODESINFO "iklot"
#define SRV_CHANMODESPARAMINFO "klo"
#define SRV_CHANMODES ",k,l,it"
#define SRV_PREFIX "(o)@"
#define SRV_USERMODESINFO "iow"
#define SRV_USERMODES ",,,iow"

/* ************************************************************************** */
/*                                                                            */
/*                                 properties                                 */
/*                                                                            */
/* ************************************************************************** */

#define PRP_LOCALMAX "LOCALMAX"
#define PRP_GLOBALMAX "GLOBALMAX"
#define PRP_SERVERNAME "SERVERNAME"
#define PRP_SERVERHOST "SERVERHOST"
#define PRP_SERVERPORT "SERVERPORT"
#define PRP_SERVERPASS "SERVERPASS"
#define PRP_MAXEVENTS "MAXEVENTS"
#define PRP_POLLTIMEOUT "POLLTIMEOUT"
#define PRP_OPERNAME "OPERNAME"
#define PRP_OPERPASS "OPERPASS"
#define PRP_MOTDFILE "MOTDFILE"
#define PRP_MOTDLINELEN "MOTDLINELEN"

#define SRV_LOCALMAX (static_cast<std::size_t>(10))
#define SRV_GLOBALMAX (static_cast<std::size_t>(4096))
#define SRV_SERVERNAME "irc.local"
#define SRV_SERVERHOST "0.0.0.0"
#define SRV_SERVERPORT "6667"
#define SRV_SERVERPASS ""
#define SRV_MAXEVENTS (static_cast<int>(16))
#define SRV_POLLTIMEOUT (static_cast<int>(-1))
#define SRV_OPERNAME "root"
#define SRV_OPERPASS "12345"
#define SRV_MOTDFILE "ircserv.motd"
#define SRV_MOTDLINELEN (static_cast<std::size_t>(80))

/* ****************************** support list ****************************** */

#define PRP_CHANLIMIT "CHANLIMIT"
#define PRP_CHANNELLEN "CHANNELLEN"
#define PRP_HOSTLEN "HOSTLEN"
#define PRP_KEYLEN "KEYLEN"
#define PRP_KICKLEN "KICKLEN"
#define PRP_LINELEN "LINELEN"
#define PRP_MAXTARGETS "MAXTARGETS"
#define PRP_MODES "MODES"
#define PRP_NETWORK "NETWORK"
#define PRP_NICKLEN "NICKLEN"
#define PRP_TOPICLEN "TOPICLEN"
#define PRP_USERLEN "USERLEN"

#define SRV_CHANLIMIT (static_cast<std::size_t>(25))
#define SRV_CHANNELLEN (static_cast<std::size_t>(50))
#define SRV_HOSTLEN (static_cast<std::size_t>(63))
#define SRV_KEYLEN (static_cast<std::size_t>(32))
#define SRV_KICKLEN (static_cast<std::size_t>(255))
#define SRV_LINELEN (static_cast<std::size_t>(512))
#define SRV_MAXTARGETS (static_cast<std::size_t>(20))
#define SRV_MODES (static_cast<std::size_t>(4))
#define SRV_NETWORK "Local"
#define SRV_NICKLEN (static_cast<std::size_t>(9))
#define SRV_TOPICLEN (static_cast<std::size_t>(307))
#define SRV_USERLEN (static_cast<std::size_t>(12))

namespace irc
{
    class AServer
    {
    public:
        virtual ~AServer(void) throw();

        utils::t_sockinfo const &getSocket(void) const throw();

        std::string getTextProperty(std::string const &key) const;
        std::size_t getSizeProperty(std::string const &key) const;
        int getIntegerProperty(std::string const &key) const;
        std::set<std::string> getProperties(void) const;
        std::set<std::string> getSupport(void) const;

        virtual void poll(void) = 0;

    protected:
        typedef utils::s_istringmap<std::size_t>::type t_sizes;
        
        AServer(std::string const &numericserv,
                std::string const &password,
                std::string const &filename);

    private:
        typedef utils::s_istringmap<std::string>::type t_texts;
        typedef utils::s_istringmap<int>::type t_integers;

        static t_texts const SETTINGS;
        static t_texts const DEFAULT_TEXTS;
        static t_sizes const DEFAULT_SIZES;
        static t_integers const DEFAULT_INTEGERS;

        static bool is_not_host(char const &c);

        utils::t_sockinfo sockinfo;
        t_texts texts;
        t_sizes sizes;
        t_integers integers;

        AServer(void);                       /* = delete (C++11) */
        AServer(AServer const &);            /* = delete (C++11) */
        AServer &operator=(AServer const &); /* = delete (C++11) */

        std::string getProperty(std::string const &key) const;

        void setProperty(std::string const &key, std::string const &value);
        void hotfixProperties(void);
        void bindSocket(void);
    };
} // namespace irc

#endif
