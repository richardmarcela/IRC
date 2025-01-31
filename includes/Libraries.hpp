#ifndef LIBRARIES_HPP
#define LIBRARIES_HPP

#include <iostream>
#include <vector> //-> for vector
#include <sys/socket.h> //-> for socket()
#include <sys/types.h> //-> for socket()
#include <netinet/in.h> //-> for sockaddr_in
#include <fcntl.h> //-> for fcntl()
#include <unistd.h> //-> for close()
#include <arpa/inet.h> //-> for inet_ntoa()
#include <poll.h> //-> for poll()
#include <csignal> //-> for signal()
#include <cstring>
#include <sstream>
#include <bits/stdc++.h> 
#include <map>

#include "Client.hpp"
#include "Server.hpp"
#include "Channel.hpp"
#include "Parser.hpp"
#include "Commands.hpp"

// typedef std::pair<t_args, std::string> t_input

inline void ThrowException(std::string msg)
{
    throw(std::runtime_error(msg));
}

#endif // !LIBRARIES_HPP