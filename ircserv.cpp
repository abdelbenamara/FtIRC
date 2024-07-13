/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ircserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/13 16:29:49 by abenamar          #+#    #+#             */
/*   Updated: 2024/07/13 16:38:31 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <sstream>

int main(int argc, char *argv[])
{
	std::stringstream buffer;
	int port;

	if (argc < 3)
	{
		std::cerr << "Usage: ircserv <port> <password>" << std::endl;

		return (2);
	}

	buffer << argv[1];
	buffer >> port;

	if (buffer.fail() || port < 1 || port > 65535)
	{
		std::cerr << "Error: invalid port `" << argv[1] << "' (port must be in range 1-65535)" << std::endl;

		return (1);
	}

	return (0);
}
