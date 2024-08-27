/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/23 15:14:22 by abenamar          #+#    #+#             */
/*   Updated: 2024/08/27 11:49:06 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __MESSAGE_HPP__
#define __MESSAGE_HPP__

#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

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
    };

    static std::string const CRLF;
    static std::size_t const MAXSIZE, MAXCHARS;
    static char BUFFER[];

    Message(Message const &src);
    virtual ~Message(void);

    std::string const &getInput(void) const throw();

private:
    std::string const input, prefix, command;
    std::vector<std::string> const parameters;

    Message(void);
    Message(std::string const &input, std::string const &prefix, std::string const &command, std::vector<std::string> const &parameters);

    Message &operator=(Message const & /* rhs */) throw();
};

#endif
