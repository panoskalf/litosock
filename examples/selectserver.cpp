/*
** selectserver.cpp -- a cheezy C++ multiperson chat server
*/

#include <iostream>
#include <stdexcept>
#include <vector>
#include "litosock.h"

#define PORT "9034"   // port we're listening on

// Used to conveniently manage select state
struct SelectState {
	fd_set master;							// master file descriptor list
	int fdmax = 0;								// maximum file descriptor number
	litosock::Socket listener;				// the listener socket for convenient access
	std::vector<litosock::Socket> clients; 	// track active clients explicitly (for Windows portability)
};

/*
 * Return a listening socket
 */
litosock::Socket get_listener_socket()
{
	addrinfo hints{}, *p;
	int yes=1;    // for setsockopt() SO_REUSEADDR, below

	// get us a socket and bind it
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	litosock::Addrinfo ai(nullptr, PORT, &hints);
	litosock::Socket listener;
	
	for(p = ai.get(); p != nullptr; p = p->ai_next) {
		listener.set(socket(p->ai_family, p->ai_socktype,
				p->ai_protocol));
		if (!listener.valid()) { 
			continue;
		}
		
		// lose the pesky "address already in use" error message
		setsockopt(listener.get(), SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&yes),
				sizeof(int));

		if (bind(listener.get(), p->ai_addr, p->ai_addrlen) < 0) {
			continue;
		}

		break;
	}

	// if we got here, it means we didn't get bound
	if (p == nullptr) {
		throw std::runtime_error("selectserver: failed to bind\n");
	}

	// listen
	if (listen(listener.get(), 10) == -1) {
		throw std::runtime_error("listen\n");
	}

	return listener;
}

/*
 * Add new incoming connections to the proper sets
 */
void handle_new_connection(SelectState& ss)
{
	socklen_t addrlen;
	SocketHandle newfd;        // newly accepted socket descriptor
	sockaddr_storage remoteaddr; // client address

	addrlen = sizeof remoteaddr;
	newfd = accept(ss.listener.get(),
		reinterpret_cast<sockaddr*>(&remoteaddr),
		&addrlen);

	if (newfd == INVALID_HANDLE) {
		std::cerr << "accept\n";
	} else {
		FD_SET(newfd, &ss.master); // add to master set
		ss.clients.push_back(litosock::Socket{newfd}); // add Socket to clients
		if (newfd > ss.fdmax) {  // keep track of the max
			ss.fdmax = newfd;
		}
		std::cout << "selectserver: new connection from " << litosock::getIPString(&remoteaddr) << " on socket " << newfd << "\n";
	}
}

/*
 * Broadcast a message to all clients
 */
void broadcast(std::string_view msg, int i, SelectState& ss)
{
	for(int j = 0; j < static_cast<int>(ss.clients.size()) ; j++) {
		// send to everyone!
		if (FD_ISSET(ss.clients[j].get(), &ss.master)) {
			// except ourselves
			if (j != i) {
				if (send(ss.clients[j].get(), msg.data(),
				 	static_cast<int>(msg.size()), 0) == -1) {
					std::cerr << "send\n";
				}
			}
		}
	}
}

/*
 * Handle client data and hangups
 */
void handle_client_data(int& i, SelectState& ss)
{
	char buf[256];    // buffer for client data
	int nbytes;

	// handle data from a client
	if ((nbytes = recv(ss.clients[i].get(), buf, sizeof buf, 0)) <= 0) {
		// got error or connection closed by client
		if (nbytes == 0) {
			// connection closed
			std::cout << "selectserver: socket " << ss.clients[i].get() << " hung up\n";
		} else {
			std::cerr << "recv\n";
		}
		FD_CLR(ss.clients[i].get(), &ss.master); // remove client fd from master set
		ss.clients[i] = std::move(ss.clients.back()); // move closes the client and invalidates the back
		ss.clients.pop_back(); // remove the back
		i--; // reexamine the slot we just deleted
		// Note: fdmax is not decremented on disconnect intentionally
		// because FD_ISSET on returns false for removed fds
	} else {
		// we got some data from a client
		broadcast({buf, static_cast<size_t>(nbytes)}, 
					i, ss);
	}
}

/*
 * Main
 */
int main()
{
	// init select state struct
	// this contains everything we need, init listener socket in place.
	SelectState ss;
	FD_ZERO(&ss.master);    // clear the master and temp sets
	ss.listener = get_listener_socket(); // this will implicitly cause a move

	// add the listener to the master set
	FD_SET(ss.listener.get(), &ss.master);
	ss.fdmax = ss.listener.get();  // initialize fdmax to listener socket

	fd_set read_fds;    // temp file descriptor list for select()
	// main loop
	for(;;) {
		read_fds = ss.master; // copy it
		if (select(ss.fdmax+1, &read_fds, nullptr, nullptr, nullptr) == -1) {
			throw std::runtime_error("select\n");
		}

		// handle new connections on the listener socket
		if (FD_ISSET(ss.listener.get(), &read_fds))
		{
			handle_new_connection(ss);
		}

		// run through the existing client connections looking for data
		for(int i = 0; i < static_cast<int>(ss.clients.size()); i++) {
			if (FD_ISSET(ss.clients[i].get(), &read_fds)) { // we got one!!
				handle_client_data(i, ss);
			}
		}
	}
	
	return 0;
}

