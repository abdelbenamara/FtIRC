/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lusers.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/07 17:27:00 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/07 18:15:52 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.hpp"

static std::size_t users_amount(void)
{
	irc::Server::t_clients::const_iterator cit(
		irc::Server::instance().getClients().begin());
	std::size_t amount(0);

	for (; cit != irc::Server::instance().getClients().end(); ++cit)
		if (cit->second.isRegistered())
			++amount;

	return (amount);
}

static std::size_t invisibles_amount(void)
{
	irc::Server::t_clients::const_iterator cit(
		irc::Server::instance().getClients().begin());
	std::size_t amount(0);

	for (; cit != irc::Server::instance().getClients().end(); ++cit)
		if (cit->second.isRegistered() &&
			cit->second.getModes().find(USR_MODE_i) !=
				cit->second.getModes().end())
			++amount;

	return (amount);
}

static std::size_t operators_amount(void)
{
	irc::Server::t_clients::const_iterator cit(
		irc::Server::instance().getClients().begin());
	std::size_t amount(0);

	for (; cit != irc::Server::instance().getClients().end(); ++cit)
		if (cit->second.isRegistered() &&
			cit->second.getModes().find(USR_MODE_o) !=
				cit->second.getModes().end())
			++amount;

	return (amount);
}

static std::size_t unknowns_amount(void)
{
	irc::Server::t_clients::const_iterator cit(
		irc::Server::instance().getClients().begin());
	std::size_t amount(0);

	for (; cit != irc::Server::instance().getClients().end(); ++cit)
		if (!cit->second.isRegistered())
			++amount;

	return (amount);
}

void irc::Command::lusers(Message const &, Client &client)
{
	Message::Builder builder;

	Server::instance().produce(
		client,
		builder
			.withPrefix(Server::instance().getTextProperty(PRP_SERVERNAME))
			.withCommand(RPL_LUSERCLIENT)
			.withParameter(client.getNickname())
			.addParameter(
				"There are " + utils::to_string(::users_amount()) +
				" users and " + utils::to_string(::invisibles_amount()) +
				" invisible on 1 servers")
			.build());
	Server::instance().produce(
		client,
		builder
			.withCommand(RPL_LUSEROP)
			.withParameter(client.getNickname())
			.addParameter(utils::to_string(::operators_amount()))
			.addParameter("operator(s) online")
			.build());
	Server::instance().produce(
		client,
		builder
			.withCommand(RPL_LUSERUNKNOWN)
			.withParameter(client.getNickname())
			.addParameter(utils::to_string(::unknowns_amount()))
			.addParameter("unknown connection(s)")
			.build());
	Server::instance().produce(
		client,
		builder
			.withCommand(RPL_LUSERCHANNELS)
			.withParameter(client.getNickname())
			.addParameter(utils::to_string(
				Server::instance().getChannels().size()))
			.addParameter("channels formed")
			.build());
	Server::instance().produce(
		client,
		builder
			.withCommand(RPL_LUSERME)
			.withParameter(client.getNickname())
			.addParameter("I have 0 clients and 0 servers")
			.build());
	Server::instance().produce(
		client,
		builder
			.withPrefix(Server::instance().getTextProperty(PRP_SERVERNAME))
			.withCommand(RPL_LOCALUSERS)
			.withParameter(client.getNickname())
			.addParameter(utils::to_string(::users_amount()))
			.addParameter(utils::to_string(Client::getUniques() - 1))
			.addParameter(
				"Current local users " + utils::to_string(::users_amount()) +
				", max " + utils::to_string(Client::getUniques() - 1))
			.build());

	return (Server::instance().produce(
		client,
		builder
			.withPrefix(Server::instance().getTextProperty(PRP_SERVERNAME))
			.withCommand(RPL_GLOBALUSERS)
			.withParameter(client.getNickname())
			.addParameter(utils::to_string(::users_amount()))
			.addParameter(utils::to_string(Client::getUniques() - 1))
			.addParameter(
				"Current global users " + utils::to_string(::users_amount()) +
				", max " + utils::to_string(Client::getUniques() - 1))
			.build()));
}
