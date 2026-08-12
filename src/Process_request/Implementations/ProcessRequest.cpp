#include "../Definitions/ProcessRequest.hpp"
#include "../Exceptions/ProcessRequestException.hpp"
#include <cstddef>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>


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

	// mock	
	resolveFilePath();

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
					processGetRequest();
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



void	ProcessRequest::processGetRequest() {
	// check if file exists and is readable
	if (access(this->_file_path.c_str(), F_OK) == -1) {
		throw ProcessRequestException(NotFound);
	}
	if (access(this->_file_path.c_str(), R_OK) == -1) {
		throw ProcessRequestException(Forbidden);
	}

	struct stat info;
	if (stat(this->_file_path.c_str(), &info) != 0) {
		throw ProcessRequestException(InternalServerError);
	}

	if (S_ISDIR(info.st_mode)) {
		if (this->_location->shared_directives.autoindex) {
			renderDirectoryListing();
			this->_request.status_code = OK;
			this->_request.setRequestState(COMPLETE);
			return;
		}

		throw ProcessRequestException(Forbidden);
	}

	// file is a regular file, send it
	else {
		if (!this->_client_connection->openFileToSend(this->_file_path)) {
			throw ProcessRequestException(InternalServerError);
		}
		this->client_connection->response.serve_file = true;
		this->request.status_code = OK;
		this->_request.setRequestState(COMPLETE);
	}
}

void	ProcessRequest::renderDirectoryListing() {
	DIR* directory = opendir(this->_file_path.c_str());
	if (directory == NULL) {
		throw ProcessRequestException(InternalServerError);
	}

	String html;
	html += "<!DOCTYPE html>\r\n";
	html += "<html><head><title>Directory listing</title></head><body>\r\n";
	html += "<h1>Index of " + this->_request.target_resource + "</h1>\r\n";
	html += "<ul>\r\n";

	struct dirent* entry;
	while ((entry = readdir(directory)) != NULL) {
		String name = entry->d_name;
		if (name == "." || name == "..") {
			continue;
		}

		html += "<li><a href=\"" + name + "\">" + name + "</a></li>\r\n";
	}
	closedir(directory);

	html += "</ul>\r\n";
	html += "</body></html>\r\n";

	// set the response headers for the directory listing
	std::stringstream size;
	size << html.size();
	this->_client_connection->response.appendHeader("Content-Length", size.str());
	this->_client_connection->response.appendHeader("Content-Type", "text/html", true);

	this->_client_connection->response.appendDirectoryListeningBody(html);
}
