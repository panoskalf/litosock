#include "litosock.h"

// Ensures platformInit() runs before any litosock API is used.
// platformInit() is defined in the platform-specific file (litosock_windows.cpp /
// litosock_linux.cpp), so this reference also forces the linker to include that obj.
namespace { const int _platformInit = (litosock::platformInit(), 0); }

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

std::string litosock::getIPString(const sockaddr_storage* ss)
{
    char ipstr[INET6_ADDRSTRLEN];
    void* addr;
    if (ss->ss_family == AF_INET) // IPv4
    {
        auto* ipv4 = reinterpret_cast<const sockaddr_in*>(ss);
        addr = const_cast<in_addr*>(&ipv4->sin_addr);
    }
    else // IPv6
    {
        auto* ipv6 = reinterpret_cast<const sockaddr_in6*>(ss);
        addr = const_cast<in6_addr*>(&ipv6->sin6_addr);
    }
    inet_ntop(ss->ss_family, addr, ipstr, sizeof(ipstr));
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

litosock::Socket& litosock::Socket::operator=(Socket&& other) noexcept 
{
    if (this != &other) {
        if (valid()) closeSocket(m_fd); // close the old fd
        m_fd = other.m_fd;
        other.m_fd = INVALID_HANDLE;
    }
    return *this;
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

