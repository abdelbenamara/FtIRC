#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <string>
#include <vector>

#include "Client.hpp"

class Channel
{
public :
	Channel() throw();
	Channel(std::string name, std::string _topic, std::string _key, int _limit) throw();
	Channel(Channel const &src) throw();
	virtual ~Channel() throw();	
	Channel& operator=(Channel const &rhs) throw();

	void addOperator(Client client) throw();
	void removeOperator(Client client) throw();

	void addMember(Client client) throw();
	void removeMember(Client client) throw();

	void notifyClient(void) throw();



private :
	std::string _name;
	std::string _topic;
	std::string _key;
	int			_limit;
	std::vector<Client *> operators;
	std::vector<Client *> clients;

};
#endif