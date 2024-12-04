/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mode.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/01 20:21:27 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/04 02:56:32 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.hpp"

static std::string user_mode(std::string const &modestring,
                             irc::Client &client)
{
    std::size_t cap(irc::Server::instance().getSizeProperty(PRP_MODES)),
        cur(0),
        pos;
    bool add(true);
    std::string applied;

    for (; cap > 0 && cur < modestring.length();)
    {
        if (modestring.at(cur) == '+')
            add = true;
        else if (modestring.at(cur) == '-')
            add = false;
        else
        {
            switch (modestring.at(cur))
            {
            case USR_MODE_o:
                if (client.getModes().find(modestring.at(cur)) ==
                    client.getModes().end())
                {
                    irc::Server::instance().produce(
                        ERR_NOPRIVILEGES,
                        client,
                        "Permission Denied - You are not an IRC operator");

                    break;
                }

                /* FALLTHROUGH */

            case USR_MODE_i:
                /* FALLTHROUGH */

            case USR_MODE_w:
                if (add && client.getModes().find(modestring.at(cur)) ==
                               client.getModes().end())
                {
                    client.addMode(modestring.at(cur));

                    pos = applied.find_last_of("+-");

                    if (pos == std::string::npos || applied.at(pos) == '-')
                        applied += '+';

                    applied += modestring.at(cur);
                }
                else if (!add && client.getModes().find(modestring.at(cur)) !=
                                     client.getModes().end())
                {
                    client.removeMode(modestring.at(cur));

                    pos = applied.find_last_of("-+");

                    if (pos == std::string::npos || applied.at(pos) == '+')
                        applied += '-';

                    applied += modestring.at(cur);
                }

                break;

            default:
                irc::Command::reply(ERR_UMODEUNKNOWNFLAG,
                                    client,
                                    std::string(1, modestring.at(cur)));

                break;
            }

            --cap;
        }

        cur = modestring.find_first_not_of(modestring.at(cur), cur);
    }

    return (applied);
}

static std::vector<std::string> channel_mode(
    std::vector<std::string> const &params,
    irc::Channel &channel,
    irc::Client &client)
{
    std::vector<std::string> changes;
    irc::Message::Builder builder;
    std::size_t cap(irc::Server::instance().getSizeProperty(PRP_MODES)),
        cur(0),
        idx(1),
        pos;
    bool chanop(channel.getOperators().find(&client) !=
                channel.getOperators().end()),
        add(true);
    irc::Server::t_clients::iterator userit;

    changes.resize(1);
    builder.withCommand(ERR_INVALIDMODEPARAM);

    for (; cap > 0 && cur < params.at(1).length();)
    {
        if (params.at(1).at(cur) == '+')
            add = true;
        else if (params.at(1).at(cur) == '-')
            add = false;
        else if (!chanop)
            irc::Command::reply(ERR_CHANOPRIVSNEEDED,
                                client,
                                channel.getName());
        else
        {
            builder
                .withParameter(client.getNickname())
                .addParameter(channel.getName());

            switch (params.at(1).at(cur))
            {
            case CHAN_MODE_k:
                /* FALLTHROUGH */

            case CHAN_MODE_l:
                /* FALLTHROUGH */

            case CHAN_MODE_o:
                if ((add || params.at(1).at(cur) != CHAN_MODE_l) &&
                    ++idx >= params.size())
                {
                    irc::Server::instance().produce(
                        client,
                        builder
                            .addParameter(params.at(1).at(cur))
                            .addParameter("*")
                            .addParameter("You must specify a mode parameter")
                            .build());

                    break;
                }

                try
                {
                    switch (params.at(1).at(cur))
                    {
                    case CHAN_MODE_k:
                        if (add && channel.getKey().empty())
                        {
                            try
                            {
                                channel.setKey(params.at(idx));
                            }
                            catch (std::exception const &)
                            {
                                throw;
                            }
                        }
                        else if (!add && !channel.getKey().empty())
                        {
                            if (params.at(idx) != channel.getKey())
                            {
                                irc::Command::reply(ERR_KEYSET,
                                                    client,
                                                    channel.getName());

                                throw std::exception();
                            }

                            channel.setKey("");
                        }

                        break;

                    case CHAN_MODE_l:
                        if (add && std::istringstream(params.at(idx)) >> pos)
                        {
                            channel.setLimit(pos);
                        }
                        else
                            throw std::domain_error(params.at(idx) +
                                                    ": is not a valid size");

                        break;

                    default: // CHAN_MODE_o
                        userit = irc::Server::instance().getUser(
                            params.at(idx));

                        if (userit ==
                            irc::Server::instance().getClients().end())
                            irc::Command::reply(ERR_NOSUCHNICK,
                                                client,
                                                params.at(idx));
                        else if (channel.getMembers().find(
                                     &(userit->second)) !=
                                     channel.getMembers().end() &&
                                 channel.getOperators().find(
                                     &(userit->second)) ==
                                     channel.getOperators().end())
                            channel.addOperator(userit->second);

                        throw std::exception();
                    }
                }
                catch (std::domain_error const &e)
                {
                    std::cerr << "Error: " << e.what() << std::endl;

                    irc::Server::instance().produce(
                        client,
                        builder
                            .addParameter(params.at(1).at(cur))
                            .addParameter(params.at(idx))
                            .addParameter("Invalid mode parameter")
                            .build());

                    break;
                }
                catch (std::exception const &)
                {
                    break;
                }

                changes.push_back(params.at(idx));

                /* FALLTHROUGH */

            case CHAN_MODE_i:
                /* FALLTHROUGH */

            case CHAN_MODE_t:
                if (add && channel.getModes().find(params.at(1).at(cur)) ==
                               channel.getModes().end())
                {
                    channel.addMode(params.at(1).at(cur));

                    pos = changes.at(0).find_last_of("+-");

                    if (pos == std::string::npos ||
                        changes.at(0).at(pos) == '-')
                        changes.at(0) += '+';

                    changes.at(0) += params.at(1).at(cur);
                }
                else if (!add &&
                         channel.getModes().find(params.at(1).at(cur)) !=
                             channel.getModes().end())
                {
                    channel.removeMode(params.at(1).at(cur));

                    pos = changes.at(0).find_last_of("-+");

                    if (pos == std::string::npos ||
                        changes.at(0).at(pos) == '+')
                        changes.at(0) += '-';

                    changes.at(0) += params.at(1).at(cur);
                }

                break;

            default:
                irc::Command::reply(ERR_UNKNOWNMODE,
                                    client,
                                    std::string(1, params.at(1).at(cur)));

                break;
            }

            --cap;
        }

        cur = params.at(1).find_first_not_of(params.at(1).at(cur), cur);
    }

    return (changes);
}

void irc::Command::mode(Message const &message, Client &client)
{
    Message::Builder builder;
    Server::t_clients::iterator userit;
    Server::t_channels::iterator chanit;
    std::vector<std::string> params;

    if (message.getParameters().empty())
        return (
            Command::reply(ERR_NEEDMOREPARAMS, client, message.getCommand()));
    else if (!client.isRegistered())
        return (
            Command::reply(ERR_NOTREGISTERED, client, message.getCommand()));

    builder
        .withPrefix(client.str())
        .withCommand(message.getCommand());

    if (Server::instance()
            .getTextProperty(PRP_CHANTYPES)
            .find(message.getParameters().at(0).at(0)) == std::string::npos)
    {
        userit = Server::instance().getUser(message.getParameters().at(0));

        if (userit == Server::instance().getClients().end())
            return (Command::reply(ERR_NOSUCHNICK,
                                   client,
                                   message.getParameters().at(0)));
        else if (&userit->second != &client)
            return (Server::instance().produce(
                ERR_USERSDONTMATCH,
                client,
                "Cannot view or change mode for other users"));
        else if (message.getParameters().size() == 1)
            return (Server::instance().produce(
                RPL_UMODEIS,
                client,
                '+' + utils::to_string(client.getModes(), "")));

        params.push_back(::user_mode(message.getParameters().at(1), client));

        if (!params.at(0).empty())
            Server::instance().produce(client,
                                       builder
                                           .withParameter(client.getNickname())
                                           .addParameter(params.at(0))
                                           .build());
    }
    else
    {
        chanit = Server::instance().getChannel(message.getParameters().at(0));

        if (chanit == Server::instance().getChannels().end())
            return (Command::reply(ERR_NOSUCHCHANNEL,
                                   client,
                                   message.getParameters().at(0)));
        else if (message.getParameters().size() == 1)
        {
            builder
                .withPrefix(Server::instance().getTextProperty(PRP_SERVERNAME))
                .withCommand(RPL_CHANNELMODEIS)
                .withParameter(client.getNickname())
                .addParameter(chanit->first)
                .addParameter('+' +
                              utils::to_string(chanit->second.getModes(), ""));

            if (chanit->second.getModes().find(CHAN_MODE_k) !=
                chanit->second.getModes().end())
            {
                if (chanit->second.getMembers().find(&client) ==
                    chanit->second.getMembers().end())
                    builder.addParameter("<key>");
                else
                    builder.addParameter(chanit->second.getKey());
            }

            if (chanit->second.getModes().find(CHAN_MODE_l) !=
                chanit->second.getModes().end())
                builder.addParameter(
                    utils::to_string(chanit->second.getLimit()));

            Server::instance().produce(client, builder.build());

            return (Server::instance().produce(
                client,
                builder
                    .withCommand(RPL_CREATIONTIME)
                    .withParameter(client.getNickname())
                    .addParameter(chanit->first)
                    .addParameter(utils::to_string(chanit->second.getStart()))
                    .build()));
        }

        params = ::channel_mode(message.getParameters(),
                                chanit->second,
                                client);

        if (!params.at(0).empty())
        {
            params.insert(params.begin(), chanit->first);
            chanit->second.publish(builder
                                       .withParameters(params)
                                       .build());
        }
    }

    return;
}
