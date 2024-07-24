#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>

#include "Client.hpp"

class Client
{
public :
	Client() throw();
	Client(std::string username, std::string nickname) throw();
	Client(Client const &src) throw();
	virtual ~Client() throw();	
	Client& operator=(Client const &rhs) throw();

	std::string getNickname() const throw();
	std::string getClientname() const throw();

	void setNickname(std::string nickname) throw();
	void setClientname(std::string username) throw();

	// void for now and we can throw exceptions later, or ints for error codes, to discuss
	void authenticate(Client client) throw();
	void joinChannel(Channel channel) throw();
	void sendMessage(std::string const message) throw();
	void receiveMessage() throw();

private :
	int 		socket_fd;
	std::string _username;
	std::string _nickname;
};

#endif