/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RuntimeErrno.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/19 15:14:28 by abenamar          #+#    #+#             */
/*   Updated: 2024/10/29 17:56:06 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __RUNTIMEERRNO_HPP__
#define __RUNTIMEERRNO_HPP__

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>

namespace irc
{
    class RuntimeErrno : public std::runtime_error
    {
    public:
        explicit RuntimeErrno(std::string const &source);
        explicit RuntimeErrno(std::string const &parent, std::string const &source);
    };
} // namespace irc

#endif
