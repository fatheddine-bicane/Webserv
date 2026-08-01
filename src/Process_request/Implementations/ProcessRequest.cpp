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
