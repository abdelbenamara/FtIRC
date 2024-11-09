/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/12 19:45:21 by abenamar          #+#    #+#             */
/*   Updated: 2024/11/09 15:37:58 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

int irc::Client::unique = 0;

bool irc::Client::isNotInNicknameFormat(char const &c)
{
	static std::string const special = "[]\\`_^{|}";

	return (c != '-' && special.find(c) == std::string::npos && !std::isalnum(c, std::locale::classic()));
}

irc::Client::Client(int const &connfd, std::string const &hostaddr)
	: uid(++Client::unique),
	  connfd(connfd),
	  hostaddr(hostaddr),
	  registered(false),
	  messages(),
	  password(),
	  nickname(1, '*'),
	  username(),
	  realname(),
	  modes(),
	  channels(&utils::istringcomp)
{
	this->nickname.reserve(Config::getInstance().getSize(PRP_NICKLEN));

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
	  modes(src.modes),
	  channels(src.channels) { return; }

irc::Client::~Client(void) throw() { return; }

bool irc::Client::operator==(Client const &rhs) const { return (this->connfd == rhs.connfd); }

bool irc::Client::operator!=(Client const &rhs) const { return (!(*this == rhs)); }

bool irc::Client::operator<(Client const &rhs) const { return (this->connfd < rhs.connfd); }

bool irc::Client::operator>(Client const &rhs) const { return (rhs < *this); }

bool irc::Client::operator<=(Client const &rhs) const { return (!(*this > rhs)); }

bool irc::Client::operator>=(Client const &rhs) const { return (!(*this < rhs)); }

int const &irc::Client::getSocket(void) const throw() { return (this->connfd); }

std::string const &irc::Client::getHostaddr(void) const throw() { return (this->hostaddr); }

bool const &irc::Client::isRegistered(void) const throw() { return (this->registered); }

std::queue<irc::Message> const &irc::Client::getMessages(void) const throw() { return (this->messages); }

std::string const &irc::Client::getPassword(void) const throw() { return (this->password); }

std::string const &irc::Client::getNickname(void) const throw() { return (this->nickname); }

std::string const &irc::Client::getUsername(void) const throw() { return (this->username); }

std::string const &irc::Client::getRealname(void) const throw() { return (this->realname); }

std::set<char> const &irc::Client::getModes(void) const throw() { return (this->modes); }

std::set<std::string, irc::utils::t_istringcomp> const &irc::Client::getChannels(void) const throw() { return (this->channels); }

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
try
{
	if (this->registered)
		throw std::runtime_error("std::runtime_error: client must not set password once registered");
	else if (password.find_first_of("\0\r\n", 0, 3) != std::string::npos)
		throw std::domain_error("std::domain_error: password must have any character except: NUL, CR, LF");

	this->password = password;

	return;
}
catch (std::exception const &e)
{
	throw std::runtime_error("irc::Client::setPassword: " + std::string(e.what()));
}

void irc::Client::setNickname(std::string const &nickname)
try
{
	if (nickname.at(0) == '-')
		throw std::invalid_argument("std::invalid_argument: nickname must not begin with a hyphen character");
	else if (std::isdigit(nickname.at(0), std::locale::classic()))
		throw std::invalid_argument("std::invalid_argument: nickname must not begin with a digit character");
	else if (nickname.length() > Config::getInstance().getSize(PRP_NICKLEN))
		throw std::length_error("std::length_error: " + utils::to_string(nickname.length()) + ": nickname must not have more than " + utils::to_string(Config::getInstance().getSize(PRP_NICKLEN)) + " characters");
	else if (std::find_if(nickname.begin(), nickname.end(), Client::isNotInNicknameFormat) != nickname.end())
		throw std::domain_error("std::domain_error: nickname must have only alphanumeric and special characters: []\\`_^{|}");

	this->nickname = nickname;

	if (!this->registered)
		this->registered = !this->username.empty();

	return;
}
catch (std::exception const &e)
{
	throw std::runtime_error("irc::Client::setNickname: " + std::string(e.what()));
}

void irc::Client::setUsername(std::string const &username)
try
{
	if (this->registered)
		throw std::runtime_error("std::runtime_error: client must not set username once registered");
	else if (username.empty())
		throw std::length_error("std::length_error: username must not be empty");
	else if (username.at(0) == ':' || username.at(0) == '$' || Config::getInstance().getText(PRP_CHANTYPES).find(username.at(0)) != std::string::npos)
		throw std::domain_error("std::domain_error: " + std::string(1, username.at(0)) + ":username must begin with any character except: COLON, $, " + Config::getInstance().getText(PRP_CHANTYPES));
	else if (username.find_first_of("\0\r\n ,*?!@", 0, 9) != std::string::npos)
		throw std::domain_error("std::domain_error: username must have any character except: NUL, CR, LF, SPACE, COMMA, *, ?, !, @");

	this->username = username;
	this->registered = this->nickname.compare("*");

	return;
}
catch (std::exception const &e)
{
	throw std::runtime_error("irc::Client::setUsername: " + std::string(e.what()));
}

void irc::Client::setRealname(std::string const &realname)
try
{
	if (this->registered)
		throw std::runtime_error("std::runtime_error: client must not set realname once registered");
	else if (realname.find_first_of("\0\r\n", 0, 3) != std::string::npos)
		throw std::domain_error("std::domain_error: realname must have any character except: NUL, CR, LF");

	this->realname = realname;

	return;
}
catch (std::exception const &e)
{
	throw std::runtime_error("irc::Client::setRealname: " + std::string(e.what()));
}

void irc::Client::addMode(char const &mode)
{
	if (Config::getInstance().getText(PRP_USERMODESINFO).find(mode) == std::string::npos)
		throw std::domain_error("irc::Client::addMode: std::domain_error: " + std::string(1, mode) + ": mode must be any supported user mode: " + Config::getInstance().getText(PRP_USERMODESINFO));

	this->modes.insert(mode);

	return;
}

void irc::Client::removeMode(char const &mode)
{
	if (Config::getInstance().getText(PRP_USERMODESINFO).find(mode) == std::string::npos)
		throw std::domain_error("irc::Client::addMode: std::domain_error: " + std::string(1, mode) + ": mode must any supported user mode: " + Config::getInstance().getText(PRP_USERMODESINFO));

	this->modes.erase(mode);

	return;
}

void irc::Client::joinChannel(Channel const &channel)
try
{
	if (!this->registered)
		throw std::runtime_error("std::runtime_error: client must be registered to perform this action");
	else if (this->channels.size() == Config::getInstance().getSize(PRP_CHANLIMIT))
		throw std::length_error("std::length_error: a client must not be a member of more than " + utils::to_string(Config::getInstance().getSize(PRP_CHANLIMIT)) + " channels");

	this->channels.insert(channel.getName());

	return;
}
catch (std::exception const &e)
{
	throw std::runtime_error("irc::Client::joinChannel: " + std::string(e.what()));
}

void irc::Client::leaveChannel(Channel const &channel)
try
{
	std::set<std::string, utils::t_istringcomp>::const_iterator cit;

	if (!this->registered)
		throw std::runtime_error("std::runtime_error: client must be registered to perform this action");

	cit = this->channels.find(channel.getName());

	if (cit == this->channels.end())
		throw std::out_of_range("std::out_of_range: " + channel.getName() + ": client must be a channel member to perform this action");

	this->channels.erase(cit);

	return;
}
catch (std::exception const &e)
{
	throw std::runtime_error("irc::Client::leaveChannel: " + std::string(e.what()));
}
