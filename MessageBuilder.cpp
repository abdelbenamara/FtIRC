/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MessageBuilder.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karimasadykova <karimasadykova@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/23 15:49:24 by abenamar          #+#    #+#             */
/*   Updated: 2024/08/29 22:31:00 by karimasadyk      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Message.hpp"
#include <iostream>

Message::Builder::Builder(void) : input(), prefix(), command(), parameters() { return; }

Message::Builder::Builder(Builder const & /* src */) : input(), prefix(), command(), parameters() { return; }

Message::Builder::~Builder(void) throw() { return; }

Message::Builder &Message::Builder::operator=(Builder const & /* rhs */) throw() { return (*this); }

bool Message::Builder::checkServ(const std::string& name)
{
    if (name.empty() || name.length() > 255)
	{
        return false;
    }

    size_t start = 0;
    while (start < name.length())
	{
        size_t end = name.find('.', start);
        if (end == std::string::npos)
		{
            end = name.length();
        }
        if (end - start > 63 || end == start)
		{
            return false;
        }
        if (!std::isalnum(name[start]) || !std::isalnum(name[end - 1]))
		{
            return false;
        }
        start = end + 1;
    }

    return true;
}

bool Message::Builder::isspecial(char c)
{
	return c == '_' || c == '[' || c == ']' || c == '\\' || c == '`' || c == '^' || c == '{' || c == '}' || c == '|';
}

bool Message::Builder::checkNick(const std::string& name)
{
	if (name.empty() || name.length() > 9 || (!std::isalnum(name[0]) && !isspecial(name[0])))
	{
		return false;
	}
	
	for (size_t i = 0; i < name.length(); i++)
	{
		if (!std::isalnum(name[i]) && !isspecial(name[i]) && name[i] != '-')
		{
			return false;
		}
	}
	
	return true;
}

bool Message::Builder::checkUser(const std::string& name)
{
	if (name.length() < 1 || name.length() > 10)
	{
		return false;
	}
	
	for (size_t i = 0; i < name.length(); i++)
	{
		if (!std::isalnum(name[i]) && !isspecial(name[i]))
		{
			return false;
		}
	}
	
	return true;
}

void Message::Builder::withPrefix(size_t &pos, const std::string& message)
{
	if (pos < message.length() && message[pos] == ':')
	{
		pos++;
		size_t end = message.find(' ', pos);
		if (end == std::string::npos) {
			throw std::invalid_argument("Error: MessageBuilder.cpp::withPrefix: std::invalid_argument: No space after prefix");
		}
		this->prefix = message.substr(pos, end - pos);
		pos = end + 1;

		size_t isUser = this->prefix.find('!');
		size_t isHost = this->prefix.find('@');

		if ((isUser != std::string::npos && isHost == std::string::npos) || (isUser > isHost)) {
			throw std::invalid_argument("Error: MessageBuilder.cpp::withPrefix: std::invalid_argument: Invalid prefix structure");
		}

		if (isHost != std::string::npos)
		{
			if (isUser != std::string::npos)
			{
				this->name = this->prefix.substr(0, isUser);
				this->user = this->prefix.substr(isUser + 1, isHost - isUser - 1);
				this->host = this->prefix.substr(isHost + 1);
				if ((!checkServ(this->name) && !checkNick(this->name)) || !checkUser(this->user) || !checkServ(this->host))
				{
					throw std::invalid_argument("Error: MessageBuilder.cpp::withPrefix: std::invalid_argument: Invalid prefix content");
				}
			}
			else
			{
				this->name = this->prefix.substr(0, isHost);
				this->host = this->prefix.substr(isHost + 1);
				if ((!checkServ(this->name) && !checkNick(this->name)) || !checkServ(this->host))
				{
					throw std::invalid_argument("Error: MessageBuilder.cpp::withPrefix: std::invalid_argument: Invalid server or host in prefix");
				}
			}
		}
		else
		{
			if (!checkServ(this->prefix) && !checkNick(this->prefix))
			{
				throw std::invalid_argument("Error: MessageBuilder.cpp::withPrefix: std::invalid_argument: Invalid server or nickname");
			}
		}
	}
}

bool Message::Builder::isValidCmd(const std::string& command)
{
    static const char* validCmdsArray[] =
	{
        "PASS", "NICK", "USER", "QUIT", "JOIN", "MODE", "TOPIC", "INVITE", "KICK", "PRIVMSG",
        "pass", "nick", "user", "quit", "join", "mode", "topic", "invite", "kick", "privmsg"
    };

    std::vector<std::string> validCmds(validCmdsArray, validCmdsArray + sizeof(validCmdsArray) / sizeof(validCmdsArray[0]));

	return std::find(validCmds.begin(), validCmds.end(), command) != validCmds.end();
}

void Message::Builder::withCommand(size_t &pos, const std::string& message)
{
	// :nick!user@host PRIVMSG #channel :Hello, World!
	size_t end = message.find(' ', pos);
	if (end == std::string::npos)
	{
		end = message.length();
	}

	this->command = message.substr(pos, end - pos);
	pos = end;
	
	if (this->command.empty() || this->command.length() > 15)
	{
		throw std::invalid_argument("Error: MessageBuilder.cpp::withCommand: std::invalid_argument: Invalid command");
	}

	if (isdigit(this->command[0]))
	{
		if (this->command.length() != 3 || !isdigit(this->command[1]) || !isdigit(this->command[2]))
		{
			throw std::invalid_argument("Error: MessageBuilder.cpp::withCommand: std::invalid_argument: Invalid command");
		}
	}
	else
	{
		if (!isValidCmd(this->command)
			|| (this->command.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ") != std::string::npos
			&& this->command.find_first_not_of("abcdefghijklmnopqrstuvwxyz") != std::string::npos))
		{
			throw std::invalid_argument("Error: MessageBuilder.cpp::withCommand: std::invalid_argument: Invalid command");
		}
	}
}

// nospcrlfcl =  %x01-09 / %x0B-0C / %x0E-1F / %x21-39 / %x3B-FF
                    // ; any octet except NUL, CR, LF, " " and ":"
bool Message::Builder::isNospcrlfcl(char c)
{
    unsigned char asciiValue = static_cast<unsigned char>(c);

    // Check if the ASCII value falls within the nospcrlfcl ranges
	if ((asciiValue >= 0x01 && asciiValue <= 0x09) || 
            (asciiValue >= 0x0B && asciiValue <= 0x0C) ||  
            (asciiValue >= 0x0E && asciiValue <= 0x1F) ||  
            (asciiValue >= 0x21 && asciiValue <= 0x39) || 
            (asciiValue >= 0x3B))
	{
		return true;
	}
	return false;
                 
}

// middle     =  nospcrlfcl *( ":" / nospcrlfcl )
bool Message::Builder::checkMiddle(const std::string& middle)
{
	if (middle.empty() || !isNospcrlfcl(middle[0]))
	{
		return false;
	}
	for (size_t i = 0; i < middle.length(); i++)
	{
		if (!isNospcrlfcl(middle[i]) && middle[i] != ':')
		{
			return false;
		}
	}
	return true;
}

// trailing   =  *( ":" / " " / nospcrlfcl )
bool Message::Builder::checkTrailing(const std::string& trailing)
{
	for (size_t i = 0; i < trailing.length(); i++)
	{
		if (!isNospcrlfcl(trailing[i]) && trailing[i] != ' ' && trailing[i] != ':')
		{
			return false;
		}
	}
	return true;
}

bool Message::Builder::checkParams(const std::vector<std::string>& params)
{
	if (params[0][0] == ':')
	{
		return false;
	}
	for (size_t i = 0; i < params.size(); i++)
	{
		if (params[i][0] == ':')
		{
            if (!checkTrailing(params[i]))
			{
                return false;
            }
        }
		else if (!checkMiddle(params[i]))
		{
            return false;
        }
	}
	return true;
}

void Message::Builder::withParams(size_t &pos, const std::string& message)
{
	// params     =  *14( SPACE middle ) [ SPACE ":" trailing ]
    //            =/ 14( SPACE middle ) [ SPACE [ ":" ] trailing ]

    // middle     =  nospcrlfcl *( ":" / nospcrlfcl )
    // trailing   =  *( ":" / " " / nospcrlfcl )

	if (message[pos] == ' ')
	{
		// while (pos < message.length() && message[pos] == ' ')
		// {
		// 	pos++;
		// }

		size_t start = pos + 1;
		size_t nbParams = 0;

		// CMD PARAM1 PARAM2 ... PARAM14 :PARAM3
		while (nbParams < 14 && start < message.length())
		{
			if (message[start] == ':')
			{
				this->parameters.push_back(message.substr(start));
				break;
			}

			size_t end = message.find(' ', start);
			if (end == std::string::npos)
			{
				// Plus d'espace, on est a la fin du message
				this->parameters.push_back(message.substr(start));
				break;
			}

			this->parameters.push_back(message.substr(start, end - start));
			nbParams++;

			start = end + 1;
		}

		if (nbParams == 14 && start < message.length())
		{
			// Ajout de : au dernier parametre pour indiquer que c'est un trailing parameter
			this->parameters.push_back(message.substr(start));
			if (this->parameters.back()[0] != ':')
			{
				this->parameters.back().insert(0, ":");
			}
		}

		if (!checkParams(this->parameters))
		{
			throw std::invalid_argument("Error: MessageBuilder.cpp::withParams: Invalid parameters.");
		}
	}
}

Message::Builder &Message::Builder::withInput(std::string const &input)
{
    this->input = input;
	size_t pos = 0;

	if (input.empty())
	{
		throw std::invalid_argument("Error: MessageBuilder.cpp::withInput: std::invalid_argument: Empty input");
	}
	withPrefix(pos, this->input);
	withCommand(pos, this->input);
	withParams(pos, this->input);

	return (*this);
}

Message Message::Builder::build(void) { return (Message(this->input, this->prefix, this->command, this->parameters)); }
