#pragma once
// LitoSock - minimal cross-platform socket wrapper
#include <stdexcept>
#include <string>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    using SocketHandle = SOCKET;
    constexpr SocketHandle INVALID_HANDLE = INVALID_SOCKET;
    inline void closeSocket(SocketHandle fd) { closesocket(fd); }
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netdb.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    using SocketHandle = int;
    constexpr SocketHandle INVALID_HANDLE = -1;
    inline void closeSocket(SocketHandle fd) { ::close(fd); }
#endif


namespace litosock
{
    // Called once at startup by litosock.cpp. Platform-specific implementation
    // in litosock_windows.cpp / litosock_linux.cpp.
    void platformInit();

    // Extracts a human readable IP string from and addrinfo node.
    // Handles both IPv4 and IPv6, the cast dance is unavoidable with the BSD API
    std::string getIPString(const addrinfo* p);

    // Wraps addrinfo
    class Addrinfo
    {
    public:
        explicit Addrinfo(addrinfo* res) : m_res(res) {}
        Addrinfo(const char* node, const char* service, const addrinfo* hints) {
            int rc = getaddrinfo(node, service, hints, &m_res);
            if (rc != 0) throw std::runtime_error(gai_strerror(rc));
        }
        ~Addrinfo() { if (m_res) { freeaddrinfo(m_res); } }
        addrinfo* get() const { return m_res; }
        // No copy
        Addrinfo(const Addrinfo&) = delete;
        Addrinfo& operator=(const Addrinfo&) = delete;
    private:
        addrinfo* m_res = nullptr;
    };

    // Wraps socket to ensure cleanup
    class Socket
    {
    public:
        // Default constructor creates an empty socket with invalid fd
        Socket() noexcept : m_fd(INVALID_HANDLE) {}

        // Opens a socket via ::socket()
        Socket(int family, int type, int protocol);

        // Takes ownership of an existing fd (for accept())
        explicit Socket(SocketHandle fd);

        // Closes on destruction
        ~Socket();

        // No copy
        Socket(const Socket&) = delete;
        Socket& operator=(const Socket&) = delete;

        // Move is ok
        Socket(Socket&&) noexcept;

        // Takes ownership of fd. Closes any previously existing socket.
        // The caller must not use fd after this call.
        void set(SocketHandle fd) noexcept;

        // Get the raw resource when you pass it to API functions
        SocketHandle get(void) const;

        // Check if valid
        bool valid() const;

    private:
        SocketHandle m_fd;
    };

};