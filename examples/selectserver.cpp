/*
** selectserver.cpp -- a cheezy C++ multiperson chat server
*/

#include <iostream>
#include <stdexcept>
#include "litosock.h"

#define PORT "9034"   // port we're listening on


/*
 * Return a listening socket
 */
litosock::Socket get_listener_socket(void)
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
		setsockopt(listener.get(), SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&yes),
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
void handle_new_connection(int listener, fd_set *master, int *fdmax)
{
	socklen_t addrlen;
	int newfd;        // newly accept()ed socket descriptor
	struct sockaddr_storage remoteaddr; // client address

	addrlen = sizeof remoteaddr;
	newfd = accept(listener,
		(struct sockaddr *)&remoteaddr,
		&addrlen);

	if (newfd == -1) {
		std::cerr << "accept\n";
	} else {
		FD_SET(newfd, master); // add to master set
		if (newfd > *fdmax) {  // keep track of the max
			*fdmax = newfd;
		}
		std::cout << "selectserver: new connection from " << litosock::getIPString(&remoteaddr) << " on socket " << newfd << "\n";
	}
}

/*
 * Broadcast a message to all clients
 */
void broadcast(char *buf, int nbytes, int listener, int s,
               fd_set *master, int fdmax)
{
	for(int j = 0; j <= fdmax; j++) {
		// send to everyone!
		if (FD_ISSET(j, master)) {
			// except the listener and ourselves
			if (j != listener && j != s) {
				if (send(j, buf, nbytes, 0) == -1) {
					std::cerr << "send\n";
				}
			}
		}
	}
}

/*
 * Handle client data and hangups
 */
void handle_client_data(int s, int listener, fd_set *master,
                        int fdmax)
{
	char buf[256];    // buffer for client data
	int nbytes;

	// handle data from a client
	if ((nbytes = recv(s, buf, sizeof buf, 0)) <= 0) {
		// got error or connection closed by client
		if (nbytes == 0) {
			// connection closed
			std::cout << "selectserver: socket " << s << " hung up\n";
		} else {
			std::cerr << "recv\n";
		}
		close(s); // bye!
		FD_CLR(s, master); // remove from master set
	} else {
		// we got some data from a client
		broadcast(buf, nbytes, listener, s, master, fdmax);
	}
}

/*
 * Main
 */
int main(void)
{
	fd_set master;    // master file descriptor list
	fd_set read_fds;  // temp file descriptor list for select()
	int fdmax;        // maximum file descriptor number

	FD_ZERO(&master);    // clear the master and temp sets
	FD_ZERO(&read_fds);

	litosock::Socket listener = get_listener_socket();

	// add the listener to the master set
	FD_SET(listener.get(), &master);

	// keep track of the biggest file descriptor
	fdmax = listener.get(); // so far, it's this one

	// main loop
	for(;;) {
		read_fds = master; // copy it
		if (select(fdmax+1, &read_fds, nullptr, nullptr, nullptr) == -1) {
			throw std::runtime_error("select\n");
		}

		// run through the existing connections looking for data
		// to read
		for(int i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) { // we got one!!
				if (i == listener.get())
					handle_new_connection(i, &master, &fdmax);
				else
					handle_client_data(i, listener.get(), &master, fdmax);
			}
		}
	}
	
	return 0;
}

