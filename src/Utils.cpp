/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/29 16:19:14 by abenamar          #+#    #+#             */
/*   Updated: 2024/10/29 17:54:30 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils.hpp"

namespace irc
{
    std::string utils::charset_to_string(std::set<char> sequence, std::string const &delimiter)
    {
        std::set<char>::const_iterator cit = sequence.begin();
        std::ostringstream o;

        if (sequence.empty())
            return ("");

        o << *cit++;

        for (; cit != sequence.end(); ++cit)
            o << delimiter << *cit;

        return (o.str());
    }
} // namespace irc
