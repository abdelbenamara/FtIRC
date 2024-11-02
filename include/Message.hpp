/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/23 15:14:22 by abenamar          #+#    #+#             */
/*   Updated: 2024/11/01 17:48:25 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __MESSAGE_HPP__
#define __MESSAGE_HPP__

#include <algorithm>
#include <iostream>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "utils.hpp"

#define MSG_LINE_LEN ((unsigned int)512)

namespace irc
{
    class Message
    {
    public:
        class Builder
        {
        public:
            Builder(void);

            virtual ~Builder(void) throw();

            Builder &withPrefix(std::string const &prefix);
            Builder &withCommand(std::string const &command);
            Builder &addParameter(std::string const &parameter);
            Builder &withParameters(std::vector<std::string> const &parameters);

            Message build(void);

        private:
            static bool isNotInCommandFormat(char const &c);

            bool trailing;
            std::string prefix, command;
            std::vector<std::string> parameters;

            Builder(Builder const &);            /* = delete (C++11) */
            Builder &operator=(Builder const &); /* = delete (C++11) */
        };

        static std::locale const LOCALE;
        static std::string const CRLF;
        static std::size_t const MAX_LEN, MAX_CHARS, NUM_RPL_LEN, PARAMS_MAX_LEN;

        static Message from(std::string const &input);

        Message(Message const &src);

        virtual ~Message(void);

        std::string const &getPrefix(void) const throw();
        std::string const &getCommand(void) const throw();
        std::vector<std::string> const &getParameters(void) const throw();

        std::string str(void) const;

    private:
        std::string const prefix, command;
        std::vector<std::string> const parameters;

        Message(std::string const &prefix, std::string const &command, std::vector<std::string> const &parameters);

        Message(void);                       /* = delete (C++11) */
        Message &operator=(Message const &); /* = delete (C++11) */
    };
} // namespace irc

std::ostream &operator<<(std::ostream &o, irc::Message const &i);

#endif
