/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerUtils.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/07 12:49:06 by abenamar          #+#    #+#             */
/*   Updated: 2024/08/08 02:26:22 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __SERVERUTILS_HPP__
#define __SERVERUTILS_HPP__

#include <cerrno>
#include <cstring>
#include <istream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <fcntl.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

class ServerUtils
{
public:
	static char const *gai_strerror(int errcode) throw();
	static int initSocket(char const *const numericserv) throw(std::invalid_argument, std::runtime_error);

	template <typename T>
	static T *initArray(int const size, char const *const type) throw(std::invalid_argument, std::runtime_error)
	{
		std::ostringstream err;
		T *tmp;

		if (size < 1)
		{
			err << "ServerUtils::initArray: std::invalid_argument: `" << size << "': " << type << " parameter must be a strictly positive integer";

			throw std::invalid_argument(err.str());
		}

		try
		{
			tmp = new T[size + 1];
		}
		catch (std::exception const &e)
		{
			err << "ServerUtils::initArray: std::bad_alloc: `" << size << "': try with a smaller integer for " << type << " parameter";

			throw std::runtime_error(err.str());
		}

		return (tmp);
	}

	static void interrupt(int /* signum */) throw();

private:
	ServerUtils(void) throw();
	ServerUtils(ServerUtils const & /* src */) throw();
	virtual ~ServerUtils(void) throw();

	ServerUtils operator=(ServerUtils const & /* rhs */) throw();
};

#endif
