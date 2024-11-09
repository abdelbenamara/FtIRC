/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/23 15:20:15 by abenamar          #+#    #+#             */
/*   Updated: 2024/11/09 15:13:09 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Message.hpp"

std::string const irc::Message::CRLF = "\r\n";

std::size_t const irc::Message::NUM_RPL_LEN = 3, irc::Message::MAX_PARAMS = 15;

bool irc::Message::Builder::isNotInCommandFormat(char const &c) { return (!std::isalpha(c, std::locale::classic())); }

irc::Message::Builder::Builder(void) : trailing(false), prefix(), command(), parameters() { return; }

irc::Message::Builder::~Builder(void) throw() { return; }

irc::Message::Builder &irc::Message::Builder::withPrefix(std::string const &prefix)
try
{
    if (prefix.empty())
        throw std::length_error("std::length_error: prefix must not be empty");
    else if (prefix.at(0) == '-')
        throw std::domain_error("std::domain_error: prefix must not begin with a hyphen character");
    else if (prefix.at(0) == '.' || *prefix.rbegin() == '.')
        throw std::domain_error("std::domain_error: prefix must neither begin nor end with a period character");
    else if (prefix.find_first_of("\0\r\n ", 0, 4) != std::string::npos)
        throw std::domain_error("std::domain_error: prefix must have any character except: NUL, CR, LF, SPACE");

    this->prefix = prefix;

    return (*this);
}
catch (std::exception const &e)
{
    throw std::logic_error("irc::Message::Builder::withPrefix: " + std::string(e.what()));
}

irc::Message::Builder &irc::Message::Builder::withCommand(std::string const &command)
try
{
    if (command.empty())
        throw std::length_error("std::length_error: command must not be empty");
    else if (std::isdigit(command.at(0), std::locale::classic()))
    {
        if (command.length() != Message::NUM_RPL_LEN)
            throw std::length_error("std::length_error: " + utils::to_string(command.length()) + ": numeric reply command must have exactly " + utils::to_string(Message::NUM_RPL_LEN) + " characters");
        else if (!std::isdigit(command.at(1), std::locale::classic()) || !std::isdigit(command.at(2), std::locale::classic()))
            throw std::domain_error("std::domain_error: numeric reply command must have only digit characters");
    }
    else if (std::find_if(command.begin(), command.end(), Message::Builder::isNotInCommandFormat) != command.end())
        throw std::domain_error("std::domain_error: command must have only alpha characters");

    this->command = command;

    return (*this);
}
catch (std::exception const &e)
{
    throw std::logic_error("irc::Message::Builder::withCommand: " + std::string(e.what()));
}

irc::Message::Builder &irc::Message::Builder::addParameter(std::string const &parameter)
try
{
    if (this->parameters.size() == Message::MAX_PARAMS)
        throw std::length_error("std::length_error: a message must not have more than " + utils::to_string(Message::MAX_PARAMS) + " parameters");
    else if (parameter.find_first_of("\0\r\n", 0, 3) != std::string::npos)
        throw std::domain_error("std::domain_error: parameter must have any character except: NUL, CR, LF");
    else if (this->trailing)
        throw std::logic_error("std::logic_error: no parameter must be supplied after the trailing one");

    this->trailing = parameter.empty() || parameter.at(0) == ':' || parameter.find(' ') != std::string::npos;

    this->parameters.push_back(parameter);

    return (*this);
}
catch (std::exception const &e)
{
    throw std::logic_error("irc::Message::Builder::addParameter: " + std::string(e.what()));
}

irc::Message::Builder &irc::Message::Builder::withParameters(std::vector<std::string> const &parameters)
try
{
    if (parameters.size() > Message::MAX_PARAMS)
        throw std::length_error("std::length_error: " + utils::to_string(parameters.size()) + ": a message must not have more than " + utils::to_string(Message::MAX_PARAMS) + " parameters");

    this->parameters.clear();

    for (std::vector<std::string>::const_iterator cit = parameters.begin(); cit != parameters.end(); ++cit)
        this->addParameter(*cit);

    return (*this);
}
catch (std::exception const &e)
{
    throw std::logic_error("irc::Message::Builder::withParameters: " + std::string(e.what()));
}

irc::Message irc::Message::Builder::build(void)
try
{
    return (Message(this->prefix, this->command, this->parameters));
}
catch (std::exception const &e)
{
    throw std::runtime_error("irc::Message::Builder::build: " + std::string(e.what()));
}

irc::Message irc::Message::from(std::string const &input)
try
{
    static std::string const sep = ' ' + Message::CRLF;
    std::size_t cur, max, pos;
    Message::Builder builder;

    if (input.empty() || input.length() > Config::getInstance().getSize(PRP_LINELEN))
        throw std::length_error("std::length_error: " + utils::to_string(input.length()) + ": input must not be empty or have more than " + utils::to_string(Config::getInstance().getSize(PRP_LINELEN)) + " characters");
    else if (input.find('\0') != std::string::npos)
        throw std::domain_error("std::domain_error: input must not contain a NUL character");
    else if (input.compare(input.length() - Message::CRLF.length(), std::string::npos, Message::CRLF))
        throw std::invalid_argument("std::invalid_argument: input must be terminated with a CRLF pair");
    else if (input.find_first_of(Message::CRLF) < input.length() - Message::CRLF.length())
        throw std::invalid_argument("std::invalid_argument: input must not contain CR or LF characters, only the trailing CRLF is expected");

    cur = input.find_first_not_of(' ');
    max = input.length() - Message::CRLF.length();
    pos = input.find_first_of(sep, cur);

    if (input.at(0) == ':')
    {
        if (pos == max)
            throw std::invalid_argument("std::invalid_argument: message format must be: [ \":\" prefix SPACE ] command [ params ] CRLF");

        builder.withPrefix(input.substr(1, pos - 1));

        cur = input.find_first_not_of(sep, pos);
        pos = input.find_first_of(sep, cur);
    }

    builder.withCommand(input.substr(cur, pos - cur));

    for (int i = 0; i < 15; ++i)
    {
        if (pos == max)
            break;

        cur = input.find_first_not_of(' ', pos);

        if (cur == max)
            break;

        pos = input.find_first_of(sep, cur);

        if (input.at(cur) == ':')
        {
            ++cur;
            pos = max;
        }

        if (i == 14)
            pos = max;

        builder.addParameter(input.substr(cur, pos - cur));
    }

    return (builder.build());
}
catch (std::exception const &e)
{
    throw std::logic_error("irc::Message::from: " + std::string(e.what()));
}

irc::Message::Message(Message const &src)
    : prefix(src.prefix),
      command(src.command),
      parameters(src.parameters) { return; }

irc::Message::Message(std::string const &prefix, std::string const &command, std::vector<std::string> const &parameters)
    : prefix(prefix),
      command(command),
      parameters(parameters) { return; }

irc::Message::~Message(void) { return; }

std::string const &irc::Message::getPrefix(void) const throw() { return (this->prefix); }

std::string const &irc::Message::getCommand(void) const throw() { return (this->command); }

std::vector<std::string> const &irc::Message::getParameters(void) const throw() { return (this->parameters); }

std::string irc::Message::str(void) const
{
    std::vector<std::string>::const_iterator cit = this->parameters.begin();
    std::ostringstream o;

    if (!this->prefix.empty())
        o << ':' << this->prefix << ' ';

    o << command;

    for (; cit != this->parameters.end() - 1; ++cit)
        o << ' ' << *cit;

    if (cit == this->parameters.end() - 1)
        o << " :" << *cit;

    o << Message::CRLF;

    return (o.str());
}

std::ostream &operator<<(std::ostream &o, irc::Message const &i)
{
    o << "[Message ";

    if (!i.getPrefix().empty())
        o << "prefix=" << i.getPrefix() << ", ";

    o << "command=" << i.getCommand() << ", parameters=(";

    if (!i.getParameters().empty())
    {
        o << '{' << i.getParameters().front() << '}';

        for (std::vector<std::string>::const_iterator cit = ++i.getParameters().begin(); cit != i.getParameters().end(); ++cit)
            o << ", {" << *cit << '}';
    }

    o << ")]";

    return (o);
}
