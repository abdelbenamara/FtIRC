/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/25 22:30:15 by abenamar          #+#    #+#             */
/*   Updated: 2024/11/09 15:38:20 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

irc::Channel::Channel(std::string const &name, Client &first)
try : name(name),
    topic(),
    key(),
    members(),
    operators(&utils::istringcomp),
    modes(),
    limit()
{
    if (this->name.empty())
        throw std::length_error("std::length_error: name must not be empty");
    else if (this->name.length() < 2 || this->name.length() > Config::getInstance().getSize(PRP_CHANNELLEN))
        throw std::length_error("std::length_error: " + utils::to_string(this->name.length()) + ": name must have at least 2 characters and must not have more than " + utils::to_string(Config::getInstance().getSize(PRP_CHANNELLEN)) + " characters");
    else if (Config::getInstance().getText(PRP_CHANTYPES).find(this->name.at(0)) == std::string::npos)
        throw std::domain_error("std::domain_error: " + std::string(1, this->name.at(0)) + ": name prefix must be any supported channel type: " + Config::getInstance().getText(PRP_CHANTYPES));
    else if (this->name.at(1) == ':' || *this->name.rbegin() == ':')
        throw std::invalid_argument("std::invalid_argument: name must neither begin nor end with a colon character");
    else if (this->name.find_first_of("\0\a\r\n ,", 0, 6) != std::string::npos)
        throw std::domain_error("std::domain_error: name must have any character except: NUL, BELL, CR, LF, SPACE, COMMA");

    this->addMember(first);
    this->addOperator(first);

    return;
}
catch (std::exception const &e)
{
    throw std::runtime_error("irc::Channel::Channel: " + std::string(e.what()));
}

irc::Channel::Channel(Channel const &src)
    : name(src.name),
      topic(src.topic),
      key(src.key),
      members(src.members),
      operators(src.operators),
      modes(src.modes),
      limit(src.limit) { return; }

irc::Channel::~Channel() throw() { return; }

std::string const &irc::Channel::getName(void) const throw() { return (this->name); }

std::string const &irc::Channel::getTopic(void) const throw() { return (this->topic); }

std::string const &irc::Channel::getKey(void) const throw() { return (this->key); }

std::set<irc::Client> const &irc::Channel::getMembers(void) const throw() { return (this->members); }

std::set<std::string, irc::utils::t_istringcomp> const &irc::Channel::getOperators(void) const throw() { return (this->operators); }

std::set<char> const &irc::Channel::getModes(void) const throw() { return (this->modes); }

std::size_t const &irc::Channel::getLimit(void) const throw() { return (this->limit); }

std::string irc::Channel::users(void) const
{
    std::set<Client>::const_iterator cit = this->members.begin();
    std::stringstream o;

    if (!this->operators.empty())
        o << "@" << utils::sequence_to_string(this->operators, " @");
    else
        o << cit++->getNickname();

    for (; cit != this->members.end(); ++cit)
        if (this->operators.find(cit->getNickname()) == this->operators.end())
            o << ' ' << cit->getNickname();

    return (o.str());
}

void irc::Channel::publish(Client const &sender, Message const &message) const
try
{
    Message forwarded(Message::Builder()
                          .withPrefix(sender.str())
                          .withCommand(message.getCommand())
                          .withParameters(message.getParameters())
                          .build());

    if (forwarded.str().length() > Config::getInstance().getSize(PRP_LINELEN))
        throw std::length_error("std::length_error: message with sender prefix must not have more than " + utils::to_string(Config::getInstance().getSize(PRP_LINELEN)) + " characters");

    for (std::set<Client>::const_iterator cit = this->members.begin(); cit != this->members.end(); ++cit)
        if (*cit != sender)
            Server::getInstance().produce(*cit, forwarded);

    return;
}
catch (std::exception const &e)
{
    throw std::runtime_error("irc::Channel::publish: " + std::string(e.what()));
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

void irc::Channel::addMember(Client &client)
{
    try
    {
        if (this->modes.find('l') != this->modes.end() && this->members.size() >= this->limit)
            throw std::length_error("std::length_error: a channel must not have more than " + utils::to_string(this->limit) + " members");

        client.joinChannel(*this);
        this->members.insert(client);
    }
    catch (std::exception const &e)
    {
        throw std::runtime_error("irc::Channel::addMember: " + std::string(e.what()));
    }

    return;
}

void irc::Channel::removeMember(Client &client)
{
    std::set<Client>::const_iterator cit = this->members.find(client);

    try
    {
        if (cit == this->members.end())
            throw std::out_of_range("std::out_of_range: " + client.getNickname() + ": client must be a channel member to perform this action");

        if (this->operators.find(client.getNickname()) != this->operators.end())
            this->removeOperator(client);

        client.leaveChannel(*this);
        this->members.erase(cit);
    }
    catch (std::exception const &e)
    {
        throw std::runtime_error("irc::Channel::removeMember: " + std::string(e.what()));
    }

    return;
}

void irc::Channel::addOperator(Client const &client)
{
    if (this->members.find(client) == this->members.end())
        throw std::out_of_range("irc::Channel::addOperator: std::out_of_range: " + client.getNickname() + ": client must be a channel member to obtain channel operator privileges");

    this->operators.insert(client.getNickname());

    return;
}

void irc::Channel::removeOperator(Client const &client)
{
    std::set<std::string, utils::t_istringcomp>::const_iterator cit = this->operators.find(client.getNickname());

    if (cit == this->operators.end())
        throw std::out_of_range("irc::Channel::removeOperator: std::out_of_range: " + client.getNickname() + ": client must be a channel operator to lose its privileges");

    this->operators.erase(cit);

    return;
}

void irc::Channel::addMode(char const &mode)
{
    if (Config::getInstance().getText(PRP_CHANMODESINFO).find(mode) == std::string::npos)
        throw std::domain_error("irc::Channel::addMode: std::domain_error: " + std::string(1, mode) + ": mode must be any supported channel mode: " + Config::getInstance().getText(PRP_CHANMODESINFO));

    this->modes.insert(mode);

    return;
}

void irc::Channel::removeMode(char const &mode)
{
    if (Config::getInstance().getText(PRP_CHANMODESINFO).find(mode) == std::string::npos)
        throw std::domain_error("irc::Channel::removeMode: std::domain_error: " + std::string(1, mode) + ": mode must any supported channel mode: " + Config::getInstance().getText(PRP_CHANMODESINFO));

    this->modes.erase(mode);

    return;
}

void irc::Channel::setLimit(std::size_t const &limit)
{
    if (this->modes.find('l') == this->modes.end())
        throw std::runtime_error("irc::Channel::setLimit: std::runtime_error: channel mode +l must be set to perform this action");

    this->limit = limit;

    return;
}
