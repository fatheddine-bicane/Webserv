#pragma once

#include <list>
#include <map>
#include <set>
#include <utility>
#include <vector>

#include "../../../Core_modules/Typedef.hpp"


/**
* @brief config file shared directives.
*/
class SharedDirectives {
public:
	String								root; // default: html
	std::map<int, String>				error_page;
	unsigned long						client_max_body_size; // default: 1m
	String								client_body_temp_path;
	bool								autoindex; // defaule: false
	std::vector<String>					index; // defaule: index, index.html;
	std::set<String>					dav_methods; // default: off--deny all methods

public:
	/**
	 * @brief Initializes the HTTP block directive context with default values.
	 */
	SharedDirectives();

	SharedDirectives &operator=(const SharedDirectives &other);

	/**
	 * @brief Initializes the directives from an inherited context.
	 *
	 * The server will inherit from the HTTP block, and each location
	 * will inherit from the server block it is nested within.
	 *
	 * @param other The inherited directives to copy from.
	 */
	SharedDirectives(const SharedDirectives &other);
};

class Location
{
public:
	String									path;
	SharedDirectives						shared_directives;
	String									alias;
	std::set<String>						limit_except;
	std::pair<int, String>					return_d;
	std::map<String, String>				cgi_pass;

public:
	Location() {}
	Location(const SharedDirectives& inherited_directives);
};

class Server {
public:
	SharedDirectives		shared_directives;
	std::list<Location>		locations;

public:
	Server() {}
	Server(const SharedDirectives& inherited_directives);
};
