/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/25 22:30:15 by abenamar          #+#    #+#             */
/*   Updated: 2024/11/02 14:54:09 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

std::size_t const irc::Channel::NANE_MAX_LEN = CHAN_NAME_LEN, irc::Channel::KEY_MAX_LEN = CHAN_KEY_LEN;

std::set<char> const
    irc::Channel::TYPES = utils::array_to_set(
        (char const[]){
            CHAN_TYPE_LOCAL,
            CHAN_TYPE_STANDARD},
        2),
    irc::Channel::MODES = utils::array_to_set(
        (char const[]){
            CHAN_MODE_I,
            CHAN_MODE_T,
            CHAN_MODE_K,
            CHAN_MODE_O,
            CHAN_MODE_L},
        5);

irc::Channel::Channel(std::string const &name)
try : name(name),
    exclusive(false),
    topic(),
    key(),
    members(),
    operators(),
    limit(Server::MAX_CLIENTS)
{
    if (this->name.empty())
        throw std::length_error("std::length_error: name must not be empty");
    else if (this->name.length() < 2 || this->name.length() > irc::Channel::NANE_MAX_LEN)
        throw std::length_error("std::length_error: " + utils::to_string(this->name.length()) + ": name must have at least 2 characters and must not have more than " + utils::to_string(irc::Channel::NANE_MAX_LEN) + " characters");
    else if (std::string(irc::Channel::TYPES.begin(), irc::Channel::TYPES.end()).find(this->name.at(0)) == std::string::npos)
        throw std::domain_error("std::domain_error: " + std::string(1, this->name.at(0)) + ": name prefix must be any supported channel type: " + utils::sequence_to_string(irc::Channel::TYPES, ", "));
    else if (this->name.at(1) == ':' || *this->name.rbegin() == ':')
        throw std::invalid_argument("std::invalid_argument: name must neither begin nor end with a colon character");
    else if (this->name.find_first_of("\0\a\r\n ,", 0, 6) != std::string::npos)
        throw std::domain_error("std::domain_error: name must have any character except: NUL, BELL, CR, LF, SPACE, COMMA");

    return;
}
catch (std::exception const &e)
{
    throw std::runtime_error("irc::Channel::Channel: " + std::string(e.what()));
}

irc::Channel::Channel(Channel const &src)
    : name(src.name),
      exclusive(src.exclusive),
      topic(src.topic),
      key(src.key),
      members(src.members),
      operators(src.operators),
      limit(src.limit) { return; }

irc::Channel::~Channel() throw() { return; }

std::string const &irc::Channel::getName(void) const throw() { return (this->name); }

bool const &irc::Channel::isExclusive(void) const throw() { return (this->exclusive); }

std::string const &irc::Channel::getTopic(void) const throw() { return (this->topic); }

std::string const &irc::Channel::getKey(void) const throw() { return (this->key); }

std::map<std::string, irc::Client> const &irc::Channel::getMembers(void) const throw() { return (this->members); }

std::set<std::string> const &irc::Channel::getOperators(void) const throw() { return (this->operators); }

void irc::Channel::setExclusive(bool const &exclusive)
{
    this->exclusive = exclusive;

    return;
}

void irc::Channel::setTopic(std::string const &topic)
{
    this->topic = topic;

    return;
}

void irc::Channel::setKey(std::string const &key)
{
    this->key = key;

    return;
}

void irc::Channel::addMember(Client const &client)
{
    if (!client.isRegistered())
        throw std::runtime_error("irc::Channel::addMember: std::runtime_error: client must be registered to perform this action");

    this->members.insert(std::make_pair(client.getNickname(), client));

    return;
}

void irc::Channel::removeMember(Client const &client)
{
    try
    {
        if (!client.isRegistered())
            throw std::runtime_error("std::runtime_error: client must be registered to perform this action");
        else if (this->members.find(client.getNickname()) == this->members.end())
            throw std::out_of_range("std::out_of_range: " + client.getNickname() + ": client must be a channel member to perform this action");
    }
    catch (std::exception const &e)
    {
        throw std::runtime_error("irc::Channel::removeMember: " + std::string(e.what()));
    }

    return;
}

void irc::Channel::addOperator(Client const &client)
{
    if (this->members.find(client.getNickname()) == this->members.end())
        throw std::out_of_range("irc::Channel::addOperator: std::out_of_range: " + client.getNickname() + ": client must be a channel member to obtain channel operator privileges");

    this->operators.insert(client.getNickname());

    return;
}

void irc::Channel::removeOperator(Client const &client)
{
    if (this->operators.find(client.getNickname()) == this->operators.end())
        throw std::out_of_range("irc::Channel::removeOperator: std::out_of_range: " + client.getNickname() + ": client must be a channel operator to lose its privileges");

    this->operators.erase(client.getNickname());

    return;
}
