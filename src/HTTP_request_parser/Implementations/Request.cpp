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
			case HEADERS: parseFieldLine(); break;
			case BODY: parseBody(); break;

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




// --------------------------------------------

// INFO: parse request helpers
void	Request::parseStartLine() {
	String start_line = consumeLine();
	if (start_line == LINE_NOT_READY || start_line == BAD_VALUE) {
		return;
	}

	replaceBareCRWithSP(start_line);

	if (!parseMethod(start_line)) return;
	if (!parseTargetResource(start_line)) return;
	if (!parseHTTPVersion(start_line)) return;

	this->_state = HEADERS;
}



void	Request::parseFieldLine() {
	String field_line = consumeLine();
	if (field_line == LINE_NOT_READY) {
		return;
	} else if (field_line == CRLF) {
		// TODO: check headers for content length or cuncks to read body
		// and change the request state
		this->_state = BODY;
		return;
	}

	replaceBareCRWithSP(field_line);

	if (field_line[0] == '\t' || field_line[0] == ' ') {
		malformedRequest(400); // 400 Bad Request
		return;
	}

	String field_name = parseFieldName(field_line);
	if (field_name == BAD_VALUE) {
		return;
	}

	String field_value = parseFieldValue(field_line);

	this->headers[field_name] = field_value;
}



// WARNING:
// A server that receives a request message with a
// transfer coding it does not understand SHOULD
// respond with 501 (Not Implemented).
void	Request::parseBody() {
	// a request cannot contain both transfer-encoding and content-length,
	// allowing the existance of both headers leads to desyncing the 
	// server and proxy.(protecting against is just a good practice)
	if (this->headers.find("transfer-encoding") != this->headers.end()
		&& this->headers.find("content-length") != this->headers.end()) {
		malformedRequest(400); // 400 Bad Request
	}

	if (!linkServerObject()) return;
}



// INFO: parse start line helpers

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



// INFO: parse headers helpers

String	Request::parseFieldName(String& start_line) {
	size_t pos = start_line.find(':');
	if (pos == String::npos) {
		malformedRequest(400); // 400 Bad Request
		return BAD_VALUE;
	}

	String field_name = start_line.substr(0, pos);
	if (field_name.find_last_of(WHITE_SPACES) != String::npos) {
		malformedRequest(400); // 400 Bad Request
		return BAD_VALUE;
	}

	// convert field-name to lowercase for lookups later on
	std::transform(field_name.begin(), field_name.end(), field_name.begin(), ::tolower);

	return field_name;
}



String	Request::parseFieldValue(String& start_line) {
	size_t pos = start_line.find(':');

	String field_value = start_line.substr(pos + 1);
	trimString(field_value);

	return field_value;
}



// INFO: helper functions

void	Request::readSocketBuffer() {
	char	buffer[_4KB + 1];

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
		if (this->_buffer.length() > _8KB) {
			malformedRequest(413); // 413 Content Too Large
			return BAD_VALUE;
		}
		return LINE_NOT_READY;
	}

	// extract the line from the buffer
	String line = this->_buffer.substr(0, pos);
	if (line.empty()) {
		if (isRequestState(START_LINE)) {
			return LINE_NOT_READY;
		} else if (isRequestState(HEADERS)) {
			return CRLF;
		}

		// syntax error
		malformedRequest(400); // 400 Bad Request
		return BAD_VALUE;
	}

	// line contain only white spaces
	else if (line.find_first_not_of(WHITE_SPACES) == String::npos) {
		malformedRequest(400); // 400 Bad Request
		return BAD_VALUE;
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



bool	Request::linkServerObject() {

	if (this->_client_connection->server != NULL) {
		return true;
	}


	Headers::iterator host_field = this->headers.find("host");

	float HTTP_version = static_cast<float>(std::atof(this->HTTP_version.c_str()));
	String host_value;
	if (host_field == this->headers.end()) {
		// including the host field in HTTP 1.0 is optional
		// if its HTTP 1.0 use default server
		if (HTTP_version > 1.0f) {
			return malformedRequest(400); // 400 Bad Request
		} else {
			host_value = "default";
		}
	} else {
		// NOTE: a hostname field value will always contain
		// host:port combination, since this program cannot
		// run and listen on privlaged ports-- 1->1023.

		// extract the first part of the hostname value
		size_t pos = host_field->second.find(':');
		host_value = host_field->second.substr(0, pos);
	}

	// get the servers maped to the socket this client
	// was connected through
	String& ip_port = this->_client_connection->ip_port;
	Servers::iterator ip_port_servers = this->_client_connection->servers.find(ip_port);

	// find the server block mapped to the socket and the hostname
	std::map<String, Server>::iterator server;
	server = ip_port_servers->second.find(host_value);

	// if not found and the there was no server defined
	// with 'default' as its host name, then use the first server
	if (server == ip_port_servers->second.end()) {
		server = ip_port_servers->second.begin();
	}


	// assign the connection server pointer to the correct server block
	this->_client_connection->server = &server->second;
	return true;
}


// --------------------------------------------
