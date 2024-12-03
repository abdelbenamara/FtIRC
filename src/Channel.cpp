/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/25 22:30:15 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/02 21:46:22 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

bool irc::Channel::is_not_key(char const &c)
{
    return (c == '\0' || std::isspace(c, std::locale::classic()));
}

bool irc::Channel::client_ptr_less(Client const *const &lhs,
                                   Client const *const &rhs)
{
    return (utils::i_string_less(lhs->getNickname(), rhs->getNickname()));
}

irc::Channel::Channel(std::string const &name)
    : start(::time(NULL)),
      name(name),
      members(&Channel::client_ptr_less),
      operators(&Channel::client_ptr_less),
      topic(),
      key(),
      topicwhotime(NULL, 0),
      modes(),
      limit()
{
    if (this->name.empty())
        throw std::length_error("channel name must not be empty");
    else if (this->name.length() < 2 ||
             this->name.length() >
                 Server::instance().getSizeProperty(PRP_CHANNELLEN))
        throw std::length_error(
            utils::to_string(this->name.length()) +
            ": channel name must have at least 2 characters"
            " and must not have more than " +
            utils::to_string(
                Server::instance().getSizeProperty(PRP_CHANNELLEN)) +
            " characters");
    else if (Server::instance()
                 .getTextProperty(PRP_CHANTYPES)
                 .find(this->name.at(0)) == std::string::npos)
        throw std::domain_error(
            std::string(1, this->name.at(0)) +
            ": channel name prefix must be any supported channel type: " +
            Server::instance().getTextProperty(PRP_CHANTYPES));
    else if (this->name.at(1) == ':' || *this->name.rbegin() == ':')
        throw std::invalid_argument(
            "channel name must neither begin nor end with a colon character");
    else if (this->name.find_first_of("\0\a\r\n ,", 0, 6) != std::string::npos)
        throw std::domain_error("channel name must have any character except"
                                ": NUL, BELL, CR, LF, SPACE, COMMA");

    return;
}

irc::Channel::Channel(Channel const &src)
    : start(src.start),
      name(src.name),
      members(src.members),
      operators(src.operators),
      topic(src.topic),
      key(src.key),
      topicwhotime(src.topicwhotime),
      modes(src.modes),
      limit(src.limit) { return; }

irc::Channel::~Channel() throw() { return; }

time_t const &irc::Channel::getStart(
    void) const throw() { return (this->start); }

std::string const &irc::Channel::getName(
    void) const throw() { return (this->name); }

irc::Channel::t_clients const &irc::Channel::getMembers(
    void) const throw() { return (this->members); }

irc::Channel::t_clients const &irc::Channel::getOperators(
    void) const throw() { return (this->operators); }

std::string const &irc::Channel::getTopic(
    void) const throw() { return (this->topic); }

std::string const &irc::Channel::getKey(
    void) const throw() { return (this->key); }

irc::Channel::t_topicwhotime const &irc::Channel::getTopicWhoTime(
    void) const throw() { return (this->topicwhotime); }

std::set<char> const &irc::Channel::getModes(
    void) const throw() { return (this->modes); }

std::size_t const &irc::Channel::getLimit(
    void) const throw() { return (this->limit); }

std::string irc::Channel::namesList(Client const &target) const
{
    bool const isMember(this->members.find(&target) != this->members.end());
    Channel::t_clients::const_iterator cit(this->members.begin());
    std::stringstream out;

    for (; cit != this->members.end(); ++cit)
        if (isMember || (*cit)->getModes().find(USR_MODE_i) ==
                            (*cit)->getModes().end())
            out << Channel::prefixedName(**cit) << ' ';

    return (out.str());
}

void irc::Channel::publish(Message const &message) const
{
    Channel::t_clients::const_iterator cit;

    for (cit = this->members.begin(); cit != this->members.end(); ++cit)
        Server::instance().produce(**cit, message);

    return;
}

void irc::Channel::addMember(Client &client)
{
    if (this->modes.find('l') != this->modes.end() &&
        this->members.size() >= this->limit)
        throw std::length_error("channel must not have more than " +
                                utils::to_string(this->limit) + " members");

    client.joinChannel(*this);

    if (this->members.empty())
        this->operators.insert(&client);

    this->members.insert(&client);

    return;
}

void irc::Channel::removeMember(Client &client)
{
    Channel::t_clients::const_iterator cit(this->members.find(&client));

    if (cit == this->members.end())
        throw std::out_of_range(
            client.getNickname() +
            ": client must be a channel member to be removed");

    if (this->operators.find(&client) != this->operators.end())
        this->removeOperator(client);

    client.leaveChannel(*this);

    return (this->members.erase(cit));
}

void irc::Channel::addOperator(Client const &client)
{
    if (this->members.find(&client) == this->members.end())
        throw std::out_of_range(client.getNickname() +
                                ": client must be a channel member to"
                                " obtain channel operator privileges");

    this->operators.insert(&client);

    return;
}

void irc::Channel::removeOperator(Client const &client)
{
    Channel::t_clients::const_iterator cit(this->operators.find(&client));

    if (cit == this->operators.end())
        throw std::out_of_range(
            client.getNickname() +
            ": client must be a channel operator to lose its privileges");

    return (this->operators.erase(cit));
}

void irc::Channel::setTopic(std::string const &topic)
{
    this->topic = topic;

    return;
}

void irc::Channel::setKey(std::string const &key)
{
    if (std::find_if(key.begin(),
                     key.end(),
                     Channel::is_not_key) != key.end())
        throw std::domain_error("channel key must have any character except"
                                ": NULL, CR, LF, FF, h/v TABs, and SPACE");

    this->key = key;

    return;
}

void irc::Channel::setTopicWhoTime(Client const &client)
{
    this->topicwhotime = std::make_pair(&client, ::time(NULL));

    return;
}

void irc::Channel::addMode(char const &mode)
{
    if (mode == CHAN_MODE_o)
        throw std::invalid_argument(
            "operator mode is meant to be set on a member of the channel"
            ", not on the channel");
    else if (Server::instance().getTextProperty(PRP_CHANMODESINFO).find(mode) ==
             std::string::npos)
        throw std::domain_error(
            std::string(1, mode) +
            ": mode must be any supported channel mode: " +
            Server::instance().getTextProperty(PRP_CHANMODESINFO));

    this->modes.insert(mode);

    return;
}

void irc::Channel::removeMode(char const &mode)
{
    if (mode == CHAN_MODE_o)
        throw std::invalid_argument(
            "operator mode is never set on the channel itself");
    else if (Server::instance().getTextProperty(PRP_CHANMODESINFO).find(mode) ==
             std::string::npos)
        throw std::domain_error(
            std::string(1, mode) +
            ": mode must any supported channel mode: " +
            Server::instance().getTextProperty(PRP_CHANMODESINFO));

    this->modes.erase(mode);

    return;
}

void irc::Channel::setLimit(std::size_t const &limit)
{
    if (this->modes.find('l') == this->modes.end())
        throw std::runtime_error(
            "channel mode +l must be set to define a limit");

    this->limit = limit;

    return;
}

std::string irc::Channel::prefixedName(Client const &client) const
{
    if (this->operators.find(&client) == this->operators.end())
        return (client.getNickname());

    return ('@' + client.getNickname());
}
