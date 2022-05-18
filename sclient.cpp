// Secure WebSocket client sample
#include <string_view>
#include <string>
#include <system_error>
#include <iostream>
#include "websocket.hpp"
#include "glib_event_loop.hpp"

using std::string_view, std::string, std::to_string;
using std::error_code;
using std::cout, std::endl;
using namespace std::string_literals;

constexpr size_t PORT = 41001;
constexpr char const * PATH = "/test";

struct cout_channel : public websocket::client_channel {
public:
	using websocket::client_channel::client_channel;

private:
	void on_message(string_view msg) override {
		cout << ">>> " << msg << "\n";
	}
};

int main(int argc, char * argv[]) {
	glib_event_loop loop;

	cout_channel ch{"localhost.crt"};
	string address = "wss://localhost:"s + to_string(PORT) + PATH;
	ch.connect(address, [&ch](error_code const & ec){
		// say hello
		ch.send("hello!");
		cout << "<<< hello!\n";
	});

	cout << "connectiong to " << address << " WebSocket address...\n"
		<< "press ctrl+c to quit" << endl;

	loop.go();  // blocking
	return 0;
}
