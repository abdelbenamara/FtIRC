/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/23 15:20:15 by abenamar          #+#    #+#             */
/*   Updated: 2024/10/13 20:23:05 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Message.hpp"

std::locale const Message::LOCALE;

std::string const Message::CRLF = "\r\n";

std::size_t const Message::MAXSIZE = 512, Message::MAXCHARS = Message::MAXSIZE - Message::CRLF.length();

char Message::BUFFER[Message::MAXSIZE];

bool Message::isNotInServernameFormat(char const &c) { return (c != '.' && c != '-' && !std::isalnum(c, Message::LOCALE)); }

bool Message::isNotInNicknameFormat(char const &c)
{
    static std::string const special = "[]\\`_^{|}";

    return (c != '-' && special.find(c) == std::string::npos && !std::isalnum(c, Message::LOCALE));
}

bool Message::isNotInCommandFormat(char const &c) { return (!std::isalpha(c, Message::LOCALE)); }

Message::Message(void) : input(), prefix(), command(), parameters() { return; }

Message::Message(std::string const &input, std::string const &prefix, std::string const &command, std::vector<std::string> const &parameters) : input(input), prefix(prefix), command(command), parameters(parameters) { return; }

Message::Message(Message const &src) : input(src.input), prefix(src.prefix), command(src.command), parameters(src.parameters) { return; }

Message::~Message(void) { return; }

Message &Message::operator=(Message const & /* rhs */) throw() { return (*this); }

std::string const &Message::getInput(void) const throw() { return (this->input); }

std::string const &Message::getPrefix(void) const throw() { return (this->prefix); }

std::string const &Message::getCommand(void) const throw() { return (this->command); }

std::vector<std::string> const &Message::getParameters(void) const throw() { return (this->parameters); }

std::string Message::toString(void) const
{
    std::ostringstream o;

    o << "[Message ";

    if (!this->prefix.empty())
        o << "prefix=" << this->prefix << ", ";

    o << "command=" << this->command;

    if (!this->parameters.empty())
        o << ", parameters=" << this->parameters.front();

    for (std::vector<std::string>::const_iterator it = ++this->parameters.begin(); it != this->parameters.end(); ++it)
        o << " " << *it;

    o << ']';

    return (o.str());
}
