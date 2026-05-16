#include <iostream>
#include "litosock.h"

int main(void)
{
	PollFd pfds[1]; // More if you want to monitor more

	pfds[0].fd = 0;          // Standard input
	pfds[0].events = POLLIN; // Tell me when ready to read

	// If you needed to monitor other things, as well:
	//pfds[1].fd = some_socket; // Some socket descriptor
	//pfds[1].events = POLLIN;  // Tell me when ready to read

	std::cout << "Hit RETURN or wait 2.5 seconds for timeout\n";

	int num_events = pollSockets(pfds, 1, 2500); // 2.5 second timeout

	if (num_events == 0) {
		std::cout << "Poll timed out!\n";
	} else {
		int pollin_happened = pfds[0].revents & POLLIN;

		if (pollin_happened) {
			std::cout << "File descriptor " << pfds[0].fd << " is ready to read\n";
		} else {
			std::cout << "Unexpected event occurred: " << pfds[0].revents << "\n";
		}
	}

	return 0;
}
