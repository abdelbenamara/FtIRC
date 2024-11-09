/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/08 16:59:22 by abenamar          #+#    #+#             */
/*   Updated: 2024/11/09 16:19:24 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

std::map<std::string, std::string, irc::utils::t_istringcomp> irc::Config::texts = utils::arrays_to_imap<std::string>(
    (char const *[]){
        PRP_CONFIGFILE,
        PRP_VERSION,
        PRP_CASEMAPPING,
        PRP_CHANTYPES,
        PRP_CHANMODESINFO,
        PRP_CHANMODESPARAMINFO,
        PRP_CHANMODES,
        PRP_PREFIX,
        PRP_USERMODESINFO,
        PRP_USERMODES,
        PRP_SERVERNAME,
        PRP_SERVERPORT,
        PRP_SERVERPASS,
        PRP_OPERNAME,
        PRP_OPERPASS,
        PRP_MOTDFILE,
        PRP_NETWORK},
    (char const *[]){
        SRV_CONFIGFILE,
        SRV_VERSION,
        SRV_CASEMAPPING,
        SRV_CHANTYPES,
        SRV_CHANMODESINFO,
        SRV_CHANMODESPARAMINFO,
        SRV_CHANMODES,
        SRV_PREFIX,
        SRV_USERMODESINFO,
        SRV_USERMODES,
        SRV_SERVERNAME,
        SRV_SERVERPORT,
        SRV_SERVERPASS,
        SRV_OPERNAME,
        SRV_OPERPASS,
        SRV_MOTDFILE,
        SRV_NETWORK},
    17);

std::map<std::string, std::size_t, irc::utils::t_istringcomp> irc::Config::sizes = utils::arrays_to_imap<std::size_t>(
    (char const *[]){
        PRP_MAXEVENTS,
        PRP_MAXCLIENTS,
        PRP_CHANNELLEN,
        PRP_CHANLIMIT,
        PRP_KEYLEN,
        PRP_KICKLEN,
        PRP_LINELEN,
        PRP_MAXTARGETS,
        PRP_MODES,
        PRP_NICKLEN,
        PRP_TOPICLEN,
        PRP_USERLEN},
    (std::size_t[]){
        SRV_MAXEVENTS,
        SRV_MAXCLIENTS,
        SRV_CHANNELLEN,
        SRV_CHANLIMIT,
        SRV_KEYLEN,
        SRV_KICKLEN,
        SRV_LINELEN,
        SRV_MAXTARGETS,
        SRV_MODES,
        SRV_NICKLEN,
        SRV_TOPICLEN,
        SRV_USERLEN},
    12);

irc::Config &irc::Config::getInstance(std::string const &filename)
try
{
    static Config instance(filename);

    return (instance);
}
catch (std::exception const &e)
{
    throw std::runtime_error("irc::Config::getInstance: " + std::string(e.what()));
}

irc::Config::Config(std::string const &filename)
try
{
    std::ifstream ifs(filename.c_str());
    std::string line, key;
    std::size_t pos;

    while (std::getline(ifs, line))
    {
        pos = line.find_first_not_of(' ');

        if (pos == std::string::npos || line.at(pos) == '#')
            continue;

        key = line.substr(pos, line.find('=', pos) - pos);

        if (this->texts.find(key) != this->texts.end())
        {
            pos = line.find('"', pos + key.length());

            if (pos == std::string::npos || pos == line.find_last_of('"'))
                continue;

            this->texts.find(key)->second = line.substr(pos + 1, line.find_last_of('"') - pos - 1);
        }
        else if (this->sizes.find(key) != this->sizes.end())
        {
            pos = line.find('=', pos + key.length());

            if (pos == std::string::npos || !(std::istringstream(line.substr(pos + 1)) >> pos))
                continue;

            this->sizes.find(key)->second = pos;
        }
    }

    ifs.close();

    return;
}
catch (std::exception const &e)
{
    throw std::runtime_error("irc::Config::Config: " + std::string(e.what()));
}

irc::Config::~Config(void) throw() { return; }

std::string irc::Config::getText(std::string const &key) const
try
{
    std::map<std::string, std::string, utils::t_istringcomp>::const_iterator cit = Config::texts.find(key);

    if (cit == Config::texts.end())
        throw std::out_of_range("std::out_of_range: " + key + ": key must be any supported sever configuration text property");

    return (cit->second);
}
catch (std::exception const &e)
{
    throw std::runtime_error("irc::Config::getText: " + std::string(e.what()));
}

std::size_t irc::Config::getSize(std::string const &key) const
try
{
    std::map<std::string, std::size_t, utils::t_istringcomp>::const_iterator cit = Config::sizes.find(key);

    if (cit == Config::sizes.end())
        throw std::out_of_range("std::out_of_range: " + key + ": key must be any supported sever configuration size property");

    return (cit->second);
}
catch (std::exception const &e)
{
    throw std::runtime_error("irc::Config::getSize: " + std::string(e.what()));
}
