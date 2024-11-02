/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/29 16:14:22 by abenamar          #+#    #+#             */
/*   Updated: 2024/10/31 21:35:59 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>

namespace irc
{
    namespace utils
    {
        template <typename T>
        std::string to_string(T const &value);

        template <typename T>
        std::string sequence_to_string(T const &sequence, std::string const &delimiter);

        template <typename T>
        std::set<T> array_to_set(T const *const values, std::size_t const &len);

        template <typename T, typename U>
        std::map<std::string, T> arrays_to_map(char const *const *keys, U const *values, std::size_t const &len);
    } // namespace utils
} // namespace irc

#include "utils.tpp"

#endif
