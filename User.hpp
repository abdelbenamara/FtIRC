#ifndef USER_HPP
# define USER_HPP

#include <string>

#include "Channel.hpp"

class User
{
public :
	User() throw();
	User(std::string username, std::string nickname) throw();
	User(User const &src) throw();
	virtual ~User() throw();	
	User& operator=(User const &rhs) throw();

	std::string getNickname() const throw();
	std::string getUsername() const throw();

	void setNickname() throw();
	void setUsername() throw();

	// void for now and we can throw exceptions later, or ints for error codes, to discuss
	void authenticate(User user) throw();
	void joinChannel(Channel channel) throw();
	void sendMessage(std::string const message) throw();
	void receiveMessage() throw();

private :
	std::string _username;
	std::string _nickname;
};

#endif