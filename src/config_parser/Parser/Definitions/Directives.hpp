#pragma once

#include <list>
#include <set>
#include <string>
#include <utility>
#include <vector>

typedef std::string String;


/**
* @brief config file shared directives.
*/
class SharedDirectives {
public:
	String								root; // default: html
	std::pair<std::set<int>, String>	error_page;
	unsigned long						client_max_body_size; // default: 1m
	String								client_body_temp_path;
	bool								autoindex; // defaule: false
	std::vector<String>					index; // defaule: index, index.html;
	std::set<String>					dav_methods; // default: off--deny all methods
	bool								create_full_put_path; // default: false

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
	SharedDirectives						shared_directives;
	String									alias;
	std::set<String>						limit_except;
	std::pair<int, String>					return_d;
	std::vector<std::pair<String, String> >	cgi_pass;

public:
	Location() {}
	Location(const SharedDirectives& inherited_directives);
};

class Server {
public:
	SharedDirectives		shared_directives;
	std::list<Location>		locations;
	std::pair<int, String>	return_d;

public:
	Server() {}
	Server(const SharedDirectives& inherited_directives);
};
