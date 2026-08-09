#include "../Definitions/ProcessRequest.hpp"
#include "../Exceptions/ProcessRequestException.hpp"
#include <algorithm>
#include <cstddef>
#include <vector>


// INFO: constructor
// -----------------------------------------------------------

ProcessRequest::ProcessRequest(Request& request, Server& server)
	: _request(request),
	  _server(server) {
	this->_location = request.location;
}

// -----------------------------------------------------------




// INFO: api
// -----------------------------------------------------------

void	ProcessRequest::processRequest() {
	if (this->_request.isRequestState(MALFORMED)) {
		return;
	}

	else {
		try {

			if (isCGIRequest()) {
				// handle cgi
				processCGIRequest();
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
}

// -----------------------------------------------------------


void	ProcessRequest::processCGIRequest() {
	std::vector<String> env;

	setPathEnvVariables(env);
	setHeadersEnvVariables(env);

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

