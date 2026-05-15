/*
** talker.cpp -- a C++ datagram "client" demo
*/

#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include "litosock.h"

#define SERVERPORT "4950"   // the port users will be connecting to

int main(int argc, char *argv[])
{
	addrinfo hints{}, *p;
	int numbytes;

	if (argc != 3) {
		std::cerr << "usage: talker hostname message\n";
		exit(1);
	}

	hints.ai_family = AF_INET6; // set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_DGRAM;

	litosock::Addrinfo servinfo(argv[1], SERVERPORT, &hints);
	litosock::Socket sock;

	// loop through all the results and make a socket
	for(p = servinfo.get(); p != NULL; p = p->ai_next) {
		sock.set(socket(p->ai_family, p->ai_socktype,
				p->ai_protocol));
		if (!sock.valid()) {
			std::cerr << "talker: socket\n";
			continue;
		}

		break;
	}

	if (p == nullptr) {
		throw std::runtime_error("talker: failed to create socket\n");
	}

	std::string_view msg = argv[2];
	if ((numbytes = sendto(sock.get(), msg.data(), msg.size(), 0,
			 p->ai_addr, p->ai_addrlen)) == -1) {
		throw std::runtime_error("talker: sendto");
	}

	std::cout << "talker: sent " << numbytes << " bytes to " << argv[1] << "\n";

	return 0;
}
