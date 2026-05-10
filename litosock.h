#pragma once
// LitoSock - minimal cross-platform socket wrapper

#include <netdb.h>
#include <string>

namespace litosock 
{
    // Extracts a human readable IP string from and addrinfo node.
    // Handles both IPv4 and IPv6, the cast dance is unavoidable with the BSD API
    std::string getIPString(const addrinfo* p);
};