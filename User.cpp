#include "User.hpp"

std::string User::getNickname() const throw() { return this->_nickname; }

std::string User::getUsername() const throw() { return this->_username; }

void User::setNickname(std::string nickname) throw() { this->_nickname = nickname ; }

void User::setUsername(std::string username) throw() { this->_username = username; }