#pragma once

#include "../../Includes/Webserv.hpp"


#define MAX_REQUEST_SIZE 2047

enum HTTPmethod{
	GET, POST, DELETE, PUT, UNKNOWN
};

enum RequesState {
	STARTLINE, HEADERS, BODY, COMPLETE
};

class Request {
private:
	SOCKET _fd;
	ClientConnection* _client_connection;
	String _buffer;

	RequesState _state;
	HTTPmethod _method;

	String _HTTP_version;
	
	String _uri;

	std::map<String, String> _headers;

	size_t _body_lenght;
	bool _has_body;

	String consumeLine();
public:
	Request();
	~Request();

	String getMethod() const;
	String getUri() const;
	String getHttpVersion() const;
	String getHeader(const std::string& fieldName) const;
	std::map<String, String> getAllHeaders() const;
	String getBody() const;
	size_t getBodyLength() const;
	RequesState getRequestState() const;
	void parseStartLine();
	void parseHeaders();
	bool expectsBody() const;
};
