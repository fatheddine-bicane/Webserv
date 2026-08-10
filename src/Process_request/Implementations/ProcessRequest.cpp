#include "../Definitions/ProcessRequest.hpp"
#include "../Exceptions/ProcessRequestException.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <sys/_types/_pid_t.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <vector>


// INFO: constructor
// -----------------------------------------------------------

ProcessRequest::ProcessRequest(Request& request, Server& server,
							   ClientConnection& client_connection)
	: _request(request),
	  _server(server),
	  _client_connection(client_connection) {
	this->_location = request.location;
}

// -----------------------------------------------------------




// INFO: api
// -----------------------------------------------------------

void	ProcessRequest::processRequest() {
	if (this->_request.isRequestState(MALFORMED)) {
		return;
	}

	resolveFilePath();

	try {

		if (isCGIRequest()) {
			// handle cgi
			processCGIRequest();
			this->_request.setRequestState(CGI);
			return;
		}


		// NOTE: each handler should mark there request state
		// as 'COMPLETE' in case of success and in the case of
		// failure mark the request state 'MALFORMED' using the
		// the exposed setRequestState() method, and set the
		// status code to the right HTTP status code and throw
		// the 'ProcessRequestException' exception
		switch (this->_request.method) {
			case GET:
				// handle get
				break;

			case POST:
				// handle post
				break;

			case DELETE:
				// handle delete
				break;

			case PUT:
				// handle put
				break;
		}
	}

	// handler function couldnt process the request
	catch (ProcessRequestException& e) {
		// handl error
		this->_request.status_code = e.status_code;
		this->_request.setRequestState(MALFORMED);
	}
}

// -----------------------------------------------------------



void	ProcessRequest::resolveFilePath() {
	// strip the query string if it exists
	String clean_uri = this->_client_connection.request.target_resource;

	size_t query_pos = clean_uri.find('?');
	if (query_pos != String::npos) {
		clean_uri = clean_uri.substr(0, query_pos);
	}

	// ROOT directive gets priority
	if (!this->_location->shared_directives.root.empty()) {
		this->_file_path = this->_location->shared_directives.root + clean_uri;
	} 
	// ALIAS directive fallback
	else if (!this->_location->alias.empty()) {
		String uri_remainder;

		// Strip the matched location path from the URI
		if (clean_uri.find(this->_location->path) == 0) {
			uri_remainder = clean_uri.substr(this->_location->path.length());
		} else {
			uri_remainder = clean_uri; 
		}

		this->_file_path = this->_location->alias + uri_remainder;
	}


	// WARNING: this needs to be adjusted to force atleast one directive 
	// Default fallback
	else {
		this->_file_path = clean_uri;
	}

}




void	ProcessRequest::processCGIRequest() {
	std::vector<String> env;

	setPathEnvVariables(env);
	setHeadersEnvVariables(env);

	// create a pipe
	PIPE fds;
	if (!IsValidPipe(pipe(fds))) {
		throw ProcessRequestException(InternalServerError);
	}

	this->_client_connection.pid = fork();

	if (isChildProcess(this->_client_connection.pid)) {
		// if there is a body file dup the stdin
		if (!this->_request.tmp_body_file_name.empty()) {
			File body_file = open(this->_request.tmp_body_file_name.c_str(), O_RDONLY);
			if (body_file == -1 || dup2(body_file, STDIN_FILENO)) {
				std::exit(EXIT_FAILURE);
			}
			close(body_file);
		}

		// dup the stdout
		if (dup2(fds[1], STDOUT_FILENO) == -1) {
			std::exit(EXIT_FAILURE);
		}

		close(fds[0]);
		close(fds[1]);


		// arguments
		char* args[2] = {
			const_cast<char*>(this->_file_path.c_str()),
			NULL
		};

		// env variables
		char* env_variables[env.size() + 1];
		for (int i = 0; i < env.size(); i++) {
			env_variables[i] = const_cast<char*>(env[i].c_str());
		}
		env_variables[env.size()] = NULL;

		if (this->_interpreter == "python3") {
			execve(PYTHON_INTERPRETER, args, env_variables);
		} else if (this->_interpreter == "node") {
			execve(NODE_INTERPRETER, args, env_variables);
		}

		// fallback for execve
		std::exit(EXIT_FAILURE);

	}

	else if (isParentProcess(this->_client_connection.pid)) {

	}

}



void	ProcessRequest::setPathEnvVariables(std::vector<String>& env) {
	String& url = this->_request.target_resource;

	// set the script name variable
	String script_name = url.substr(0, this->_extention_pos);
	env.push_back("SCRIPT_NAME=" + script_name);

	// url contains the script name alone
	if (this->_extention_pos == url.length()) {
		return;
	}

	size_t query_pos = url.find_first_of('?');

	// there is a query string
	if (query_pos != String::npos) {

		// there is path info
		if (this->_extention_pos != query_pos) {
			// set the path info variable
			String path_info = url.substr(this->_extention_pos, query_pos - this->_extention_pos);
			env.push_back("PATH_INFO=" + path_info);
		}

		// set the query string variable
		String query_string = url.substr(query_pos + 1);
		env.push_back("QUERY_STRING=" + query_string);
	}

	// there is only a path info
	else {
		// empthy QUERY_STRING according to cgi rfc
		env.push_back("QUERY_STRING=");

		// set the path info variable
		String path_info = url.substr(this->_extention_pos);
		env.push_back("PATH_INFO=" + path_info);
	}
}



void	ProcessRequest::setHeadersEnvVariables(std::vector<String>& env) {
	Headers::iterator header = this->_request.headers.begin();
	Headers::iterator headers_end = this->_request.headers.end();
	for (; header != headers_end; header++) {
		if (header->first == "content-length") {
			env.push_back("CONTENT_LENGTH=" + header->second);
		} else if (header->first == "content-type") {
			env.push_back("CONTENT_TYPE=" + header->second);
		} else {
			String header_name = header->first;

			// capitalize the header name
			std::transform(header_name.begin(), header_name.end(),
						   header_name.begin(), ::toupper);

			// replace hyphen with underscore
			size_t hyphen_pos = header_name.find('-');
			if (hyphen_pos != String::npos) {
				header_name[hyphen_pos] = '_';
			}

			// push the newly created env variable
			env.push_back("HTTP_" + header_name + '=' + header->second);
		}
	}
}



bool	ProcessRequest::isCGIRequest() {
	String& url = this->_request.target_resource;

	// isolate the URI path by removing the query string
	size_t queryPos = url.find('?');
	String path = (queryPos != String::npos) ? url.substr(0, queryPos) : url;

	// convert the path to lowercase for case-insensitive matching
    for (size_t i = 0; i < path.length(); ++i) {
        path[i] = std::tolower(static_cast<unsigned char>(path[i]));
    }

    // search for Python extension followed by end-of-string or '/' (PATH_INFO)
    size_t pyPos = path.find(".py");
    while (pyPos != String::npos) {
        if (pyPos + 3 == path.length() || path[pyPos + 3] == '/') {
            this->_interpreter = "python3";
			this->_extention_pos = pyPos + 3;
            return true;
        }
        pyPos = path.find(".py", pyPos + 1);
    }

    // else search for JavaScript extension followed by end-of-string or '/' (PATH_INFO)
    size_t jsPos = path.find(".js");
    while (jsPos != String::npos) {
        if (jsPos + 3 == path.length() || path[jsPos + 3] == '/') {
            this->_interpreter = "node";
			this->_extention_pos = jsPos + 3;
            return true;
        }
        jsPos = path.find(".js", jsPos + 1);
    }

    return false;
}

