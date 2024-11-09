/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/07 02:18:22 by abenamar          #+#    #+#             */
/*   Updated: 2024/11/09 15:54:54 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

#include <cstddef>
#include <fstream>
#include <map>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>

#include "utils.hpp"

/* ************************************************************************** */
/*                                                                            */
/*                                    core                                    */
/*                                                                            */
/* ************************************************************************** */

#define PRP_CONFIGFILE "CONFIGFILE"
#define PRP_MAXEVENTS "MAXEVENTS"
#define PRP_VERSION "VERSION"

#define SRV_CONFIGFILE "./etc/ircserv.conf"
#define SRV_MAXEVENTS ((std::size_t)16)
#define SRV_VERSION "ft_irc-7.1"

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

#define PRP_MAXCLIENTS "MAXCLIENTS"
#define PRP_SERVERNAME "SERVERNAME"
#define PRP_SERVERPORT "SERVERPORT"
#define PRP_SERVERPASS "SERVERPASS"
#define PRP_OPERNAME "OPERNAME"
#define PRP_OPERPASS "OPERPASS"
#define PRP_MOTDFILE "MOTDFILE"

#define SRV_MAXCLIENTS ((std::size_t)10)
#define SRV_SERVERNAME "ft.irc.local"
#define SRV_SERVERPORT "6667"
#define SRV_SERVERPASS ""
#define SRV_OPERNAME "root"
#define SRV_OPERPASS "12345"
#define SRV_MOTDFILE "./etc/ircserv.motd"

/* ****************************** support list ****************************** */

#define PRP_CHANNELLEN "CHANNELLEN"
#define PRP_CHANLIMIT "CHANLIMIT"
#define PRP_KEYLEN "KEYLEN"
#define PRP_KICKLEN "KICKLEN"
#define PRP_LINELEN "LINELEN"
#define PRP_MAXTARGETS "MAXTARGETS"
#define PRP_MODES "MODES"
#define PRP_NETWORK "NETWORK"
#define PRP_NICKLEN "NICKLEN"
#define PRP_TOPICLEN "TOPICLEN"
#define PRP_USERLEN "USERLEN"

#define SRV_CHANNELLEN ((std::size_t)50)
#define SRV_CHANLIMIT ((std::size_t)25)
#define SRV_KEYLEN ((std::size_t)32)
#define SRV_KICKLEN ((std::size_t)255)
#define SRV_LINELEN ((std::size_t)512)
#define SRV_MAXTARGETS ((std::size_t)20)
#define SRV_MODES ((std::size_t)4)
#define SRV_NETWORK "I-KEA"
#define SRV_NICKLEN ((std::size_t)9)
#define SRV_TOPICLEN ((std::size_t)307)
#define SRV_USERLEN ((std::size_t)18)

namespace irc
{
    class Config
    {
    public:
        static Config &getInstance(std::string const &filename = SRV_CONFIGFILE);

        virtual ~Config(void) throw();

        std::string getText(std::string const &key) const;
        std::size_t getSize(std::string const &key) const;

    private:
        static std::map<std::string, std::string, utils::t_istringcomp> texts;
        static std::map<std::string, std::size_t, utils::t_istringcomp> sizes;

        Config(std::string const &filename);

        Config(void);                      /* = delete (C++11) */
        Config(Config const &);            /* = delete (C++11) */
        Config &operator=(Config const &); /* = delete (C++11) */
    };
} // namespace irc

#endif
