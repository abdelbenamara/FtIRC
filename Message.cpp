/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/23 15:20:15 by abenamar          #+#    #+#             */
/*   Updated: 2024/08/27 11:50:02 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Message.hpp"

std::string const Message::CRLF = "\r\n";

std::size_t const Message::MAXSIZE = 512, Message::MAXCHARS = Message::MAXSIZE - Message::CRLF.length();

char Message::BUFFER[Message::MAXSIZE];

Message::Message(void) : input(), prefix(), command(), parameters() { return; }

Message::Message(std::string const &input, std::string const &prefix, std::string const &command, std::vector<std::string> const &parameters) : input(input), prefix(prefix), command(command), parameters(parameters) { return; }

Message::Message(Message const &src) : input(src.input), prefix(src.prefix), command(src.command), parameters(src.parameters) { return; }

Message::~Message(void) { return; }

Message &Message::operator=(Message const & /* rhs */) throw() { return (*this); }

std::string const &Message::getInput(void) const throw() { return (this->input); }
