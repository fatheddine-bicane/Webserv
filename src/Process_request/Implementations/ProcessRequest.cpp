#include "../Definitions/ProcessRequest.hpp"
#include "../Exceptions/ProcessRequestException.hpp"
#include <cstddef>
#include <list>
#include <sys/stat.h>


// INFO: constructor
// -----------------------------------------------------------

ProcessRequest::ProcessRequest(Request& request, Server& server)
	: _request(request),
	  _server(server) {
	this->_location = NULL;
}

// -----------------------------------------------------------




// INFO: api
// -----------------------------------------------------------

void	ProcessRequest::processRequest() {
	if (this->_request.isRequestState(MALFORMED)) {
		// handle error
	}
	
	else {
		try {
			switch (this->_request.method) {
				case GET:
					// handle get
					break;

				case POST:
					// handle post
					findLocationBlock();
					processPostRequest();
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
			std::cout << "ProcessRequestException: " << e.what() << std::endl;
		}
	}
}

// -----------------------------------------------------------



void	ProcessRequest::findLocationBlock() {
	std::list<Location>::iterator it = this->_server.locations.begin();
	std::list<Location>::iterator end = this->_server.locations.end();

	Location*	default_path_location = NULL;

	int matched_char = 0;
	for (; it != end; it++) {
		if (this->_request.target_resource == it->path) {
			this->_location = &(*it);
			return;
		}

		// match the longest uri
		else {
			if (it->path == "/") {
				default_path_location = &(*it);
			}

			if (this->_request.target_resource.find(it->path) == 0) {

				int current_path_length = it->path.length();

				if (current_path_length > matched_char) {
					matched_char = current_path_length;
					this->_location = &(*it);
				}
			}
		}
	}

	if (matched_char == 0 && this->_location == NULL) {
		if (default_path_location) {
			this->_location = default_path_location;
		}

		else {
			throw ProcessRequestException(NotFound);
		}
	}
}

static String getMethodName(HTTPMethod method) {
	switch (method) {
		case GET: return "GET";
		case POST: return "POST";
		case DELETE: return "DELETE";
		case PUT: return "PUT";
		default: return "";
	}
}

void ProcessRequest::processPostRequest(){
	// 1 check if POST is allowed in _location
	if (!this->_location->limit_except.empty()) {
		String methodName = getMethodName(this->_request.method);
		if (this->_location->limit_except.find(methodName) == this->_location->limit_except.end()) {
			throw ProcessRequestException(MethodNotAllowed);
		}
	}
	// 2 check if location is for CGI
	// if cgi ...
	// else

	// 3 Handle as upload
	String root = this->_location->shared_directives.root;
	if (!root.empty() && root[root.length() - 1] == '/') {
		root.erase(root.length() - 1);
	}
	String filePath = root + this->_request.target_resource;

	// Open the temporary file where the request body was stored
	std::ifstream inFile(this->_request.tmp_body_file_name.c_str(), std::ios::binary);
	if (!inFile) {
		throw ProcessRequestException(InternalServerError);
	}
	// Open the destination file
	std::ofstream outFile(("./nginx/" + filePath).c_str(), std::ios::binary);
	if (!outFile) {
		inFile.close();
		throw ProcessRequestException(InternalServerError);
	}
	// Write the contents from the temporary file to the destination file
	outFile << inFile.rdbuf();

	inFile.close();
	outFile.close();

	remove(this->_request.tmp_body_file_name.c_str());

}
