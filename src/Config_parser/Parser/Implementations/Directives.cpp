#include "../Definitions/Directives.hpp"


// INFO: SharedDirectives
// ---------------------------------------------------
SharedDirectives::SharedDirectives() {
	// root directive
	this->root = "./html/";

	// error_page empty set means default value

	// client_max_body_size directive
	this->client_max_body_size = 1000000;

	// client_body_temp_path directive
	// TODO: assign the default path
	this->client_body_temp_path = "./default-path";

	// autoindex directive
	this->autoindex = false;

	// index directive
	this->index.push_back("index");
	this->index.push_back("index.html");

	// dav_methods empty set means the default value

	// this->create_full_put_path = false;
}

SharedDirectives&	SharedDirectives::operator=(const SharedDirectives& other) {
	if (this == &other) {
		return *this;
	}

	this->root = other.root;
	this->error_page = other.error_page;
	this->client_max_body_size = other.client_max_body_size;
	this->client_body_temp_path = other.client_body_temp_path;
	this->autoindex = other.autoindex;
	this->index = other.index;
	this->dav_methods = other.dav_methods;
	// this->create_full_put_path = other.create_full_put_path;

	return *this;
}

SharedDirectives::SharedDirectives(const SharedDirectives& other) {
	*this = other;
}

// ---------------------------------------------------


// INFO: Location
// ---------------------------------------------------
Location::Location(const SharedDirectives& inherited_directives)
	: shared_directives(inherited_directives) {}
// ---------------------------------------------------


// INFO: Server
// ---------------------------------------------------
Server::Server(const SharedDirectives& inherited_directives)
	: shared_directives(inherited_directives) {}
// ---------------------------------------------------
