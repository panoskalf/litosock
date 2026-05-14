#include "litosock.h"

// prevent other .cpp files from declaring WinsockGuard as extern
namespace
{
    // use RAII for init and cleanup in Windows
    struct WinsockGuard {
        WinsockGuard()  { WSADATA d; WSAStartup(MAKEWORD(2,2), &d); }
        ~WinsockGuard() { WSACleanup(); }
    };
    // local instance
    WinsockGuard winsockGuard;
}