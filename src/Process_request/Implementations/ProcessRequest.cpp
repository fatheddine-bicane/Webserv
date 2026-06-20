#include "../Definitions/ProcessRequest.hpp"
#include "../Exceptions/ProcessRequestException.hpp"
#include <cstddef>
#include <list>


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
