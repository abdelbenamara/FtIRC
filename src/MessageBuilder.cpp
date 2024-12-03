/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MessageBuilder.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/01 20:56:21 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/02 20:48:03 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Message.hpp"

static bool is_not_alpha(char const &c)
{
    return (!std::isalpha(c, std::locale::classic()));
}

irc::Message::Builder::Builder(void)
    : trailing(false),
      prefix(),
      command(),
      parameters() { return; }

irc::Message::Builder::~Builder(void) throw() { return; }

std::string const &irc::Message::Builder::getPrefix(
    void) const throw() { return (this->prefix); }

std::string const &irc::Message::Builder::getCommand(
    void) const throw() { return (this->command); }

std::vector<std::string> const &irc::Message::Builder::getParameters(
    void) const throw() { return (this->parameters); }

irc::Message::Builder &irc::Message::Builder::withoutPrefix(void)
{
    this->prefix.clear();

    return (*this);
}

irc::Message::Builder &irc::Message::Builder::withPrefix(
    std::string const &prefix)
{
    if (prefix.empty())
        throw std::length_error("prefix must not be empty");
    else if (prefix.at(0) == '-')
        throw std::domain_error(
            "prefix must not begin with a hyphen character");
    else if (prefix.at(0) == '.' || *prefix.rbegin() == '.')
        throw std::domain_error(
            "prefix must neither begin nor end with a period character");
    else if (prefix.find_first_of("\0\r\n ", 0, 4) != std::string::npos)
        throw std::domain_error(
            "prefix must have any character except: NUL, CR, LF, SPACE");

    this->prefix = prefix;

    return (*this);
}

irc::Message::Builder &irc::Message::Builder::withCommand(
    std::string const &command)
{
    if (command.empty())
        throw std::length_error("std::length_error: command must not be empty");
    else if (utils::is_digit(command.at(0)))
    {
        if (command.length() != Message::NUM_RPL_LEN)
            throw std::length_error(
                "command length: " + utils::to_string(command.length()) +
                ": numeric reply command must have exactly " +
                utils::to_string(Message::NUM_RPL_LEN) + " characters");
        else if (!utils::is_digit(command.at(1)) ||
                 !utils::is_digit(command.at(2)))
            throw std::domain_error(
                "numeric reply command must have only digit characters");
    }
    else if (std::find_if(command.begin(),
                          command.end(),
                          ::is_not_alpha) != command.end())
        throw std::domain_error("command must have only alpha characters");

    this->command.resize(command.length());
    std::transform(command.begin(),
                   command.end(),
                   this->command.begin(),
                   utils::to_upper);

    return (*this);
}

irc::Message::Builder &irc::Message::Builder::withoutParameters(void)
{
    this->parameters.clear();

    this->trailing = false;

    return (*this);
}

irc::Message::Builder &irc::Message::Builder::addParameter(
    std::string const &parameter)
{
    if (this->parameters.size() == Message::PARAMETERS_SIZE)
        throw std::length_error(
            "a message must not have more than " +
            utils::to_string(Message::PARAMETERS_SIZE) + " parameters");
    else if (parameter.find_first_of("\0\r\n", 0, 3) != std::string::npos)
        throw std::domain_error(
            "parameter must have any character except: NUL, CR, LF");
    else if (this->trailing)
        throw std::logic_error(
            "no parameter must be supplied after the trailing one");

    this->trailing = parameter.empty() ||
                     parameter.at(0) == ':' ||
                     parameter.find(' ') != std::string::npos;

    this->parameters.push_back(parameter);

    return (*this);
}

irc::Message::Builder &irc::Message::Builder::addParameter(
    char const &parameter)
{
    return (this->addParameter(std::string(1, parameter)));
}

irc::Message::Builder &irc::Message::Builder::withParameter(
    std::string const &parameter)
{
    this->withoutParameters();

    return (this->addParameter(parameter));
}

irc::Message::Builder &irc::Message::Builder::withParameters(
    std::vector<std::string> const &parameters)
{
    std::vector<std::string>::const_iterator cit;

    if (parameters.size() > Message::PARAMETERS_SIZE)
        throw std::length_error(
            "parameters size: " + utils::to_string(parameters.size()) +
            ": a message must not have more than " +
            utils::to_string(Message::PARAMETERS_SIZE) + " parameters");

    this->withoutParameters();

    for (cit = parameters.begin(); cit != parameters.end(); ++cit)
        this->addParameter(*cit);

    return (*this);
}

irc::Message irc::Message::Builder::build(void)
{
    return (Message(this->prefix, this->command, this->parameters));
}
