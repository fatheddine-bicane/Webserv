#pragma once

#include "../../Includes/Typedef.hpp"
#include <iterator>


enum HTTPmethod{
	GET, POST, DELETE, PUT
};

enum RequesState {
	STARTLINE, HEADERS, BODY, COMPLETE
};

class Request {
private:
	RequesState _state;
	HTTPmethod _method;

	String _HTTP_version;
	
	String _uri;

	std::map<String, String> _headers;

	size_t _body_lenght;
	bool _has_body;
	String _body;
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
	void parseStartLine(const String& start_line);
	void parseHeaders(const String& headers_block);
	bool expectsBody() const;
};
