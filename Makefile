# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/07/14 22:40:33 by abenamar          #+#    #+#              #
#    Updated: 2024/10/29 18:28:24 by abenamar         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME := ircserv

INCLUDES := -I$(CURDIR)/include

SRCDIR := src

SRCS := RuntimeErrno.cpp
SRCS += Utils.cpp
SRCS += Message.cpp
SRCS += Client.cpp 
SRCS += Server.cpp
SRCS += Command.cpp
SRCS += Channel.cpp
SRCS += ircserv.cpp

OBJDIR := obj

OBJS := $(SRCS:%.cpp=$(OBJDIR)/%.o)

CC := c++

CFLAGS := -Wall
CFLAGS += -Wextra
CFLAGS += -Werror
CFLAGS += -std=c++98
CFLAGS += -g3

RM := rm -f

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
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