/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/12 19:45:21 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/07 18:02:25 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"
#include "Client.hpp"
#include "Server.hpp"

std::string const irc::Client::SPECIAL_CHARS("[]\\`_^{|}");

int irc::Client::unique(0);

std::size_t irc::Client::getUniques(void) { return (Client::unique); }

bool irc::Client::channel_ptr_less(Channel const *const &lhs,
								   Channel const *const &rhs)
{
	return (utils::i_string_less(lhs->getName(), rhs->getName()));
}

bool irc::Client::is_not_nick(char const &c)
{
	return (c != '-' && SPECIAL_CHARS.find(c) == std::string::npos &&
			!utils::is_alnum(c));
}

irc::Client::Client(utils::t_sockinfo const &sockinfo)
	: uid(++Client::unique),
	  sockinfo(sockinfo),
	  registered(false),
	  messages(),
	  password(),
	  nickname(1, '*'),
	  username(),
	  realname(),
	  modes(),
	  channels(&Client::channel_ptr_less),
	  invites(&Client::channel_ptr_less)
{
	this->nickname.reserve(Server::instance().getSizeProperty(PRP_NICKLEN));

	return;
}

irc::Client::Client(Client const &src)
	: uid(src.uid),
	  sockinfo(src.sockinfo),
	  registered(src.registered),
	  messages(src.messages),
	  password(src.password),
	  nickname(src.nickname),
	  username(src.username),
	  realname(src.realname),
	  modes(src.modes),
	  channels(src.channels),
	  invites(src.invites) { return; }

irc::Client::Client(void const *const, Client const &src)
	: uid(src.uid),
	  sockinfo(-1, src.sockinfo.second),
	  registered(src.registered),
	  messages(src.messages),
	  password(src.password),
	  nickname(src.nickname),
	  username(src.username),
	  realname(src.realname),
	  modes(src.modes),
	  channels(src.channels),
	  invites(src.invites) { return; }

irc::Client::~Client(void) throw() { return; }

irc::utils::t_sockinfo const &irc::Client::getSocket(
	void) const throw() { return (this->sockinfo); }

bool const &irc::Client::isRegistered(
	void) const throw() { return (this->registered); }

std::queue<irc::Message> const &irc::Client::getMessages(
	void) const throw() { return (this->messages); }

std::string const &irc::Client::getPassword(
	void) const throw() { return (this->password); }

std::string const &irc::Client::getNickname(
	void) const throw() { return (this->nickname); }

std::string const &irc::Client::getUsername(
	void) const throw() { return (this->username); }

std::string const &irc::Client::getRealname(
	void) const throw() { return (this->realname); }

std::set<char> const &irc::Client::getModes(
	void) const throw() { return (this->modes); }

irc::Client::t_channels const &irc::Client::getChannels(
	void) const throw() { return (this->channels); }

irc::Client::t_channels const &irc::Client::getInvites(
	void) const throw() { return (this->invites); }

std::string irc::Client::userId(void) const
{
	std::ostringstream out;

	if (this->username.empty())
		out << "UID" << std::setfill('0') << std::setw(7) << this->uid;
	else
		out << this->username;

	out << '@' << utils::get_haddr(this->sockinfo);

	return (out.str());
}

std::string irc::Client::str(void) const
{
	std::ostringstream out;

	if (!this->nickname.empty())
		out << this->nickname << '!';

	out << this->userId();

	return (out.str());
}

void irc::Client::publish(Message const &message) const
{
	Server::t_clients::const_iterator cit(
		Server::instance().getClients().begin());

	for (; cit != Server::instance().getClients().end(); ++cit)
	{
		if (&cit->second == this)
			continue;
		else if (std::find_first_of(this->channels.begin(),
									this->channels.end(),
									cit->second.getChannels().begin(),
									cit->second.getChannels().end()) !=
				 this->channels.end())
			Server::instance().produce(cit->second, message);
	}
}

void irc::Client::apply(Message const &) const { return; }

void irc::Client::produce(Message const &message)
{
	return (this->messages.push(message));
}

irc::Message irc::Client::consume(void)
{
	Message message(this->messages.front());

	this->messages.pop();

	return (message);
}

void irc::Client::setPassword(std::string const &password)
{
	if (this->registered)
		throw std::runtime_error(
			"client must not set password once registered");
	else if (password.find_first_of("\0\r\n", 0, 3) != std::string::npos)
		throw std::domain_error(
			"client password must have any character except: NUL, CR, LF");

	this->password = password;

	return;
}

void irc::Client::setNickname(std::string const &nickname)
{
	if (nickname.empty())
		throw std::length_error("client nickname must not be empty");
	else if (nickname.at(0) == '-')
		throw std::invalid_argument(
			"client nickname must not begin with a hyphen character");
	else if (utils::is_digit(nickname.at(0)))
		throw std::invalid_argument(
			"client nickname must not begin with a digit character");
	else if (nickname.length() >
			 Server::instance().getSizeProperty(PRP_NICKLEN))
		throw std::length_error(
			"client nickname length: " + utils::to_string(nickname.length()) +
			": nickname must not have more than " +
			utils::to_string(Server::instance().getSizeProperty(PRP_NICKLEN)) +
			" characters");
	else if (std::find_if(nickname.begin(),
						  nickname.end(),
						  Client::is_not_nick) != nickname.end())
		throw std::domain_error("client nickname must have only hyphen"
								", alphanumeric and special characters: " +
								SPECIAL_CHARS);

	this->nickname = nickname;

	if (!this->registered)
		this->registered = !this->username.empty();

	return;
}

void irc::Client::clearNickname(void)
{
	this->nickname = "*";

	return;
}

void irc::Client::setUsername(std::string const &username)
{
	if (this->registered)
		throw std::runtime_error(
			"client must not set username once registered");
	else if (username.empty())
		throw std::length_error("client username must not be empty");

	this->username = username.substr(
		0,
		Server::instance().getSizeProperty(PRP_USERLEN));
	this->registered = this->nickname.compare("*");

	return;
}

void irc::Client::setRealname(std::string const &realname)
{
	if (this->registered)
		throw std::runtime_error(
			"client must not set realname once registered");
	else if (realname.empty())
		throw std::length_error("client realname must not be empty");
	else if (realname.find_first_of("\0\r\n", 0, 3) != std::string::npos)
		throw std::domain_error(
			"client realname must have any character except: NUL, CR, LF");

	this->realname = realname;

	return;
}

void irc::Client::addMode(char const &mode)
{
	if (Server::instance().getTextProperty(PRP_USERMODESINFO).find(mode) ==
		std::string::npos)
		throw std::domain_error(
			std::string(1, mode) +
			": client mode must be any supported user mode: " +
			Server::instance().getTextProperty(PRP_USERMODESINFO));

	this->modes.insert(mode);

	return;
}

void irc::Client::removeMode(char const &mode)
{
	if (Server::instance().getTextProperty(PRP_USERMODESINFO).find(mode) ==
		std::string::npos)
		throw std::domain_error(
			std::string(1, mode) +
			": client mode must any supported user mode: " +
			Server::instance().getTextProperty(PRP_USERMODESINFO));

	this->modes.erase(mode);

	return;
}

void irc::Client::joinChannel(Channel const &channel)
{
	if (!this->registered)
		throw std::runtime_error(
			"client must be registered to join a channel");
	else if (this->channels.size() ==
			 Server::instance().getSizeProperty(PRP_CHANLIMIT))
		throw std::length_error(
			"client must not be a member of more than " +
			utils::to_string(
				Server::instance().getSizeProperty(PRP_CHANLIMIT)) +
			" channels");

	this->channels.insert(&channel);
	this->invites.erase(&channel);

	return;
}

void irc::Client::leaveChannel(Channel const &channel)
{
	t_channels::const_iterator cit;

	if (!this->registered)
		throw std::runtime_error(
			"client must be registered to leave a channel");

	cit = this->channels.find(&channel);

	if (cit == this->channels.end())
		throw std::out_of_range(
			channel.getName() +
			": client must be a channel member to perform this action");

	return (this->channels.erase(cit));
}

void irc::Client::addInvite(Channel const &channel)
{
	if (!this->registered)
		throw std::runtime_error(
			"client must be registered to be invited to a channel");

	this->invites.insert(&channel);

	return;
}
