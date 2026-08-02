#pragma once


enum TokenType {
	// single character token
	SEMICOLON, CONTEXT_START, CONTEXT_END, HASH,

	// directives argument
	VALUE,

	// directive keyword
	EVENTS, HTTP, SERVER, LOCATION, LIMIT_EXCEPT,
	LISTEN, ERROR_PAGE, ROOT, CLIENT_MAX_BODY_SIZE,
	AUTOINDEX, INDEX, ALIAS, RETURN, DAV_METHODS,
	CLIENT_BODY_TEMP_PATH, CGI_PASS, SERVER_NAME,

	// end of file
	END_OF_FILE
};
