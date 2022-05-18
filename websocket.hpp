#pragma once
#include <functional>
#include <set>
#include <string>
#include <string_view>
#include <filesystem>
#include <system_error>
#include <boost/noncopyable.hpp>
#include <libsoup/soup.h>

namespace websocket {

/*! WebSocket (Secure) 1:1 client channel implementation.
Implementation allows to connect to server WebSocket and send string messages.

To create "plain" Websocket connection, type

\code
client_channel ch;
ch.connect("ws://localhost:4651/echo", [&ch](error_code const & ec) {
	ch.send("hello!");
});
\endcode

To create secure connection, just use constructor with certificate attribute and
connect to `wss://`, this way

\code
client_channel ch{"localhost.crt"};
ch.connect("wss://localhost:4651/echo", [&ch](error_code const & ec) {
	ch.send("hello!");
});
\endcode */
class client_channel : private boost::noncopyable {
public:
	using connected_handler = std::function<void (std::error_code const & ec)>;

	client_channel();  //!< Creates plain WebSocket channel.
	explicit client_channel(std::filesystem::path const & ssl_cert_file);  //!< Creates WebSocket Secure (WSS) channel.
	~client_channel();
	void connect(std::string const & address, connected_handler && handler);
	void reconnect();
	void send(std::string const & msg);
	void close();

protected:
	virtual void on_message(std::string_view msg) {}

private:
	void connection_handler(GAsyncResult * res);
	void message_handler(SoupWebsocketDataType data_type, GBytes const * message);

	// libsoup callback handlers
	static void connection_handler_cb(SoupSession * session, GAsyncResult * res, gpointer data);
	static void message_handler_cb(SoupWebsocketConnection * conn, SoupWebsocketDataType type, GBytes * message, gpointer data);

	SoupSession * _sess;
	SoupWebsocketConnection * _conn;
	std::string _address;
	connected_handler _connected_handler;
};

/*! WebSocket (Secure) 1:N server channel implementation for communication with a group of clients.
\note To create secure channel use server_channel(ssl_cert_file, ssl_key_file) constructor. */
class server_channel : private boost::noncopyable {
public:
	server_channel();  //!< Creates plain WebSocket channel.
	server_channel(std::filesystem::path const & ssl_cert_file, std::filesystem::path const & ssl_key_file);  //!< Creates WebSocket Secure (WSS) server channel.
	~server_channel();
	bool listen(int port, std::string const & path);  //!< param[in] path e.g. "/echo"
	void send_all(std::string const & msg);

protected:
	virtual void on_message(std::string_view msg);

private:
	void message_handler(SoupWebsocketConnection * connection,
		SoupWebsocketDataType data_type, GBytes const * message);

	void connection_handler(SoupWebsocketConnection * connection, char const * path,
		SoupClientContext * client);

	void closed_handler(SoupWebsocketConnection * connection);

	// libsoup handlers
	static void websocket_handler_cb(SoupServer * server, SoupWebsocketConnection * connection,
		char const * path, SoupClientContext * client, gpointer user_data);

	static void websocket_closed_handler_cb(SoupWebsocketConnection * connection,
		gpointer user_data);

	static void websocket_message_handler_cb(SoupWebsocketConnection * connection,
		SoupWebsocketDataType data_type, GBytes * message, gpointer user_data);

	GTlsCertificate * _cert;  //!< SSL certificate in case of secure connection
	SoupServer * _server;
	std::set<SoupWebsocketConnection *> _clients;
};

}  // websocket
