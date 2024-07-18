#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <string>
#include <vector>

#include "User.hpp"

class Channel
{
public :
	Channel() throw();
	Channel(std::string _topic, std::string _key, int _limit) throw();
	Channel(Channel const &src) throw();
	virtual ~Channel() throw();	
	Channel& operator=(Channel const &rhs) throw();

	void addOperator(User user) throw();
	void removeOperator(User user) throw();

	void addMember(User user) throw();
	void removeMember(User user) throw();

	void notifyUsers(void) throw();

private :
	std::string _topic;
	std::string _key;
	int			_limit;
	std::vector<User *> operators;
	std::vector<User *> users;

};
#endif