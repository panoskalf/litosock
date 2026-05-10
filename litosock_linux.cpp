#include "litosock.h"
#include <arpa/inet.h>
#include <netinet/in.h>

std::string litosock::getIPString(const addrinfo *p)
{
    char ipstr[INET6_ADDRSTRLEN];
    void *addr;

    if (p->ai_family == AF_INET) // IPv4
    { 
        auto* ipv4 = reinterpret_cast<const sockaddr_in*>(p->ai_addr);
        addr = const_cast<in_addr*>(&ipv4->sin_addr);
    } 
    else // IPv6
    { 
        auto* ipv6 = reinterpret_cast<const sockaddr_in6*>(p->ai_addr);
        addr = const_cast<in6_addr*>(&ipv6->sin6_addr);
    }

    inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
    return std::string(ipstr);
}