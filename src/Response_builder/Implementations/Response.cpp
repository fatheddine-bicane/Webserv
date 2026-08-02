#include "../Definitions/Response.hpp"
#include <sstream>



// INFO: api
// -----------------------------------------------------------

Response::Response(ClientConnection* client_connection) 
	: _client_connection(client_connection) {}

// -----------------------------------------------------------




// INFO: api
// -----------------------------------------------------------

void	Response::initialHeaders(HTTPStatus HTTP_status) {
	// transform the status code from an enum to a string
	std::ostringstream oss;
	oss << HTTP_status;
	String status_code = oss.str();

	this->_headers = "HTTP/1.1 " + status_code + "  \r\n"
					 "Connection: close\r\n";
}



void	Response::appendHeaders(const String& key, const String& value, bool last_header) {
	this->_headers += key + ": " + value + "\r\n";

	if (last_header) {
		this->_headers += "\r\n";
	}
}



// -----------------------------------------------------------


