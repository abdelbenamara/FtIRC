#ifndef OPERATOR_HPP
# define OPERATOR_HPP

#include "Client.hpp"
#include "Channel.hpp"

class Operator : public Client
{
public :
	// do we still have to do the canonical form ?
	Operator() throw();
	Operator(std::string username, std::string nickname);
	Operator(Operator const &src) throw();
	virtual ~Operator() throw();

	Operator& operator=(Operator const &rhs) throw();

	void kick(Channel channel, Client client);
private :
	// functions for MODE


};

#endif