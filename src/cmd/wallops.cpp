/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   wallops.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/07 18:18:27 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/07 18:39:22 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.hpp"

void irc::Command::wallops(Message const &message, Client &client)
{
	Message::Builder builder;
	Server::t_clients::iterator it;

	if (message.getParameters().empty())
		return (
			Command::reply(ERR_NEEDMOREPARAMS, client, message.getCommand()));
	else if (client.getModes().find(USR_MODE_o) == client.getModes().end())
		return (irc::Server::instance().produce(
			ERR_NOPRIVILEGES,
			client,
			"Permission Denied - You are not an IRC operator"));

	builder
		.withPrefix(client.str())
		.withCommand(message.getCommand())
		.withParameter(utils::to_string(message.getParameters(), " "))
		.build();

	for (it = Server::instance().getClient(
			 Server::instance().getClients().begin()->second.getNickname());
		 it != Server::instance().getClients().end();
		 ++it)
		if (it->second.getModes().find(USR_MODE_o) !=
				it->second.getModes().end() ||
			it->second.getModes().find(USR_MODE_w) !=
				it->second.getModes().end())
			Server::instance().produce(it->second, builder.build());
}
