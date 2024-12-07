/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   who.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/07 18:51:11 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/07 20:46:43 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.hpp"

static irc::Message whoreply(
	irc::Client const &user,
	irc::Client::t_channels::const_iterator const &chancit)
{
	irc::Message::Builder builder;
	std::string buf;

	builder.withPrefix(irc::Server::instance().getTextProperty(PRP_SERVERNAME));

	buf = "*";

	if (chancit != user.getChannels().end())
		buf = (*chancit)->getName();

	builder
		.withCommand(RPL_WHOREPLY)
		.withParameter(user.getNickname())
		.addParameter(buf)
		.addParameter(user.getUsername());

	buf = irc::utils::get_haddr(user.getSocket());

	if (!buf.rfind(':', 0))
		buf.insert(buf.begin(), '0');

	builder
		.addParameter(buf)
		.addParameter(
			irc::Server::instance().getTextProperty(PRP_SERVERNAME))
		.addParameter(user.getNickname());

	buf = "H";

	if (user.getModes().find(USR_MODE_o) != user.getModes().end())
		buf += '*';

	if (chancit != user.getChannels().end() &&
		(*chancit)->getOperators().find(&user) !=
			(*chancit)->getOperators().end())
		buf += '@';

	return (builder
				.addParameter(buf)
				.addParameter("0 " + user.getRealname())
				.build());
}

void irc::Command::who(Message const &message, Client &client)
{
	Message::Builder builder;
	Server::t_clients::iterator userit;
	Server::t_channels::iterator chanit;
	Channel::t_clients::iterator memberit;

	if (message.getParameters().empty())
		return (
			Command::reply(ERR_NEEDMOREPARAMS, client, message.getCommand()));

	if (Server::instance()
			.getTextProperty(PRP_CHANTYPES)
			.find(message.getParameters().at(0).at(0)) == std::string::npos)
	{
		userit = Server::instance().getUser(message.getParameters().at(0));

		if (userit != Server::instance().getClients().end() &&
			userit->second.getModes().find(USR_MODE_i) ==
				userit->second.getModes().end())
			Server::instance().produce(
				client,
				::whoreply(userit->second,
						   userit->second.getChannels().begin()));
	}
	else
	{
		chanit = Server::instance().getChannel(message.getParameters().at(0));

		if (chanit != Server::instance().getChannels().end())
			for (memberit = chanit->second.getMembers().begin();
				 memberit != chanit->second.getMembers().end();
				 ++memberit)
				if ((*memberit)->getModes().find(USR_MODE_i) ==
						(*memberit)->getModes().end() ||
					chanit->second.getMembers().find(&client) !=
						chanit->second.getMembers().end())
					Server::instance().produce(
						client,
						::whoreply(**memberit,
								   (*memberit)->getChannels().find(
									   &(chanit->second))));
	}

	return (Server::instance().produce(
		client,
		builder
			.withPrefix(Server::instance().getTextProperty(PRP_SERVERNAME))
			.withCommand(RPL_ENDOFWHO)
			.withParameter(client.getNickname())
			.addParameter(message.getParameters().at(0))
			.addParameter("End of WHO list")
			.build()));
}
