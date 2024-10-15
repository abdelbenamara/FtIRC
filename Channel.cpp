#include "Channel.hpp"

Channel::Channel() : name(), topic(), key(), limit(), inviteOnly(true) { return; }

Channel::Channel(std::string name, std::string topic, std::string key, int limit) : name(name), topic(topic), key(key), limit(limit), inviteOnly(true) { return; }
	
Channel::Channel(Channel const &src) throw() {}

Channel::~Channel() throw()
{
	int i;

	for (i = 0; i < this->operators.size(); ++i)
    	delete operators[i];

	for (i = 0; i < this->clients.size(); ++i)
    	delete clients[i];
}

Channel& Channel::operator=(Channel const &rhs) throw() {}

void Channel::addOperator(Client client) throw()
{
	operators.push_back(&client);
}

void Channel::removeOperator(Client client) throw()
{
	operators.erase(std::remove(operators.begin(), operators.end(), client), operators.end());
}

void Channel::addMember(Client client) throw()
{
	clients.push_back(&client);
}

void Channel::removeMember(Client client) throw()
{
	clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());
}