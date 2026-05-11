#include "litosock.h"

// prevent other .cpp files from declaring WinsockGuard as extern
namespace
{
    // use RAII for init and cleanup in Windows
    struct WinsockGuard {
        WinsockGuard()  { WSADATA d; WSAStartup(MAKEWORD(2,2), &d); }
        ~WinsockGuard() { WSACleanup(); }
    };
    // the static instance is local to this file
    static WinsockGuard winsockGuard;
}

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