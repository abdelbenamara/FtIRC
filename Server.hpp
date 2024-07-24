#ifndef SERVER_HPP
# define SERVER_HPP

#include <vector>
#include <string>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <sstream>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>


#include "Channel.hpp"
#include "Client.hpp"

#define MAX_CHANNELS 100
#define MAX_CLIENTS 1000


class Server
{
public :
	Server() throw();
	Server(const char * port, std::string password) throw();
	Server(Server const &src) throw();
	virtual ~Server() throw();	
	Server& operator=(Server const &rhs) throw();

	// returns the socket fd
	int start() throw();
	void stop() throw();
	// verify channel name here !
	void addChannel(const Channel &channel) throw();
	void removeChannel(const Channel &channel) throw();
	void addClient(const Client &client) throw();
	void removeClient(const Client &client) throw();

	int	getSockFd() const;

	class SocketErrorException : public std::exception
	{
		public :
			virtual const char* what() const throw();
	};

	class GetAddrException : public std::exception
	{
		public :
			virtual const char* what() const throw();
	};

private :
	const char				*_port;
	std::string				_password;
	int						_sockfd;
	std::vector<Channel>	_channels;
	std::vector<Client> 		_clients;

};

#endif