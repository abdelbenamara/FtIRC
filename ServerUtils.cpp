/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerUtils.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/07 12:55:08 by abenamar          #+#    #+#             */
/*   Updated: 2024/08/08 02:26:50 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerUtils.hpp"

ServerUtils::ServerUtils(void) throw() { return; }

ServerUtils::ServerUtils(ServerUtils const & /* src */) throw() { return; }

ServerUtils::~ServerUtils(void) throw() { return; }

ServerUtils ServerUtils::operator=(ServerUtils const & /* rhs */) throw() { return (*this); }

char const *ServerUtils::gai_strerror(int errcode) throw()
{
	switch (errcode)
	{
	case EAI_ADDRFAMILY:
		return "EAI_ADDRFAMILY: Address family for hostname not supported";
	case EAI_AGAIN:
		return "EAI_AGAIN: Temporary failure in name resolution";
	case EAI_BADFLAGS:
		return "EAI_BADFLAGS: Bad value for ai_flags";
	case EAI_FAIL:
		return "EAI_FAIL: Non-recoverable failure in name resolution";
	case EAI_FAMILY:
		return "EAI_FAMILY: ai_family not supported";
	case EAI_MEMORY:
		return "EAI_MEMORY: Memory allocation failure";
	case EAI_NODATA:
		return "EAI_NODATA: No address associated with hostname";
	case EAI_NONAME:
		return "EAI_NONAME: Name or service not known";
	case EAI_SERVICE:
		return "EAI_SERVICE: Servname not supported for ai_socktype";
	case EAI_SOCKTYPE:
		return "EAI_SOCKTYPE: ai_socktype not supported";
	case EAI_SYSTEM:
		return std::string("EAI_SYSTEM: System error: ").append(strerror(errno)).c_str();
	default:
		break;
	}

	return "Unknown error";
}

int ServerUtils::initSocket(char const *const numericserv) throw(std::invalid_argument, std::runtime_error)
{
	int tmp;
	std::istream const &check = std::istringstream(numericserv) >> tmp;
	addrinfo hints, *info;

	if (check.fail() || !check.eof() || tmp < 0 || tmp > std::numeric_limits<in_port_t>::max())
		throw std::invalid_argument(std::string("ServerUtils::initSocket: std::invalid_argument: `") + numericserv + "': port number must be between 1 and 65535, or it can be 0 to use a random available port");

	hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV | AI_V4MAPPED | AI_ADDRCONFIG;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_addrlen = 0;
	hints.ai_addr = NULL;
	hints.ai_canonname = NULL;
	hints.ai_next = NULL;
	tmp = getaddrinfo(NULL, numericserv, &hints, &hints.ai_next);

	if (tmp)
		throw std::runtime_error(std::string("ServerUtils::initSocket: std::runtime_error: getaddrinfo") + ServerUtils::gai_strerror(tmp));

	for (info = hints.ai_next; info != NULL; info = info->ai_next)
	{
		tmp = socket(info->ai_family, info->ai_socktype, info->ai_protocol);

		if (tmp == -1)
			continue;

		if (!bind(tmp, info->ai_addr, info->ai_addrlen))
			break;

		close(tmp);
	}

	freeaddrinfo(hints.ai_next);

	if (info == NULL)
		throw std::runtime_error(std::string("ServerUtils::initSocket: std::runtime_error: socket, bind: ") + strerror(errno));

	if (fcntl(tmp, F_SETFL, O_NONBLOCK) == -1)
		throw std::runtime_error(std::string("ServerUtils::initSocket: std::runtime_error: fcntl: ") + strerror(errno));

	if (listen(tmp, SOMAXCONN) == -1)
		throw std::runtime_error(std::string("ServerUtils::initSocket: std::runtime_error: listen: ") + strerror(errno));

	return (tmp);
}

void ServerUtils::interrupt(int /* signum */) throw() { return; }
