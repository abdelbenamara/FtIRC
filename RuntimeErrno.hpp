/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RuntimeErrno.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/19 15:14:28 by abenamar          #+#    #+#             */
/*   Updated: 2024/10/24 23:18:31 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __RUNTIMEERRNO_HPP__
#define __RUNTIMEERRNO_HPP__

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>

class RuntimeErrno : public std::runtime_error
{
public:
    explicit RuntimeErrno(std::string const &source);
    explicit RuntimeErrno(std::string const &parent, std::string const &source);
};

#endif
