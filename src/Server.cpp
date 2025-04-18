#include "Server.hpp"

bool Server::signal = false;

void Server::sendResponse(std::string response, int fd) {
    // Convert std::string to const char* for send() function
    const char* msg = response.c_str();
    // if (send(fd, msg, response.size(), 0) == -1) {
    //     std::cerr << "Response send() failed: " << strerror(errno) << std::endl;
    // }
    send(fd, msg, response.size(), 0);
}

void Server::SignalHandler(int signum)
{
    (void)signum;
    cout << endl << "Signal received!" << endl;
    // Server::signal = true;
    if (signum == SIGPIPE) {
            std::cout << "SIGPIPE received: ignoring..." << std::endl;
        } else {
            std::cout << std::endl << "Signal received!" << std::endl;
            Server::signal = true;
        }
}

void Server::ClearClients(int fd)
{
    std::map<int, Client *>::iterator it = this->clients.begin();
	
    for (size_t i = 0; i < fds.size(); i++) //remove da poll
    {
        if (fds[i].fd == fd)
        {
            fds.erase(fds.begin() + i);
            break;
        }
    }

    while (it != this->clients.end()) {
        if (it->second->GetFd() == fd){
            this->removeClient(it->first);
            break;
        }
		it++;
	}
}

//acho que isso pode ir para utils
void Server::CloseFds()
{
    std::map<int, Client *>::iterator it = this->clients.begin();

    while (it != this->clients.end()){
        close(it->second->GetFd());
		it++;
	}
    if (server_socket != -1)
    {
        cout << "Server <" << server_socket << "> disconnected" << endl;
        close(server_socket);
    }
}

void Server::AcceptNewClient()
{
    Client *cli = new Client();
    struct sockaddr_in cli_add;
    struct pollfd new_poll;
    socklen_t len = sizeof(cli_add);

    int incofd = accept(server_socket, (sockaddr *)&(cli_add), &len); //aceitar o novo cliente
    if (incofd == -1)
    {cout << "accept() failed" << endl;return;}
    
    if (fcntl(incofd, F_SETFL, O_NONBLOCK) == -1) //non blocking 
    {cout << "fcntl() failed" << endl; return;}

    new_poll.fd = incofd; //adicionar o socket do client ao pollfd
    new_poll.events = POLLIN;
    new_poll.revents = 0;

    cli->SetFd(incofd); //settar o fd do cliente
    cli->SetIpAdd(inet_ntoa((cli_add.sin_addr))); //converte o endereco de ip para string e setta
    this->addClient(cli);
    fds.push_back(new_poll); //adiciona o socket do cliente ao pollfd

}

void Server::ReceiveNewData(int fd)
{
    char buff[1024]; //para os dados recebidos
    memset(buff, 0, sizeof(buff)); //limpar o buffer
    Client *cli = this->clients[fd];
    std::vector<std::string> cmd;   

    ssize_t bytes = recv(fd, buff, sizeof(buff) - 1, 0); //recebe os dados

    if (bytes <= 0) //cliente desconectou
    {
        std::vector<std::string> toRemove;
        for (std::map<std::string, Channel*>::iterator it = this->channels.begin(); it != this->channels.end(); ++it) {
            std::string quitMsg = ":" + cli->GetNickname() + "!" + cli->GetUsername() + "@localhost QUIT :Leaving\r\n";
            Channel* chan = it->second;
            if (!chan) continue;
    
            const std::string& chanName = it->second->GetName();
    
            if (chan->GetClientByNick(cli->GetNickname()) || chan->GetAdminByNick(cli->GetNickname())) {
                chan->SendToAll(quitMsg, fd, *this);
            }
    
            if (chan->GetClientByNick(cli->GetNickname()))
                chan->RemoveClient(fd);
    
            if (chan->GetAdminByNick(cli->GetNickname()))
                chan->RemoveAdmin(cli->GetNickname());
    
            if (chan->GetClients().empty() && chan->GetAdmins().empty())
                toRemove.push_back(chanName);
        }

        // Remove canais vazios
        for (size_t i = 0; i < toRemove.size(); ++i) {
            this->removeChannel(toRemove[i]);
        }

        cout << "Client <" << fd << "> disconnected" << endl;
        ClearClients(fd);
        close(fd);
        delete(cli);
    }
    
    else //imprime os dados recebidos
    {
        //associar o buffer ao cliente aqui
        buff[bytes] = '\0';
        //fazer parse/split do buffer, e para cada posicao do vetor retornado fazer parse do cmd
        //here you can add your code to process the received data: parse, check, authenticate, handle the command, etc...
        cmd = SplitBuffer(buff);

        for (size_t i = 0; i < cmd.size(); i++) {
            cli->SetBuffer("");
            if (!ends_with(cmd[i], "\n")) {
                cli->SetTemp(cmd[i]);
                continue;
            } 
            
            else {
                cli->VectAdd(cli->GetTemp() + cmd[i]);
                cli->SetTemp("");
            }
        }

    }
}

void Server::ServerInit()
{
    struct sockaddr_in server_address;
	struct pollfd pollfd;

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(this->port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
        ThrowException("failed to create socket");

    int en = 1;
    //reutilizar endereco para quando reiniciar o programa
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1)
        ThrowException("failed to set option (SO_REUSEADDR) on socket");
    //settar non blocking (execucao nao trava se uma acao nao finalizar)
    if (fcntl(server_socket, F_SETFL, O_NONBLOCK) == 1)
        ThrowException("failed to set option (O_NONBLOCK) on socket");
    //associar o socket ao endereco
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
        ThrowException("failed to bind the socket");
    //receber conexoes, setted para ser socket passivo (apenas esperar para ouvir)
    if (listen(server_socket, SOMAXCONN) == -1)
        ThrowException("listen() failed");

    this->setTime();

    pollfd.fd = server_socket; //socket para ser monitorado
    pollfd.events = POLLIN; //prepara para ler dados
    pollfd.revents = 0; //eventos que aconteceram, sera preenchido pelo poll
    fds.push_back(pollfd); //adiciono o socket criado ao FDS, essa sera a lista monitorada pelo poll

    while (Server::signal == false)
    {
        if ((poll(&fds[0], fds.size(), -1)) == -1 && Server::signal == false) //monitora a lista de sockets, espera por um evento
            ThrowException("poll() failed");

        for (size_t i = 0; i < fds.size(); i++)
        {
            if (fds[i].revents & POLLIN) //se tiver um evento neste socket
            {
                if (fds[i].fd == server_socket)
                    AcceptNewClient();
                else {
                    ReceiveNewData(fds[i].fd);
                    fds[i].events = POLLOUT;
                }
            }
            
            else if (fds[i].revents & POLLOUT) {
                Client *client;
                
                client = this->GetClient(fds[i].fd);
                if (!client)
                    return ;
                
                for (size_t j = 0; j < client->GetVect().size(); j++) {
                    Handler(fds[i].fd, client->GetVect()[j], *this);
                }

                //Check auth.
                if ( !client->GetNickname().empty() && !client->GetUsername().empty() 
                && !client->GetLoggedIn() ){

                    //Server without password
                    if (this->getPassword().empty())
                        this->SendMessages(client->GetFd());

                    //Server with password
                    else if (client->GetPassword() == this->getPassword())
                        this->SendMessages(client->GetFd());

                }

                client->VectReset();
                fds[i].events = POLLIN;
            }
        }
    }
    CloseFds();
    closeClientConnections();
    cleanupChannels();
}

std::vector<std::string> Server::SplitBuffer(std::string str)
{
    std::vector<std::string> vec;
    std::string line;

    for (size_t i = 0; i < str.size(); ++i) {
        line += str[i];

        if (str[i] == '\n') {
            vec.push_back(line);
            line.clear();
        } else if (str[i] == '\r') {
            // Check for \r\n
            if (i + 1 < str.size() && str[i + 1] == '\n') {
                line += '\n';  // include \n in the line
                ++i; // skip the next character since we've processed it
            }
            vec.push_back(line);
            line.clear();
        }
    }

    // Push the remaining line (in case it doesn't end with newline)
    if (!line.empty()) {
        vec.push_back(line);
    }

    return vec;
}

void Server::SendMessages(int fd)
{
    Client *client = this->GetClient(fd); // Get client reference to avoid multiple lookups
    client->SetLogged(true);

    // 001 - Welcome message
    sendResponse(RPL_CONNECTED(client->GetNickname()), fd);

    // 002 - Host info
    sendResponse(RPL_HOSTINFO(client->GetNickname()), fd);

    // 003 - Server creation time
    sendResponse(RPL_CREATIONTIME(client->GetNickname(), this->getTime()), fd);

    // 004 - Server information and supported modes
    sendResponse(RPL_SERVERINFO(client->GetNickname()), fd);

    // 005 - Supported server features (including CHANLIMIT)
    sendResponse(RPL_ISUPPORT(client->GetNickname()), fd);

    // 375 - Start of MOTD
    sendResponse(RPL_MOTDSTART(client->GetNickname()), fd);

    // 372 - MOTD lines
    sendResponse(RPL_MOTDLINES(client->GetNickname(), "Welcome to our IRC server!"), fd);
    sendResponse(RPL_MOTDLINES(client->GetNickname(), "Please be respectful and follow the rules."), fd);
    sendResponse(RPL_MOTDLINES(client->GetNickname(), "Have fun chatting!"), fd);

    // 376 - End of MOTD
    sendResponse(RPL_MOTDEND(client->GetNickname()), fd);
}

bool Server::registered(int fd) 
{
    if (GetClient(fd) && !GetClient(fd)->GetNickname().empty() && !GetClient(fd)->GetUsername().empty() && GetClient(fd)->GetNickname() != "*" && GetClient(fd)->GetLoggedIn())
		return false;
	return true;
}

void Server::closeClientConnections() 
{
    for (std::map<int, Client *>::iterator it = clients.begin(); it != clients.end(); ++it) {
        // Send final shutdown message
        std::string message = "Server is shutting down, goodbye!";
        send(it->second->GetFd(), message.c_str(), message.size(), 0);
        
        // Close the client socket
        close(it->second->GetFd());
        delete(it->second);
    }
}

void Server::cleanupChannels() 
{
    for (std::map<std::string, Channel *>::iterator it = channels.begin(); it != channels.end(); ++it) {
        //it->second->ClearClients();
        delete(it->second);
    }
    channels.clear();
}