# FtIRC

## Compile

```
make
```
```
make bonus
```

### Mandatory

- [x] Code written in C++ 98
- [x] No IRC client (only an IRC server)
- [x] No server-to-server communication
- [x] Excutable runs as follow: ```./ircserv <port> <password>```
- [x] Multiple clients are handled at the same time without server hanging
- [x] No fork (e.g. no child process)
- [x] All I/O operations are non-blocking
- [x] Only one call to epoll_wait() handles all operations (read, write, listen, etc)
- [x] No read/write without calling epoll_wait()
- [x] Communication is done with TCP/IP (v4 or v6)
- [x] Reference IRC client works without any error (e.g. [irssi](https://irssi.org))
- [x] Partial data reception (e.g. Ctrl + D when using netcat)
- [x] Clean code (e.g. all messages are created with the builder design pattern)

Client commands:

- [x] PASS
- [x] NICK
- [x] USER
- [x] OPER
- [x] JOIN
- [x] TOPIC
- [x] INVITE
- [x] KICK
- [x] MODE (channel modes: iklot)
- [x] PRIVMSG
- [x] NOTICE

Numeric replies:

- [x] RPL_WELCOME
- [x] RPL_CHANNELMODEIS
- [x] RPL_CREATIONTIME
- [x] RPL_NOTOPIC
- [x] RPL_TOPIC
- [x] RPL_TOPICWHOTIME
- [x] RPL_INVITELIST
- [x] RPL_ENDOFINVITELIST
- [x] RPL_INVITING
- [x] RPL_YOUREOPER
- [x] ERR_NOSUCHNICK
- [x] ERR_NOSUCHCHANNEL
- [x] ERR_NORECIPIENT
- [x] ERR_NOTEXTTOSEND
- [x] ERR_NONICKNAMEGIVEN
- [x] ERR_ERRONEUSNICKNAME
- [x] ERR_USERNOTINCHANNEL
- [x] ERR_NOTONCHANNEL
- [x] ERR_USERONCHANNEL
- [x] ERR_NEEDMOREPARAMS
- [x] ERR_ALREADYREGISTRED
- [x] ERR_PASSWDMISMATCH
- [x] ERR_KEYSET
- [x] ERR_CHANNELISFULL
- [x] ERR_UNKNOWNMODE
- [x] ERR_INVITEONLYCHAN
- [x] ERR_BADCHANNELKEY
- [x] ERR_BADCHANMASK
- [x] ERR_CHANOPRIVSNEEDED
- [x] ERR_INVALIDKEY
- [x] ERR_INVALIDMODEPARAM

### Bonus

- [x] Bot (PRIVMSG to **GameBot** or on **#jan-ken-pon** channel)
- [x] File transfer (initiated with PRIVMSG thanks to [irssi DCC](https://irssi.org/documentation/help/dcc/))

### Enhancement

- [x] Configuration file: ```./ircserv <port> <password> [<configuration file>]``` (cf. [ircserv.conf](etc/ircserv.conf))

Client commands:

- [x] USER (second parameter is an integer translated to bitmask to set _i_ and _w_ modes respectively on _1000_ and _0100_)
- [x] PING
- [x] QUIT
- [x] LUSERS
- [x] MOTD
- [x] VERSION
- [x] NAMES
- [x] MODE (user modes: iow)
- [x] WHO
- [x] WALLOPS
- [x] SUMMON (e.g. summon disabled)
- [x] USERS (e.g. users disabled)

Server commands:

- [x] ERROR
- [x] PONG

Numeric replies:

- [x] RPL_YOURHOST
- [x] RPL_CREATED
- [x] RPL_MYINFO
- [x] RPL_ISUPPORT
- [x] RPL_UMODEIS
- [x] RPL_LUSERCLIENT
- [x] RPL_LUSEROP
- [x] RPL_LUSERUNKNOWN
- [x] RPL_LUSERCHANNELS
- [x] RPL_LUSERME
- [x] RPL_LOCALUSERS
- [x] RPL_GLOBALUSERS
- [x] RPL_ENDOFWHO
- [x] RPL_VERSION
- [x] RPL_WHOREPLY
- [x] RPL_NAMREPLY
- [x] RPL_ENDOFNAMES
- [x] RPL_MOTD
- [x] RPL_MOTDSTART
- [x] RPL_ENDOFMOTD
- [x] ERR_NOSUCHSERVER
- [x] ERR_TOOMANYCHANNELS
- [x] ERR_TOOMANYTARGETS
- [x] ERR_NOORIGIN
- [x] ERR_INPUTTOOLONG
- [x] ERR_UNKNOWNCOMMAND
- [x] ERR_NOMOTD
- [x] ERR_SUMMONDISABLED
- [x] ERR_USERSDISABLED
- [x] ERR_NOTREGISTERED
- [x] ERR_NOPRIVILEGES
- [x] ERR_UMODEUNKNOWNFLAG
- [x] ERR_USERSDONTMATCH
