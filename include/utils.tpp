/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.tpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/30 12:00:20 by abenamar          #+#    #+#             */
/*   Updated: 2024/10/31 21:36:08 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

template <typename T>
std::string irc::utils::to_string(T const &value) { return (reinterpret_cast<std::stringbuf *>((std::ostringstream() << value).rdbuf())->str()); }

template <typename T>
std::string irc::utils::sequence_to_string(T const &sequence, std::string const &delimiter)
{
    typename T::const_iterator cit = sequence.begin();
    std::ostringstream o;

    if (!sequence.empty())
    {
        o << *cit++;

        for (; cit != sequence.end(); ++cit)
            o << delimiter << *cit;
    }

    return (o.str());
}

template <typename T>
std::set<T> irc::utils::array_to_set(T const *const values, std::size_t const &len) { return std::set<T>(values, values + len); }

template <typename T, typename U>
std::map<std::string, T> irc::utils::arrays_to_map(char const *const *keys, U const *values, std::size_t const &len)
{
    std::map<std::string, T> map;

    for (std::size_t i = 0; i < len; ++i)
        map.insert(std::make_pair(keys[i], values[i]));

    return (map);
}
