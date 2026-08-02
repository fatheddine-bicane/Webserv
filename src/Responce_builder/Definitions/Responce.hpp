#include "../../Core_modules/Typedef.hpp"
#include "../../Config_parser/Parser/Definitions/Directives.hpp"
#include "../../Core_modules/HTTPStatus.hpp"


class Responce {
private:
	String	_headers;


public:
	// INFO: constructor
	Responce() {}

	// INFO: api
	void	initialHeaders(HTTPStatus HTTP_status);
	void	appendHeaders(const String& key, const String& value, bool last_header=false);
};
