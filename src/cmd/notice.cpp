/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   notice.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/04 01:57:17 by abenamar          #+#    #+#             */
/*   Updated: 2024/12/04 02:45:42 by abenamar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.hpp"

void irc::Command::notice(Message const &message, Client &client)
{
    Client dummy(NULL, client);

    try
    {
        Command::privmsg(message, dummy);
    }
    catch (std::exception const &)
    {
        std::cerr << "Error: " << client.str() << ": " << message << std::endl;
    }

    return;
}
