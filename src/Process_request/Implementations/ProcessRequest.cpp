#include "../Definitions/ProcessRequest.hpp"
#include "../Exceptions/ProcessRequestException.hpp"
#include <cstddef>
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <list>
#include <sys/stat.h>
#include <sys/types.h>
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

    std::ifstream inFile(tmpFileName.c_str(), std::ios::binary);
    if (!inFile.is_open()) throw ProcessRequestException(InternalServerError);

    String line;
    String filename;
    
    // 1. Read headers line-by-line until filename is found and \r\n\r\n is reached
    while (std::getline(inFile, line)) {
        if (!line.empty() && line[line.size() - 1] == '\r') {
            line.erase(line.size() - 1);
        }
        
        if (line.find("filename=\"") != String::npos) {
            size_t start = line.find("filename=\"") + 10;
            size_t end = line.find("\"", start);
            if (start < end) {
                filename = sanitizeFilename(line.substr(start, end - start));
            }
        }
        
        // Blank line marks the end of headers and start of raw binary file data
        if (line.empty()) {
            break;
        }
    }

    if (filename.empty()) {
        inFile.close();
        throw ProcessRequestException(BadRequest);
    }

    // 2. Open output file destination
    String finalFilePath = uploadDir + "/" + filename;
    std::ofstream outFile(finalFilePath.c_str(), std::ios::binary);
    if (!outFile.is_open()) {
        inFile.close();
        throw ProcessRequestException(InternalServerError);
    }

    // 3. Stream body using constant 8KB memory window
    const size_t BUFFER_SIZE = 8192;
    char buffer[BUFFER_SIZE];
    
    String boundaryMarker = "\r\n--" + boundary;
    String slidingWindow;

    while (inFile.read(buffer, BUFFER_SIZE) || inFile.gcount() > 0) {
        size_t bytesRead = inFile.gcount();
        slidingWindow.append(buffer, bytesRead);

        // Find boundary position inside sliding window
        size_t boundaryPos = slidingWindow.find(boundaryMarker);
        if (boundaryPos != String::npos) {
            outFile.write(slidingWindow.data(), boundaryPos);
            break;
        }

        // Keep safe margin to prevent splitting the boundary across buffer chunks
        if (slidingWindow.size() > boundaryMarker.size()) {
            size_t safeWriteSize = slidingWindow.size() - boundaryMarker.size();
            outFile.write(slidingWindow.data(), safeWriteSize);
            slidingWindow.erase(0, safeWriteSize);
        }
    }

    outFile.close();
    inFile.close();
}

static void removeTmpBodyFile(const String& tmpFileName) {
	if (!tmpFileName.empty()) {
		remove(tmpFileName.c_str());
	}
}

static bool isDirectory(const String& path) {
	struct stat pathInfo;
	if (stat(path.c_str(), &pathInfo) != 0) {
		return false;
	}

	return S_ISDIR(pathInfo.st_mode);
}

static bool ensureDirectoryExists(const String& path) {
	if (path.empty() || isDirectory(path)) {
		return true;
	}

	size_t separator = path.find_last_of('/');
	if (separator != String::npos) {
		String parent = path.substr(0, separator);
		if (!parent.empty() && !ensureDirectoryExists(parent)) {
			return false;
		}
	}

	if (mkdir(path.c_str(), 0755) == 0) {
		return true;
	}

	return isDirectory(path);
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
		if (!ensureDirectoryExists(upload_dir)) {
			throw ProcessRequestException(InternalServerError);
		}

		try {
			handleMultipartUpload(this->_request.tmp_body_file_name, content_type->second, upload_dir);
			removeTmpBodyFile(this->_request.tmp_body_file_name);
			this->_request.status_code = Created;
		} catch (...) {
			removeTmpBodyFile(this->_request.tmp_body_file_name);
			throw;
		}
	}
	else {
		String filePath = root + this->_request.target_resource;
		String outputPath = "./nginx/" + filePath;
		size_t parentSeparator = outputPath.find_last_of('/');
		if (parentSeparator != String::npos) {
			String parentDir = outputPath.substr(0, parentSeparator);
			if (!ensureDirectoryExists(parentDir)) {
				throw ProcessRequestException(InternalServerError);
			}
		}

		std::ifstream inFile(this->_request.tmp_body_file_name.c_str(), std::ios::binary);
		if (!inFile) {
			throw ProcessRequestException(InternalServerError);
		}

		std::ofstream outFile(outputPath.c_str(), std::ios::binary);
		if (!outFile) {
			inFile.close();
			throw ProcessRequestException(InternalServerError);
		}

		outFile << inFile.rdbuf();

		inFile.close();
		outFile.close();
		removeTmpBodyFile(this->_request.tmp_body_file_name);
		this->_request.status_code = Created;
	}

}
