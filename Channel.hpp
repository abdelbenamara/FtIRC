#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <string>
#include <vector>
# define MAX_CLIENTS 15

#include "Client.hpp"

class Channel
{
public :
	Channel() throw();
	Channel(std::string name, std::string topic, std::string key, int limit);
	Channel(Channel const &src) throw();
	virtual ~Channel() throw();	
	Channel& operator=(Channel const &rhs) throw();

	void addOperator(Client client) throw();
	void removeOperator(Client client) throw();

	void addMember(Client client) throw();
	void removeMember(Client client) throw();

	void notifyClient(void) throw();

	bool isInviteOnly(void) throw();
	bool isUserInvited(Client client) throw();
	bool hasKey() throw();
	bool isFull() throw();

	std::string getKey() throw();
	std::string getTopic() throw();

private :
	std::string name;
	std::string topic;
	std::string key;
	int			limit;
	bool		inviteOnly;
	std::vector<Client *> operators;
	std::vector<Client *> clients;
};
#endif