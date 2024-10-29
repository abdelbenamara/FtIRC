/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/25 22:30:15 by abenamar          #+#    #+#             */
/*   Updated: 2024/10/29 17:52:52 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

namespace irc
{
    unsigned int const Channel::USER_LIMT = std::min(CHAN_USER_LIMIT, Server::MAX_CLIENTS);

    std::size_t const Channel::NANE_MAX_LEN = 50;

    std::set<char> const Channel::TYPES = Channel::initTypes(), Channel::MODES = Channel::initModes();

    std::set<char> Channel::initTypes(void)
    {
        std::set<char> modes;

        modes.insert(CHAN_TYPE_LOCAL);
        modes.insert(CHAN_TYPE_STANDARD);
        modes.insert(CHAN_TYPE_NO_MODES);

        return (modes);
    }

    std::set<char> Channel::initModes(void)
    {
        std::set<char> modes;

        modes.insert(CHAN_MODE_I);
        modes.insert(CHAN_MODE_T);
        modes.insert(CHAN_MODE_K);
        modes.insert(CHAN_MODE_O);
        modes.insert(CHAN_MODE_L);

        return (modes);
    }

    Channel::Channel(std::string const &name)
    try : name(name)
    {
        std::ostringstream err;

        if (this->name.empty())
            throw std::length_error("std::length_error: name must not be empty");
        else if (this->name.length() > Channel::NANE_MAX_LEN)
            throw std::length_error(reinterpret_cast<std::ostringstream &>(err << "std::length_error: " << this->name.length() << ": name must not have more than " << Client::NICK_MAX_LEN << " characters").str());
        else if (std::string(Channel::TYPES.begin(), Channel::TYPES.end()).find(this->name.at(0)) == std::string::npos)
            throw std::domain_error(reinterpret_cast<std::ostringstream &>(err << "std::domain_error: " << this->name.at(0) << ":name prefix must be any supported channel type: " << utils::charset_to_string(Channel::TYPES, ", ")).str());
        else if (this->name.find_first_of("\0\a\r\n ,:", 0, 7) != std::string::npos)
            throw std::domain_error("std::domain_error: name must have any character except: NUL, BELL, CR, LF, SPACE, COMMA, :");

        return;
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error("Channel::Channel: " + std::string(e.what()));
    }

    Channel::Channel(Channel const &src) : name(src.name) {}

    Channel::~Channel() throw() {}
} // namespace irc
