#include <string>
#include <string_view>
#include <iostream>
#include <cassert>
#include "websocket.hpp"

using std::string_view, std::string, std::cout;
using std::filesystem::exists, std::filesystem::path;

namespace websocket {

namespace detail {

void on_close(SoupWebsocketConnection * conn, gpointer data);

}  // detail

client_channel::client_channel()
	: _sess{nullptr}
	, _conn{nullptr}
{
	_sess = soup_session_new();
	assert(_sess);
}

client_channel::client_channel(path const & ssl_cert_file)
	: _sess{nullptr}
	, _conn{nullptr}
{
	assert(exists(ssl_cert_file));

	char const * https_aliases[] = {"wss", nullptr};
	_sess = soup_session_new_with_options(
		SOUP_SESSION_SSL_STRICT, FALSE,  // https://libsoup.org/libsoup-2.4/SoupSession.html#SoupSession--ssl-strict
//		SOUP_SESSION_SSL_USE_SYSTEM_CA_FILE, TRUE,  // https://libsoup.org/libsoup-2.4/SoupSession.html#SoupSession--ssl-use-system-ca-file
		SOUP_SESSION_SSL_CA_FILE, ssl_cert_file.c_str(),  // https://libsoup.org/libsoup-2.4/SoupSession.html#SoupSession--ssl-ca-file
		SOUP_SESSION_HTTPS_ALIASES, https_aliases, // https://libsoup.org/libsoup-2.4/SoupSession.html#SoupSession--https-aliases
		nullptr);

	assert(_sess);
}

client_channel::~client_channel() {
	close();
}

void client_channel::close() {
	if (_conn) {
		soup_websocket_connection_close(_conn, SOUP_WEBSOCKET_CLOSE_GOING_AWAY, "closing client channel");
		g_object_unref(G_OBJECT(_conn));
		_conn = nullptr;
	}

	if (_sess) {
		soup_session_abort(_sess);
		g_object_unref(G_OBJECT(_sess));
		_sess = nullptr;
	}
}

void client_channel::connect(string const & address, connected_handler && handler) {
	assert(_sess);

	_address = address;
	_connected_handler = move(handler);

	SoupMessage * msg = soup_message_new(SOUP_METHOD_GET, _address.c_str());
	soup_session_websocket_connect_async(_sess, msg, nullptr, nullptr, nullptr,
		(GAsyncReadyCallback)connection_handler_cb, this);

	g_object_unref(G_OBJECT(msg));
}

void client_channel::reconnect() {
	assert(_sess);

	SoupMessage * msg = soup_message_new(SOUP_METHOD_GET, _address.c_str());

	soup_session_websocket_connect_async(_sess, msg, nullptr, nullptr, nullptr,
		(GAsyncReadyCallback)connection_handler_cb, this);

	g_object_unref(G_OBJECT(msg));
}

void client_channel::send(string const & msg) {
	assert(_conn);
	soup_websocket_connection_send_text(_conn, msg.c_str());
}

void client_channel::connection_handler(GAsyncResult * res) {
	//	assert(!_conn);  // FIXME: we have resource leak there, _conn must be properly closed

	GError * error = nullptr;
	_conn = soup_session_websocket_connect_finish(_sess, res, &error);

	if (error) {
		cout << "websocket: unable connect to \"" << _address << "\" address, what: " << error->message << "\n";
		g_error_free(error);
		return;
	}
	assert(_conn);

	g_object_ref(G_OBJECT(_conn));

	// handle signals
	g_signal_connect(_conn, "message", G_CALLBACK(message_handler_cb), this);
	g_signal_connect(_conn, "closed", G_CALLBACK(detail::on_close), nullptr);

	_connected_handler(std::error_code{});
}

void client_channel::message_handler(SoupWebsocketDataType data_type, GBytes const * message) {
	switch (data_type) {
		case SOUP_WEBSOCKET_DATA_BINARY: {
			cout << "websocket: unknown binary message received, ignored\n";
			return;
		}

		case SOUP_WEBSOCKET_DATA_TEXT: {
			gsize size = 0;
			gchar * str = (gchar *)g_bytes_get_data((GBytes *)message, &size);
			assert(str);
			on_message(string_view{str, size});
			return;
		}

		default:
			assert(0);
	}
}

void client_channel::connection_handler_cb(SoupSession *, GAsyncResult * res,
	gpointer data) {

	client_channel * p = static_cast<client_channel *>(data);
	assert(p);
	p->connection_handler(res);
}

void client_channel::message_handler_cb(SoupWebsocketConnection *, SoupWebsocketDataType type,
	GBytes * message, gpointer data) {

	client_channel * p = static_cast<client_channel *>(data);
	assert(p);
	p->message_handler(type, message);
}


server_channel::server_channel()
	: _cert{nullptr}
	, _server{nullptr}
{}

server_channel::server_channel(path const & ssl_cert_file, path const & ssl_key_file)
	: _server{nullptr} {
	assert(exists(ssl_cert_file) && exists(ssl_key_file));

	// load certificate
	GError * error = nullptr;
	_cert = g_tls_certificate_new_from_files(ssl_cert_file.c_str(), ssl_key_file.c_str(), &error);
	if (error) {
		cout << "unable to load (" << ssl_cert_file << ", " << ssl_key_file << "), what: " << error->message << "\n";
		return;
	}

	assert(_cert);
}

server_channel::~server_channel() {
	assert(!_cert);

	// free connections
	for_each(begin(_clients), end(_clients), [](SoupWebsocketConnection * client){
		g_object_unref(G_OBJECT(client));
	});

	soup_server_disconnect(_server);
	g_object_unref(G_OBJECT(_server));
}

bool server_channel::listen(int port, string const & path) {
	SoupServerListenOptions options = (SoupServerListenOptions)0;
	if (_cert) {  // create secure connection
		_server = soup_server_new(SOUP_SERVER_SERVER_HEADER, "WebSocket Secure server",
			SOUP_SERVER_TLS_CERTIFICATE, _cert, nullptr);
		options = SOUP_SERVER_LISTEN_HTTPS;
		g_clear_object(&_cert);  // release certificate
	}
	else  // create plain connection
		_server = soup_server_new(SOUP_SERVER_SERVER_HEADER, "WebSocket server", nullptr);

	if (!_server)
		return false;

	assert(_server);
	soup_server_add_websocket_handler(_server, path.c_str(), nullptr, nullptr, websocket_handler_cb, (gpointer)this, nullptr);
	return soup_server_listen_all(_server, port, options, nullptr) == TRUE;
}

void server_channel::send_all(string const & msg) {
	for (SoupWebsocketConnection * client : _clients)
		soup_websocket_connection_send_text(client, msg.c_str());  // TOOD: we can maybe call `soup_websocket_connection_send_binary` instead of text version which would allow us to use string_view instead string
}

void server_channel::on_message(string_view msg) {}

void server_channel::message_handler(SoupWebsocketConnection * connection,
	SoupWebsocketDataType data_type, GBytes const * message) {

	switch (data_type) {
		case SOUP_WEBSOCKET_DATA_BINARY: {
			cout << "websocket: unknown binary message received, ignored\n";
			return;
		}

		case SOUP_WEBSOCKET_DATA_TEXT: {
			gsize size = 0;
			gchar * str = (gchar *)g_bytes_get_data((GBytes *)message, &size);
			assert(str);
			on_message(string_view{str, size});
			return;
		}

		default:
			assert(0);
	}
}

void server_channel::connection_handler(SoupWebsocketConnection * connection, char const * path,
	SoupClientContext * client) {

	assert(_clients.count(connection) == 0);  // check connection is always unique
	g_object_ref(G_OBJECT(connection));
	_clients.insert(connection);
}

void server_channel::closed_handler(SoupWebsocketConnection * connection) {
	auto it = _clients.find(connection);
	assert(it != end(_clients));  // we expect connection always there, otherwise logic error

	cout << "websocket: connection " << static_cast<void *>(*it) << " closed\n";

	g_object_unref(G_OBJECT(*it));
	_clients.erase(it);
}


void server_channel::websocket_handler_cb(SoupServer * server, SoupWebsocketConnection * connection,
	char const * path, SoupClientContext * client, gpointer user_data) {

	cout << "websocket: connection " << static_cast<void *>(connection) << " request received\n";

	server_channel * channel = static_cast<server_channel *>(user_data);
	assert(channel);

	g_object_ref(G_OBJECT(connection));
	g_signal_connect(G_OBJECT(connection), "message",
		G_CALLBACK(websocket_message_handler_cb),	channel);

	// we don't want to increment connection reference counter there
	g_signal_connect(G_OBJECT(connection), "closed",
		G_CALLBACK(websocket_closed_handler_cb), channel);

	channel->connection_handler(connection, path, client);
}

void server_channel::websocket_closed_handler_cb(SoupWebsocketConnection * connection,
	gpointer user_data) {

	server_channel * channel = static_cast<server_channel *>(user_data);
	assert(channel);
	channel->closed_handler(connection);
}

void server_channel::websocket_message_handler_cb(SoupWebsocketConnection * connection,
	SoupWebsocketDataType data_type, GBytes * message, gpointer user_data)
{
	server_channel * channel = static_cast<server_channel *>(user_data);
	assert(channel);
	channel->message_handler(connection, data_type, message);
}


namespace detail {

void on_close(SoupWebsocketConnection * conn, gpointer) {
	soup_websocket_connection_close(conn, SOUP_WEBSOCKET_CLOSE_NORMAL, nullptr);
}

}  // detail

}  // websocket
