/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karimasadykova <karimasadykova@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/23 15:20:15 by abenamar          #+#    #+#             */
/*   Updated: 2024/08/29 22:43:41 by karimasadyk      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Message.hpp"

std::string const Message::CRLF = "\r\n";

std::size_t const Message::MAXSIZE = 512, Message::MAXCHARS = Message::MAXSIZE - Message::CRLF.length();

char Message::BUFFER[Message::MAXSIZE];

Message::Message(void) : input(), prefix(), command(),
						user(), host(), name(), parameters() { return; }

Message::Message(std::string const &input, std::string const &prefix, std::string const &command,
						std::string const &user, std::string const &host, std::string const &name,
						std::vector<std::string> const &parameters) : input(input), prefix(prefix), command(command),
																	user(user), host(host), name(name),
																	parameters(parameters) { return; }

Message::Message(Message const &src) : input(src.input), prefix(src.prefix),
										command(src.command),
										user(src.user), host(src.host), name(src.name),
										parameters(src.parameters) { return; }

Message::~Message(void) { return; }

Message &Message::operator=(Message const & /* rhs */) throw() { return (*this); }

std::string const &Message::getInput(void) const throw() { return (this->input); }

std::string const &Message::getPrefix(void) const throw() { return (this->prefix); }

std::string const &Message::getCommand(void) const throw() { return (this->command); }

std::vector<std::string> const &Message::getParameters(void) const throw() { return (this->parameters); }

std::string const &Message::getUser(void) const throw() { return (this->user); }

std::string const &Message::getHost(void) const throw() { return (this->host); }

std::string const &Message::getName(void) const throw() { return (this->name); }