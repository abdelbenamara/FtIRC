/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MessageBuilder.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/23 15:49:24 by abenamar          #+#    #+#             */
/*   Updated: 2024/10/15 10:39:51 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Message.hpp"

Message::Builder::Builder(void) : input(), prefix(), command(), parameters() { return; }

Message::Builder::Builder(Builder const & /* src */) : input(), prefix(), command(), parameters() { return; }

Message::Builder::~Builder(void) throw() { return; }

Message::Builder &Message::Builder::operator=(Builder const & /* rhs */) throw() { return (*this); }

Message::Builder &Message::Builder::withInput(std::string const &input)
{
    try
    {
        if (input.empty() || input.length() > Message::MAXSIZE)
            throw std::length_error("std::length_error: parameter must not be empty or have a length greater than `Message::MAXSIZE'");

        if (input.find('\0') != std::string::npos)
            throw std::domain_error("std::domain_error: NUL character is not allowed within parameter");

        if (input.compare(input.length() - Message::CRLF.length(), std::string::npos, Message::CRLF))
            throw std::invalid_argument("std::invalid_argument: parameter must be terminated with a CR-LF (Carriage Return - Line Feed) pair");

        if (input.find_first_of(Message::CRLF) < input.length() - Message::CRLF.length())
            throw std::invalid_argument("std::invalid_argument: CR and LF characters are not allowed within parameter, only the trailing CR-LF is allowed");
        this->input = input;
    }
    catch (std::exception const &e)
    {
        throw std::logic_error("Message::Builder::withInput: " + std::string(e.what()));
    }

    return (*this);
}

Message Message::Builder::build(void)
{
    static std::string const sep = ' ' + Message::CRLF;
    std::size_t cur = 0, max, pos;

    if (this->input.empty())
        throw std::runtime_error("std::runtime_error: an input must be supplied through `Message::Builder::withInput' to call `Message::Builder::build'");

    max = this->input.length() - Message::CRLF.length();
    pos = this->input.find_first_of(sep);

    try
    {
        if (pos != 1 && this->input.at(0) == ':')
        {
            if (pos == max)
                throw std::invalid_argument("std::invalid_argument: message format must be `[ \":\" prefix SPACE ] command [ params ] crlf'");

            this->setPrefix(this->input.substr(1, pos - 1));

            cur = pos + 1;
            pos = this->input.find_first_of(sep, cur);
        }

        this->setCommand(this->input.substr(cur, pos - cur));

        while (pos != max && this->parameters.size() != 15)
        {
            cur = pos + 1;
            pos = this->input.find_first_of(sep, cur);

            if (this->input.at(cur) == ':')
            {
                ++cur;
                pos = max;
            }

            if (this->parameters.size() == 14)
                pos = max;

            this->parameters.push_back(this->input.substr(cur, pos - cur));
        }
    }
    catch (std::exception const &e)
    {
        throw std::logic_error("Message::Builder::build: " + std::string(e.what()));
    }

    return (Message(this->input, this->prefix, this->command, this->parameters));
}

void Message::Builder::setPrefix(std::string const &prefix)
{
    static std::string const special = "[]\\`_^{|}";
    bool const isServernameFormat = std::find_if(prefix.begin(), prefix.end(), Message::isNotInServernameFormat) == prefix.end();
    bool const isNicknameFormat = std::find_if(prefix.begin(), prefix.end(), Message::isNotInNicknameFormat) == prefix.end();

    if (!isServernameFormat && !isNicknameFormat)
        throw std::invalid_argument("std::invalid_argument: prefix is neither a servername nor a nickname");

    if (prefix.at(0) == '-')
        throw std::domain_error("std::domain_error: prefix must not begin with a hyphen ('-') character");

    if (isServernameFormat)
    {
        if (prefix.length() > 63)
            throw std::length_error("std::length_error: prefix servername must have a maximum length of 63 characters");

        if (prefix.at(0) == '.')
            throw std::domain_error("std::domain_error: prefix servername must not begin with a period ('.') character");

        if (*prefix.rbegin() == '-')
            throw std::domain_error("std::domain_error: prefix servername must not end with a hyphen ('-') character");

        if (*prefix.rbegin() == '.')
            throw std::domain_error("std::domain_error: prefix servername must not end with a period ('.') character");
    }

    if (isNicknameFormat)
    {
        if (std::isdigit(prefix.at(0), Message::LOCALE))
            throw std::domain_error("std::domain_error: prefix nickname must not begin with a digit character");

        if (prefix.length() > 9)
            throw std::length_error("std::length_error: prefix nickname must have a maximum length of 9 characters");
    }

    this->prefix = prefix;

    return;
}

void Message::Builder::setCommand(std::string const &command)
{
    if (!command.empty() && std::isdigit(command.at(0), Message::LOCALE))
    {
        if (command.length() != 3)
            throw std::length_error("std::length_error: command response must have a length of exactly 3 characters");

        if (!std::isdigit(command.at(1), Message::LOCALE) || !std::isdigit(command.at(2), Message::LOCALE))
            throw std::domain_error("std::domain_error: command response must have only digit characters");
    }

    if (std::find_if(command.begin(), command.end(), Message::isNotInCommandFormat) != command.end())
        throw std::domain_error("std::domain_error: command must have only alpha characters");

    this->command = command;

    return;
}
