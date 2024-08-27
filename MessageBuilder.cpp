/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MessageBuilder.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/23 15:49:24 by abenamar          #+#    #+#             */
/*   Updated: 2024/08/27 11:50:27 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Message.hpp"

Message::Builder::Builder(void) : input(), prefix(), command(), parameters() { return; }

Message::Builder::Builder(Builder const & /* src */) : input(), prefix(), command(), parameters() { return; }

Message::Builder::~Builder(void) throw() { return; }

Message::Builder &Message::Builder::operator=(Builder const & /* rhs */) throw() { return (*this); }

Message::Builder &Message::Builder::withInput(std::string const &input)
{
    this->input = input;

    return (*this);
}

Message Message::Builder::build(void) { return (Message(this->input, this->prefix, this->command, this->parameters)); }
