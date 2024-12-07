/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GameBot.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/07 02:34:10 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/07 06:03:20 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __NEWSBOT_HPP__
#define __NEWSBOT_HPP__

#include <algorithm>
#include <string>
#include <vector>

#include "Client.hpp"
#include "utils.hpp"

namespace irc
{
    class GameBot : public Client
    {
    public:
        GameBot(utils::t_sockinfo const &sockinfo);
        GameBot(GameBot const &);
        virtual ~GameBot(void) throw();

        virtual void apply(Message const &message) const;

    private:
        GameBot(void);                       /* = delete (C++11) */
        GameBot &operator=(GameBot const &); /* = delete (C++11) */
    };
} // namespace irc

#endif
