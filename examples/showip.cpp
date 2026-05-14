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

	addrinfo hints{};
	hints.ai_family = AF_UNSPEC;  // Either IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;

	litosock::Addrinfo res(argv[1], NULL, &hints);

	std::cout << "IP addresses for " << argv[1] << ":\n\n";

	for(auto* p = res.get(); p != NULL; p = p->ai_next)
	{
		std::cout << (p->ai_family == AF_INET ? "IPv4" : "IPv6")
		<< ": " << litosock::getIPString(p) << "\n";
	}

	return 0;
}

