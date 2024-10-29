/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/25 22:31:01 by abenamar          #+#    #+#             */
/*   Updated: 2024/10/29 17:52:32 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __CHANNEL_HPP__
#define __CHANNEL_HPP__

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>
#include "Client.hpp"
#include "Server.hpp"
#include "Utils.hpp"

#define CHAN_USER_LIMIT ((unsigned int)32)
#define CHAN_TYPE_LOCAL '&'
#define CHAN_TYPE_STANDARD '#'
#define CHAN_TYPE_NO_MODES '+'
#define CHAN_MODE_I 'i'
#define CHAN_MODE_T 't'
#define CHAN_MODE_K 'k'
#define CHAN_MODE_O 'o'
#define CHAN_MODE_L 'l'

namespace irc
{
	class Channel
	{
	public:
		static unsigned int const USER_LIMT;
		static std::size_t const NANE_MAX_LEN;
		static std::set<char> const TYPES, MODES;

		Channel(std::string const &name);
		Channel(Channel const &src);

		virtual ~Channel() throw();

	private:
		static std::set<char> initTypes(void);
		static std::set<char> initModes(void);

		std::string const name;

		Channel(void);						 /* = delete (C++11) */
		Channel &operator=(Channel const &); /* = delete (C++11) */
	};
} // namespace irc

#endif
