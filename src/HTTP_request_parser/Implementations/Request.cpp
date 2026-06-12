#include "../Definitions/Request.hpp"
#include "../../Core_modules/Connection/Definitions/ClientConnection.hpp"

// INFO: constructor
// --------------------------------------------

Request::Request(SOCKET fd, ClientConnection* client_connection) {
	this->_fd = fd;
	this->_client_connection = client_connection;
	this->_state = START_LINE;
}

// --------------------------------------------




// INFO: api
// --------------------------------------------

void	Request::attemptRequestParse() {
	readSocketBuffer();

	while (isRequestState(INCOMPLETE)) {
		switch (this->_state) {
			case START_LINE: parseStartLine(); break;

			default: break;
		}
	}
}



bool	Request::isRequestState(RequestState request_state) {
	if (request_state == INCOMPLETE) {
		return (this->_state == START_LINE
				|| this->_state == HEADERS
				|| this->_state == BODY);
	}

	return (this->_state == request_state);
}

// --------------------------------------------




// INFO: helper functions
// --------------------------------------------

void	Request::readSocketBuffer() {
	char	buffer[4096];

	ssize_t bytes_read = recv(this->_fd, buffer, sizeof(buffer), 0);

	// buffer was populated append the read data to request buffer
	if (bytes_read > 0) {
		this->_buffer.append(buffer, bytes_read);
	}

	// client closed the connection
	else if (bytes_read == 0) {
		// if state complete serve request
		// close connection
		this->_client_connection->state = CLOSE;
		return;
	}

	// a network fatal error occured: bytes_read == -1
	else {
		this->_client_connection->state = CLOSE;
		return;
	}
}




size_t	Request::getCRLFPosition() {
	size_t pos = this->_buffer.find("\r\n");

	// accept a single LF as line terminator
	if (pos == String::npos) {
		pos = this->_buffer.find("\n");
	}

	return pos;
}




void	Request::replaceBareCRWithSP(String& request_line) {
	String::iterator it = request_line.begin();
	String::iterator end = request_line.end();

	for (; it != end; it++) {
		if (*it == '\r') {
			*it = ' ';
		}
	}
}



void	Request::trimString(String& string) {
	// trim right
	size_t end = string.find_last_not_of(' ');
	if (end != String::npos) {
		string.erase(end + 1);
	}

	// trim left
	size_t start = string.find_first_not_of(' ');
	if (start != String::npos) {
		string.erase(0, start);
	}
}


bool	Request::malformedRequest(STATUS_CODE status_code) {
	this->status_code = status_code;
	this->_state = MALFORMED;
	return false;
}



String	Request::consumeLine() {
	size_t pos = getCRLFPosition();
	// line not complete
	if (pos == String::npos) {
		// line is greater than 4kb
		if (this->_buffer.length() > 4096) {
			malformedRequest(413); // 413 Content Too Large
		}
		return "";
	}

	// extract the line from the buffer
	String line = this->_buffer.substr(0, pos);
	// syntax error
	if (line.empty()) {
		if (isRequestState(START_LINE)) {
			return "";
		}

		// TODO: if i header section mark the header section as finished

		malformedRequest(400); // 400 Bad Request
		return "";
	}

	// line contain only white spaces
	else if (line.find_first_not_of("\t\n") == String::npos) {
		malformedRequest(400); // 400 Bad Request
		return "";
	}

	size_t CRLF_end_position;
	if (this->_buffer[pos] == '\r') {
		CRLF_end_position = 2;
	} else {
		CRLF_end_position = 1;
	}

	this->_buffer = this->_buffer.substr(pos + CRLF_end_position);

	return line;
}




void	Request::parseStartLine() {
	String start_line = consumeLine();
	if (start_line.empty()) {
		return;
	}

	replaceBareCRWithSP(start_line);
	trimString(start_line);

	if (!parseMethod(start_line)) return;
	if (!parseTargetResource(start_line)) return;
	if (!parseHTTPVersion(start_line)) return;

	this->_state = HEADERS;
}



bool	Request::parseMethod(String& start_line) {
	size_t pos = start_line.find(' ');
	if (pos == String::npos) {
		return malformedRequest(400); // 400 Bad Request
	}

	String method = start_line.substr(0, pos);
	// if method supported
	if (method == "GET") {
		this->method = GET;
	} else if (method == "POST") {
		this->method = POST;
	} else if (method == "DELETE") {
		this->method = DELETE;
	} else if (method == "PUT") {
		this->method = PUT;
	}

	// if server dosent recognize the method
	else {
		return malformedRequest(501); // 501 Not Implemented
	}

	// update line
	start_line = start_line.substr(pos + 1);
	return true;
}



bool	Request::parseTargetResource(String& start_line) {
	size_t pos = start_line.find(' ');
	if (pos == String::npos) {
		return malformedRequest(400); // 400 Bad Request
	}

	String target_resource = start_line.substr(0, pos);

	// server limit refusing to process long URIs
	if (target_resource.length() >= 100) {
		return malformedRequest(414); // 414 URI Too Long
	}

	this->target_resource = target_resource;

	// update line
	start_line = start_line.substr(pos + 1);
	return true;
}



bool	Request::parseHTTPVersion(String& HTTP_version) {
	String http = HTTP_version.substr(0, HTTP_version.find('/'));
	if (http != "HTTP") {
		return malformedRequest(400); // 400 Bad Request
	}

	size_t pos = HTTP_version.find('/');
	if (pos == String::npos) {
		return malformedRequest(400); // 400 Bad Request
	}

	String value = HTTP_version.substr(pos + 1);
	if (value.length() != 3) {
		return malformedRequest(400); // 400 Bad Request
	}
	if (!isdigit(value[0]) || !isdigit(value[2])) {
		return malformedRequest(400); // 400 Bad Request
	}
	if (value[1] != '.') {
		return malformedRequest(400); // 400 Bad Request
	}

	this->HTTP_version = value;
	return true;
}


// --------------------------------------------
