#include "../Definitions/ProcessRequest.hpp"
#include "../Exceptions/ProcessRequestException.hpp"
#include <cstddef>
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <list>
#include <sys/stat.h>
#include <vector>


// INFO: constructor
// -----------------------------------------------------------

ProcessRequest::ProcessRequest(Request& request, Server& server)
	: _request(request),
	  _server(server) {
	this->_location = NULL;
}

// -----------------------------------------------------------




// INFO: api
// -----------------------------------------------------------

void	ProcessRequest::processRequest() {
	if (this->_request.isRequestState(MALFORMED)) {
		// handle error
	}
	
	else {
		try {
			switch (this->_request.method) {
				case GET:
					// handle get
					break;

				case POST:
					// handle post
					findLocationBlock();
					processPostRequest();
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
			std::cout << "ProcessRequestException: " << e.what() << std::endl;
		}
	}
}

// -----------------------------------------------------------



void	ProcessRequest::findLocationBlock() {
	std::list<Location>::iterator it = this->_server.locations.begin();
	std::list<Location>::iterator end = this->_server.locations.end();

	Location*	default_path_location = NULL;

	int matched_char = 0;
	for (; it != end; it++) {
		if (this->_request.target_resource == it->path) {
			this->_location = &(*it);
			return;
		}

		// match the longest uri
		else {
			if (it->path == "/") {
				default_path_location = &(*it);
			}

			if (this->_request.target_resource.find(it->path) == 0) {

				int current_path_length = it->path.length();

				if (current_path_length > matched_char) {
					matched_char = current_path_length;
					this->_location = &(*it);
				}
			}
		}
	}

	if (matched_char == 0 && this->_location == NULL) {
		if (default_path_location) {
			this->_location = default_path_location;
		}

		else {
			throw ProcessRequestException(NotFound);
		}
	}
}

static String getMethodName(HTTPMethod method) {
	switch (method) {
		case GET: return "GET";
		case POST: return "POST";
		case DELETE: return "DELETE";
		case PUT: return "PUT";
		default: return "";
	}
}

bool isMultiPartFromData(String header){
	return header.find("multipart/form-data") != String::npos;
}

static String sanitizeFilename(const String& filename) {
	if (filename.empty()) {
		return "";
	}

	String::size_type slashPos = filename.find_last_of("/\\");
	if (slashPos == String::npos) {
		return filename;
	}

	if (slashPos + 1 >= filename.length()) {
		return "";
	}

	return filename.substr(slashPos + 1);
}

String getBoundary(const String& header) {
	size_t pos = header.find("boundary=");
	if (pos == String::npos) return "";
	return header.substr(pos + 9);
}

void handleMultipartUpload(const String& tmpFileName, const String& header, const String& uploadDir) {
	String boundary = getBoundary(header);
	if (boundary.empty()) throw ProcessRequestException(BadRequest);

	String startBoundary = "--" + boundary;

	std::ifstream inFile(tmpFileName.c_str(), std::ios::binary);
	if (!inFile) throw ProcessRequestException(InternalServerError);

	std::vector<char> buffer((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
	inFile.close();

	std::vector<char>::iterator it = buffer.begin();
	std::vector<char>::iterator end = buffer.end();

	std::vector<char>::iterator boundaryPos = std::search(it, end, startBoundary.begin(), startBoundary.end());
	if (boundaryPos == end) throw ProcessRequestException(BadRequest);

	std::string bufferStr(boundaryPos, end);
	size_t fileOptPos = bufferStr.find("filename=\"");
	if (fileOptPos == std::string::npos) throw ProcessRequestException(BadRequest);

	size_t filenameStart = fileOptPos + 10;
	size_t filenameEnd = bufferStr.find("\"", filenameStart);
	String filename = bufferStr.substr(filenameStart, filenameEnd - filenameStart);
	filename = sanitizeFilename(filename);
	if (filename.empty()) throw ProcessRequestException(BadRequest);

	size_t dCrlfPos = bufferStr.find("\r\n\r\n", fileOptPos);
	if (dCrlfPos == std::string::npos) throw ProcessRequestException(BadRequest);

	std::vector<char>::iterator fileDataStart = boundaryPos + dCrlfPos + 4;

	std::vector<char>::iterator fileDataEnd = std::search(fileDataStart, end, startBoundary.begin(), startBoundary.end());
	if (fileDataEnd == end) throw ProcessRequestException(BadRequest);

	if (fileDataEnd - fileDataStart >= 2) {
		fileDataEnd -= 2;
	}

	String finalFilePath = uploadDir + "/" + filename;
	std::ofstream outFile(finalFilePath.c_str(), std::ios::binary);
	if (!outFile) throw ProcessRequestException(InternalServerError);

	if (fileDataStart < fileDataEnd) {
		outFile.write(&*fileDataStart, std::distance(fileDataStart, fileDataEnd));
	}
	outFile.close();
}

static void removeTmpBodyFile(const String& tmpFileName) {
	if (!tmpFileName.empty()) {
		remove(tmpFileName.c_str());
	}
}

void ProcessRequest::processPostRequest(){
	// 1 check if POST is allowed in _location
	if (!this->_location->limit_except.empty()) {
		String methodName = getMethodName(this->_request.method);
		if (this->_location->limit_except.find(methodName) == this->_location->limit_except.end()) {
			throw ProcessRequestException(MethodNotAllowed);
		}
	}
	// 2 check if location is for CGI
	// if cgi ...
	// else

	// 3 Handle as upload
	String root = this->_location->shared_directives.root;
	if (!root.empty() && root[root.length() - 1] == '/') {
		root.erase(root.length() - 1);
	}

	Headers::const_iterator content_type = this->_request.headers.find("content-type");
	const bool is_multipart = (content_type != this->_request.headers.end()
		&& isMultiPartFromData(content_type->second));

	if (is_multipart) {
		String upload_dir = "./nginx" + root + this->_request.target_resource;
		try {
			handleMultipartUpload(this->_request.tmp_body_file_name, content_type->second, upload_dir);
			removeTmpBodyFile(this->_request.tmp_body_file_name);
		} catch (...) {
			removeTmpBodyFile(this->_request.tmp_body_file_name);
			throw;
		}
	}
	else {
		String filePath = root + this->_request.target_resource;

		std::ifstream inFile(this->_request.tmp_body_file_name.c_str(), std::ios::binary);
		if (!inFile) {
			throw ProcessRequestException(InternalServerError);
		}

		std::ofstream outFile(("./nginx/" + filePath).c_str(), std::ios::binary);
		if (!outFile) {
			inFile.close();
			throw ProcessRequestException(InternalServerError);
		}

		outFile << inFile.rdbuf();

		inFile.close();
		outFile.close();
		removeTmpBodyFile(this->_request.tmp_body_file_name);
	}

}
