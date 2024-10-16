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

Channel& Channel::operator=(Channel const &rhs) throw() { return (*this); }

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

bool Channel::isInviteOnly(void) throw()
{
	return this->inviteOnly;
}

bool Channel::isUserInvited(Client client) throw()
{
	// faire une liste des utilisateurs invites ?
}

bool Channel::hasKey() throw()
{
	if (this->key.empty())
		return false;
	return true;
}

bool Channel::isFull() throw()
{
	// voir comment on va gerer et si clients contient tout le monde
	// ici je suppose que non
	if (this->operators.size() + this->clients.size() >= MAX_CLIENTS)
		return true;
	return false;
}

std::string Channel::getKey() throw() { return (this->key); }

std::string Channel::getTopic() throw() { return (this->topic); }