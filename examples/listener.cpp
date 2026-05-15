/*
** listener.cpp -- a C++ datagram sockets "server" demo
*/

#include <iostream>
#include <stdexcept>
#include "litosock.h"

#define MYPORT "4950"	// the port users will be connecting to

#define MAXBUFLEN 100


int main()
{
	addrinfo hints{}, *p;
	int numbytes;
	sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;

	hints.ai_family = AF_INET6; // or set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	litosock::Addrinfo servinfo(nullptr, MYPORT, &hints);
	litosock::Socket sock;

	// loop through all the results and bind to the first we can
	for(p = servinfo.get(); p != nullptr; p = p->ai_next) {
		sock.set(socket(p->ai_family, p->ai_socktype,
				p->ai_protocol));

		if (!sock.valid()) {
			std::cerr << "listener: socket\n";
			continue;
		}

		if (bind(sock.get(), p->ai_addr, p->ai_addrlen) == -1) {
			std::cerr << "listener: bind";
			continue;
		}

		break;
	}

	if (p == nullptr) {
		throw  std::runtime_error("listener: failed to bind socket\n");
	}

	std::cout << "listener: waiting to recvfrom...\n";

	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sock.get(), buf, MAXBUFLEN-1 , 0,
		 reinterpret_cast<sockaddr*>(&their_addr), &addr_len)) == -1) {
		throw std::runtime_error("recvfrom");
	}

	buf[numbytes] = '\0';
	std::cout << "listener: got packet from " << litosock::getIPString(&their_addr) << "\n";
	std::cout << "listener: packet is " << numbytes << " bytes long\n";
	std::cout << "listener: packet contains \"" << buf << "\"\n";

	return 0;
}
