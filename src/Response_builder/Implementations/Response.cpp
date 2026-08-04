#include "../Definitions/Response.hpp"
#include "../../Core_modules/Connection/Definitions/ClientConnection.hpp"
#include <cstddef>



// INFO: constructor
// -----------------------------------------------------------

Response::Response(ClientConnection* client_connection) 
	: _client_connection(client_connection),
	  _bytes_sent(0) {

	this->serve_file = false;
	appendHeaders("Connection", "close");
	appendHeaders("Server", "Webserv/1.0");
}

Response::~Response() {
	this->_file_to_send.close();
}

// -----------------------------------------------------------




// INFO: api
// -----------------------------------------------------------


void	Response::initializeResponseObject() {
	HTTPStatus HTTP_status = this->_client_connection->request.status_code;

	buildStatusLine(HTTP_status);

	// responding with an error page
	if (HTTP_status >= 400) {
		// check if the config file defined an error page for this status code
		if (attemptOpeningErrorPageFile(HTTP_status)) {
			this->_staging_buffer = this->_headers;
			this->_response_state = DISK_FILE;
		}

		// else build one
		else {
			this->_staging_buffer = this->_headers + buildErrorPage(HTTP_status);
			this->_response_state = STAGED_BUFFER;
		}
	}

	// responde with a file
	else if (this->serve_file) {
		// append file related headers
		appendHeaders("Content-Type", getContentType());
		appendHeaders("Content-Length", getContentLength(), true);

		this->_staging_buffer = this->_headers;
		this->_response_state = DISK_FILE;
	}

	// else there is no file to serve just headers
	else {
		this->_staging_buffer = this->_headers;
		this->_response_state = STAGED_BUFFER;
	}

}



// WARNING: in the case where there is no file to serve, the PprocessRequest
// caller of this function should pass true to last element when append the
// last header, otherwise its the Response object responsibility to pass it.
void	Response::appendHeaders(const String& key, const String& value,
								bool last_header) {
	this->_headers += key + ": " + value + "\r\n";

	if (last_header) {
		this->_headers += "\r\n";
	}
}



void	Response::sendResponse() {

	switch (this->_response_state) {

		default: break;
	}





	// // Safety check: ensure there is still data left to send
	// if (this->_bytes_sent >= this->_response_str.length()) {
	// 	return;
	// }
	//
	// size_t bytes_remaining = this->_response_str.length() - this->_bytes_sent;
	// const char* current_ptr = this->_response_str.c_str() + this->_bytes_sent;
	//
	// // Execute exactly ONE send() call per epoll_wait() event
	// ssize_t sent = send(this->_client_fd, current_ptr, bytes_remaining, 0);
	//
	// if (sent > 0) {
	// 	// The kernel accepted a chunk of data. Advance the offset.
	// 	this->_bytes_sent += sent;
	//
	// 	// Check if the entire payload has now been transmitted
	// 	if (this->_bytes_sent == this->_response_str.length()) {
	// 		// Transition the connection state here 
	// 		// (e.g., EPOLL_CTL_MOD back to EPOLLIN, or close the socket)
	// 	}
	// }
	// else {
	// 	// sent <= 0
	// 	// Because epoll told us the socket was ready, a return of 0 (client disconnected)
	// 	// or -1 (underlying socket error) means the connection is dead.
	// 	// We do not check errno. We immediately drop the connection.
	//
	// 	// Execute socket cleanup and remove from epoll here.
	// }
}


// -----------------------------------------------------------



// INFO: helpers
// -----------------------------------------------------------

void	Response::buildStatusLine(HTTPStatus HTTP_status) {
	// transform the status code from an enum to a string
	std::ostringstream ss;
	ss << HTTP_status;
	String status_code = ss.str();
    String reason_phrase = extractReasonPhrase(HTTP_status);

	// clear the stream
	ss.str("");
	ss.clear();

	ss << "HTTP/1.1 " << status_code << " " << reason_phrase << "\r\n"
	   << this->_headers;
	this->_headers = ss.str();
}



String Response::getContentType() {
	return getContentType(this->_file_to_send_name);
}



String	Response::getContentType(const String& file_name) {
	// find the last occurrence of the dot character
	size_t dotPos = file_name.find_last_of('.');

	// if no dot is found or the dot is the last char return bynary stream
	if (dotPos == String::npos || dotPos == file_name.length() - 1) {
		return "application/octet-stream";
	}

	// extract the substring after the dot
	String extention = file_name.substr(dotPos + 1);

	// convert the extension to lowercase to ensure case-insensitive matching
	for (size_t i = 0; i < extention.length(); i++) {
		extention[i] = std::tolower(static_cast<unsigned char>(extention[i]));
	}

	// compare and return the standard MIME types
	if (extention == "html" || extention == "htm") return "text/html";
	if (extention == "css")                  return "text/css";
	if (extention == "js")                   return "application/javascript";
	if (extention == "json")                 return "application/json";
	if (extention == "jpg" || extention == "jpeg") return "image/jpeg";
	if (extention == "png")                  return "image/png";
	if (extention == "gif")                  return "image/gif";
	if (extention == "txt")                  return "text/plain";

	// fallback for unknown extensions
	return "application/octet-stream";
}



bool	Response::attemptOpeningErrorPageFile(HTTPStatus HTTP_status) {
	std::map<int, String>& error_pages =
		this->_client_connection->request.location->shared_directives.error_page;

	std::map<int, String>::iterator error_page = error_pages.find(HTTP_status);

	// if the error page is not found
	if (error_page == error_pages.end()) return false;

	if (!openFileToSend(error_page->second)) return false;

	String file_size = getContentLength();

	// append file related headers
	appendHeaders("Content-Type", getContentType(error_page->second));
	appendHeaders("Content-Length", file_size, true);

	return true;
}



String	Response::buildErrorPage(HTTPStatus HTTP_status) {
    String reason_phrase = extractReasonPhrase(HTTP_status);

	// construct the standard HTML document
    std::ostringstream html_body;
    html_body << "<html>\r\n"
              << "<head><title>" << HTTP_status << " " << reason_phrase << "</title></head>\r\n"
              << "<body>\r\n"
              << "<center><h1>" << HTTP_status << " " << reason_phrase << "</h1></center>\r\n"
              << "<hr><center>Web Server</center>\r\n"
              << "</body>\r\n"
              << "</html>\r\n";

    String error_page = html_body.str();

	// transform content length
	std::ostringstream oss;
	oss << error_page.length();
	String content_length = oss.str();

	appendHeaders("Content-Type", "text/html");
	appendHeaders("Content-Length", content_length, true);

	return error_page;
}



bool	Response::openFileToSend(const String& file_name) {
	// INFO: opening the file with the ate flag to position at the end
	//       of the file and get its size for the content-length header
	this->_file_to_send.open(file_name.c_str(),
						  std::ios::binary | std::ios::in | std::ios::ate);

	if (!this->_file_to_send.is_open()) return false;

	this->_file_to_send_name = file_name;
	return true;;
}



String	Response::getContentLength() {
	// get the file size
	std::streamsize size = this->_file_to_send.tellg();
	std::stringstream file_size;
	file_size << size;

	// reset file stream to the begining
	this->_file_to_send.seekg(0, std::ios::beg);

	return file_size.str();
}



String Response::extractReasonPhrase(HTTPStatus HTTP_status) {
    switch (HTTP_status) {
        // 1xx: Informational
        case Continue: return "Continue";
        case SwitchingProtocols: return "Switching Protocols";

        // 2xx: Successful
        case OK: return "OK";
        case Created: return "Created";
        case Accepted: return "Accepted";
        case NoContent: return "No Content";
        case PartialContent: return "Partial Content";

        // 3xx: Redirection
        case MovedPermanently: return "Moved Permanently";
        case Found: return "Found";
        case SeeOther: return "See Other";
        case NotModified: return "Not Modified";
        case TemporaryRedirect: return "Temporary Redirect";
        case PermanentRedirect: return "Permanent Redirect";

        // 4xx: Client Error
        case BadRequest: return "Bad Request";
        case Unauthorized: return "Unauthorized";
        case Forbidden: return "Forbidden";
        case NotFound: return "Not Found";
        case MethodNotAllowed: return "Method Not Allowed";
        case RequestTimeout: return "Request Timeout";
        case Conflict: return "Conflict";
        case Gone: return "Gone";
        case LengthRequired: return "Length Required";
        case PayloadTooLarge: return "Payload Too Large";
        case URITooLong: return "URI Too Long";
        case UnsupportedMediaType: return "Unsupported Media Type";
        case ExpectationFailed: return "Expectation Failed";
        case UpgradeRequired: return "Upgrade Required";

        // 5xx: Server Error
        case InternalServerError: return "Internal Server Error";
        case NotImplemented: return "Not Implemented";
        case BadGateway: return "Bad Gateway";
        case ServiceUnavailable: return "Service Unavailable";
        case GatewayTimeout: return "Gateway Timeout";
        case HTTPVersionNotSupported: return "HTTP Version Not Supported";

        default: return "Error";
    }
}

// -----------------------------------------------------------
