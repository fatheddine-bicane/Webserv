#pragma once

#include "../../Core_modules/Typedef.hpp"
#include "../../Config_parser/Parser/Definitions/Directives.hpp"
#include "../../Core_modules/HTTPStatus.hpp"

class ClientConnection;


class Response {
private:
	String	_headers;
	ClientConnection* _client_connection;

public:
	String	file_to_send;


public:
	// INFO: constructor
	Response(ClientConnection* client_connection);


	// INFO: api
	void	initialHeaders(HTTPStatus HTTP_status);
	void	appendHeaders(const String& key, const String& value, bool last_header=false);

	String	getContentType();
	String	getContentType(const String& file_name);
	String	extractReasonPhrase(HTTPStatus HTTP_status);
};
