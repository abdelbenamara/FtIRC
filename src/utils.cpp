/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/09 03:17:21 by abenamar          #+#    #+#             */
/*   Updated: 2024/11/09 14:05:28 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

irc::utils::RuntimeErrno::RuntimeErrno(std::string const &source) : std::runtime_error("std::runtime_error: " + source + ": " + strerror(errno)) { return; }

bool irc::utils::icharcomp(char const &lhs, char const &rhs) { return (std::toupper(lhs, std::locale::classic()) < std::toupper(rhs, std::locale::classic())); }

bool irc::utils::istringcomp(std::string const &lhs, std::string const &rhs) { return (std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), utils::icharcomp)); }
