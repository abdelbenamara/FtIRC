/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.tpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/30 12:00:20 by abenamar          #+#    #+#             */
/*   Updated: 2024/11/23 00:53:29 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __UTILS_TPP__
#define __UTILS_TPP__

#include "utils.hpp"

template <typename T>
std::string irc::utils::to_string(T const &value)
{
    std::ostringstream out;

    return (reinterpret_cast<std::stringbuf *>((out << value).rdbuf())->str());
}

template <typename T>
std::string irc::utils::to_string(T first, T last, char const *delim)
{
    std::ostringstream out;

    if (first == last)
        return (std::string());

    std::copy(first,
              --last,
              std::ostream_iterator<typename T::value_type>(out, delim));

    out << *last;

    return (out.str());
}

template <typename T>
std::string irc::utils::to_string(T const &seq, char const *delim)
{
    return (utils::to_string(seq.begin(), seq.end(), delim));
}

template <typename T, typename U>
typename irc::utils::s_istringmap<T>::type irc::utils::to_istringmap(
    char const *const *const keys,
    U const *const values,
    std::size_t const &len)
{
    typename s_istringmap<T>::type map(&utils::i_string_less);

    for (std::size_t i = 0; i < len; ++i)
        map.insert(std::make_pair(keys[i], values[i]));

    return (map);
}

#endif
