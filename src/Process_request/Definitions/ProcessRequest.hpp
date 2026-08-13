#pragma once

#include "../../HTTP_request_parser/Definitions/Request.hpp"
#include "../../Core_modules/Connection/Definitions/ClientConnection.hpp"
#include <cstddef>
#include <fstream>
#include <iterator>
#include <vector>
#include <sys/epoll.h>
#include <sys/wait.h>





class ProcessRequest {

private:
	enum CGIState {
		READING_HEADERS, READING_BODY
	};


private:
	Request&	_request;
	Server&		_server;
	ClientConnection&	_client_connection;

	// cgi
	String		_interpreter;
	size_t		_extention_pos;
	EP_INSTANCE	_epfd;
	String			_cgi_body_file_name;
	CGIState		_cgi_state;
	String			_cgi_pipe_buffer;
	String			_cgi_headers;


	String		_file_path;

public:
	// INFO: constructor
	ProcessRequest(Request& request, Server& server,
				   ClientConnection& client_connection);


public:
	// INFO: api
	void	processRequest();
	// send response

	// cgi processors
	void	monitoreCGIPipe(EP_INSTANCE epfd);
	void	readCGIPipe();
	void	parseCGIHeaders();

private:
	String	getFilePath();
	void	resolveFilePath();



	// cgi
	void	processCGIRequest();
	void	setPathEnvVariables(std::vector<String>& env);
	void	setHeadersEnvVariables(std::vector<String>& env);
	void	setUpChildProcess(PIPE& fds, std::vector<String>& env);
	void	setUpParentProcess(PIPE& fds);
	bool	isCGIRequest();
	void	readCGIHeaders(char* buffer, ssize_t bytes_read);
	void	readCGIBody(char* buffer, ssize_t bytes_read);




	void	processGetRequest();
	void	processPostRequest();
	void	processDeleteRequest();
	void	processPutRequest();
	void	processMalformedRequest();

};
