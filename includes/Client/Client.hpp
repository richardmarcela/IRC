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

using std::cout;
using std::endl;

class Client {
    int fd;
    std::string ip_add;

    public:
        Client();
        int GetFd();
        void SetFd(int fd);
        void SetIpAdd(std::string ip_add);
};