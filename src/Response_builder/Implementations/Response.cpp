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



// -----------------------------------------------------------


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
