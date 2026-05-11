/*
** showip.cpp
**
** show IP addresses for a host given on the command line
** C++ implementation
*/

#include <iostream>
#include "litosock.h"

int main(int argc, char *argv[])
{

	if (argc != 2)
	{
		std::cerr << "usage: showip hostname\n";
	    return 1;
	}

	struct addrinfo hints{}, *res;
	hints.ai_family = AF_UNSPEC;  // Either IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;
	int status;

	if ((status = getaddrinfo(argv[1], NULL, &hints, &res) != 0))
	{
		std::cerr << "getaddrinfo: " << gai_strerror(status) << "\n";
		return 2;
	}

	std::cout << "IP addresses for " << argv[1] << ":\n\n";

	for(auto* p = res;p != NULL; p = p->ai_next)
	{
		std::cout << (p->ai_family == AF_INET ? "IPv4" : "IPv6")
		<< ": " << litosock::getIPString(p) << "\n";
	}

	freeaddrinfo(res); // free the linked list
	return 0;
}

