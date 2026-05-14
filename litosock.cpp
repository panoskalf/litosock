#include "litosock.h"

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

litosock::Socket::Socket(int family, int type, int protocol)
{
    m_fd = ::socket(family, type, protocol);    
    if (m_fd == INVALID_HANDLE)
    {
        throw std::runtime_error("socket() failed");
    }
}

litosock::Socket::Socket(SocketHandle fd)
{
    if (fd == INVALID_HANDLE)
    {
        throw std::runtime_error("Socket: invalid handle");
    }
    m_fd = fd;
}

litosock::Socket::~Socket()
{
    if (valid()) 
    {
        closeSocket(m_fd);
    }
}

litosock::Socket::Socket(Socket&& other) noexcept : m_fd(other.m_fd)
{
    other.m_fd = INVALID_HANDLE; // prevent double close
}

void litosock::Socket::set(SocketHandle fd) noexcept
{
    SocketHandle temp = m_fd;
    m_fd = fd;
    if (temp != INVALID_HANDLE)
    {
        closeSocket(temp);
    }
}

SocketHandle litosock::Socket::get(void) const
{
    return m_fd;
}

bool litosock::Socket::valid() const
{
    return m_fd != INVALID_HANDLE;
}

