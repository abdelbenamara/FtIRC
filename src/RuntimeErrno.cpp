/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RuntimeErrno.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/19 15:10:40 by abenamar          #+#    #+#             */
/*   Updated: 2024/10/31 16:54:20 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RuntimeErrno.hpp"

irc::RuntimeErrno::RuntimeErrno(std::string const &source) : std::runtime_error("std::runtime_error: " + source + ": " + strerror(errno)) { return; }

irc::RuntimeErrno::RuntimeErrno(std::string const &parent, std::string const &source) : std::runtime_error(parent + ": std::runtime_error: " + source + ": " + strerror(errno)) { return; }
