/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/11 20:21:22 by abenamar          #+#    #+#             */
/*   Updated: 2024/11/01 19:53:38 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __CLIENT_HPP__
#define __CLIENT_HPP__

#include <iomanip>
#include <queue>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>

#include "Message.hpp"
#include "Server.hpp"
#include "utils.hpp"

#define USR_NICK_LEN ((std::size_t)9)
#define USR_CHAN_LIMIT ((std::size_t)10)

#define USR_MODE_I 'i'
#define USR_MODE_W 'w'
#define USR_MODE_O 'o'

namespace irc
{
	class Client
	{
	public:
		static std::size_t const NICK_MAX_LEN, MAX_CHANNELS;
		static std::set<char> const USER_MODES;

		Client(int const &connfd, std::string const &hostaddr);
		Client(Client const &src);

		virtual ~Client(void) throw();

		bool operator==(Client const &rhs) const;
		bool operator!=(Client const &rhs) const;

		int const &getSocket(void) const throw();
		std::string const &getHostaddr(void) const throw();
		bool const &isRegistered(void) const throw();
		std::queue<Message> const &getMessages(void) const throw();
		std::string const &getPassword(void) const throw();
		std::string const &getNickname(void) const throw();
		std::string const &getUsername(void) const throw();
		std::string const &getRealname(void) const throw();
		std::set<char> const &getModes(void) const throw();

		std::string userId(void) const;
		std::string str(void) const;

		void produce(Message const &message);
		Message consume(void);
		void setPassword(std::string const &password);
		void setNickname(std::string const &nickname);
		void setUsername(std::string const &username);
		void setRealname(std::string const &realname);
		void addMode(char const &mode);
		void removeMode(char const &mode);

	private:
		static int unique;

		static bool isNotInNicknameFormat(char const &c);

		int const uid, connfd;
		std::string const hostaddr;

		bool registered;
		std::queue<Message> messages;
		std::string password, nickname, username, realname;
		std::set<char> modes;
		std::set<std::string> channels;

		Client(void);					   /* = delete (C++11) */
		Client &operator=(Client const &); /* = delete (C++11) */
	};
} // namespace irc

#endif
