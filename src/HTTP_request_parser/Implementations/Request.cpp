#include "../Definitions/Request.hpp"
#include "../../Core_modules/Connection/Definitions/ClientConnection.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <string>

// INFO: constructor
// --------------------------------------------

Request::Request(SOCKET fd, ClientConnection* client_connection) {
	this->_fd = fd;
	this->_connection = client_connection;
	this->_state = START_LINE;
	this->_expect_CRLF = false;
}

// --------------------------------------------




// INFO: api
// --------------------------------------------

void	Request::attemptRequestParse() {
	readSocketBuffer();

	while (isRequestState(INCOMPLETE) && this->_buffer.length() != 0) {
		switch (this->_state) {
			case START_LINE: parseStartLine(); break;
			case HEADERS: parseFieldLine(); break;
			case DETERMINING_MESSAGE_BODY_LENGTH:
				determiningMessageBodyLength();
				break;
			case BODY: parseBody(); break;

			default: break;
		}
	}
}



bool	Request::isRequestState(RequestState request_state) {
	if (request_state == INCOMPLETE) {
		return (this->_state == START_LINE
				|| this->_state == HEADERS
				|| this->_state == DETERMINING_MESSAGE_BODY_LENGTH
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
		// link the server object
		if (!linkServerObject()) return;

		// if no content length or encoding header was sent
		// then the request dosent contain body and its complete
		else if (!transferEncodingPresent() || !contentLengthPresent()) {
			this->_state = COMPLETE;
			return;
		}

		// a body is present change state to parse body
		else {
			this->_state = DETERMINING_MESSAGE_BODY_LENGTH;
			return;
		}
	}

	replaceBareCRWithSP(field_line);

	if (field_line[0] == '\t' || field_line[0] == ' ') {
		malformedRequest(BadRequest);
		return;
	}

	String field_name = parseFieldName(field_line);
	if (field_name == BAD_VALUE) {
		return;
	}

	String field_value = parseFieldValue(field_line);

	this->headers[field_name] = field_value;
}



void	Request::determiningMessageBodyLength() {
	if (transferEncodingPresent() && contentLengthPresent()) {
		malformedRequest(BadRequest);
		return;
	}

	// open tmp file
	if (!openTmpBodyFile()) return;

	if (transferEncodingPresent()) {
		if (!defineTransferEncoding()) return;
	} else if (contentLengthPresent()) {
		if (!defineConetentLength()) return;
	}

	this->_state = BODY;
}







void	Request::parseBody() {

	switch (this->_mesage_body_length) {
		case CHUNKED:
			readBodyWithTransferEncoding();
			break;
		case CONTENT_LENGTH:
			readBodyWithContentLengt();
			break;
	
	}

	// close the tmp file once the request is completed or malformed
	if (this->_state != BODY) {
		this->tmp_body_file.close();
	}
}



// INFO: parse start line helpers

bool	Request::parseMethod(String& start_line) {
	size_t pos = start_line.find(' ');
	if (pos == String::npos) {
		return malformedRequest(BadRequest);
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
		return malformedRequest(NotImplemented);
	}

	// update line
	start_line = start_line.substr(pos + 1);
	return true;
}



bool	Request::parseTargetResource(String& start_line) {
	size_t pos = start_line.find(' ');
	if (pos == String::npos) {
		return malformedRequest(BadRequest);
	}

	String target_resource = start_line.substr(0, pos);

	// server limit refusing to process long URIs
	if (target_resource.length() >= 100) {
		return malformedRequest(URITooLong);
	}

	this->target_resource = target_resource;

	// update line
	start_line = start_line.substr(pos + 1);
	return true;
}



bool	Request::parseHTTPVersion(String& HTTP_version) {
	String http = HTTP_version.substr(0, HTTP_version.find('/'));
	if (http != "HTTP") {
		return malformedRequest(BadRequest);
	}

	size_t pos = HTTP_version.find('/');
	if (pos == String::npos) {
		return malformedRequest(BadRequest);
	}

	String value = HTTP_version.substr(pos + 1);
	if (value.length() != 3) {
		return malformedRequest(BadRequest);
	}
	if (!isdigit(value[0]) || !isdigit(value[2])) {
		return malformedRequest(BadRequest);
	}
	if (value[1] != '.') {
		return malformedRequest(BadRequest);
	}

	this->HTTP_version = value;
	return true;
}



// INFO: parse headers helpers

String	Request::parseFieldName(String& start_line) {
	size_t pos = start_line.find(':');
	if (pos == String::npos) {
		malformedRequest(BadRequest);
		return BAD_VALUE;
	}

	String field_name = start_line.substr(0, pos);
	if (field_name.find_last_of(WHITE_SPACES) != String::npos) {
		malformedRequest(BadRequest);
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




// INFO: parse body helpers
bool	Request::openTmpBodyFile() {
	String file_name = generateRandomFileName();
	String& tmp_path = this->_connection->server->shared_directives.client_body_temp_path;
	this->tmp_body_file_name = "./" + tmp_path + "/" + file_name;

	this->tmp_body_file.open(this->tmp_body_file_name.c_str());
	if (!this->tmp_body_file.is_open()) {
		return malformedRequest(InternalServerError);
	}

	return true;
}



bool	Request::defineTransferEncoding() {
	String& transfer_encoding = this->headers.at("transfer-encoding");

	// deny any encoding not implemented by the server
	if (transfer_encoding != "chunked") {
		return malformedRequest(NotImplemented);
	}

	this->_mesage_body_length = CHUNKED;
	this->_chunk_state = CHUNK_SIZE;

	return true;
}



bool	Request::defineConetentLength() {
	// get body length
	String& body_length_str = this->headers.at("content-length");
	char* end = NULL;
	long body_length = std::strtol(body_length_str.c_str(), &end, 10);
	if (*end != '\0') {
		return malformedRequest(BadRequest);
	}

	// check if the length is more than the allowed
	long client_max_body_size =
		this->_connection->server->shared_directives.client_max_body_size;
	if (body_length > client_max_body_size) {
		return malformedRequest(PayloadTooLarge);
	}

	this->_body_length = body_length;
	this->_mesage_body_length = CONTENT_LENGTH;

	return true;
}



void	Request::readBodyWithTransferEncoding() {
	switch (this->_chunk_state) {
		case CHUNK_SIZE: consumeChunkSize(); break;
		case CHUNK_DATA: consumeChunkData(); break;
		case CHUNK_CRLF: consumeChunkCRLF(); break;
		case CHUNK_TRAILERS: consumeTrailerSection(); break;
	}
}


void	Request::consumeChunkSize() {
	// consume the chunk metadata
	size_t pos = this->_buffer.find("\r\n");
	size_t crlf_length = 2;
	if (pos == String::npos) {
		pos = this->_buffer.find('\n');
		crlf_length = 1;

		if (pos == String::npos) {
			// client sending a malecious
			if (this->_buffer.length() > _8KB) {
				malformedRequest(PayloadTooLarge);
			}

			// else wait for more content
			return;
		}
	}

	// extract metadata line
	String chunk_size_str = this->_buffer.substr(0, pos);
	this->_buffer = this->_buffer.substr(pos + crlf_length);

	// ignore any chunk extentions
	pos = chunk_size_str.find(';');
	if (pos != String::npos) {
		chunk_size_str = chunk_size_str.substr(0, pos);
	}

	trimString(chunk_size_str);

	if (chunk_size_str.empty()) {
		malformedRequest(BadRequest);
		return;
	}

	// set the chunk size
	char* end = NULL;
	this->_chunk_size = std::strtol(chunk_size_str.c_str(), &end, 16);
	if (*end != '\0') {
		malformedRequest(BadRequest);
		return;
	}

	// determine the next state
	if (this->_chunk_size == 0) {
		this->_chunk_state = CHUNK_TRAILERS;
	} else {
		this->_chunk_state = CHUNK_DATA;
	}
}



void	Request::consumeChunkData() {
	if (this->_buffer.empty()) {
		return;
	}

	// get the available bytes to write
	size_t bytes_to_read = std::min(this->_buffer.size(), this->_chunk_size);

	this->tmp_body_file.write(this->_buffer.data(), bytes_to_read);
	this->_buffer.erase(0, bytes_to_read);
	this->_chunk_size -= bytes_to_read;

	// if the chunk data was read parse the CRLF
	if (this->_chunk_size == 0) {
		this->_chunk_state = CHUNK_CRLF;
	}
}



void	Request::consumeChunkCRLF() {
	// check if the CRLF syntax is wrong befor blocking
	if (this->_buffer.size() < 2) {
		if (this->_buffer.size() == 1
			&& this->_buffer[0] != '\r'
			&& this->_buffer[0] != '\n') {

			malformedRequest(BadRequest);
		}
		return;
	}

	if (this->_buffer[0] == '\r' && this->_buffer[1] == '\n') {
		this->_buffer.erase(0, 2);
	} else if (this->_buffer[0] == '\n') {
		this->_buffer.erase(0, 1);
	} else {
		malformedRequest(BadRequest);
		return;
	}

	// read next chunk
	this->_chunk_state = CHUNK_SIZE;
}



void	Request::consumeTrailerSection() {
	String line = consumeLine();

	// line not ready wait for more data
	if (line == LINE_NOT_READY) {
		return;
	}

	// security a trailer section is too large
	// BAD_VALUE is returned in the case of line exeds 8kb
	if (line == BAD_VALUE) {
		return;
	}

	// if last line received mark the request complete
	if (line == CRLF) {
		this->_state = COMPLETE;
	}

	// if its a valid trailer under 8kb return to read next line
}



void	Request::readBodyWithContentLengt() {
	if (this->_buffer.length() >= this->_body_length) {
		// write to the tmp fie
		this->tmp_body_file.write(this->_buffer.data(), this->_body_length);

		// update the buffer
		this->_buffer.erase(0, this->_body_length);

		this->_state = COMPLETE;
	}

	else {
		// write to the tmp fie
		this->tmp_body_file.write(this->_buffer.data(), this->_buffer.length());

		// update buffer length
		this->_body_length -= this->_buffer.length();

		// update the buffer
		this->_buffer.clear();
	}
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
		this->_connection->state = CLOSE;
		return;
	}

	// a network fatal error occured: bytes_read == -1
	else {
		this->_connection->state = CLOSE;
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
			malformedRequest(PayloadTooLarge);
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
			// return that a line is CRLF only
			line = CRLF;
		}

		// syntax error
		else {
			malformedRequest(BadRequest);
			return BAD_VALUE;
		}
	}

	// line contain only white spaces
	else if (line.find_first_not_of(WHITE_SPACES) == String::npos) {
		malformedRequest(BadRequest);
		return BAD_VALUE;
	}

	// truncate buffer
	size_t CRLF_end_position;
	if (this->_buffer[pos] == '\r') {
		CRLF_end_position = 2;
	} else {
		CRLF_end_position = 1;
	}
	this->_buffer = this->_buffer.substr(pos + CRLF_end_position);

	return line;
}



bool	Request::transferEncodingPresent() {
	if (this->headers.find("transfer-encoding") != this->headers.end()) {
		return true;
	}

	return false;
}



bool	Request::contentLengthPresent() {
	if (this->headers.find("content-length") != this->headers.end()) {
		return true;
	}

	return false;
}



bool	Request::linkServerObject() {

	if (this->_connection->server != NULL) {
		return true;
	}


	Headers::iterator host_field = this->headers.find("host");

	float HTTP_version = static_cast<float>(std::atof(this->HTTP_version.c_str()));
	String host_value;
	if (host_field == this->headers.end()) {
		// including the host field in HTTP 1.0 is optional
		// if its HTTP 1.0 use default server
		if (HTTP_version > 1.0f) {
			malformedRequest(BadRequest);
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
	String& ip_port = this->_connection->ip_port;
	Servers::iterator ip_port_servers = this->_connection->servers.find(ip_port);

	// find the server block mapped to the socket and the hostname
	std::map<String, Server>::iterator server;
	server = ip_port_servers->second.find(host_value);

	// if not found and the there was no server defined
	// with 'default' as its host name, then use the first server
	if (server == ip_port_servers->second.end()) {
		server = ip_port_servers->second.begin();
	}


	// assign the connection server pointer to the correct server block
	this->_connection->server = &server->second;
	return true;
}



String	Request::generateRandomFileName() {
	// seed the rand() engine
	std::srand(std::time(NULL));

	// define allowed characters
	const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
						   "abcdefghijklmnopqrstuvwxyz"
						   "0123456789";

	int max_index = sizeof(charset) - 1;

	String file_name;
	for (int i = 0; i < 20; i++) {
		// generate a random index and append the character at that index
		file_name += charset[std::rand() % max_index];
	}

	return file_name;
}

// --------------------------------------------
