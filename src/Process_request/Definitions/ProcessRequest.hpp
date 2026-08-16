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
	String			_cgi_body_file_name;
	CGIState		_cgi_state;
	String			_cgi_pipe_buffer;
	String			_cgi_headers;


	// url
	String		_script_name;

	String		_file_path;
	String		_path_info;
	String		_query_string;

	bool		_is_cgi_request;

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
	void	readCGIPipe(EP_INSTANCE epfd);
	void	parseCGIHeaders();
	bool	isCGISucceed(std::vector<pid_t>& cgis_to_reap);

private:
	void	splitURLFromQeury();


	String	getFilePath();
	void	resolveFilePath();
	void	checkPotentialIndex();



	// cgi
	void	processCGIRequest();
	void	setPathEnvVariables(std::vector<String>& env);
	void	setHeadersEnvVariables(std::vector<String>& env);
	void	setUpChildProcess(PIPE& fds, std::vector<String>& env);
	void	setUpParentProcess(PIPE& fds);

	void	checkPotentialCGIRequest();
	bool	findScriptInterpreter(const String& extention);

	void	readCGIHeaders(char* buffer, ssize_t bytes_read);
	void	readCGIBody(char* buffer, ssize_t bytes_read);

	void	processGetRequest();
	void	renderDirectoryListing();



	void	processPostRequest();
	void	processDeleteRequest();
	void	processPutRequest();
	void	processMalformedRequest();

};
