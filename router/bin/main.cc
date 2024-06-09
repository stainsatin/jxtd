#include <string>
#include "proto/taskdep/message.h"
#include "management/taskdep/taskdep.h"
#include "management/inferprovider/task.h"
#include "router/lib/cmd.h"
#include "router/lib/flag/flag.h"
#include "misc/parser/at/ATParser.h"
#include <unordered_map>
#include <functional>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>

using Taskresult = ::jxtd::router::Taskresult;
using Taskmanager = ::jxtd::management::inferprovider::Task_Manager;
using Method = ::jxtd::management::inferprovider::Trigger_method;
using Task = ::jxtd::management::inferprovider::Task;
using Taskdepmanagement = ::jxtd::management::taskdep::ManagementClient;
using Endpoint = ::jxtd::proto::taskdep::Endpoint;
using ATMessage = ::jxtd::misc::parser::at::ATMessage;
using ATParser = ::jxtd::misc::parser::at::ATParser;
using Flag = ::jxtd::router::flag::Flag;

int set_endpoint(Endpoint &endpoint, std::string endpoint_)
{
    size_t pos = endpoint_.find_first_of(":");
    if (pos < 0)
        return -1;
    std::string proto = endpoint_.substr(0, pos);
    endpoint_.erase(0, pos + 3);
    pos = endpoint_.find_first_of(":");
    if (pos < 0)
        return -1;
    std::string url = endpoint_.substr(0, pos);
    endpoint_.erase(0, pos + 1);
    pos = endpoint_.find_first_of("/");
    int port = std::stoi(endpoint_);
    std::string path;
    if (pos > 0)
        path = endpoint_.substr(pos + 1);
    endpoint.set_proto(proto);
    endpoint.set_url(url);
    endpoint.set_port(port);
    endpoint.set_path(path);
    endpoint.set_authtype(0x0); // token
    return 0;
};

Taskresult &task = ::jxtd::router::task;
Taskdepmanagement &management_client = ::jxtd::router::management;
Taskmanager &manager = ::jxtd::router::manager;
Flag &flag = ::jxtd::router::flag_;

int main(int argc, char *argv[])
{
    if(std::strcmp(argv[1], "--help") == 0 || std::strcmp(argv[1], "-h") == 0) {
        // 执行帮助操作
        std::cout << "First parameter: management taskdep uri" << std::endl;
        std::cout << "Second parameter: computation taskdep uri" << std::endl;
        std::cout << "Third parameter: storage taskdep uri" << std::endl;
        std::cout << "Forth parameter: router datatransfer uri" << std::endl;
        std::cout << "Fifth parameter: storage datatransfer uri" << std::endl;
        std::cout << "Sixth parameter: datatransfer ssl(true or false)" << std::endl;
        std::cout << "Seventh parameter: listen port(default value:2878)" << std::endl;
        std::cout << "Eighth parameter: listen ipv4 address(default value:192.168.28.1)" << std::endl;
        std::cout << "Ninth parameter: storage ipv6 address(default value:fe80::2878::1)" << std::endl;
        return 0;
    }

    if(argc < 7 || argc > 10)
    {
        std::cerr << "command line number of arguments is wrong" << std::endl;
        return 1;
    }
    Endpoint management, computation, storage;
    flag.management = argv[1];
    flag.computation = argv[2];
    flag.storage = argv[3];
    if(set_endpoint(management, flag.management) == -1 || set_endpoint(computation, flag.computation) == -1 || set_endpoint(storage, flag.storage) == -1)
    {
        std::cerr << "taskdep address is misconfigured" << std::endl;
        return 2;
    }
    flag.datatransfer_router = argv[4];
    flag.datatransfer_storage = argv[5];
    if(strcmp(argv[6], "false") == 0)
        flag.ssl = false;
    else if(strcmp(argv[6], "true") == 0)
        flag.ssl = true;
    else
    {
        std::cerr << "taskdep ssl is wrong" << std::endl;
        return 3;
    }

    if(argc == 8)
    {
        flag.port = std::atoi(argv[7]);
    }
    else if(argc == 9)
    {
        flag.port = std::atoi(argv[7]);
        flag.ipv4 = argv[8];
    }
    else if(argc == 10)
    {
        flag.port = std::atoi(argv[7]);
        flag.ipv4 = argv[8];
        flag.ipv6 = argv[9];
    }
    else
    {
        ;
    }
    task.start();
    // token
    management_client.setup({management, computation, storage, "", flag.ssl});
    std::unordered_map<std::string, std::function<std::string(ATMessage &)>> functions;
    functions["REG"] = ::jxtd::router::REG;
    functions["RTUNREG"] = ::jxtd::router::RTUNREG;
    functions["RTBUSY"] = ::jxtd::router::RTBUSY;
    functions["MDDECL"] = ::jxtd::router::MDDECL;
    functions["MDRM"] = ::jxtd::router::MDRM;
    functions["MDPUSH"] = ::jxtd::router::MDPUSH;
    functions["CPSTART"] = ::jxtd::router::CPSTART;
    functions["CPMOD"] = ::jxtd::router::CPMOD;
    functions["CPRDY"] = ::jxtd::router::CPRDY;
    functions["CPVAL"] = ::jxtd::router::CPVAL;
    functions["TRANS"] = ::jxtd::router::TRANS;
    functions["STRANS"] = ::jxtd::router::STRANS;

    int server_fd;
    // ipv4
    server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // ipv6
    // server_fd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    // if(server_fd == -1)
    // {
    //     std::cerr << "Failed to create socket" << std::endl;
    //     return 4;
    // }

    // ipv4
    sockaddr_in serverAddr{};
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    if(inet_pton(AF_INET, flag.ipv4.c_str(), &(serverAddr.sin_addr)) <= 0) {
        std::cerr << "Invalid ipv4 address." << std::endl;
        return 5;
    }
    serverAddr.sin_port = htons(flag.port);

    // ipv6
    // sockaddr_in6 serverAddr{};
    // memset(&serverAddr, 0, sizeof(serverAddr));
    // serverAddr.sin6_family = AF_INET6;
    // serverAddr.sin6_port = flag.port;
    // if(inet_pton(AF_INET6, flag.ipv6.c_str(), &(serverAddr.sin6_addr)) <= 0) {
    //     std::cerr << "Invalid ipv6 address" << std::endl;
    //     return 6;
    // }

    if (bind(server_fd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Failed to bind socket" << std::endl;
        return 7;
    }

    if (listen(server_fd, SOMAXCONN) == -1) {
        std::cerr << "Failed to listen" << std::endl;
        return 8;
    }

    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(server_fd, &readSet);
    std::vector<int> clientSockets; // 存储客户端套接字

    while (true) {
        fd_set tempSet = readSet;

        if (select(FD_SETSIZE, &tempSet, nullptr, nullptr, nullptr) == -1) {
            std::cerr << "Failed to select" << std::endl;
            close(server_fd);
            return 1;
        }

        for (int i = 0; i < FD_SETSIZE; ++i) {
            if (FD_ISSET(i, &tempSet)) {
                if (i == server_fd) {
                    sockaddr_in clientAddress{};
                    socklen_t clientAddressLength = sizeof(clientAddress);
                    int clientSocket = accept(server_fd, reinterpret_cast<sockaddr*>(&clientAddress), &clientAddressLength);
                    if (clientSocket == -1) {
                        std::cerr << "Failed to accept connection" << std::endl;
                        close(server_fd);
                        return 10;
                    }

                    FD_SET(clientSocket, &readSet);
                    clientSockets.push_back(clientSocket);
                } else {
                    char buffer[4096];
                    memset(buffer, 0, sizeof(buffer));
                    ssize_t bytesRead = recv(i, buffer, sizeof(buffer) - 1, 0);
                    if (bytesRead <= 0) {
                        std::cerr << "Connection closed" << std::endl;
                        close(i);
                        FD_CLR(i, &readSet);

                        auto it = std::find(clientSockets.begin(), clientSockets.end(), i);
                        if (it != clientSockets.end()) {
                            clientSockets.erase(it);
                        }
                    } else {
                        ATParser *parser = new ATParser(buffer);
                        ATMessage *message = parser->parse();
                        std::string send_;
                        auto iter = functions.find(message->cmd);
                        if(iter != functions.end())
                            send_ = std::move(iter->second(std::ref(*message)));
                        else
                            send_ = "AT+ENVAL";
                        send_ += "\r\n";
                        send(i, send_.data(), send_.size(), 0);
                    }
                }
            }
        }
    }
    task.stop();
    close(server_fd);
    return 0;
}