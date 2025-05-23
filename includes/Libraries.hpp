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
#include <ctime>
#include <memory>
#include <string>

#include "Client.hpp"
#include "Server.hpp"
#include "Channel.hpp"
#include "Parser.hpp"
#include "ACommands.hpp"

#define CRLF "\r\n"

#define RPL_CONNECTED(nickname) \
    (":myserver 001 " + nickname + " :Welcome to the IRC server!" + CRLF)

#define RPL_HOSTINFO(nickname) \
    (":myserver 002 " + nickname + " :Your host is running IRC Server, running version 1.0" + CRLF)

#define RPL_CREATIONTIME(nickname, time) \
    (":myserver 003 " + nickname + " :This server was created " + time + CRLF)

#define RPL_SERVERINFO(nickname) \
    (":myserver 004 " + nickname + " myserver 1.0 oiw btkl" + CRLF)

#define RPL_ISUPPORT(nickname) \
    (":myserver 005 " + nickname + " CHANTYPES=# PREFIX=(o,v)@+ MAXNICKLEN=30 CHANLIMIT=#&:10 :are supported by this server" + CRLF)

#define RPL_MOTDSTART(nickname) \
    (":myserver 375 " + nickname + " :- IRC Message of the day - AMELO" + CRLF)

#define RPL_MOTDLINES(nickname, line) \
    (":myserver 372 " + nickname + " :- " + line + CRLF)

#define RPL_MOTDEND(nickname) \
    (":myserver 376 " + nickname + " :End of MOTD command." + CRLF)

#define RPL_JOINMSG(nickname, username, channelname) \
    (":" + nickname + "!" + username + " JOIN " + channelname + CRLF)

#define RPL_NAMREPLY(nickname, channelname, clientslist) \
    (":myserver 353 " + nickname + " = " + channelname + " :" + clientslist + CRLF)

#define RPL_ENDOFNAMES(nickname, channelname) \
    (":myserver 366 " + nickname + " " + channelname + " :End of /NAMES list" + CRLF)

#define RPL_KICKMSG(nickname, channelname, target) \
    (":" + nickname + " KICK " + channelname + " " + target + CRLF)

#define RPL_TOPICIS(nickname, channelname, topic) \
    (":myserver 332 " + nickname + " " + channelname + " :" + topic + CRLF)

#define ERR_NOSUCHCHANNEL(nickname, channel) \
    (":myserver 403 " + nickname + " " + channel + " :No such channel" + CRLF)
    
#define ERR_INVITEONLYCHAN(nickname, channel) \
    (":myserver 473 " + nickname + " " + channel + " :Cannot join channel (+i)" + CRLF)

#define ERR_TOOMANYCHANNELS(nickname, channel) \
    (":myserver 405 " + nickname + " " + channel + " :You have joined too many channels" + CRLF)

#define ERR_BADCHANNELKEY(nickname, channel) \
    (":myserver 475 " + nickname + " " + channel + " :Cannot join channel (+k) - bad key" + CRLF)

#define ERR_CHANNELISFULL(nickname, channel) \
    (":myserver 471 " + nickname + " " + channel + " :Cannot join channel (+l)" + CRLF)

#define ERR_NEEDMOREPARAMS(cmd) \
    (":myserver 461 " + cmd + " :Not enough parameters" + CRLF)

#define RPL_PARTMSG(nickname, channel) \
    (":" + nickname + " PART " + channel + CRLF)
    
#define RPL_MODEMSG(nickname, username, channelname, mode, target) \
    (":" + nickname + "!" + username + " MODE " + channelname + " " + mode + " " + target + CRLF)

#define ERR_CHANOPRIVSNEEDED(nickname, channel) \
    (":myserver 482 " + nickname + " " + channel + " :You're not a channel operator" + CRLF)

#define RPL_TOPIC(nickname, channel, topic) \
    (":myserver 332 " + nickname + " " + channel + " :" + topic + CRLF)

#define RPL_NOTOPIC(nickname, channel) \
    (":myserver 331 " + nickname + " " + channel + " :No topic is set" + CRLF)

#define RPL_TOPICMSG(nickname, channel, topic) \
    (":" + nickname + " TOPIC " + channel + " :" + topic + CRLF)

#define ERR_NOSUCHNICK(nickname, target) \
    (":myserver 401 " + nickname + " " + target + " :No such nick/channel" + CRLF)

#define ERR_USERNOTINCHANNEL(client, nick, channel) \
    (":myserver 441 " + client + " " + nick + " " + channel + " :They aren't on that channel\r\n")

#define ERR_INVITERINCHANNEL(nickname, name) \
    (":myserver 442 " + nickname + " " + name + " :You're not on that channel" + CRLF)

#define ERR_USERCANINVITE(nickname, name) \
    (":myserver 482 " + nickname + " " + name + " :You're not channel operator" + CRLF)

#define ERR_ALREADYINCHANNEL(nickname, name) \
    (":myserver 443 " + nickname + " " + name + " :Already in channel" + CRLF)

#define ERR_NORECIPIENT(nickname, command) \
    (":myserver 411 " + nickname + " :No recipient given (" + command + ")" + CRLF)

#define ERR_NOTEXTTOSEND(nickname) \
    (":myserver 412 " + nickname + " :No text to send" + CRLF)

#define ERR_CANNOTSENDTOCHAN(nickname, channel) \
    (":myserver 404 " + nickname + " " + channel + " :Cannot send to channel" + CRLF)

#define ERR_TOOMANYTARGETS(nickname, target) \
    (":myserver 407 " + nickname + " " + target + " :Too many targets" + CRLF)

#define ERR_UMODEUNKNOWNFLAG(nickname) \
    (":myserver 501 " + nickname + " :Unknown MODE flag" + CRLF)

#define ERR_USERSDONTMATCH(nickname) \
    (":myserver 502 " + nickname + " :Can't change mode for other users" + CRLF)

#define CLIENT_NOTOPERATOR(nickname) \
    (nickname + " :Client is not an operator" + CRLF)

#define ALREADY_OPERATOR(nickname) \
    (nickname + " :Already a channel operator" + CRLF)

#define CLIENT_NOTFOUND(nickname) \
    (nickname + " :Client not found in channel" + CRLF)

#define ERR_NOTREGISTERED(cmd) \
    (":myserver 451 " + cmd + " :You have not registered" + CRLF)


#define RED "\033[0;31m"
#define RESET "\033[0m"

// typedef std::pair<t_args, std::string> t_input

inline void ThrowException(std::string msg)
{
    throw(std::runtime_error(msg));
}

std::string toLowerString(const std::string& str);
bool ends_with(const std::string& str, const std::string& suffix);

#endif // !LIBRARIES_HPP
