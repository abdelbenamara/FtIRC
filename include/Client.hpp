/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/11 20:21:22 by abenamar          #+#    #+#             */
/*   Updated: 2024/11/09 15:34:58 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __CLIENT_HPP__
#define __CLIENT_HPP__

#include <iomanip>
#include <locale>
#include <queue>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>

#include "Channel.hpp"
#include "Config.hpp"
#include "Message.hpp"
#include "utils.hpp"

namespace irc
{
	class Channel;

	class Client
	{
	public:
		Client(int const &connfd, std::string const &hostaddr);
		Client(Client const &src);

		virtual ~Client(void) throw();

		bool operator==(Client const &rhs) const;
		bool operator!=(Client const &rhs) const;
		bool operator<(Client const &rhs) const;
		bool operator>(Client const &rhs) const;
		bool operator<=(Client const &rhs) const;
		bool operator>=(Client const &rhs) const;

		int const &getSocket(void) const throw();
		std::string const &getHostaddr(void) const throw();
		bool const &isRegistered(void) const throw();
		std::queue<Message> const &getMessages(void) const throw();
		std::string const &getPassword(void) const throw();
		std::string const &getNickname(void) const throw();
		std::string const &getUsername(void) const throw();
		std::string const &getRealname(void) const throw();
		std::set<char> const &getModes(void) const throw();
		std::set<std::string, utils::t_istringcomp> const &getChannels(void) const throw();

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
		void joinChannel(Channel const &channel);
		void leaveChannel(Channel const &channel);

	private:
		static int unique;

		static bool isNotInNicknameFormat(char const &c);

		int const uid, connfd;
		std::string const hostaddr;

		bool registered;
		std::queue<Message> messages;
		std::string password, nickname, username, realname;
		std::set<char> modes;
		std::set<std::string, utils::t_istringcomp> channels;

		Client(void);					   /* = delete (C++11) */
		Client &operator=(Client const &); /* = delete (C++11) */
	};
} // namespace irc

#endif
