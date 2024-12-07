/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/11 20:21:22 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/07 18:01:37 by abenamar         ###   ########.fr       */
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
#include "utils.hpp"

#define USR_MODE_i 'i'
#define USR_MODE_o 'o'
#define USR_MODE_w 'w'

namespace irc
{
	class Channel;

	class Client
	{
	public:
		typedef bool (*t_channel_ptr_comp)(Channel const *const &,
										   Channel const *const &);
		typedef std::set<Channel const *, t_channel_ptr_comp> t_channels;

		static std::size_t getUniques(void);

		Client(utils::t_sockinfo const &sockinfo);
		Client(Client const &src);
		Client(void const *const, Client const &src);

		virtual ~Client(void) throw();

		utils::t_sockinfo const &getSocket(void) const throw();
		bool const &isRegistered(void) const throw();
		std::queue<Message> const &getMessages(void) const throw();
		std::string const &getPassword(void) const throw();
		std::string const &getNickname(void) const throw();
		std::string const &getUsername(void) const throw();
		std::string const &getRealname(void) const throw();
		std::set<char> const &getModes(void) const throw();
		t_channels const &getChannels(void) const throw();
		t_channels const &getInvites(void) const throw();

		std::string userId(void) const;
		std::string str(void) const;
		void publish(Message const &message) const;
		virtual void apply(Message const &) const;

		void produce(Message const &message);
		Message consume(void);
		void setPassword(std::string const &password);
		void setNickname(std::string const &nickname);
		void clearNickname(void);
		void setUsername(std::string const &username);
		void setRealname(std::string const &realname);
		void addMode(char const &mode);
		void removeMode(char const &mode);
		void joinChannel(Channel const &channel);
		void leaveChannel(Channel const &channel);
		void addInvite(Channel const &channel);

	private:
		static std::string const SPECIAL_CHARS;

		static int unique;

		static bool channel_ptr_less(Channel const *const &lhs,
									 Channel const *const &rhs);
		static bool is_not_nick(char const &c);

		int const uid;
		utils::t_sockinfo const sockinfo;

		bool registered;
		std::queue<Message> messages;
		std::string password, nickname, username, realname;
		std::set<char> modes;
		t_channels channels;
		t_channels invites;

		Client(void);					   /* = delete (C++11) */
		Client &operator=(Client const &); /* = delete (C++11) */
	};
} // namespace irc

#endif
