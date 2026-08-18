#include "../Definitions/Response.hpp"
#include "../../Core_modules/Connection/Definitions/ClientConnection.hpp"
#include <cstddef>



// INFO: constructor
// -----------------------------------------------------------

Response::Response(ClientConnection* client_connection) 
	: _client_connection(client_connection),
	  _response_state(DEFAULT),
	  _bytes_sent(0),
	  serve_file(false),
	  is_served(false),
	  content_type_is_set(false),
	  content_length_is_set(false),
	  is_cgi_response(false) {

	appendHeaders("Server", "Webserv/1.0");
	appendHeaders("Date", getHttpDateHeaderValue());
	appendHeaders("Connection", "close");
}

Response::~Response() {
	this->_file_to_send.close();

	// if its a cgi remove the tmp body file
	if (this->is_cgi_response && this->serve_file) {
		unlink(this->_file_to_send_name.c_str());
	}
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
            // send the stored headers
            this->_disk_file_state = STAGED_BUFFER_READY;
		}

		// else build one
		else {
			this->_staging_buffer = this->_headers + buildErrorPage(HTTP_status);
			this->_response_state = BUILT_BODY;
		}
	}

	// responde with a file
	else if (this->serve_file) {
		this->_staging_buffer = this->_headers;
		this->_response_state = DISK_FILE;
		// send the stored headers
		this->_disk_file_state = STAGED_BUFFER_READY;
	}

	else if (this->_response_state == DIRECTORY_LISTING_HTML_BODY) {
		this->_staging_buffer = this->_headers + this->_staging_buffer;
	}

	// else there is no file to serve just headers
	else {
		this->_staging_buffer = this->_headers;
		this->_response_state = NAKED_HEADERS;
	}

}



void	Response::appendHeaders(const String& key, const String& value) {
	if (key == "Content-Length" && this->content_length_is_set) return;
	else if (key == "Content-Type" && this->content_type_is_set) return;

	this->_headers += key + ": " + value + "\r\n";

	if (key == "Content-Length") {
		this->content_length_is_set = true;
	} else if (key == "Content-Type") {
		this->content_type_is_set = true;
	}
}



void	Response::sendResponse() {

	switch (this->_response_state) {
		case DISK_FILE:
			if (this->_disk_file_state == STAGED_BUFFER_SENT) {
				populateStagingBufferFromFileToSend();
			}
			if (this->_disk_file_state == STAGED_BUFFER_READY) {
				sendStagedBufferPayload();
			}

			if (this->_file_to_send.eof()) {
				this->_response_state = RESPONS_SERVED;
			}
			break;

		case BUILT_BODY:
		case NAKED_HEADERS:
		case DIRECTORY_LISTING_HTML_BODY:
			sendStagedBufferPayload();
			break;

		// warning silencer
		default: break;
	}

	// in the case of any error or response is sent,
	// close the connection and clear the connection object
	if (this->_response_state == RESPONS_SERVED
		|| this->_response_state == CONNECTION_CLOSED) {
		this->is_served = true;
	}

	// response is not fully served yet
}


void	Response::appendDirectoryListeningBody(const String& body) {
	this->_response_state = DIRECTORY_LISTING_HTML_BODY;
	this->_staging_buffer = body;
}

void	Response::appendCTLF() {
	this->_headers += "\r\n";
}

// -----------------------------------------------------------



// INFO: helpers
// -----------------------------------------------------------



String	Response::getHttpDateHeaderValue() {
	char buffer[128];
	time_t rawtime;

	// get current time
	time(&rawtime);
	// convert to coordinated universal time
	struct tm* timeinfo = gmtime(&rawtime);

	// format according to RFC (Tue, D MM YYYY HH:MM:SS GMT)
	strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", timeinfo);

	return String(buffer);
}



void	Response::populateStagingBufferFromFileToSend() {
	char chunk_buffer[_8KB];
	this->_file_to_send.read(chunk_buffer, _8KB);

	// check for the exact number of bytes extracted
	std::streamsize bytes_read = this->_file_to_send.gcount();
	if (bytes_read > 0) {
		this->_staging_buffer.append(chunk_buffer, bytes_read);
	}

	this->_disk_file_state = STAGED_BUFFER_READY;
}



void	Response::sendStagedBufferPayload() {
	size_t bytes_remaining = this->_staging_buffer.length() - this->_bytes_sent;
	const char* current_ptr = this->_staging_buffer.c_str() + this->_bytes_sent;

	SOCKET client_fd = this->_client_connection->fd;
	ssize_t sent = send(client_fd, current_ptr, bytes_remaining, 0);

	if (sent > 0) {
		this->_bytes_sent += sent;

		// if the entire payload has now been transmitted
		// mark the buffer as server
		if (this->_bytes_sent == this->_staging_buffer.length()) {
			// if serving a file mark the current buffer as sent
			if (this->_response_state == DISK_FILE) {
				this->_disk_file_state = STAGED_BUFFER_SENT;
				this->_bytes_sent = 0;
				this->_staging_buffer.clear();
			}

			// any other serving method means that the response was
			// served nothing more to serve
			else {
				this->_response_state = RESPONS_SERVED;
			}
		}
	}

	// client disconnected
	else if (sent == 0) {
		this->_response_state = CONNECTION_CLOSED;
	}

	// underlying socket error
	else {
		throw ClientSocketErrorException();
	}
}



void	Response::buildStatusLine(HTTPStatus HTTP_status) {
	// transform the status code from an enum to a string
	std::ostringstream ss;
	ss << HTTP_status;
	String status_code = ss.str();
    String reason_phrase = extractReasonPhrase(HTTP_status);

	// clear the stream
	ss.str("");
	ss.clear();



	ss << "HTTP/1.1 " << status_code << " " << reason_phrase << "\r\n";

	if (HTTP_status < 400) {
		// append file related headers
		if (this->serve_file) {
			appendHeaders("Content-Type", getContentType());
			appendHeaders("Content-Length", getContentLength());
		}

		appendCTLF();
		
		ss << this->_headers;
	} else {
		ss << this->_headers;
	}

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
	if (this->_client_connection->request.location == NULL) return false;

	std::map<int, String>& error_pages =
		this->_client_connection->request.location->shared_directives.error_page;

	std::map<int, String>::iterator error_page = error_pages.find(HTTP_status);

	// if the error page is not found
	if (error_page == error_pages.end()) return false;

	if (!openFileToSend(error_page->second)) return false;

	// append file related headers
	appendHeaders("Content-Type", getContentType(error_page->second));
	appendHeaders("Content-Length", getContentLength());
	appendCTLF();

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
	appendHeaders("Content-Length", content_length);
	appendCTLF();

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
