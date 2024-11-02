/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/25 22:31:01 by abenamar          #+#    #+#             */
/*   Updated: 2024/11/02 14:53:45 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __CHANNEL_HPP__
#define __CHANNEL_HPP__

#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "Client.hpp"
#include "Server.hpp"
#include "utils.hpp"

#define CHAN_NAME_LEN ((std::size_t)50)
#define CHAN_KEY_LEN ((std::size_t)32)

#define CHAN_TYPE_LOCAL '&'
#define CHAN_TYPE_STANDARD '#'

#define CHAN_MODE_I 'i'
#define CHAN_MODE_T 't'
#define CHAN_MODE_K 'k'
#define CHAN_MODE_O 'o'
#define CHAN_MODE_L 'l'

namespace irc
{
	class Client;

	class Channel
	{
	public:
		static std::size_t const NANE_MAX_LEN, KEY_MAX_LEN, MAX_MEMBERS;
		static std::set<char> const TYPES, MODES;

		Channel(std::string const &name);
		Channel(Channel const &src);

		virtual ~Channel() throw();

		std::string const &getName(void) const throw();
		bool const &isExclusive(void) const throw();
		std::string const &getTopic(void) const throw();
		std::string const &getKey(void) const throw();
		std::map<std::string, Client> const &getMembers(void) const throw();
		std::set<std::string> const &getOperators(void) const throw();

		void setExclusive(bool const &exclusive);
		void setTopic(std::string const &topic);
		void setKey(std::string const &key);
		void addMember(Client const &client);
		void removeMember(Client const &client);
		void addOperator(Client const &client);
		void removeOperator(Client const &client);

	private:
		std::string const name;

		bool exclusive;
		std::string topic, key;
		std::map<std::string, Client> members;
		std::set<std::string> operators;
		std::size_t limit;

		Channel(void);						 /* = delete (C++11) */
		Channel &operator=(Channel const &); /* = delete (C++11) */
	};
} // namespace irc

#endif
