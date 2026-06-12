#pragma once

#include <map>
#include <sys/socket.h>
#include <cerrno>

#include "../../Core_modules/Typedef.hpp"


class ClientConnection;

enum RequestState {
	// parse ongoing
	INCOMPLETE,
	START_LINE, HEADERS, BODY,
	// request parsed and its correct
	COMPLETE,
	// request parsed and its not correct
	MALFORMED
};


enum HTTPMethod {
	GET, POST, DELETE, PUT, UNSUPPORTED
};


// INFO: main class
class Request {
private:
	RequestState		_state;
	SOCKET				_fd;
	String				_buffer;
	ClientConnection*	_client_connection;
	

public:
	// HTTP message
	// start-line
	HTTPMethod					method;
	String						target_resource;
	String						HTTP_version;
	// headers
	std::map<String, String>	headers;
	// TODO:body handling to be determined

	STATUS_CODE			status_code;

public:
	// INFO: constructor
	Request(SOCKET fd, ClientConnection* client_connection);


public:
	// INFO: api
	void	attemptRequestParse();
	bool	isRequestState(RequestState request_state);




private:
	// INFO: helper functions
	void	readSocketBuffer();
	size_t	getCRLFPosition();
	void	replaceBareCRWithSP(String& request_line);
	void	trimString(String& string);
	bool	malformedRequest(STATUS_CODE status_code);
	String	consumeLine();

	// INFO: parse start line
	void	parseStartLine();
	bool	parseMethod(String& start_line);
	bool	parseTargetResource(String& start_line);
	bool	parseHTTPVersion(String& start_line);

};
