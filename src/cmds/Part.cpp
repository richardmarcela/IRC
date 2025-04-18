#include "ACommands.hpp"

Part::Part(Server &server): ACommands(server){
	this->server = server;
}

void Part::execute(int fd, const std::string &line) {
    std::vector<std::string> tokens;
    std::istringstream ss(line);
    std::string token;
    Channel *channel = NULL;
    std::string cmd = "PART";

    while (std::getline(ss, token, ' ')) {
        tokens.push_back(token);
    }

    if (tokens.size() < 2) {
        this->server.sendResponse(ERR_NEEDMOREPARAMS(cmd), fd);
        return;
    }

    std::string channelsString = tokens[1];
    std::vector<std::string> channelTokens;
    std::istringstream channelStream(channelsString);

    while (std::getline(channelStream, token, ',')) {
        channelTokens.push_back(token);
    }

    Client *client = this->server.GetClient(fd);
    if (!client || !client->GetLoggedIn()) {
        this->server.sendResponse(ERR_NOTREGISTERED(cmd), fd);
        return;
    }

    for (size_t i = 0; i < channelTokens.size(); ++i) {
        const std::string &channelName = channelTokens[i];

        if (channelName.empty() || channelName[0] != '#') {
            this->server.sendResponse(ERR_NOSUCHCHANNEL(client->GetNickname(), channelName), fd);
            continue;
        }

        channel = this->server.GetChannel(channelName);
        if (!channel) {
            this->server.sendResponse(ERR_NOSUCHCHANNEL(client->GetNickname(), channelName), fd);
            continue;
        }

        if (!channel->GetClientInChannel(this->server.GetClient(fd)->GetNickname())) {
            this->server.sendResponse(ERR_INVITERINCHANNEL(client->GetNickname(), channelName), fd);
            continue;
        }

        if (channel->GetClientByNick(client->GetNickname())) {
            channel->RemoveClient(fd);
        }
        if (channel->GetAdminByNick(client->GetNickname())) {
            channel->RemoveAdmin(client->GetNickname());
        }
        channel->SendToAll(RPL_PARTMSG(client->GetNickname(), channelName), fd, this->server);
        this->server.sendResponse(RPL_PARTMSG(client->GetNickname(), channelName), fd);

        cout << channel->GetClients().empty() << endl;
        cout << channel->GetAdmins().empty() << endl;
        // Verifica se o canal está vazio e remove
        if (channel->GetClients().empty() && channel->GetAdmins().empty()) {
            this->server.removeChannel(channelName);
        }
    }
}
