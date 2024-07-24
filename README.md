# README - ft_irc
## This project was done by Anes (Abdel) Benamara, Elina (Elina) Jankovskaja and Karima (Karimo) Sadykova

This project aims to create an IRC server.

Below will be the description of our project and how we decided to implement it as well as some discussions about functions
and how we chose what would be best for us. Some definitions as well because why not.

Let's start with the most basic question: _what is an IRC server ?_

An **IRC** (Internet Relay protocol) is a protocol based on multiple continuous exchanges. It allows two types of connections: the classic client/server connections, which allow a client to connect to the network, and server/server connections, which enable the management of large networks supporting high scalability. It is asynchronous: servers and clients send their data without truly worrying about the other's response. An example of an IRC server is for example _telnet_.

Next, comes the concept of a **socket**, a socket is a communication interface that allows data to be exchanged between 2 processes. Either on the same machine or on 2 different machines through a network. It works like an endpoint for sending and receiving data.

After that we explored the concept of a **poll**, the man says its purpose is to wait for some event on a file descriptor.
But there are many polls available. poll(), epoll(), kqueue and select(). Those lead to an interesting discussion.
poll() and select() are deprecated versions of epoll() and kequeue().

Let n the number of file descriptors

| Function | OS compatibility | Comments | Complexity
| --- | --- | --- | --- |
| select() | Everywhere | Not optimised, not practical, but since its installed on all platforms its often used as fallback | O(n) |
| poll() | Linux | More practical then select() in the way that it counts file descriptors, the complexity is still O(n). it works like this: when you call Paul, the kernel needs to setup the necessary data structures to monitor the list of file descriptors you provided, it then waits for an event from those fds, as soon as an event is triggered, Paul returns, and the kernel tears down all the monitoring it did in the beginning. These repeated actions aren't efficient. Thank you Paul. | O(n) |
epoll() | Linux | This is poll but better. Unlike poll, epoll maintains a persistent state, the kernel sets up watches for fds once and you have to remove them yourself, it can wait for events without having to redo the setup each time. It can handle more files than poll() | O(1)
kqueue() | OS X, FreeBSD | Option not explored since on Mac | O(1) |

Next we looked at **bind**, used to associate a socket to a local adress (with an IP and a specific port). Its used on the servers side to specify on which adress and which port the server must listen to entering connections.

**connect** is used by a _client_ to establish a connexion with a server. Again IP and port of the server necessary.

**htons, htonl** are functions to convert between bytes from the host and the network.

**ntohs, ntohl** are functions to do the contrary so from the network to the host.

So basically, connect would use htons (host to network short).

### Additional files

- ft_irc.excalidraw : an excalidraw file to view the UML scheme



### Bibliography:

- [IRC explanation](https://mathieu-lemoine.developpez.com/tutoriels/irc/protocole/?page=page-2)

- [Poll discussions](https://stackoverflow.com/questions/26420947/what-are-the-underlying-differences-among-select-epoll-kqueue-and-evport)

- [Poll discussions](https://www.reddit.com/r/C_Programming/comments/1clu2fg/difference_betweel_poll_and_epoll/)

- [RFC 2810](https://datatracker.ietf.org/doc/html/rfc2810)

- [RFC 2811](https://datatracker.ietf.org/doc/html/rfc2811)

- [RFC 2812](https://datatracker.ietf.org/doc/html/rfc2812)

- [RFC 2813](https://datatracker.ietf.org/doc/html/rfc2813)

