#include "ACommands.hpp"
#include <string>

Mode::Mode(Server &server) : ACommands(server) 
{
    this->server = server;
}

void Mode::execute(int fd, const std::string &line)
{
    std::istringstream stream(line);
    std::string word;
    std::string target, modes;
    std::vector<std::string> args;
    Channel *channel;
    Client *client;
    std::string cmd = "MODE";
    int i;

    i = 0;
    while (stream >> word){
        i++;
        if (i == 2)
            target = word;
        else if (i == 3)
            modes = word;
        else if (i > 3)
            args.push_back(word);
    };


    if (target.empty() || modes.empty()) {
        //this->server.sendResponse(ERR_NEEDMOREPARAMS(cmd), fd);
        return ;
    }
    
    if (target[0] != '#'){
        std::cout << "Only channel modes are available" << std::endl;
        return ;
    }

    channel = this->server.GetChannelByName(target);
    client = this->server.GetClient(fd);

    if (!client->GetLoggedIn()) {
        this->server.sendResponse(ERR_NOTREGISTERED(cmd), fd);
        return;
    }

    if (!channel) {
        this->server.sendResponse(ERR_NOSUCHCHANNEL(client->GetNickname(), target), fd);
        return ;
    }

    if (!channel->IsAdmin(fd)){
        this->server.sendResponse(ERR_CHANOPRIVSNEEDED(client->GetNickname(), channel->GetName()), fd);
        return ;
    }

    this->channel(fd, target, modes, args);
    
}

void Mode::channel(int fd, const std::string &target, std::string &modes, std::vector<std::string> args)
{
    bool value;

    std::string::iterator str_it = modes.begin();
    std::vector<std::string>::iterator arg_it = args.begin();
    Channel *channel;
    std::string cmd = "MODE";

    Client *client = this->server.GetClient(fd);
    Client *tmp;
    value = true;

    //Handle modes
    while (str_it != modes.end()){

        //Handle set invite positive/negative
        if (*str_it == 'i'){
            if (!value){
                this->server.GetChannelByName(target)->SetInvite(false);
                this->server.sendResponse(RPL_MODEMSG(client->GetNickname(), client->GetUsername(), target, "-i", ""), fd);
                this->server.GetChannelByName(target)->SendToAll(RPL_MODEMSG(client->GetNickname(), client->GetUsername(), target, "-i", ""), fd, this->server);
            }
            else {
                this->server.GetChannelByName(target)->SetInvite(true);
                this->server.sendResponse(RPL_MODEMSG(client->GetNickname(), client->GetUsername(), target, "+i", ""), fd);
                this->server.GetChannelByName(target)->SendToAll(RPL_MODEMSG(client->GetNickname(), client->GetUsername(), target, "+i", ""), fd, this->server);
            }
        }


        //Handle topic restricted positive/negative
        else if (*str_it == 't') { 
            if (!value){
                this->server.GetChannelByName(target)->SetTopicRestricted(false);
                this->server.sendResponse(RPL_MODEMSG(client->GetNickname(), client->GetUsername(), target, "-t", ""), fd);
                this->server.GetChannelByName(target)->SendToAll(RPL_MODEMSG(client->GetNickname(), client->GetUsername(), target, "-t", ""), fd, this->server);
            }
            else {
                this->server.GetChannelByName(target)->SetTopicRestricted(true);
                this->server.sendResponse(RPL_MODEMSG(client->GetNickname(), client->GetUsername(), target, "+t", ""), fd);
                this->server.GetChannelByName(target)->SendToAll(RPL_MODEMSG(client->GetNickname(), client->GetUsername(), target, "+t", ""), fd, this->server);
            }
        }


        //Handle channel password
        else if (*str_it == 'k'){
            //Remove password from channel
            if (!value){
                this->server.GetChannelByName(target)->SetPassword("");
                this->server.sendResponse(RPL_MODEMSG(client->GetNickname(), client->GetUsername(), target, "-k", ""), fd);
                this->server.GetChannelByName(target)->SendToAll(RPL_MODEMSG(client->GetNickname(), client->GetUsername(), target, "-k", ""), fd, this->server);
            }

            //Set password on channel
            else {
                //Check if there is argument for password
                if (arg_it == args.end()){
                    this->server.sendResponse(ERR_NEEDMOREPARAMS(cmd), fd);
                    str_it++;
                    continue ;
                }
                
                this->server.GetChannelByName(target)->SetPassword(*arg_it);
                this->server.sendResponse(RPL_MODEMSG(client->GetNickname(), client->GetUsername(), target, "+k", *arg_it), fd);
                this->server.GetChannelByName(target)->SendToAll(RPL_MODEMSG(client->GetNickname(), client->GetUsername(), target, "+k", *arg_it), fd, this->server);
                arg_it++;
            }

        }

        //Handle moderation
        else if (*str_it == 'o') {
            channel = this->server.GetChannelByName(target);

            if (arg_it == args.end()){
                this->server.sendResponse(ERR_NEEDMOREPARAMS(cmd), fd);
                str_it++;
                continue ;
            }

            //Add client to admin first. Remove from client later
            if (!value) {
                
                if (!channel->GetAdminByNick(*arg_it) && !channel->GetClientByNick(*arg_it)) {
                    this->server.sendResponse(ERR_NOSUCHNICK(client->GetNickname(), *arg_it), fd);
                    return ;
                }

                // Get the client and check if they are already an admin (operator)
                tmp = channel->GetAdminByNick(*arg_it);
                if (!tmp) {
                    // If the client isn't an operator, respond with a message
                    this->server.sendResponse(CLIENT_NOTOPERATOR(*arg_it), fd);
                    cout << "Client is not an operator." << endl;
                    return;
                }

                channel->AddClient(channel->GetAdminByNick(*arg_it));
                channel->RemoveAdmin(*arg_it);
                channel->SendToAll(RPL_MODEMSG(client->GetNickname(), client->GetUsername(), channel->GetName(), "-o", *arg_it), fd, this->server);
                this->server.sendResponse(RPL_MODEMSG(client->GetNickname(), client->GetUsername(), channel->GetName(), "-o", *arg_it), fd);
            } 
            
            //Add client to admin first. Remove from admin later
            else {

                if (!channel->GetAdminByNick(*arg_it) && !channel->GetClientByNick(*arg_it)) {
                    this->server.sendResponse(ERR_NOSUCHNICK(client->GetNickname(), *arg_it), fd);
                    return ;
                }

                // Get the client and check if they are already an admin (operator)
                tmp = channel->GetAdminByNick(*arg_it);
                if (tmp) {
                    // If the client is already an operator, respond with a message
                    this->server.sendResponse(ALREADY_OPERATOR(*arg_it), fd);
                    cout << "Already a channel operator" << endl;
                    return;
                }

                // Get the client by nickname
                tmp = channel->GetClientByNick(*arg_it);
                if (!tmp) {
                    // If the client doesn't exist in the channel, respond with an error message
                    this->server.sendResponse(CLIENT_NOTFOUND(*arg_it), fd);
                    cout << "Client not found in channel" << endl;
                    return;
                }

                channel->AddAdmin(channel->GetClientByNick(*arg_it));
                channel->RemoveClientNick(*arg_it);
                channel->SendToAll(RPL_MODEMSG(client->GetNickname(), client->GetUsername(), channel->GetName(), "+o", *arg_it), fd, this->server);
                this->server.sendResponse(RPL_MODEMSG(client->GetNickname(), client->GetUsername(), channel->GetName(), "+o", *arg_it), fd);

            }
            arg_it++;
        }
            

        //Handle max number of users per channel
        else if (*str_it == 'l')
        {
            //Set limit to -1 in case of false
            if (!value){
                this->server.GetChannelByName(target)->SetLimit(-1);
                arg_it++;
                this->server.sendResponse(RPL_MODEMSG(client->GetNickname(), client->GetUsername(), target, "-l", ""), fd);
                this->server.GetChannelByName(target)->SendToAll(RPL_MODEMSG(client->GetNickname(), client->GetUsername(), target, "-l", ""), fd, this->server);
            }

            //Set limit to whatever argument
            else {
                //Check if there is argument for password
                if (arg_it == args.end()){
                    this->server.sendResponse(ERR_NEEDMOREPARAMS(cmd), fd);
                    str_it++;
                    continue ;
                }
                
                this->server.sendResponse(RPL_MODEMSG(client->GetNickname(), client->GetUsername(), target, "+l", *arg_it), fd);
                this->server.GetChannelByName(target)->SendToAll(RPL_MODEMSG(client->GetNickname(), client->GetUsername(), target, "+l", *arg_it), fd, this->server);
                this->server.GetChannelByName(target)->SetLimit(std::atoi((*arg_it).c_str())); 
                arg_it++;
            }
        }

        else if (*str_it == '+')
            value = true;

        //Change to false if its minus
        else if (*str_it == '-')
            value = false; 

        //Check if it's invalid characters/mode
        else
            this->server.sendResponse(ERR_UMODEUNKNOWNFLAG(client->GetNickname()), fd); 
        

        str_it++;
    }
}