/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/25 22:31:01 by abenamar          #+#    #+#             */
/*   Updated: 2024/11/09 15:34:42 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __CHANNEL_HPP__
#define __CHANNEL_HPP__

#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Client.hpp"
#include "Config.hpp"
#include "Message.hpp"
#include "Server.hpp"
#include "utils.hpp"

namespace irc
{
	class Client;

	class Channel
	{
	public:
		Channel(std::string const &name, Client &first);
		Channel(Channel const &src);

		virtual ~Channel() throw();

		std::string const &getName(void) const throw();
		std::string const &getTopic(void) const throw();
		std::string const &getKey(void) const throw();
		std::set<Client> const &getMembers(void) const throw();
		std::set<std::string, utils::t_istringcomp> const &getOperators(void) const throw();
		std::set<char> const &getModes(void) const throw();
		std::size_t const &getLimit(void) const throw();

		std::string users(void) const;
		void publish(Client const &sender, Message const &message) const;

		void setTopic(std::string const &topic);
		void setKey(std::string const &key);
		void addMember(Client &client);
		void removeMember(Client &client);
		void addOperator(Client const &client);
		void removeOperator(Client const &client);
		void addMode(char const &mode);
		void removeMode(char const &mode);
		void setLimit(std::size_t const &limit);

	private:
		std::string const name;

		std::string topic, key;
		std::set<Client> members;
		std::set<std::string, utils::t_istringcomp> operators;
		std::set<char> modes;
		std::size_t limit;

		Channel(void);						 /* = delete (C++11) */
		Channel &operator=(Channel const &); /* = delete (C++11) */
	};
} // namespace irc

#endif
