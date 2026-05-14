/*
** server.cpp -- a C++ stream socket server demo
*/

#include <iostream>
#include <thread>
#include <cstring>
#include "litosock.h"

#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10   // how many pending connections queue will hold


void handleClient(litosock::Socket client)
{
    if(send(client.get(), "Hello, world!", 13, 0) == -1)
        std::cerr << "send\n";
    // client closes here via destructor
}

int main()
{
    // listen on sock_fd, new connection on new_fd
    addrinfo hints, *p;
    sockaddr_storage their_addr; // connector's address info
    socklen_t sin_size;
    int yes=1;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    // wraps call to getaddrinfo, keeps result addrinfo in the object, throws in failure
    litosock::Addrinfo servinfo(nullptr, PORT, &hints);

    // invalid for now
    litosock::Socket sock;
    // loop through all the results and bind to the first we can
    for(p = servinfo.get(); p != NULL; p = p->ai_next) {
        sock.set(socket(p->ai_family, p->ai_socktype, p->ai_protocol));
        if (!sock.valid())
        {
            std::cerr << "server: socket\n";
            continue;
        }

        // reinterpret cast makes this cross platform
        if (setsockopt(sock.get(), SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&yes),
                sizeof(int)) == -1) {
            throw std::runtime_error("setsockopt\n");
        }

        if (bind(sock.get(), p->ai_addr, p->ai_addrlen) == -1) {
            std::cerr << "server: bind\n";
            continue;
        }

        break;
    }

    if (p == nullptr)  {
        throw std::runtime_error("server failed to bind\n");
    }

    if (listen(sock.get(), BACKLOG) == -1) {
        throw std::runtime_error("listen");
    }

    std::cout << "server: waiting for connections...\n";

    while(true) {  // main accept() loop
        sin_size = sizeof their_addr;
        litosock::Socket client;
        client.set(accept(sock.get(), (struct sockaddr *)&their_addr,
            &sin_size));

        if (!client.valid()) {
            std::cerr << "accept\n";
            continue;
        }

        std::cout << "server: got connection from "
        << litosock::getIPString(&their_addr) << "\n";

        // Note: Beej's C version uses fork() + sigaction to handle zombie processes.
        // We use std::thread here for cross-platform simplicity.
        std::thread([c = std::move(client)]() mutable {
            handleClient(std::move(c));
        }).detach();
    }

    return 0;
}