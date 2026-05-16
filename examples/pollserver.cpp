/*
** pollserver.cpp -- a cheezy C++ multiperson chat server
*/

#include <iostream>
#include <sys/socket.h>
#include <vector>
#include "litosock.h"


#define PORT "9034"   // Port we're listening on


/*
 * Return a listening socket.
 */
litosock::Socket get_listener_socket(void)
{
	int yes=1;		// For setsockopt() SO_REUSEADDR, below

	addrinfo hints{}, *p;

	// Get us a socket and bind it
	hints.ai_family = AF_UNSPEC; // use IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	litosock::Addrinfo ai(nullptr, PORT, &hints);
	litosock::Socket listener;  // listener socket object, empty for now
	
	for(p = ai.get(); p != nullptr; p = p->ai_next) {
		listener.set(socket(p->ai_family, p->ai_socktype,
				p->ai_protocol));
		if (!listener.valid()) { 
			continue;
		}
		
		// Lose the pesky "address already in use" error message
		setsockopt(listener.get(), SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int));

		if (bind(listener.get(), p->ai_addr, p->ai_addrlen) < 0) {
			continue;
		}

		break;
	}

	// If we got here, it means we didn't get bound
	if (p == nullptr) {
		throw std::runtime_error("error getting listening socket\n");
	}

	// Listen
	if (listen(listener.get(), 10) == -1) {
		throw std::runtime_error("error getting listening socket\n");
	}

	// Named Return Value Optimization (NRVO) will actually
	// move this so we don't have to.
	return listener;
}

/*
 * Handle incoming connections.
 */
void handle_new_connection(SocketHandle listener, std::vector<PollFd>& pfds)
{
	sockaddr_storage remoteaddr; // Client address
	socklen_t addrlen = sizeof remoteaddr;
	
	// Newly accepted socket descriptor
	SocketHandle newfd = accept(listener, reinterpret_cast<sockaddr*>(&remoteaddr),
			&addrlen);

	if (newfd == INVALID_HANDLE) {
		std::cerr << "accept\n";
	} else {
		// Add the new file descriptor to the vector.
		// We choose to handle new connection lifetime manually,
		// using a vector<litosock::Socket> and passing it around
		// would work but it has drawbacks here.
		pfds.push_back({newfd, POLLIN, 0});
	
		std::cout << "pollserver: new connection from " << litosock::getIPString(&remoteaddr) 
				  << " on socket " << newfd << "\n";
	}
}

/*
 * Handle regular client data or client hangups.
 */
void handle_client_data(SocketHandle listener, 
		std::vector<PollFd>& pfds, int& i)
{
	char buf[256];	// Buffer for client data

	int nbytes = recv(pfds[i].fd, buf, sizeof buf, 0);

	int sender_fd = pfds[i].fd;

	if (nbytes <= 0) { // Got error or connection closed by client
		if (nbytes == 0) {
			// Connection closed
			std::cout << "pollserver: socket " << sender_fd << " hung up\n";
		} else {
			std::cerr << "recv\n";
		}

		// we choose to close new connection sockets manually
		// with our cross platform closeSocket alias
		closeSocket(pfds[i].fd); // Bye!

		// Remove the file descriptor from the vector.
		// But first - copy the one from the end over this one
		pfds[i] = pfds.back();
		pfds.pop_back();

		// reexamine the slot we just deleted
		i--;

	} else { // We got some good data from a client
		 // newline will be in the buf
		std::cout << "pollserver: recv from fd " << sender_fd 
				  << ": " << std::string_view(buf, nbytes);
		// Send to everyone!
		for(int j = 0; j < static_cast<int>(pfds.size()); j++) {
			int dest_fd = pfds[j].fd;

			// Except the listener and ourselves
			if (dest_fd != listener && dest_fd != sender_fd) {
				if (send(dest_fd, buf, nbytes, 0) == -1) {
					std::cerr << "send";
				}
			}
		}
	}
}

/*
 * Process all existing connections.
 */
void process_connections(SocketHandle listener, std::vector<PollFd>& pfds)
{
	for(int i = 0; i < static_cast<int>(pfds.size()); i++) {

		// Check if someone's ready to read
		if (pfds[i].revents & (POLLIN | POLLHUP)) {
			// We got one!!

			if (pfds[i].fd == listener) {
				// If we're the listener, it's a new connection
				handle_new_connection(listener,	pfds);
			} else {
				// Otherwise we're just a regular client
				handle_client_data(listener, pfds, i);
			}
		}
	}
}

/*
 * Main: create a listener and connection set, loop forever
 * processing connections.
 */
int main()
{
	// We'll use a vector for easy realloc
	// and our cross-platform PollFd
	std::vector<PollFd> pfds;
	// Start off with room for 5 connections
	pfds.reserve(5);

	// Set up and get a listening socket
	litosock::Socket listener = get_listener_socket();

	// Add the listener to the vector
	// Report ready to read on incoming connection
	pfds.push_back({listener.get(), POLLIN, 0});

	std::cout << "pollserver: waiting for connections...\n";

	// Main loop
	for(;;) {
        // Use our cross-platform pollSockets
		int poll_count = pollSockets(pfds.data(), pfds.size(), -1);

		if (poll_count == -1) {
			throw std::runtime_error("poll");
		}

		// Run through connections looking for data to read
		process_connections(listener.get(), pfds);
	}
}

