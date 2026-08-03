#pragma once

#include "../../Core_modules/Typedef.hpp"
#include "../../Config_parser/Parser/Definitions/Directives.hpp"
#include "../../Core_modules/HTTPStatus.hpp"
#include <cstddef>
#include <fstream>

class ClientConnection;


class Response {
private:
	enum ResponseState {
		DISK_FILE, BUILT_BODY
	};

private:
	String				_headers;
	ClientConnection*	_client_connection;

	ResponseState		_response_state;
	size_t				_bytes_sent;
	String				_staging_buffer;
	std::ifstream		_file_to_send;


public:
	String				file_to_send_name;


public:
	// INFO: constructor
	Response(ClientConnection* client_connection);
	~Response();


	// INFO: api
	void	initializeResponseObject();
	void	sendResponse();
	void	appendHeaders(const String& key, const String& value,
						  bool last_header=false);


	// INFO: helper functions
private:
	void	buildStatusLine(HTTPStatus HTTP_status);
	String	getContentType();
	String	getContentType(const String& file_name);
	bool	attemptOpeningErrorPageFile(HTTPStatus HTTP_status);
	String	buildErrorPage(HTTPStatus HTTP_status);
	bool	openFileToSend();
	bool	openFileToSend(const String& file_name);
	String	getContentLength();
	String	extractReasonPhrase(HTTPStatus HTTP_status);
};
