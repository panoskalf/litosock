/*
** client.cpp -- a C++ stream socket client demo
*/

#include <iostream>
#include <stdexcept>
#include "litosock.h"


#define PORT "3490" // the port client will be connecting to

#define MAXDATASIZE 100 // max number of bytes we can get at once


int main(int argc, char *argv[])
{
	int numbytes;
	char buf[MAXDATASIZE];
	addrinfo hints{}, *p;
	char s[INET6_ADDRSTRLEN];

	if (argc != 2) {
	    throw std::runtime_error("usage: client hostname\n");
	}

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	litosock::Addrinfo servinfo(argv[1], PORT, &hints);
	// empty for now
	litosock::Socket sock;

	// loop through all the results and connect to the first we can
	for(p = servinfo.get(); p != nullptr; p = p->ai_next) {
        sock.set(socket(p->ai_family, p->ai_socktype,
				p->ai_protocol));
		if (!sock.valid()) {
			std::cerr << "client: socket\n";
			continue;
		}

        std::cout << "client: attempting connection to "
		<< litosock::getIPString(p)
		<< "\n";

		if (connect(sock.get(), p->ai_addr, p->ai_addrlen) == -1) {
			std::cerr << "client: connect\n";
			continue;
		}

		break;
	}

	if (p == nullptr) {
		throw std::runtime_error("client: failed to connect\n");
	}

	std::cout << "client: connected to "
			<< litosock::getIPString(p)
			<< "\n";

	if ((numbytes = recv(sock.get(), buf, MAXDATASIZE-1, 0)) == -1) {
	    throw std::runtime_error("recv\n");
	}

	buf[numbytes] = '\0';

	std::cout << "client: received '" << buf << "'\n";

	return 0;
}

