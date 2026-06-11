#include "../Definitions/Request.hpp"
#include "../../Includes/ClientConnection.hpp"

// INFO: constructor
// --------------------------------------------

Request::Request(SOCKET fd, ClientConnection* client_connection) {
	this->_fd = fd;
	this->_client_connection = client_connection;
	this->_state = START_LINE;
}

// --------------------------------------------




// INFO: api
// --------------------------------------------

void	Request::attemptRequestParse() {
	readSocketBuffer();


}



bool	Request::isRequestState(RequestState request_state) {
	if (request_state == INCOMPLETE) {
		return (this->_state == START_LINE
				|| this->_state == HEADERS
				|| this->_state == BODY);
	}

	return (this->_state == request_state);
}

// --------------------------------------------




// INFO: helper functions
// --------------------------------------------

void	Request::readSocketBuffer() {
	char	buffer[4096];

	while (true) {
		size_t bytes_read = recv(this->_fd, buffer, sizeof(buffer), 0);

		// buffer was populated append the read data to request buffer
		if (bytes_read > 0) {
			this->_buffer.append(buffer, bytes_read);
		}

		// client closed the connection
		else if (bytes_read == 0) {
			// if state complete serve request
			// close connection
			break;
		}

		// an error occured: bytes_read == -1
		else {
			// socket read buffer is fully drained
			// attempting to read will send the process on sleep mode
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				break;
			}

			// read was intrupted by a signal not data was read
			// signal was handled (if any handler was installed)
			// attempt to read again
			else if (errno == EINTR) {
				continue;
			}

			// a network fatal error close the connection
			else {
				break;
			}
		}
	}
}

// --------------------------------------------
