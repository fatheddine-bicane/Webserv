#include "../Definitions/Responce.hpp"
#include <string>



// INFO: api
// -----------------------------------------------------------

void	Responce::initialHeaders(HTTPStatus HTTP_status) {
	this->_headers = "HTTP/1.1 " + std::to_string(HTTP_status) + "  \r\n"
					 "Connection: close\r\n";
}



void	Responce::appendHeaders(const String& key, const String& value, bool last_header) {
	this->_headers += key + ": " + value + "\r\n";

	if (last_header) {
		this->_headers += "\r\n";
	}
}

// -----------------------------------------------------------


