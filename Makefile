# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: abenamar <abenamar@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/07/14 22:40:33 by abenamar          #+#    #+#              #
#    Updated: 2024/12/24 13:29:06 by abenamar         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME := ircserv

INCLUDES := -I$(CURDIR)/include

SRCDIR := src

CMDDIR := cmd

SRCS := utils.cpp
SRCS += AServer.cpp
SRCS += Server.cpp
SRCS += MessageBuilder.cpp
SRCS += Message.cpp
SRCS += Client.cpp 
SRCS += Channel.cpp
SRCS += Command.cpp
SRCS += GameBot.cpp
SRCS += ircserv.cpp

CMDS := pass.cpp
CMDS += nick.cpp
CMDS += user.cpp
CMDS += ping.cpp
CMDS += oper.cpp
CMDS += quit.cpp
CMDS += lusers.cpp
CMDS += motd.cpp
CMDS += version.cpp
CMDS += names.cpp
CMDS += join.cpp
CMDS += topic.cpp
CMDS += invite.cpp
CMDS += kick.cpp
CMDS += mode.cpp
CMDS += privmsg.cpp
CMDS += notice.cpp
CMDS += who.cpp
CMDS += wallops.cpp

OBJDIR := obj

OBJS := $(SRCS:%.cpp=$(OBJDIR)/%.o)
OBJS += $(CMDS:%.cpp=$(OBJDIR)/$(CMDDIR)/%.o)

DEPS := $(OBJS:%.o=%.d)

CC := c++

CFLAGS := -MMD
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Werror
CFLAGS += -std=c++98
CFLAGS += -g3

MKDIR := mkdir -p

RM := rm -f

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDES)

$(NAME): $(OBJDIR) $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS) $(INCLUDES)

-include $(DEPS)

$(OBJDIR):
	$(MKDIR) $(OBJDIR)/$(CMDDIR)

bonus: $(NAME)

all: $(NAME)

clean:
	$(RM) $(OBJS)
	$(RM) $(DEPS)

fclean: clean
	$(RM) -r $(OBJDIR)
	$(RM) $(NAME)

re: fclean all

.PHONY: re fclean clean all bonus