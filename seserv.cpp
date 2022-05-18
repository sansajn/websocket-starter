// Secure WebSocket echo server sample
#include <iostream>
#include "echo_server.hpp"
#include "glib_event_loop.hpp"

using std::cout, std::endl;

constexpr size_t PORT = 41001;
constexpr char const * PATH = "/test";

int main(int argc, char * argv[]) {
	glib_event_loop loop;

	echo_server serv{"localhost.crt", "localhost.key"};
	if (!serv.listen(PORT, PATH))
		return 1;  // can not listen, exit

	cout << "listenning on wss://localhost:" << PORT << PATH << " WebSocket address\n"
		<< "press ctrl+c to quit" << endl;

	loop.go();  // blocking
	return 0;
}
