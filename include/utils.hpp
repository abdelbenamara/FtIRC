/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/29 16:14:22 by abenamar          #+#    #+#             */
/*   Updated: 2024/11/09 15:32:57 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <locale>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>

namespace irc
{
    namespace utils
    {
        class RuntimeErrno : public std::runtime_error
        {
        public:
            explicit RuntimeErrno(std::string const &source);
        };

        typedef bool (*t_istringcomp)(std::string const &, std::string const &);

        bool icharcomp(char const &lhs, char const &rhs);
        bool istringcomp(std::string const &lhs, std::string const &rhs);

        template <typename T>
        std::string to_string(T const &value);

        template <typename T>
        std::string sequence_to_string(T const &sequence, std::string const &delimiter);

        template <typename T, typename U>
        std::map<std::string, T, t_istringcomp> arrays_to_imap(char const *const *keys, U const *values, std::size_t const &len);
    } // namespace utils
} // namespace irc

#include "utils.tpp"

#endif
