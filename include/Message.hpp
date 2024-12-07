/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/23 15:14:22 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/07 03:04:36 by abenamar         ###   ########.fr       */
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

            std::string const &getPrefix(void) const throw();
            std::string const &getCommand(void) const throw();
            std::vector<std::string> const &getParameters(void) const throw();

            Builder &withoutPrefix(void);
            Builder &withPrefix(std::string const &prefix);
            Builder &withCommand(std::string const &command);
            Builder &addParameter(std::string const &parameter);
            Builder &addParameter(char const &parameter);
            Builder &withParameters(std::vector<std::string> const &parameters);
            Builder &withParameter(std::string const &parameter);

            Message build(void);

        private:
            bool trailing;
            std::string prefix, command;
            std::vector<std::string> parameters;

            Builder(Builder const &);            /* = delete (C++11) */
            Builder &operator=(Builder const &); /* = delete (C++11) */
        };

        static std::string const CRLF;
        static std::size_t const NUM_RPL_LEN, PARAMETERS_SIZE;

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

        Message(std::string const &prefix,
                std::string const &command,
                std::vector<std::string> const &parameters);

        Message(void);                       /* = delete (C++11) */
        Message &operator=(Message const &); /* = delete (C++11) */
    };
} // namespace irc

std::ostream &operator<<(std::ostream &o, irc::Message const &i);

#endif
