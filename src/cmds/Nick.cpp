#include "ACommands.hpp"

//Check existing user
//

Nick::Nick(Server &server): ACommands(server){
	this->server = server;
}

void Nick::execute(int fd, const std::string &line){
	std::istringstream stream(line);
	std::string word;
	Client *client;
	std::map<int, Client *>::iterator it;
	int i;
	
	it = server.getClients()->find(fd);
	client = it->second;
	it = server.getClients()->begin();
	i = -1;

	//Check auth handling
	if (server.getPassword() != "" && client->GetPassword() != server.getPassword()) {
		server.sendResponse(":myserver 464 " 
			+ (!client->GetNickname().empty() ? client->GetNickname() : "*")  
			+ " :Password incorrect\r\n", fd);
		return ;
	}
	while (stream >> word) {
		++i;
		if (i == 2) break;
		if (i == 1) {
			//Invalid Nickname
			if(word[0] == ':'){
				server.sendResponse(":myserver 432 " 
					+ (!client->GetNickname().empty() ? client->GetNickname() : "*") 
					+ " :Erroneous nickname\r\n", fd);
				return ;
			}

			//Nickname in use
			while (it != server.getClients()->end()) {
				if (toLowerString(it->second->GetNickname()) == toLowerString(word) && fd != it->second->GetFd()) {
					server.sendResponse(":myserver 433 " 
						+ (!client->GetNickname().empty() ? client->GetNickname() : "*") 
						+ " :Nickname given is already in use.\r\n", fd);
					return ;
				}
				it++;
			}

			it = server.getClients()->find(fd);
			if (it == server.getClients()->end())
				return ;
			server.sendResponse(":" + client->GetNickname() + "!" + client->GetUsername() + "@localhost NICK " + word + "\r\n", fd);
			this->SendSharedChannels(client, word, fd);
			it->second->SetNickname(word);
		}
	}

	//Missing argument
	if (i == 0)
		server.sendResponse(":myserver 431 " +  client->GetNickname() + " : No nickname has been provided.\r\n", fd);
}

void Nick::SendSharedChannels(Client *client, std::string nickname, int fd) {
    if (this->server.getChannels()->empty() || !client || nickname.empty()) {
        return;
    }
	
    std::map<std::string, Channel *>::iterator it = this->server.getChannels()->begin();
    while (it != this->server.getChannels()->end()) {
        if (it->second->GetClientInChannel(client->GetNickname())) {
            it->second->SendToAll(":" + client->GetNickname() + "!" + client->GetUsername() + "@localhost NICK " + nickname + "\r\n", fd, this->server);
        }
        it++;
    }
}	