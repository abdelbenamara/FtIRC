/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.tpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/30 12:00:20 by abenamar          #+#    #+#             */
/*   Updated: 2024/11/09 15:33:08 by abenamar         ###   ########.fr       */
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

template <typename T, typename U>
std::map<std::string, T, irc::utils::t_istringcomp> irc::utils::arrays_to_imap(char const *const *keys, U const *values, std::size_t const &len)
{
    std::map<std::string, T, t_istringcomp> map(&utils::istringcomp);

    for (std::size_t i = 0; i < len; ++i)
        map.insert(std::make_pair(keys[i], values[i]));

    return (map);
}
