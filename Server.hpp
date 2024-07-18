#ifndef SERVER_HPP
# define SERVER_HPP

#include <vector>
#include <string>

#include "Channel.hpp"
#include "User.hpp"

class Server
{
public :
	Server() throw();
	Server(int port, int password) throw();
	Server(Server const &src) throw();
	virtual ~Server() throw();	
	Server& operator=(Server const &rhs) throw();

	void start() throw();
	void stop() throw();
	void addChannel(Channel channel) throw();
	void removeChannel(Channel channel) throw();
	void addClient(User client) throw();
	void removeClient(User client) throw();
private :
	int			_port;
	std::string	_password;
	std::vector<Channel>	_channels;
	std::vector<User> 		_clients;

};

#endif