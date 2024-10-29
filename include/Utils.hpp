/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/29 16:14:22 by abenamar          #+#    #+#             */
/*   Updated: 2024/10/29 17:54:34 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <set>
#include <sstream>
#include <string>

namespace irc
{
    namespace utils
    {
        std::string charset_to_string(std::set<char> sequence, std::string const &delimiter);
    } // namespace utils
} // namespace utils

#endif
