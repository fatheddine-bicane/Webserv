#pragma once

#include "../../Core_modules/Typedef.hpp"
#include "../../Config_parser/Parser/Definitions/Directives.hpp"
#include "../../Core_modules/HTTPStatus.hpp"
#include "../Exceptions/ResponseExceptions.hpp"
#include <cstddef>
#include <fstream>

#define _8KB 8192

class ClientConnection;


class Response {
private:
	enum ResponseState {
		DISK_FILE, BUILT_BODY,
		NAKED_HEADERS, RESPONS_SERVED,
		CONNECTION_CLOSED
	};

	enum DiskFileState {
		STAGED_BUFFER_READY,
		STAGED_BUFFER_SENT,
	};

private:
	String				_headers;
	ClientConnection*	_client_connection;

	ResponseState		_response_state;
	DiskFileState		_disk_file_state;

	size_t				_bytes_sent;
	String				_staging_buffer;
	std::ifstream		_file_to_send;
	String				_file_to_send_name;


public:
	bool				serve_file;
	bool				is_served;


public:
	// INFO: constructor
	Response(ClientConnection* client_connection);
	~Response();


	// INFO: api
	void	initializeResponseObject();
	void	sendResponse();
	bool	openFileToSend(const String& file_name);
	void	appendHeaders(const String& key, const String& value,
						  bool last_header=false);


	// INFO: helper functions
private:
	void	populateStagingBufferFromFileToSend();
	void	sendStagedBufferPayload();
	void	buildStatusLine(HTTPStatus HTTP_status);
	String	getContentType();
	String	getContentType(const String& file_name);
	bool	attemptOpeningErrorPageFile(HTTPStatus HTTP_status);
	String	buildErrorPage(HTTPStatus HTTP_status);
	String	getContentLength();
	String	extractReasonPhrase(HTTPStatus HTTP_status);
};
