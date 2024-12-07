/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/29 16:14:22 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/07 03:07:39 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <iterator>
#include <locale>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace irc
{
    namespace utils
    {
        typedef std::pair<int, sockaddr_storage> t_sockinfo;
        typedef bool (*t_string_comp)(std::string const &lhs,
                                      std::string const &rhs);

        std::vector<std::string> split(
            std::string const &str,
            char const &delim,
            std::size_t const &len = std::string::npos);

        std::string &trim(std::string &str, std::string const &buf);

        std::string strerrno(std::string const &source);

        char is_alnum(char const &c);
        char is_digit(char const &c);
        char to_upper(char const &c);

        bool i_char_less(char const &lhs, char const &rhs);
        bool i_string_less(std::string const &lhs, std::string const &rhs);

        std::string get_haddr(t_sockinfo const &si);
        in_port_t get_hport(t_sockinfo const &si);
        t_sockinfo connect_socket(utils::t_sockinfo const &sockinfo);

        template <typename T>
        std::string to_string(T const &value);

        template <typename T>
        std::string to_string(T first, T last, char const *delim);

        template <typename T>
        std::string to_string(T const &seq, char const *delim);

        template <typename T>
        struct s_istringmap
        {
            typedef std::map<std::string, T, t_string_comp> type;
        };

        template <typename T, typename U>
        typename s_istringmap<T>::type to_istringmap(
            char const *const *const keys,
            U const *const values,
            std::size_t const &len);
    } // namespace utils
} // namespace irc

#include "utils.tpp"

#endif
