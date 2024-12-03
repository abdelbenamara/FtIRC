/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/09 03:17:21 by abenamar          #+#    #+#             */
/*   Updated: 2024/11/27 17:22:45 by abenamar         ###   ########.fr       */
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
