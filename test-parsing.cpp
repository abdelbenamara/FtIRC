/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test-parsing.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karimasadykova <karimasadykova@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/30 14:00:00 by karimasadyk       #+#    #+#             */
/*   Updated: 2024/08/29 22:36:06 by karimasadyk      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Message.hpp"
#include <iostream>
#include <vector>

void printMessage(const Message& message)
{
    std::cout << "Input: |" << message.getInput() << "|" << std::endl;
    std::cout << "Prefix: |" << message.getPrefix() << "|" << std::endl;
    std::cout << "Command: |" << message.getCommand() << "|" << std::endl;
    std::cout << "User: |" << message.getUser() << "|" << std::endl;
    std::cout << "Host: |" << message.getHost() << "|" << std::endl;
    std::cout << "Name: |" << message.getName() << "|" << std::endl;
    std::cout << "Parameters:" << std::endl;

    std::vector<std::string> params = message.getParameters();
    for (size_t i = 0; i < params.size(); i++)
	{
        std::cout << "  [" << i << "]: |" << params[i] << std::endl;
    }
}


// c++ -std=c++98 test-parsing.cpp Message.cpp MessageBuilder.cpp -o test
int main() {
    std::string input1 = ":nick!user@host PRIVMSG #channel :Hello, World!";
    std::string input2 = "NICK newnick";
    //std::string input2 = "NICK newnick";
    

    try {
        // Test case 1
        Message::Builder builder1;
        builder1.withInput(input1);
        Message message1 = builder1.build();

        std::cout << "Test Case 1:" << std::endl;
        printMessage(message1);

        // Test case 2
        Message::Builder builder2;
        builder2.withInput(input2);
        Message message2 = builder2.build();

        std::cout << "\nTest Case 2:" << std::endl;
        printMessage(message2);

    } catch (const std::exception& e) {
        std::cerr << e.what() << "|" << std::endl;
    }

    return 0;
}
