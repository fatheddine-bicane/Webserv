#include "../Definitions/Response.hpp"
#include <sstream>



// INFO: api
// -----------------------------------------------------------

Response::Response(ClientConnection* client_connection) 
	: _client_connection(client_connection) {}

// -----------------------------------------------------------




// INFO: api
// -----------------------------------------------------------

void	Response::initialHeaders(HTTPStatus HTTP_status) {
	// transform the status code from an enum to a string
	std::ostringstream oss;
	oss << HTTP_status;
	String status_code = oss.str();

	this->_headers = "HTTP/1.1 " + status_code + "  \r\n"
					 "Connection: close\r\n";
}



void	Response::appendHeaders(const String& key, const String& value, bool last_header) {
	this->_headers += key + ": " + value + "\r\n";

	if (last_header) {
		this->_headers += "\r\n";
	}
}



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
	return getContentType(this->file_to_send);
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
	for (int i = 0; i < extention.length(); i++) {
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
