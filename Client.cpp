/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejankovs <ejankovs@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/12 19:45:21 by abenamar          #+#    #+#             */
/*   Updated: 2024/10/16 19:49:47 by ejankovs         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(void) : connfd(-1) { return; }

Client::Client(int const connfd) : connfd(connfd), isMessageTooLong(false), authorized(false), identified(false), gone(false), input(), nickname(), username(), host(), messages()
{
	// TODO: set host depending of Address Family ...
	this->input.reserve(Message::MAXSIZE);
	this->nickname.reserve(9);

	return;
}

Client::Client(Client const & /* src */) : connfd(-1) { return; }

Client::~Client(void) throw()
{
	close(this->connfd);

	return;
}

Client &Client::operator=(Client const & /* rhs */) throw() { return (*this); }

int Client::getSocket(void) const throw() { return (this->connfd); }

bool Client::updateInput(void)
{
	ssize_t incap, nread;

	try
	{
		do
		{
			incap = Message::MAXSIZE - this->input.length();
			nread = recv(connfd, Message::BUFFER, incap, 0);

			if (nread == -1)
			{
				if (errno == EAGAIN || errno == EWOULDBLOCK)
					break;

				throw RuntimeErrno("recv");
			}

			if (!nread)
				return (false);

			this->input.append(Message::BUFFER, nread);
			this->addMessage(this->input.find_first_of(Message::CRLF, this->input.length() - nread));
		} while (incap == nread);
	}
	catch (std::exception const &e)
	{
		throw std::runtime_error("Client::updateInput: " + std::string(e.what()));
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

void Client::removeMessage(void)
{
	if (this->messages.empty())
		throw std::runtime_error("Client::removeMessage: std::runtime_error: no message, `Client::hasMessage' must return true to call `Client::removeMessage'");

	this->messages.pop();

	return;
}

bool Client::isAuthorized(void) const throw() { return (this->authorized); }

void Client::setAuthorized(bool const &isAuthorized) throw()
{
	this->authorized = isAuthorized;

	return;
}

bool Client::isIdentified(void) throw() { return (false); }

void Client::identify(std::string const &username)
{
	if (this->identified)
		throw std::runtime_error("Client::identify: std::runtime_error: client already identified, `Client::identify' must only be called once per client");

	this->username = username;
	this->identified = true;

	return;
}

std::string const &Client::getNickname(void) const throw() { return (this->nickname); }

void Client::setNickname(std::string const &nickname) throw()
{
	this->nickname = nickname;

	return;
}

bool Client::isRegistered(void) throw() { return (this->authorized && this->identified && !this->nickname.empty()); }

bool Client::isGone(void) throw() { return (this->gone); }

void Client::quit(void) throw()
{
	this->gone = true;

	return;
}

void Client::cleanupInput(std::size_t const &crlfpos)
{
	this->isMessageTooLong = false;

	this->input.erase(0, this->input.find_first_not_of(Message::CRLF, crlfpos));

	return;
}

void Client::addMessage(std::size_t const &crlfpos)
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

		this->cleanupInput(crlfpos);
	}
	catch (const std::exception &e)
	{
		this->cleanupInput(crlfpos);

		throw;
	}

	return (this->addMessage(this->input.find_first_of(Message::CRLF)));
}

std::map<int, Channel *const> &Client::getChannels(void) throw()
{
	return (this->channels);
}