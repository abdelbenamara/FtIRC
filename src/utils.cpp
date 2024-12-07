/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/09 03:17:21 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/07 03:08:04 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

std::vector<std::string> irc::utils::split(std::string const &str,
                                           char const &delim,
                                           std::size_t const &len)
{
    bool const strict(len == std::string::npos);
    std::istringstream in(str);
    std::vector<std::string> tokens;
    std::string buf;

    while (tokens.size() != len && std::getline(in, buf, delim))
        if (strict || !buf.empty())
            tokens.push_back(buf);

    if (!tokens.empty() && std::getline(in, buf))
        tokens.back() += delim + buf;

    if (!strict)
        tokens.resize(len);

    return (tokens);
}

std::string &irc::utils::trim(std::string &str, std::string const &buf)
{
    std::size_t const pos(str.find_first_not_of(buf));

    if (pos != std::string::npos)
        str.erase(str.find_last_not_of(buf) + 1);

    return (str.erase(0, pos));
}

std::string irc::utils::strerrno(std::string const &source)
{
    return (source + ": " + ::strerror(errno));
}

char irc::utils::is_alnum(char const &c)
{
    return (std::isalnum(c, std::locale::classic()));
}

char irc::utils::is_digit(char const &c)
{
    return (std::isdigit(c, std::locale::classic()));
}

char irc::utils::to_upper(char const &c)
{
    return (std::toupper(c, std::locale::classic()));
}

bool irc::utils::i_char_less(char const &lhs, char const &rhs)
{
    return (utils::to_upper(lhs) < utils::to_upper(rhs));
}

bool irc::utils::i_string_less(std::string const &lhs, std::string const &rhs)
{
    return (std::lexicographical_compare(lhs.begin(),
                                         lhs.end(),
                                         rhs.begin(),
                                         rhs.end(),
                                         utils::i_char_less));
}

std::string irc::utils::get_haddr(t_sockinfo const &si)
{
    static char buf[INET6_ADDRSTRLEN];
    void const *addr;

    if (si.second.ss_family == AF_INET6)
        addr = &reinterpret_cast<sockaddr_in6 const *>(&si.second)->sin6_addr;
    else
        addr = &reinterpret_cast<sockaddr_in const *>(&si.second)->sin_addr;

    if (::inet_ntop(si.second.ss_family,
                    addr,
                    buf,
                    INET6_ADDRSTRLEN) == NULL)
        throw std::runtime_error(strerrno("inet_ntop"));

    return (buf);
}

in_port_t irc::utils::get_hport(t_sockinfo const &si)
{
    in_port_t const *port;

    if (si.second.ss_family == AF_INET6)
        port = &reinterpret_cast<sockaddr_in6 const *>(&si.second)->sin6_port;
    else
        port = &reinterpret_cast<sockaddr_in const *>(&si.second)->sin_port;

    return (::ntohs(*port));
}

irc::utils::t_sockinfo irc::utils::connect_socket(
    irc::utils::t_sockinfo const &sockinfo)
{
    addrinfo hints, *info;
    int eai;
    std::string buf;
    irc::utils::t_sockinfo conninfo;

    hints.ai_flags = AI_NUMERICHOST |
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
    eai = ::getaddrinfo(
        irc::utils::get_haddr(sockinfo).c_str(),
        irc::utils::to_string(irc::utils::get_hport(sockinfo)).c_str(),
        &hints,
        &hints.ai_next);

    if (eai)
    {
        buf = ::gai_strerror(eai);

        if (eai == EAI_SYSTEM)
            buf = irc::utils::strerrno(buf);

        throw std::runtime_error("getaddrinfo: " + buf);
    }

    for (info = hints.ai_next; info != NULL; info = info->ai_next)
    {
        conninfo.first = ::socket(info->ai_family,
                                  info->ai_socktype,
                                  info->ai_protocol);

        if (conninfo.first == -1)
            continue;
        else if (!::connect(conninfo.first, info->ai_addr, info->ai_addrlen))
            break;

        ::close(conninfo.first);
    }

    ::freeaddrinfo(hints.ai_next);

    try
    {
        if (info == NULL)
            throw std::runtime_error(irc::utils::strerrno("connect"));
        else if (::getsockname(conninfo.first,
                               reinterpret_cast<sockaddr *>(&conninfo.second),
                               &hints.ai_addrlen) == -1)
            throw std::runtime_error(irc::utils::strerrno("getsockname"));
    }
    catch (std::exception const &)
    {
        ::close(conninfo.first);

        throw;
    }

    return (conninfo);
}
