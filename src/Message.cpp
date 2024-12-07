/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/23 15:20:15 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/07 03:04:46 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Message.hpp"
#include "Server.hpp"

std::string const irc::Message::CRLF("\r\n");

std::size_t const irc::Message::NUM_RPL_LEN(3),
    irc::Message::PARAMETERS_SIZE(15);

irc::Message irc::Message::from(std::string const &input)
{
    std::size_t pos(input.length() - Message::CRLF.length());
    std::string buf(input.substr(0, pos));
    std::vector<std::string> tokens;
    Message::Builder builder;
    std::vector<std::string>::const_iterator cit;

    if (input.empty() ||
        input.length() > Server::instance().getSizeProperty(PRP_LINELEN))
        throw std::length_error(
            "input length: " + utils::to_string(input.length()) +
            ": input must not be empty or have more than " +
            utils::to_string(Server::instance().getSizeProperty(PRP_LINELEN)) +
            " characters");
    else if (input.find('\0') != std::string::npos)
        throw std::domain_error("input must not contain a NUL character");
    else if (input.compare(pos, std::string::npos, Message::CRLF))
        throw std::invalid_argument("input must be terminated with CRLF");
    else if (input.find_first_of(Message::CRLF) < pos)
        throw std::invalid_argument("input must not contain CR or LF characters"
                                    ", only the trailing CRLF is expected");

    tokens = utils::split(buf, ' ', 2);

    if (!tokens.at(0).rfind(':', 0))
    {
        builder.withPrefix(tokens.at(0).erase(0, 1));

        tokens = utils::split(tokens.at(1), ' ', 2);
    }

    builder.withCommand(tokens.at(0));

    if (!tokens.at(1).rfind(':', 0))
        tokens.at(1).insert(tokens.at(1).begin(), ' ');

    pos = tokens.at(1).find(" :");
    tokens.at(0) = tokens.at(1).substr(0, pos);
    buf = tokens.at(1).erase(0, pos);
    tokens = utils::split(tokens.at(0), ' ', Message::PARAMETERS_SIZE);

    if (tokens.back().empty())
        buf.erase(0, 2);

    tokens.back() += buf;

    for (cit = tokens.begin(); cit != tokens.end(); ++cit)
        if (!cit->empty())
            builder.addParameter(*cit);

    if (tokens.back().empty() && pos != std::string::npos)
        builder.addParameter(tokens.back());

    return (builder.build());
}

irc::Message::Message(Message const &src)
    : prefix(src.prefix),
      command(src.command),
      parameters(src.parameters) { return; }

irc::Message::Message(std::string const &prefix,
                      std::string const &command,
                      std::vector<std::string> const &parameters)
    : prefix(prefix),
      command(command),
      parameters(parameters) { return; }

irc::Message::~Message(void) { return; }

std::string const &irc::Message::getPrefix(
    void) const throw() { return (this->prefix); }

std::string const &irc::Message::getCommand(
    void) const throw() { return (this->command); }

std::vector<std::string> const &irc::Message::getParameters(
    void) const throw() { return (this->parameters); }

std::string irc::Message::str(void) const
{
    std::ostringstream out;

    if (!this->prefix.empty())
        out << ':' << this->prefix << ' ';

    out << this->command;

    if (!this->parameters.empty())
    {
        out << ' ';

        std::copy(this->parameters.begin(),
                  --this->parameters.end(),
                  std::ostream_iterator<std::string>(out, " "));

        out << ':' << this->parameters.back();
    }

    out << Message::CRLF;

    return (out.str());
}

std::ostream &operator<<(std::ostream &o, irc::Message const &message)
{
    o << "[Message ";

    if (!message.getPrefix().empty())
        o << "prefix=" << message.getPrefix() << ", ";

    o << "command=" << message.getCommand() << ", parameters=(";

    if (!message.getParameters().empty())
        o << '{'
          << irc::utils::to_string(message.getParameters().begin(),
                                   message.getParameters().end(),
                                   "}, {")
          << '}';

    return (o << ")]");
}
