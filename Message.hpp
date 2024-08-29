/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karimasadykova <karimasadykova@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/23 15:14:22 by abenamar          #+#    #+#             */
/*   Updated: 2024/08/29 22:44:14 by karimasadyk      ###   ########.fr       */
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
		void withPrefix(size_t &pos, const std::string& message);
        void withCommand(size_t &pos, const std::string& message);
        void withParams(size_t &pos, const std::string& message);
        Message build(void);

    private:
        std::string input, prefix, command;
        std::string user, host, name; // Added to parse prefix details
        std::vector<std::string> parameters;

        Builder(Builder const & /* src */);

        Builder &operator=(Builder const & /* rhs */) throw();

        // Helper functions
        bool checkServ(const std::string& name);
        bool isspecial(char c);
        bool checkNick(const std::string& name);
        bool checkUser(const std::string& name);
        bool isValidCmd(const std::string& command);
        bool isNospcrlfcl(char c);
        bool checkMiddle(const std::string& middle);
        bool checkTrailing(const std::string& trailing);
        bool checkParams(const std::vector<std::string>& params);
    };

    static std::string const CRLF;
    static std::size_t const MAXSIZE, MAXCHARS;
    static char BUFFER[];

    Message(Message const &src);
    virtual ~Message(void);

    std::string const &getInput(void) const throw();
    std::string const &getPrefix(void) const throw();
    std::string const &getCommand(void) const throw();
    std::vector<std::string> const &getParameters(void) const throw();
    std::string const &getUser(void) const throw();
    std::string const &getHost(void) const throw();
    std::string const &getName(void) const throw();

private:
    std::string const input, prefix, command;
    std::string const user, host, name; // prefix part, name is the nickname/servername
    std::vector<std::string> const parameters;

    Message(void);
    Message(std::string const &input, std::string const &prefix, std::string const &command,
			std::string const &user, std::string const &host, std::string const &name,
			std::vector<std::string> const &parameters);

    Message &operator=(Message const & /* rhs */) throw();
};

#endif
