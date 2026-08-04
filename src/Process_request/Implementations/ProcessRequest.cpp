#include "../Definitions/ProcessRequest.hpp"
#include "../Exceptions/ProcessRequestException.hpp"
#include <cstddef>


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
