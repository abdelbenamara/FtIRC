/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/12 19:45:21 by abenamar          #+#    #+#             */
/*   Updated: 2024/11/01 17:47:10 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

std::size_t const irc::Client::NICK_MAX_LEN = USR_NICK_LEN, irc::Client::MAX_CHANNELS = USR_CHAN_LIMIT;

std::set<char> const irc::Client::USER_MODES = utils::array_to_set(
	(char const[]){
		USR_MODE_I,
		USR_MODE_W,
		USR_MODE_O},
	3);

int irc::Client::unique = 0;

bool irc::Client::isNotInNicknameFormat(char const &c)
{
	static std::string const special = "[]\\`_^{|}";

	return (c != '-' && special.find(c) == std::string::npos && !std::isalnum(c, Message::LOCALE));
}

irc::Client::Client(int const &connfd, std::string const &hostaddr)
	: uid(++irc::Client::unique),
	  connfd(connfd),
	  hostaddr(hostaddr),
	  registered(false),
	  messages(),
	  password(),
	  nickname(1, '*'),
	  username(),
	  realname(),
	  modes(),
	  channels()
{
	this->nickname.reserve(irc::Client::NICK_MAX_LEN);

	return;
}

irc::Client::Client(Client const &src)
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

irc::Client::~Client(void) throw() { return; }

bool irc::Client::operator==(Client const &rhs) const { return (this->connfd == rhs.connfd); }

bool irc::Client::operator!=(Client const &rhs) const { return (!(*this == rhs)); }

int const &irc::Client::getSocket(void) const throw() { return (this->connfd); }

std::string const &irc::Client::getHostaddr(void) const throw() { return (this->hostaddr); }

bool const &irc::Client::isRegistered(void) const throw() { return (this->registered); }

std::queue<irc::Message> const &irc::Client::getMessages(void) const throw() { return (this->messages); }

std::string const &irc::Client::getPassword(void) const throw() { return (this->password); }

std::string const &irc::Client::getNickname(void) const throw() { return (this->nickname); }

std::string const &irc::Client::getUsername(void) const throw() { return (this->username); }

std::string const &irc::Client::getRealname(void) const throw() { return (this->realname); }

std::set<char> const &irc::Client::getModes(void) const throw() { return (this->modes); }

std::string irc::Client::userId(void) const
{
	std::stringstream o;

	if (this->username.empty())
		o << "UID" << std::setfill('0') << std::setw(7) << this->uid;
	else
		o << this->username;

	o << '@' << this->hostaddr;

	return (o.str());
}

std::string irc::Client::str(void) const
{
	std::stringstream o;

	if (!this->nickname.empty())
		o << this->nickname << '!';

	o << this->userId();

	return (o.str());
}

void irc::Client::produce(Message const &message)
{
	this->messages.push(message);

	return;
}

irc::Message irc::Client::consume(void)
{
	Message message(this->messages.front());

	this->messages.pop();

	return (message);
}

void irc::Client::setPassword(std::string const &password)
{
	try
	{
		if (this->registered)
			throw std::runtime_error("std::runtime_error: client must not set password once registered");
		else if (password.find_first_of("\0\r\n", 0, 3) != std::string::npos)
			throw std::domain_error("std::domain_error: password must have any character except: NUL, CR, LF");

		this->password = password;
	}
	catch (std::exception const &e)
	{
		throw std::runtime_error("irc::Client::setPassword: " + std::string(e.what()));
	}

	return;
}

void irc::Client::setNickname(std::string const &nickname)
{
	try
	{
		if (nickname.at(0) == '-')
			throw std::invalid_argument("std::invalid_argument: nickname must not begin with a hyphen character");
		else if (std::isdigit(nickname.at(0), Message::LOCALE))
			throw std::invalid_argument("std::invalid_argument: nickname must not begin with a digit character");
		else if (nickname.length() > irc::Client::NICK_MAX_LEN)
			throw std::length_error("std::length_error: " + utils::to_string(nickname.length()) + ": nickname must not have more than " + utils::to_string(irc::Client::NICK_MAX_LEN) + " characters");
		else if (std::find_if(nickname.begin(), nickname.end(), irc::Client::isNotInNicknameFormat) != nickname.end())
			throw std::domain_error("std::domain_error: nickname must have only alphanumeric and special characters: []\\`_^{|}");

		this->nickname = nickname;

		if (!this->registered)
			this->registered = !this->username.empty();
	}
	catch (std::exception const &e)
	{
		throw std::runtime_error("irc::Client::setNickname: " + std::string(e.what()));
	}

	return;
}

void irc::Client::setUsername(std::string const &username)
{
	try
	{
		if (this->registered)
			throw std::runtime_error("std::runtime_error: client must not set username once registered");
		else if (username.find_first_of("\0\r\n @", 0, 5) != std::string::npos)
			throw std::domain_error("std::domain_error: username must have any character except: NUL, CR, LF, SPACE, @");

		this->username = username;
		this->registered = this->nickname.compare("*");
	}
	catch (std::exception const &e)
	{
		throw std::runtime_error("irc::Client::setUsername: " + std::string(e.what()));
	}

	return;
}

void irc::Client::setRealname(std::string const &realname)
{
	try
	{
		if (this->registered)
			throw std::runtime_error("std::runtime_error: client must not set realname once registered");
		else if (realname.find_first_of("\0\r\n", 0, 3) != std::string::npos)
			throw std::domain_error("std::domain_error: realname must have any character except: NUL, CR, LF");

		this->realname = realname;
	}
	catch (std::exception const &e)
	{
		throw std::runtime_error("irc::Client::setRealname: " + std::string(e.what()));
	}

	return;
}

void irc::Client::addMode(char const &mode)
{
	if (irc::Client::USER_MODES.find(mode) == irc::Client::USER_MODES.end())
		throw std::domain_error("irc::Client::addMode: std::domain_error: " + std::string(1, mode) + ": mode must be any supported user mode: " + utils::sequence_to_string(irc::Client::USER_MODES, ", "));

	this->modes.insert(mode);

	return;
}

void irc::Client::removeMode(char const &mode)
{
	if (irc::Client::USER_MODES.find(mode) == irc::Client::USER_MODES.end())
		throw std::domain_error("irc::Client::addMode: std::domain_error: " + std::string(1, mode) + ": mode must any supported user mode: " + utils::sequence_to_string(irc::Client::USER_MODES, ", "));

	this->modes.erase(mode);

	return;
}
