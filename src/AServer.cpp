/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AServer.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/19 20:46:50 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/01 15:31:52 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "AServer.hpp"

irc::AServer::t_texts const
    irc::AServer::SETTINGS = utils::to_istringmap<std::string>(
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
            PRP_USERMODES},
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
            SRV_USERMODES},
        10);

irc::AServer::t_texts const
    irc::AServer::DEFAULT_TEXTS = utils::to_istringmap<std::string>(
        (char const *[]){
            PRP_SERVERNAME,
            PRP_SERVERHOST,
            PRP_SERVERPORT,
            PRP_SERVERPASS,
            PRP_OPERNAME,
            PRP_OPERPASS,
            PRP_MOTDFILE,
            PRP_NETWORK},
        (char const *[]){
            SRV_SERVERNAME,
            SRV_SERVERHOST,
            SRV_SERVERPORT,
            SRV_SERVERPASS,
            SRV_OPERNAME,
            SRV_OPERPASS,
            SRV_MOTDFILE,
            SRV_NETWORK},
        8);

irc::AServer::t_sizes const
    irc::AServer::DEFAULT_SIZES = utils::to_istringmap<std::size_t>(
        (char const *[]){
            PRP_LOCALMAX,
            PRP_GLOBALMAX,
            PRP_MOTDLINELEN,
            PRP_CHANLIMIT,
            PRP_CHANNELLEN,
            PRP_HOSTLEN,
            PRP_KEYLEN,
            PRP_KICKLEN,
            PRP_LINELEN,
            PRP_MAXTARGETS,
            PRP_MODES,
            PRP_NICKLEN,
            PRP_TOPICLEN,
            PRP_USERLEN},
        (std::size_t[]){
            SRV_LOCALMAX,
            SRV_GLOBALMAX,
            SRV_MOTDLINELEN,
            SRV_CHANLIMIT,
            SRV_CHANNELLEN,
            SRV_HOSTLEN,
            SRV_KEYLEN,
            SRV_KICKLEN,
            SRV_LINELEN,
            SRV_MAXTARGETS,
            SRV_MODES,
            SRV_NICKLEN,
            SRV_TOPICLEN,
            SRV_USERLEN},
        14);

irc::AServer::t_integers const
    irc::AServer::DEFAULT_INTEGERS = utils::to_istringmap<int>(
        (char const *[]){
            PRP_MAXEVENTS,
            PRP_POLLTIMEOUT},
        (int[]){
            SRV_MAXEVENTS,
            SRV_POLLTIMEOUT},
        2);

bool irc::AServer::is_not_host(char const &c)
{
    return (c != '.' && c != '-' && !utils::is_alnum(c));
}

irc::AServer::AServer(std::string const &numericserv,
                      std::string const &password,
                      std::string const &filename)
    : sockinfo(-1, sockaddr_storage()),
      texts(AServer::DEFAULT_TEXTS),
      sizes(AServer::DEFAULT_SIZES),
      integers(AServer::DEFAULT_INTEGERS)
{
    std::ifstream ifs(filename.c_str());
    std::size_t lineno(0);
    std::string buf;
    std::vector<std::string> tokens;

    this->texts.find(PRP_SERVERPORT)->second = numericserv;
    this->texts.find(PRP_SERVERPASS)->second = password;

    try
    {
        if (!ifs.is_open())
            throw std::runtime_error(filename + ": " + utils::strerrno("open"));

        while (std::getline(ifs, buf))
        {
            ++lineno;

            buf.erase(std::find(buf.begin(), buf.end(), '#'), buf.end());

            tokens = utils::split(buf, '=', 2);
            buf = "\t ";

            if (tokens.at(0).empty())
                continue;
            else if (!tokens.at(1).rfind(':', 0))
                tokens.at(1).erase(tokens.at(1).begin());
            else
                utils::trim(tokens.at(1), buf);

            try
            {
                this->setProperty(utils::trim(tokens.at(0), buf), tokens.at(1));
            }
            catch (std::exception const &e)
            {
                std::cerr << "Error: " << filename
                          << ": line " << utils::to_string(lineno)
                          << ": " << e.what() << std::endl;
            }
        }

        ifs.close();
    }
    catch (std::exception const &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    if (this->texts.find(PRP_SERVERPASS)
            ->second.find_first_of("\0\r\n", 0, 3) != std::string::npos)
        throw std::invalid_argument(
            "server password must have any character except: NUL, CR, LF");

    this->texts.insert(AServer::SETTINGS.begin(), AServer::SETTINGS.end());
    this->hotfixProperties();
    this->bindSocket();

    this->texts.find(PRP_SERVERHOST)->second = utils::get_haddr(this->sockinfo);
    this->texts.find(PRP_SERVERPORT)
        ->second = utils::to_string(utils::get_hport(this->sockinfo));
    this->texts.find(PRP_CONFIGFILE)->second = filename;

    return;
}

irc::AServer::~AServer(void) throw()
{
    ::close(this->sockinfo.first);

    return;
}

irc::utils::t_sockinfo const &irc::AServer::getSocket(void) const throw()
{
    return (this->sockinfo);
}

std::string irc::AServer::getTextProperty(std::string const &key) const
{
    t_texts::const_iterator cit(this->texts.find(key));

    if (cit == this->texts.end())
        throw std::invalid_argument(key + ": no such word property");

    return (cit->second);
}

std::size_t irc::AServer::getSizeProperty(std::string const &key) const
{
    t_sizes::const_iterator cit(this->sizes.find(key));

    if (cit == this->sizes.end())
        throw std::invalid_argument(key + ": no such size property");

    return (cit->second);
}

int irc::AServer::getIntegerProperty(std::string const &key) const
{
    t_integers::const_iterator cit(this->integers.find(key));

    if (cit == this->integers.end())
        throw std::invalid_argument(key + ": no such integer property");

    return (cit->second);
}

std::set<std::string> irc::AServer::getProperties(void) const
{
    std::set<std::string> properties;
    std::set<std::string>::const_iterator pcit(properties.end());
    t_texts::const_iterator wcit(this->texts.begin());
    t_sizes::const_iterator scit(this->sizes.begin());
    t_integers::const_iterator icit(this->integers.begin());

    for (; wcit != this->texts.end(); ++wcit)
        pcit = properties.insert(pcit, wcit->first + '=' + wcit->second);

    for (; scit != this->sizes.end(); ++scit)
        pcit = properties.insert(
            pcit,
            scit->first + '=' + utils::to_string(scit->second));

    for (; icit != this->integers.end(); ++icit)
        pcit = properties.insert(
            pcit,
            icit->first + '=' + utils::to_string(icit->second));

    return (properties);
}

std::set<std::string> irc::AServer::getSupport(void) const
{
    std::set<std::string> tokens;

    tokens.insert(this->getProperty(PRP_CASEMAPPING));
    tokens.insert(PRP_CHANLIMIT "=" +
                  this->getTextProperty(PRP_CHANTYPES) + ':' +
                  utils::to_string(this->getSizeProperty(PRP_CHANLIMIT)));
    tokens.insert(this->getProperty(PRP_CHANMODES));
    tokens.insert(this->getProperty(PRP_CHANNELLEN));
    tokens.insert(this->getProperty(PRP_CHANTYPES));
    tokens.insert(this->getProperty(PRP_HOSTLEN));
    tokens.insert(this->getProperty(PRP_KEYLEN));
    tokens.insert(this->getProperty(PRP_KICKLEN));
    tokens.insert(this->getProperty(PRP_LINELEN));
    tokens.insert(this->getProperty(PRP_MAXTARGETS));
    tokens.insert(this->getProperty(PRP_MODES));
    tokens.insert(this->getProperty(PRP_NETWORK));
    tokens.insert(this->getProperty(PRP_NICKLEN));
    tokens.insert(this->getProperty(PRP_PREFIX));
    tokens.insert(this->getProperty(PRP_TOPICLEN));
    tokens.insert(this->getProperty(PRP_USERLEN));
    tokens.insert(this->getProperty(PRP_USERMODES));

    return (tokens);
}

std::string irc::AServer::getProperty(std::string const &key) const
{
    t_texts::const_iterator wcit(this->texts.find(key));
    t_sizes::const_iterator scit;
    t_integers::const_iterator icit;
    std::string value;

    if (wcit == this->texts.end())
    {
        scit = this->sizes.find(key);

        if (scit == this->sizes.end())
        {
            icit = this->integers.find(key);

            if (icit == this->integers.end())
                throw std::invalid_argument(key + ": no such property");
            else
                value = utils::to_string(icit->second);
        }
        else
            value = utils::to_string(scit->second);
    }
    else
        value = wcit->second;

    return (key + '=' + value);
}

void irc::AServer::setProperty(std::string const &key, std::string const &value)
{
    std::istringstream in(value);
    t_texts::iterator wit(this->texts.find(key));
    t_sizes::iterator sit;
    t_integers::iterator iit;

    if (wit == this->texts.end())
    {
        sit = this->sizes.find(key);

        if (sit == this->sizes.end())
        {
            iit = this->integers.find(key);

            if (iit == this->integers.end())
                throw std::invalid_argument(key + ": unknown property");
            else
                in >> iit->second;
        }
        else
            in >> sit->second;
    }
    else
        std::getline(in, wit->second);

    return;
}

void irc::AServer::hotfixProperties(void)
{
    std::string const &host(this->texts.find(PRP_SERVERNAME)->second);

    // HOSTLEN less than 1 would allow invalid empty hostname
    this->sizes.find(PRP_HOSTLEN)->second = std::max<std::size_t>(
        1,
        this->sizes.find(PRP_HOSTLEN)->second);

    // SERVERNAME must be a valid hostname according to RFC 1123
    if (host.empty() || host.length() > this->sizes.find(PRP_HOSTLEN)->second ||
        host.at(0) == '.' || host.at(0) == '-' ||
        *host.rbegin() == '.' || *host.rbegin() == '-' ||
        std::find_if(host.begin(),
                     host.end(),
                     AServer::is_not_host) != host.end())
        throw std::invalid_argument(host + ": invalid hostname");

    // MAXEVENTS less than 1 would cause a segmentation fault
    this->integers.find(PRP_MAXEVENTS)->second = std::max(
        1,
        this->integers.find(PRP_MAXEVENTS)->second);

    // POLLTIMEOUT less than -1 would cause undefined behavior
    this->integers.find(PRP_POLLTIMEOUT)->second = std::max(
        -1,
        this->integers.find(PRP_POLLTIMEOUT)->second);

    // LINELEN must not be less than 512 for RFCs 1459 and 2812 compatibility
    this->sizes.find(PRP_LINELEN)->second = std::max<std::size_t>(
        512,
        this->sizes.find(PRP_LINELEN)->second);

    // SERVERPASS length greater than LINELEN - 8 is silently truncated to fit
    // Note: 8 is the length of PASS command, SPACE, trailing COLON and CRLF
    this->texts.find(PRP_SERVERPASS)
        ->second.erase(std::min(
            this->texts.find(PRP_SERVERPASS)->second.length(),
            this->sizes.find(PRP_LINELEN)->second - 8));

    // NETWORK spaces are silently replaced with the \x20 hexadecimal value
    try
    {
        while (true)
            this->texts.find(PRP_NETWORK)
                ->second.replace(this->texts.find(PRP_NETWORK)
                                     ->second.find(' '),
                                 1,
                                 "\\x20");
    }
    catch (std::out_of_range const &)
    {
        // No more spaces in NETWORK
    }

    return;
}

void irc::AServer::bindSocket(void)
{
    char const *host(this->texts.find(PRP_SERVERHOST)->second.c_str());
    std::string buf(this->texts.find(PRP_SERVERPORT)->second);
    addrinfo hints, *info;
    int opt;

    hints.ai_flags = AI_PASSIVE |
                     AI_NUMERICHOST |
                     AI_NUMERICSERV |
                     AI_V4MAPPED |
                     AI_ADDRCONFIG;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_addrlen = sizeof(sockaddr_storage);
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;
    hints.ai_next = NULL;
    opt = ::getaddrinfo(host, buf.c_str(), &hints, &hints.ai_next);

    if (opt)
    {
        buf = ::gai_strerror(opt);

        if (opt == EAI_SYSTEM)
            buf = utils::strerrno(buf);

        throw std::runtime_error("getaddrinfo: " + buf);
    }

    for (opt = 1, info = hints.ai_next; info != NULL; info = info->ai_next)
    {
        this->sockinfo.first = ::socket(info->ai_family,
                                        info->ai_socktype,
                                        info->ai_protocol);

        if (this->sockinfo.first == -1)
            continue;
        else if (!::setsockopt(this->sockinfo.first,
                               SOL_SOCKET,
                               SO_REUSEADDR,
                               &opt,
                               sizeof(int)) &&
                 !::bind(this->sockinfo.first, info->ai_addr, info->ai_addrlen))
            break;

        ::close(this->sockinfo.first);
    }

    ::freeaddrinfo(hints.ai_next);

    try
    {
        if (info == NULL)
            throw std::runtime_error(utils::strerrno("bind"));
        else if (::getsockname(
                     this->sockinfo.first,
                     reinterpret_cast<sockaddr *>(&this->sockinfo.second),
                     &hints.ai_addrlen) == -1)
            throw std::runtime_error(utils::strerrno("getsockname"));
    }
    catch (std::exception const &)
    {
        ::close(this->sockinfo.first);

        throw;
    }

    return;
}
