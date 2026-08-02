#pragma once

#include <cstddef>
#include <fstream>
#include <map>
#include <sys/socket.h>
#include <cerrno>
#include <algorithm>
#include <cctype>
#include <string>
#include <unistd.h>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>

#include "../../Core_modules/Typedef.hpp"
#include "../../Config_parser/Parser/Definitions/Directives.hpp"
#include "../../Core_modules/HTTPStatus.hpp"

#define _8KB 8192
#define _4KB 4096
#define CRLF "\r\n"
#define LINE_NOT_READY "<|NOT_READY|>"
#define WHITE_SPACES "\t "
#define BAD_VALUE "<|BAD_VALUE|>"

class ClientConnection;

enum RequestState {
	// parse ongoing
	INCOMPLETE,
	START_LINE, HEADERS, BODY,
	// determining the BODY reading method
	DETERMINING_MESSAGE_BODY_LENGTH,

	//ready to serve responce
	READY_TO_SERVE,
	// request parsed and its correct
	COMPLETE,
	// request parsed and its not correct
	MALFORMED
};


enum HTTPMethod {
	GET, POST, DELETE, PUT
};

enum MessageBodyLength {
	CONTENT_LENGTH, CHUNKED
};

enum ChunkState {
    CHUNK_SIZE, CHUNK_DATA, CHUNK_CRLF, CHUNK_TRAILERS
};


// INFO: main class
class Request {
private:
	RequestState		_state;
	SOCKET				_fd;
	String				_buffer;
	ClientConnection*	_connection;


	// body helper variables
	MessageBodyLength	_mesage_body_length;
	std::ofstream		_tmp_body_file;

	// content length
	unsigned long		_body_length;

	// chunked encoding
	size_t				_chunk_size;
	bool				_expect_CRLF;
	ChunkState			_chunk_state;
	long				_total_received_bytes;
	

public:
	// HTTP message
	// start-line
	HTTPMethod					method;
	String						target_resource;
	String						HTTP_version;
	// headers
	Headers						headers;
	// body
	String						tmp_body_file_name;

	HTTPStatus					status_code;

	// mapped location block
	Location*					location;

public:
	// INFO: constructor
	Request(SOCKET fd, ClientConnection* client_connection);


public:
	// INFO: api
	void	attemptRequestParse();
	bool	isRequestState(RequestState request_state);


private:
	// INFO: parse request helpers
	void	parseStartLine();
	void	parseFieldLine();
	void	determiningMessageBodyLength();
	void	parseBody();


private:
	// INFO: parse start line helpers
	bool	parseMethod(String& start_line);
	bool	parseTargetResource(String& start_line);
	bool	parseHTTPVersion(String& start_line);

	// INFO: parse headers helpers
	String	parseFieldName(String& start_line);
	String	parseFieldValue(String& start_line);

	// INFO: parse body helpers
	bool	openTmpBodyFile();

	bool	defineTransferEncoding();
	void	readBodyWithTransferEncoding();
	void	consumeChunkSize();
	void	consumeChunkData();
	void	consumeChunkCRLF();
	void	consumeTrailerSection();

	bool	defineConetentLength();
	void	readBodyWithContentLengt();


private:
	// INFO: helper functions
	void	readSocketBuffer();
	size_t	getCRLFPosition();
	void	replaceBareCRWithSP(String& request_line);
	void	trimString(String& string);
	bool	malformedRequest(HTTPStatus status_code);
	String	consumeLine();
	bool	linkServerObject();
	bool	transferEncodingPresent();
	bool	contentLengthPresent();
	String	generateRandomFileName();
	bool	findLocationBlock();



};
