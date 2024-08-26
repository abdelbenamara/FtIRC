/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/12 19:45:21 by abenamar          #+#    #+#             */
/*   Updated: 2024/08/26 12:33:18 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(void) : connfd(-1) { return; }

Client::Client(int const connfd) : connfd(connfd), isMessageTooLong(false), input(), password(NULL), messages()
{
	this->input.reserve(Message::MAXSIZE);

	return;
}

Client::Client(Client const & /* src */) : connfd(-1) { return; }

Client::~Client(void) throw()
{
	delete password;

	close(this->connfd);

	return;
}

Client &Client::operator=(Client const & /* rhs */) throw() { return (*this); }

bool Client::appendInput(char *const buffer)
{
	ssize_t rbcap, nread;

	try
	{
		do
		{
			rbcap = Message::MAXSIZE - this->input.length();
			nread = recv(connfd, buffer, rbcap, 0);

			if (nread == -1)
			{
				if (errno == EAGAIN || errno == EWOULDBLOCK)
					break;

				throw RuntimeErrno("recv");
			}

			if (!nread)
				return (false);

			this->input.append(buffer, nread);
			this->extractMessage(this->input.find_first_of(Message::CRLF, this->input.length() - nread));
		} while (rbcap == nread);
	}
	catch (std::exception const &e)
	{
		throw std::runtime_error("Client::appendInput: " + std::string(e.what()));
	}

	return (true);
}

bool Client::hasMessage(void) const throw() { return (!this->messages.empty()); }

Message const &Client::message(void)
{
	if (this->messages.empty())
		throw std::runtime_error("Client::message: std::runtime_error: no message, `Client::hasMessage' must return true to call `Client::message'");

	return (this->messages.front());
}

void Client::dropMessage(void) throw()
{
	if (this->messages.empty())
		throw std::runtime_error("Client::dropMessage: std::runtime_error: nothing to drop, `Client::hasMessage' must return true to call `Client::dropMessage'");

	this->messages.pop();

	return;
}

void Client::extractMessage(std::size_t const crlfpos)
{
	if (crlfpos == std::string::npos)
	{
		if (this->input.length() == Message::MAXSIZE)
			this->isMessageTooLong = true;

		if (this->isMessageTooLong)
			this->input.clear();

		return;
	}

	try
	{
		if (!this->isMessageTooLong && 0 < crlfpos && crlfpos <= Message::MAXCHARS)
			this->messages.push(Message::Builder()
									.withInput(this->input.substr(0, crlfpos) + Message::CRLF)
									.build());
	}
	catch (std::exception const &e)
	{
		throw std::runtime_error("Client::appendInput: " + std::string(e.what()));
	}

	this->isMessageTooLong = false;

	this->input.erase(0, this->input.find_first_not_of(Message::CRLF, crlfpos));

	return (this->extractMessage(this->input.find_first_of(Message::CRLF)));
}
