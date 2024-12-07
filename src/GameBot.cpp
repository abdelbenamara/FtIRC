/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GameBot.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/07 02:38:27 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/07 12:08:11 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "GameBot.hpp"
#include "Server.hpp"

irc::GameBot::GameBot(utils::t_sockinfo const &conninfo)
    : Client(conninfo) { return; }

irc::GameBot::GameBot(GameBot const &src) : Client(src) { return; }

irc::GameBot::~GameBot(void) throw() { return; }

void irc::GameBot::apply(Message const &message) const
{
    Message::Builder builder;
    Server::t_clients::iterator it;
    std::vector<std::string> params, shapes;

    if (message.getParameters().size() < 2 ||
        (message.getParameters().at(1).find(".help") == std::string::npos &&
         message.getParameters().at(1).find(".play") == std::string::npos))
        return;

    it = Server::instance().getClient(
        message.getPrefix().substr(0, message.getPrefix().find('!')));
    params = utils::split(message.getParameters().at(1), ' ');

    if (Server::instance()
            .getTextProperty(PRP_CHANTYPES)
            .find(message.getParameters().at(0).at(0)) == std::string::npos)
        builder.withParameter(it->second.getNickname());
    else
        builder.withParameter(message.getParameters().at(0));

    builder
        .withPrefix(this->getNickname())
        .withCommand(CMD_PRIVMSG);
    shapes.push_back("ROCK");
    shapes.push_back("PAPER");
    shapes.push_back("SCISSORS");
    std::random_shuffle(shapes.begin(), shapes.end());
    shapes.push_back("<invalid>");

    if (params.size() > 1)
    {
        std::transform(params.at(1).begin(),
                       params.at(1).end(),
                       params.at(1).begin(),
                       utils::to_upper);

        if (std::find(shapes.begin(), --shapes.end(), params.at(1)) !=
            --shapes.end())
            shapes.at(3) = params.at(1);
    }

    if (!params.at(0).compare(".help"))
    {
        if (!shapes.at(3).compare("ROCK"))
            Server::instance().produce(
                it->second,
                builder
                    .addParameter("Info: ROCK beats SCISSORS")
                    .build());
        else if (!shapes.at(3).compare("PAPER"))
            Server::instance().produce(
                it->second,
                builder
                    .addParameter("Info: PAPER beats ROCK")
                    .build());
        else if (!shapes.at(3).compare("SCISSORS"))
            Server::instance().produce(
                it->second,
                builder
                    .addParameter("Info: SCISSORS beats PAPER")
                    .build());
        else
            Server::instance().produce(
                it->second,
                builder
                    .addParameter("Usage: .help  <ROCK | PAPER | SCISSORS>"
                                  ", .play <ROCK | PAPER | SCISSORS>")
                    .build());

        return;
    }

    if ((!shapes.at(3).compare("PAPER") && !shapes.at(0).compare("ROCK")) ||
        (!shapes.at(3).compare("ROCK") && !shapes.at(0).compare("SCISSORS")) ||
        (!shapes.at(3).compare("SCISSORS") && !shapes.at(0).compare("PAPER")))
        Server::instance().produce(
            it->second,
            builder
                .addParameter(shapes.at(3) + " beats " + shapes.at(0) +
                              ", " + it->second.getNickname() + " won!")
                .build());
    else if (shapes.at(0) == shapes.at(3))
        Server::instance().produce(
            it->second,
            builder
                .addParameter(shapes.at(0) + " against " + shapes.at(3) +
                              ", " + this->getNickname() + " and " +
                              it->second.getNickname() + " drew!")
                .build());
    else
        Server::instance().produce(
            it->second,
            builder
                .addParameter(shapes.at(0) + " beats " + shapes.at(3) +
                              ", " + this->getNickname() + " won!")
                .build());

    return;
}
