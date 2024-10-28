/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/12 19:45:21 by abenamar          #+#    #+#             */
/*   Updated: 2024/10/28 20:45:29 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

std::size_t const Client::NICK_MAX_LEN = 9;

std::set<char> const Client::USER_MODES = Client::initModes();

int Client::unique = 0;

std::set<char> Client::initModes(void)
{
	std::set<char> modes;

	modes.insert(USR_MODE_I);
	modes.insert(USR_MODE_W);
	modes.insert(USR_MODE_O);

	return (modes);
}

std::string Client::userModes(void)
{
	std::ostringstream o;

	for (std::set<char>::const_iterator cit = Client::USER_MODES.begin(); cit != Client::USER_MODES.end(); ++cit)
		o << ',' << *cit;

	return (o.str().substr(1));
}

bool Client::isNotInNicknameFormat(char const &c)
{
	static std::string const special = "[]\\`_^{|}";

	return (c != '-' && special.find(c) == std::string::npos && !std::isalnum(c, Message::LOCALE));
}

Client::Client(int const &connfd, std::string const &hostaddr)
	: uid(++Client::unique),
	  connfd(connfd),
	  hostaddr(hostaddr),
	  registered(false),
	  messages(),
	  password(Message::CRLF),
	  nickname("*"),
	  username(),
	  realname(),
	  modes()
{
	this->nickname.reserve(Client::NICK_MAX_LEN);

	return;
}

Client::Client(Client const &src)
	: uid(src.uid),
	  connfd(src.connfd),
	  hostaddr(src.hostaddr),
	  registered(src.registered),
	  messages(src.messages),
	  password(src.password),
	  nickname(src.nickname),
	  username(src.username),
	  realname(src.realname),
	  modes(src.modes) { return; }

Client::~Client(void) throw() { return; }

int const &Client::getSocket(void) const throw() { return (this->connfd); }

std::string const &Client::getHostaddr(void) const throw() { return (this->hostaddr); }

bool const &Client::isRegistered(void) const throw() { return (this->registered); }

std::queue<Message> const &Client::getMessages(void) const throw() { return (this->messages); }

std::string const &Client::getPassword(void) const throw() { return (this->password); }

std::string const &Client::getNickname(void) const throw() { return (this->nickname); }

std::string const &Client::getUsername(void) const throw() { return (this->username); }

std::string const &Client::getRealname(void) const throw() { return (this->realname); }

std::set<char> const &Client::getModes(void) const throw() { return (this->modes); }

std::string Client::userId(void) const
{
	std::stringstream o;

	if (this->username.empty())
		o << "UID" << std::setfill('0') << std::setw(7) << this->uid;
	else
		o << this->username;

	o << '@' << this->hostaddr;

	return (o.str());
}

std::string Client::str(void) const
{
	std::stringstream o;

	if (!this->nickname.empty())
		o << this->nickname << '!';

	o << this->userId();

	return (o.str());
}

void Client::produce(Message const &message)
{
	this->messages.push(message);

	return;
}

Message Client::consume(void)
{
	Message message(this->messages.front());

	this->messages.pop();

	return (message);
}

void Client::setPassword(std::string const &password)
{

	try
	{
		if (this->registered)
			throw std::runtime_error("std::runtime_error: client must not set password once registered");

		if (password.find_first_of(std::string("\0\r\n")) != std::string::npos)
			throw std::domain_error("std::domain_error: password must have any character except: NUL, CR, LF");

		this->password = password;
	}
	catch (std::exception const &e)
	{
		throw std::runtime_error("Client::setPassword: " + std::string(e.what()));
	}

	return;
}

void Client::setNickname(std::string const &nickname)
{
	std::ostringstream err;

	try
	{
		if (nickname.at(0) == '-')
			throw std::invalid_argument("std::invalid_argument: nickname must not begin with a hyphen character");
		else if (std::isdigit(nickname.at(0), Message::LOCALE))
			throw std::invalid_argument("std::invalid_argument: nickname must not begin with a digit character");
		else if (nickname.length() > Client::NICK_MAX_LEN)
			throw std::length_error(reinterpret_cast<std::ostringstream &>(err << "std::length_error: " << nickname.length() << ": nickname must not have more than " << Client::NICK_MAX_LEN << " characters").str());
		else if (std::find_if(nickname.begin(), nickname.end(), Client::isNotInNicknameFormat) != nickname.end())
			throw std::domain_error("std::domain_error: nickname must have only alphanumeric and special characters: []\\`_^{|}");

		this->nickname = nickname;

		if (!this->registered)
			this->registered = !this->username.empty();
	}
	catch (std::exception const &e)
	{
		throw std::runtime_error("Client::setNickname: " + std::string(e.what()));
	}

	return;
}

void Client::setUsername(std::string const &username)
{
	try
	{
		if (this->registered)
			throw std::runtime_error("std::runtime_error: client must not set username once registered");

		if (username.find_first_of(std::string("\0\r\n @")) != std::string::npos)
			throw std::domain_error("std::domain_error: username must have any character except: NUL, CR, LF, SPACE, @");

		this->username = username;
		this->registered = this->nickname.compare("*");
	}
	catch (std::exception const &e)
	{
		throw std::runtime_error("Client::setUsername: " + std::string(e.what()));
	}

	return;
}

void Client::setRealname(std::string const &realname)
{
	try
	{
		if (this->registered)
			throw std::runtime_error("std::runtime_error: client must not set realname once registered");

		if (realname.find_first_of(std::string("\0\r\n")) != std::string::npos)
			throw std::domain_error("std::domain_error: realname must have any character except: NUL, CR, LF");

		this->realname = realname;
	}
	catch (std::exception const &e)
	{
		throw std::runtime_error("Client::setRealname: " + std::string(e.what()));
	}

	return;
}

void Client::addMode(char const &mode)
{
	std::ostringstream err;

	if (Client::USER_MODES.find(mode) == Client::USER_MODES.end())
		throw std::domain_error(reinterpret_cast<std::ostringstream &>(err << "Client::addMode: std::domain_error: " << mode << ": mode must any supported user mode: " << Client::userModes()).str());

	this->modes.insert(mode);

	return;
}

void Client::removeMode(char const &mode)
{
	std::ostringstream err;

	if (Client::USER_MODES.find(mode) == Client::USER_MODES.end())
		throw std::domain_error(reinterpret_cast<std::ostringstream &>(err << "Client::addMode: std::domain_error: " << mode << ": mode must any supported user mode: " << Client::userModes()).str());

	this->modes.erase(mode);

	return;
}
