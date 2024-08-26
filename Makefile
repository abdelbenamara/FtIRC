# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/07/14 22:40:33 by abenamar          #+#    #+#              #
#    Updated: 2024/08/23 23:12:05 by abenamar         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME := ircserv

INCLUDES := -I$(CURDIR)

SRCS := RuntimeErrno.cpp
SRCS += Message.cpp
SRCS += MessageBuilder.cpp
SRCS += Client.cpp 
SRCS += Server.cpp
SRCS += ServerBuilder.cpp
SRCS += ircserv.cpp

OBJS := $(SRCS:.cpp=.o)

CC := c++

CFLAGS := -Wall
CFLAGS += -Wextra
CFLAGS += -Werror
CFLAGS += -std=c++98
CFLAGS += -g3

RM := rm -f

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDES)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS) $(INCLUDES)

all: $(NAME)

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: re fclean clean all