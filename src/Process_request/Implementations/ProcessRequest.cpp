#include "../Definitions/ProcessRequest.hpp"
#include "../Exceptions/ProcessRequestException.hpp"
#include <algorithm>
#include <csignal>
#include <cstddef>
#include <cstdlib>
#include <cctype>
#include <sstream>
#include <sys/fcntl.h>
#include <sys/signal.h>
#include <unistd.h>
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>


// INFO: constructor
// -----------------------------------------------------------

ProcessRequest::ProcessRequest(Request& request, Server& server,
							   ClientConnection& client_connection)
	: _request(request),
	  _server(server),
	  _client_connection(client_connection),
	  _is_cgi_request(false) {
}

// -----------------------------------------------------------




// INFO: api
// -----------------------------------------------------------

void	ProcessRequest::processRequest() {
	if (this->_request.isRequestState(MALFORMED)) {
		return;
	}

	splitURLFromQeury();

	checkPotentialCGIRequest();

	resolveFilePath();

	try {
		checkPotentialIndex();

		if (this->_is_cgi_request) {
			processCGIRequest();
			return;
		}


		// NOTE: each handler should mark there request state
		// as 'COMPLETE' in case of success and in the case of
		// failure mark the request state 'MALFORMED' using the
		// the exposed setRequestState() method, and set the
		// status code to the right HTTP status code and throw
		// the 'ProcessRequestException' exception
		switch (this->_request.method) {
			case GET:
				processGetRequest();
				// handle get
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



void	ProcessRequest::monitoreCGIPipe(EP_INSTANCE epfd) {
	File pipe_read_end = this->_client_connection.pipe_read_end;


	int flags = fcntl(pipe_read_end, F_GETFL, 0);
	fcntl(pipe_read_end, F_SETFL, flags | O_NONBLOCK);

	struct epoll_event event;
	std::memset(&event, 0, sizeof(event));
	event.events = EPOLLIN;
	event.data.ptr = &this->_client_connection;

	if (epoll_ctl(epfd, EPOLL_CTL_ADD, pipe_read_end, &event) == -1) {
		// remove the cgi process and close the read end of the pipe
		kill(this->_client_connection.pid, SIGKILL);
		waitpid(this->_client_connection.pid, NULL, 0);
		close(pipe_read_end);

		this->_request.status_code = InternalServerError;
		this->_request.setRequestState(MALFORMED);
		return;
	}

	this->_request.setRequestState(READ_CGI_PIPE);
	this->_cgi_state = READING_HEADERS;
}



void	ProcessRequest::readCGIPipe(EP_INSTANCE epfd) {
	File pipe_read_end = this->_client_connection.pipe_read_end;
	char buffer[_4KB];
	ssize_t bytes_read = read(pipe_read_end, buffer, sizeof(buffer) - 1);

	if (bytes_read > 0) {
		buffer[bytes_read] = '\0';

		switch (this->_cgi_state) {
			case READING_HEADERS:
				readCGIHeaders(buffer, bytes_read);
				break;

			case READING_BODY:
				readCGIBody(buffer, bytes_read);
				break;
		}
	}

	// no more data to read
	else if (bytes_read == 0) {
		close(pipe_read_end);
		epoll_ctl(epfd, EPOLL_CTL_DEL, pipe_read_end, NULL);

		this->_request.setRequestState(CGI_PIPE_DRAINED);
		this->_client_connection.response.openFileToSend(this->_cgi_body_file_name);
	}

	// error reading from pipe
	else if (bytes_read < 0 && errno != EAGAIN) {
		close(pipe_read_end);
		this->_request.status_code = InternalServerError;
		this->_request.setRequestState(MALFORMED);
	}
}



void	ProcessRequest::readCGIHeaders(char* buffer, ssize_t bytes_read) {
    this->_cgi_pipe_buffer.append(buffer, bytes_read);

	// parse headers and body separator
    size_t header_end = this->_cgi_pipe_buffer.find("\r\n\r\n");
    size_t separator_len = 4;
    if (header_end == String::npos) {
        header_end = this->_cgi_pipe_buffer.find("\n\n");
        separator_len = 2;
    }

	// headers not received fully
    if (header_end == String::npos) {
        return; 
    }

	// body parser
	// extract and process headers
	int seperator = (separator_len == 4) ? 2 : 1;
    this->_cgi_headers = this->_cgi_pipe_buffer.substr(0, header_end + seperator);
    String body = this->_cgi_pipe_buffer.substr(header_end + separator_len);

    if (!body.empty()) {
		std::ofstream tmp_file_stream(this->_cgi_body_file_name.c_str(),
								std::ios::binary | std::ios::app);
		if (!tmp_file_stream.is_open()) {
			this->_request.status_code = InternalServerError;
			this->_request.setRequestState(MALFORMED);
		}

		this->_client_connection.response.serve_file = true;

		tmp_file_stream.write(body.c_str(), body.length());
		tmp_file_stream.close();
	}

    this->_cgi_state = READING_BODY;
    this->_cgi_pipe_buffer.clear();
}



void	ProcessRequest::readCGIBody(char* buffer, ssize_t bytes_read) {
	this->_client_connection.response.serve_file = true;

	std::ofstream tmp_file_stream(this->_cgi_body_file_name.c_str(),
							      std::ios::binary | std::ios::app);
	if (!tmp_file_stream.is_open()) {
		this->_request.status_code = InternalServerError;
		this->_request.setRequestState(MALFORMED);
	}

	tmp_file_stream.write(buffer, bytes_read);
	tmp_file_stream.close();
}



bool	ProcessRequest::isCGISucceed(std::vector<pid_t>& cgis_to_reap) {
	pid_t pid = this->_client_connection.pid;
	int status;

	// use WNOHANG so that the main process wont hang if the cgi is still runing
	pid_t wait_res = waitpid(pid, &status, WNOHANG);

	if (wait_res > 0) {
		if (WIFEXITED(status)) {
			int exit_code = WEXITSTATUS(status);
			// cgi exited with an error
			if (exit_code != 0) {
				return false;
			}

			return true;
		}
		// cgi exited with a signal
		else if (WIFSIGNALED(status)) {
			return false;
		}

	} else if (wait_res == 0) {
		cgis_to_reap.push_back(pid);
		return true;
	}

	return false;
}



// -----------------------------------------------------------




void	ProcessRequest::splitURLFromQeury() {
	// strip the query string if it exists
	String clean_uri = this->_client_connection.request.target_resource;

	size_t query_pos = clean_uri.find('?');
	if (query_pos != String::npos) {
		this->_query_string = clean_uri.substr(query_pos + 1);
		clean_uri = clean_uri.substr(0, query_pos);
	}

	this->_script_name = clean_uri;
}



void	ProcessRequest::resolveFilePath() {
	String& clean_uri = this->_script_name;

	// ROOT directive gets priority
	if (!this->_request.location->shared_directives.root.empty()) {
		this->_file_path = this->_request.location->shared_directives.root + clean_uri;
	} 
	// ALIAS directive fallback
	else if (!this->_request.location->alias.empty()) {
		String uri_remainder;

		// Strip the matched location path from the URI
		if (clean_uri.find(this->_request.location->path) == 0) {
			uri_remainder = clean_uri.substr(this->_request.location->path.length());
		} else {
			uri_remainder = clean_uri; 
		}

		this->_file_path = this->_request.location->alias + uri_remainder;
	}

	// WARNING: this needs to be adjusted to force atleast one directive 
	// Default fallback
	else {
		this->_file_path = clean_uri;
	}
}



void    ProcessRequest::checkPotentialIndex() {
	struct stat info;
	if (stat(this->_file_path.c_str(), &info) != 0) {
		throw ProcessRequestException(InternalServerError);
	}

	if (!S_ISDIR(info.st_mode)) return;

	if (this->_file_path.empty()
		|| this->_file_path[this->_file_path.length() - 1] != '/') {
		this->_file_path += '/';

		if (this->_script_name[this->_script_name.length() - 1] != '/') {
			this->_script_name += '/';
		}
	}

	// iterate through the index arr until an index matches or none
	std::vector<String>& indexs = this->_request.location->shared_directives.index;
	for (size_t i = 0; i < indexs.size(); i++) {
		String potential_index_path = this->_file_path + indexs[i];

		struct stat index_info;
		if (stat(potential_index_path.c_str(), &index_info) != 0) continue;

		if (!S_ISREG(index_info.st_mode)) continue;

		this->_file_path = potential_index_path;
		this->_script_name = indexs[i];

		// update state if the new script is a cgi file
		size_t len = this->_file_path.length();
		if (len >= 3 && this->_file_path.substr(len - 3) == ".py") {
			if (this->_request.location->cgi_pass.find(".py")
				!= this->_request.location->cgi_pass.end()) {

				this->_is_cgi_request = true;
				this->_interpreter = ".py";
			}
		} else if (len >= 3 && this->_file_path.substr(len - 3) == ".js") {
			if (this->_request.location->cgi_pass.find(".js")
				!= this->_request.location->cgi_pass.end()) {

				this->_is_cgi_request = true;
				this->_interpreter = ".js";
			}
		}
		break;
	}
}




void	ProcessRequest::processCGIRequest() {
	std::vector<String> env;

	setPathEnvVariables(env);
	setHeadersEnvVariables(env);

	// create a pipe
	PIPE fds;
	if (!IsValidPipe(pipe(fds))) {
		throw ProcessRequestException(InternalServerError);
	}

	this->_client_connection.pid = fork();

	if (isChildProcess(this->_client_connection.pid)) {
		setUpChildProcess(fds, env);
	}

	else if (isParentProcess(this->_client_connection.pid)) {
		setUpParentProcess(fds);
		this->_request.setRequestState(MONITORE_PIPE);
	}


	// save the cgi body file name i case its used
	std::stringstream client_fd;
	client_fd << this->_client_connection.fd;
	this->_cgi_body_file_name = "./cgi-tmp-body/body_" + client_fd.str() + ".tmp";
}



void	ProcessRequest::setPathEnvVariables(std::vector<String>& env) {
	env.push_back("SCRIPT_NAME=" + this->_script_name);
	env.push_back("QUERY_STRING=" + this->_query_string);

	if (!this->_path_info.empty()) {
		env.push_back("PATH_INFO=" + this->_path_info);
	}

	// request method
	String request_method = "REQUEST_METHOD=";
	switch (this->_request.method) {
		case GET:
			request_method += "GET";
			break;

		case POST:
			request_method += "POST";
			break;

		case DELETE:
			request_method += "DELETE";
			break;

		default: break;
	}
	env.push_back(request_method);
}



void	ProcessRequest::setHeadersEnvVariables(std::vector<String>& env) {
	Headers::iterator header = this->_request.headers.begin();
	Headers::iterator headers_end = this->_request.headers.end();
	for (; header != headers_end; header++) {
		if (header->first == "content-length") {
			env.push_back("CONTENT_LENGTH=" + header->second);
		} else if (header->first == "content-type") {
			env.push_back("CONTENT_TYPE=" + header->second);
		} else {
			String header_name = header->first;

			// capitalize the header name
			std::transform(header_name.begin(), header_name.end(),
						   header_name.begin(), ::toupper);

			// replace hyphen with underscore
			size_t hyphen_pos = header_name.find('-');
			if (hyphen_pos != String::npos) {
				header_name[hyphen_pos] = '_';
			}

			// push the newly created env variable
			env.push_back("HTTP_" + header_name + '=' + header->second);
		}
	}
}



void	ProcessRequest::setUpChildProcess(PIPE& fds, std::vector<String>& env) {
	// if there is a body file dup the stdin
	if (!this->_request.tmp_body_file_name.empty()) {
		File body_file = open(this->_request.tmp_body_file_name.c_str(), O_RDONLY);
		if (body_file == -1 || dup2(body_file, STDIN_FILENO) == -1) {
			std::exit(EXIT_FAILURE);
		}
		close(body_file);
	}

	// dup the stdout
	if (dup2(fds[1], STDOUT_FILENO) == -1) {
		std::exit(EXIT_FAILURE);
	}

	close(fds[0]);
	close(fds[1]);

	// arguments
	String interpreter = this->_request.location->cgi_pass[this->_interpreter];
	char* args[3] = {
		const_cast<char*>(interpreter.c_str()),
		const_cast<char*>(this->_file_path.c_str()),
		NULL
	};

	// env variables
	char* env_variables[env.size() + 1];
	for (size_t i = 0; i < env.size(); i++) {
		env_variables[i] = const_cast<char*>(env[i].c_str());
	}
	env_variables[env.size()] = NULL;

	execve(interpreter.c_str(), args, env_variables);

	// fallback for execve
	std::exit(EXIT_FAILURE);
}



void	ProcessRequest::setUpParentProcess(PIPE& fds) {
	close(fds[1]);
	this->_client_connection.pipe_read_end = fds[0];
}



void	ProcessRequest::checkPotentialCGIRequest() {
	if (findScriptInterpreter(".py")) return;
	else if (findScriptInterpreter(".js")) return;
}

bool	ProcessRequest::findScriptInterpreter(const String& extention) {
	std::map<String, String>::iterator cgi_pass;
	cgi_pass = this->_request.location->cgi_pass.find(extention);

	// no interpreter was defined in the location for this extention
	if (cgi_pass == this->_request.location->cgi_pass.end()) return false;

	// while the extention is not part of a file/folder name
	size_t enxtention_pos = this->_script_name.find(extention);
	while (enxtention_pos != String::npos) {
		// check potential path info
		if (enxtention_pos + 3 == this->_script_name.length()
			|| this->_script_name[enxtention_pos + 3] == '/') {
			this->_interpreter = extention;

			// separate script name and path info
			if (enxtention_pos + 3 < this->_script_name.length()) {
				this->_path_info = this->_script_name.substr(enxtention_pos + 3);
				this->_script_name = this->_script_name.substr(0, enxtention_pos + 3);
			}

			this->_is_cgi_request = true;
			this->_client_connection.response.is_cgi_response = true;
			return true;
		}

		enxtention_pos = this->_script_name.find(extention, enxtention_pos + 1);
	}

	return false;
}




void	ProcessRequest::parseCGIHeaders() {
	std::istringstream header_stream(this->_cgi_headers);
	String line;

	this->_request.status_code = OK;

	while (std::getline(header_stream, line)) {
		if (line.empty()) {
			break;
		}

		if (!line.empty() && line[line.size() - 1] == '\r') {
			line.erase(line.size() - 1);
		}

		size_t colon_pos = line.find(':');
		if (colon_pos == String::npos) {
			continue;
		}

		String key = line.substr(0, colon_pos);
		String value = line.substr(colon_pos + 1);

		size_t start = key.find_first_not_of(" \t");
		size_t end = key.find_last_not_of(" \t");
		if (start != String::npos) {
			key = key.substr(start, end - start + 1);
		}

		start = value.find_first_not_of(" \t");
		end = value.find_last_not_of(" \t");
		if (start == String::npos) {
			value.clear();
		} else {
			value = value.substr(start, end - start + 1);
		}

		String lower_key = key;
		std::transform(lower_key.begin(), lower_key.end(),
					   lower_key.begin(), ::tolower);

		if (lower_key == "status") {
			String status_value = value;
			size_t space_pos = status_value.find_first_of(" \t");
			if (space_pos != String::npos) {
				status_value = status_value.substr(0, space_pos);
			}

			if (!status_value.empty()) {
				char* end_ptr = NULL;
				long parsed_status = std::strtol(status_value.c_str(), &end_ptr, 10);
				if (end_ptr != status_value.c_str()) {
					switch (parsed_status) {
						case 100: this->_request.status_code = Continue; break;
						case 101: this->_request.status_code = SwitchingProtocols; break;
						case 200: this->_request.status_code = OK; break;
						case 201: this->_request.status_code = Created; break;
						case 202: this->_request.status_code = Accepted; break;
						case 204: this->_request.status_code = NoContent; break;
						case 206: this->_request.status_code = PartialContent; break;
						case 301: this->_request.status_code = MovedPermanently; break;
						case 302: this->_request.status_code = Found; break;
						case 303: this->_request.status_code = SeeOther; break;
						case 304: this->_request.status_code = NotModified; break;
						case 307: this->_request.status_code = TemporaryRedirect; break;
						case 308: this->_request.status_code = PermanentRedirect; break;
						case 400: this->_request.status_code = BadRequest; break;
						case 401: this->_request.status_code = Unauthorized; break;
						case 403: this->_request.status_code = Forbidden; break;
						case 404: this->_request.status_code = NotFound; break;
						case 405: this->_request.status_code = MethodNotAllowed; break;
						case 408: this->_request.status_code = RequestTimeout; break;
						case 409: this->_request.status_code = Conflict; break;
						case 410: this->_request.status_code = Gone; break;
						case 411: this->_request.status_code = LengthRequired; break;
						case 413: this->_request.status_code = PayloadTooLarge; break;
						case 414: this->_request.status_code = URITooLong; break;
						case 415: this->_request.status_code = UnsupportedMediaType; break;
						case 417: this->_request.status_code = ExpectationFailed; break;
						case 426: this->_request.status_code = UpgradeRequired; break;
						case 500: this->_request.status_code = InternalServerError; break;
						case 501: this->_request.status_code = NotImplemented; break;
						case 502: this->_request.status_code = BadGateway; break;
						case 503: this->_request.status_code = ServiceUnavailable; break;
						case 504: this->_request.status_code = GatewayTimeout; break;
						case 505: this->_request.status_code = HTTPVersionNotSupported; break;
						default: this->_request.status_code = InternalServerError; break;
					}
				}
			}
			continue;
		}

		if (lower_key == "content-type") {
			this->_client_connection.response.appendHeaders("Content-Type", value);
			continue;
		}

		if (lower_key == "content-length") {
			this->_client_connection.response.appendHeaders("Content-Length", value);
			continue;
		}
	}


}



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
		if (this->_request.location->shared_directives.autoindex) {
			renderDirectoryListing();
			this->_request.status_code = OK;
			this->_request.setRequestState(COMPLETE);
			return;
		}

		throw ProcessRequestException(Forbidden);
	}

	// file is a regular file, send it
	else {
		if (!this->_client_connection.response.openFileToSend(this->_file_path)) {
			throw ProcessRequestException(InternalServerError);
		}
		this->_client_connection.response.serve_file  = true;
		this->_client_connection.request.status_code = OK;
		this->_request.setRequestState(COMPLETE);
	}
}

void	ProcessRequest::renderDirectoryListing() {
	DIR* directory = opendir(this->_file_path.c_str());
	if (directory == NULL) {
		throw ProcessRequestException(InternalServerError);
	}

	// force absolute path in the href tag for each inner file
	String base_path = this->_script_name;
	if (base_path.empty() || base_path[base_path.length() - 1] != '/') {
		base_path += '/';
	}

	String html;
	html += "<!DOCTYPE html>\r\n";
	html += "<html><head><title>Directory listing</title></head><body>\r\n";
	html += "<h1>Index of " + base_path + "</h1>\r\n";
	html += "<ul>\r\n";

	struct dirent* entry;
	while ((entry = readdir(directory)) != NULL) {
		String name = entry->d_name;
		if (name == "." || name == "..") {
			continue;
		}

		html += "<li><a href=\"" + base_path + name + "\">" + name + "</a></li>\r\n";
	}
	closedir(directory);

	html += "</ul>\r\n";
	html += "</body></html>\r\n";

	// set the response headers for the directory listing
	std::stringstream size;
	size << html.size();
	this->_client_connection.response.appendHeaders("Content-Length", size.str());
	this->_client_connection.response.appendHeaders("Content-Type", "text/html");
	this->_client_connection.response.appendCTLF();

	this->_client_connection.response.appendDirectoryListeningBody(html);
}


// -----------------------------------------------------------


