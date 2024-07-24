#include "Client.hpp"

std::string Client::getNickname() const throw() { return this->_nickname; }

std::string Client::getClientname() const throw() { return this->_username; }

void Client::setNickname(std::string nickname) throw() { this->_nickname = nickname; }

void Client::setClientname(std::string username) throw() { this->_username = username; }