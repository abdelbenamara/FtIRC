/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/23 15:20:15 by abenamar          #+#    #+#             */
/*   Updated: 2024/10/29 18:22:24 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Message.hpp"

namespace irc
{
    std::locale const Message::LOCALE;

    std::string const Message::CRLF = "\r\n";

    std::size_t const Message::MAX_LEN = MSG_SIZE, Message::MAX_CHARS = Message::MAX_LEN - Message::CRLF.length(), Message::NUM_RPL_LEN = 3, Message::PARAMS_MAX_LEN = 15;

    bool Message::Builder::isNotInCommandFormat(char const &c) { return (!std::isalpha(c, Message::LOCALE)); }

    Message::Builder::Builder(void) : trailing(false), prefix(), command(), parameters() { return; }

    Message::Builder::~Builder(void) throw() { return; }

    Message::Builder &Message::Builder::withPrefix(std::string const &prefix)
    {
        try
        {
            if (prefix.empty())
                throw std::length_error("std::length_error: prefix must not be empty");
            else if (prefix.at(0) == '-')
                throw std::domain_error("std::domain_error: prefix must not begin with a hyphen character");
            else if (prefix.at(0) == '.')
                throw std::domain_error("std::domain_error: prefix must not begin with a period character");
            else if (prefix.find_first_of("\0\r\n ", 0, 4) != std::string::npos)
                throw std::domain_error("std::domain_error: prefix must have any character except: NUL, CR, LF, SPACE");

            this->prefix = prefix;
        }
        catch (std::exception const &e)
        {
            throw std::logic_error("Message::Builder::withPrefix: " + std::string(e.what()));
        }

        return (*this);
    }

    Message::Builder &Message::Builder::withCommand(std::string const &command)
    {
        std::ostringstream err;

        try
        {
            if (command.empty())
                throw std::length_error("std::length_error: command must not be empty");
            else if (std::isdigit(command.at(0), Message::LOCALE))
            {
                if (command.length() != Message::NUM_RPL_LEN)
                    throw std::length_error(reinterpret_cast<std::ostringstream &>(err << "std::length_error: " << command.length() << ": numeric reply command must have exactly " << Message::NUM_RPL_LEN << " characters").str());
                else if (!std::isdigit(command.at(1), Message::LOCALE) || !std::isdigit(command.at(2), Message::LOCALE))
                    throw std::domain_error("std::domain_error: numeric reply command must have only digit characters");
            }
            else if (std::find_if(command.begin(), command.end(), Message::Builder::isNotInCommandFormat) != command.end())
                throw std::domain_error("std::domain_error: command must have only alpha characters");

            this->command = command;
        }
        catch (std::exception const &e)
        {
            throw std::logic_error("Message::Builder::withCommand: " + std::string(e.what()));
        }

        return (*this);
    }

    Message::Builder &Message::Builder::addParameter(std::string const &parameter)
    {
        std::ostringstream err;

        try
        {
            if (this->parameters.size() == Message::PARAMS_MAX_LEN)
                throw std::length_error(reinterpret_cast<std::ostringstream &>(err << "std::length_error: a message must not have more than " << Message::PARAMS_MAX_LEN << " parameters").str());
            else if (parameter.find_first_of("\0\r\n", 0, 3) != std::string::npos)
                throw std::domain_error("std::domain_error: parameter must have any character except: NUL, CR, LF");
            else if (this->trailing)
                throw std::logic_error("std::logic_error: no parameter must be supplied after the trailing one");

            this->trailing = parameter.empty() || parameter.at(0) == ':' || parameter.find(' ') != std::string::npos;

            this->parameters.push_back(parameter);
        }
        catch (std::exception const &e)
        {
            throw std::logic_error("Message::Builder::addParameter: " + std::string(e.what()));
        }

        return (*this);
    }

    Message::Builder &Message::Builder::withParameters(std::vector<std::string> const &parameters)
    {
        std::ostringstream err;

        try
        {
            if (parameters.size() > Message::PARAMS_MAX_LEN)
                throw std::length_error(reinterpret_cast<std::ostringstream &>(err << "std::length_error: " << parameters.size() << ": a message must not have more than " << Message::PARAMS_MAX_LEN << " parameters").str());

            for (std::vector<std::string>::const_iterator cit = parameters.begin(); cit != parameters.end(); ++cit)
                this->addParameter(*cit);
        }
        catch (std::exception const &e)
        {
            throw std::logic_error("Message::Builder::withParameters: " + std::string(e.what()));
        }

        return (*this);
    }

    Message Message::Builder::build(void)
    {
        try
        {
            return (Message(this->prefix, this->command, this->parameters));
        }
        catch (std::exception const &e)
        {
            throw std::runtime_error("Message::Builder::build: " + std::string(e.what()));
        }
    }

    Message Message::parse(std::string const &input)
    {
        static std::string const sep = ' ' + Message::CRLF;
        std::ostringstream err;
        std::size_t cur, max, pos;
        Message::Builder builder;

        try
        {
            if (input.empty() || input.length() > Message::MAX_LEN)
                throw std::length_error(reinterpret_cast<std::ostringstream &>(err << "std::length_error: " << input.length() << ": input must not be empty or have more than " << Message::MAX_LEN << " characters").str());
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
        }
        catch (std::exception const &e)
        {
            throw std::logic_error("Message::parse: " + std::string(e.what()));
        }

        return (builder.build());
    }

    Message::Message(Message const &src)
        : prefix(src.prefix),
          command(src.command),
          parameters(src.parameters) { return; }

    Message::Message(std::string const &prefix, std::string const &command, std::vector<std::string> const &parameters)
        : prefix(prefix),
          command(command),
          parameters(parameters) { return; }

    Message::~Message(void) { return; }

    std::string const &Message::getPrefix(void) const throw() { return (this->prefix); }

    std::string const &Message::getCommand(void) const throw() { return (this->command); }

    std::vector<std::string> const &Message::getParameters(void) const throw() { return (this->parameters); }

    std::string Message::str(void) const
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
} // namespace irc

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
