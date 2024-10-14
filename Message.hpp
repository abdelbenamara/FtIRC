/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejankovs <ejankovs@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/23 15:14:22 by abenamar          #+#    #+#             */
/*   Updated: 2024/10/14 16:50:53 by ejankovs         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __MESSAGE_HPP__
#define __MESSAGE_HPP__

#include <algorithm>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>

class Message
{
public:
    class Builder
    {
    public:
        Builder(void);
        virtual ~Builder(void) throw();

        Builder &withInput(std::string const &input);
        Message build(void);

    private:
        std::string input, prefix, command;
        std::vector<std::string> parameters;

        Builder(Builder const & /* src */);

        Builder &operator=(Builder const & /* rhs */) throw();

        void setPrefix(std::string const &prefix);
        void setCommand(std::string const &command);
    };

    static std::locale const LOCALE;
    static std::string const CRLF;
    static std::size_t const MAXSIZE, MAXCHARS;
    static char BUFFER[];
    static bool isNotInServernameFormat(char const &c);
    static bool isNotInNicknameFormat(char const &c);
    static bool isNotInCommandFormat(char const &c);

    Message(Message const &src);
    virtual ~Message(void);

    std::string const &getInput(void) const throw();
    std::string const &getPrefix(void) const throw();
    std::string const &getCommand(void) const throw();
    std::vector<std::string> const &getParameters(void) const throw();
    std::string toString(void) const;

private:
    std::string const input, prefix, command;
    std::vector<std::string> const parameters;

    Message(void);
    Message(std::string const &input, std::string const &prefix, std::string const &command, std::vector<std::string> const &parameters);

    Message &operator=(Message const & /* rhs */) throw();
};

#endif
