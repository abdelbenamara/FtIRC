/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/25 22:31:01 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/04 01:07:52 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __CHANNEL_HPP__
#define __CHANNEL_HPP__

#include <ctime>
#include <iterator>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "Client.hpp"
#include "Message.hpp"
#include "Server.hpp"
#include "utils.hpp"

#define CHAN_MODE_i 'i'
#define CHAN_MODE_k 'k'
#define CHAN_MODE_l 'l'
#define CHAN_MODE_o 'o'
#define CHAN_MODE_t 't'

namespace irc
{
	class Channel
	{
	public:
		typedef bool (*t_client_ptr_comp)(Client const *const &,
										  Client const *const &);
		typedef std::set<Client const *, t_client_ptr_comp> t_clients;
		typedef std::pair<Client const *, time_t> t_topicwhotime;

		Channel(std::string const &name);
		Channel(Channel const &src);

		virtual ~Channel() throw();

		time_t const &getStart(void) const throw();
		std::string const &getName(void) const throw();
		t_clients const &getMembers(void) const throw();
		t_clients const &getOperators(void) const throw();
		std::string const &getTopic(void) const throw();
		std::string const &getKey(void) const throw();
		t_topicwhotime const &getTopicWhoTime(void) const throw();
		std::set<char> const &getModes(void) const throw();
		std::size_t const &getLimit(void) const throw();

		std::string namesList(Client const &receiver) const;
		void publish(Message const &message,
					 Client const *const sender = NULL) const;

		void addMember(Client &client);
		void removeMember(Client &client);
		void addOperator(Client const &client);
		void removeOperator(Client const &client);
		void setTopic(std::string const &topic);
		void setKey(std::string const &key);
		void setTopicWhoTime(Client const &client);
		void addMode(char const &mode);
		void removeMode(char const &mode);
		void setLimit(std::size_t const &limit);

	private:
		static bool client_ptr_less(Client const *const &lhs,
									Client const *const &rhs);
		static bool is_not_key(char const &c);

		time_t const start;
		std::string const name;

		t_clients members;
		t_clients operators;
		std::string topic, key;
		t_topicwhotime topicwhotime;
		std::set<char> modes;
		std::size_t limit;

		Channel(void);						 /* = delete (C++11) */
		Channel &operator=(Channel const &); /* = delete (C++11) */

		std::string prefixedName(Client const &client) const;
	};
} // namespace irc

#endif
