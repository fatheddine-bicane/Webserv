#pragma once


enum HTTPStatus {
	// 1xx: Informational
	Continue = 100,
	SwitchingProtocols = 101,

	// 2xx: Successful
	OK = 200,
	Created = 201,
	Accepted = 202,
	NoContent = 204,
	PartialContent = 206,

	// 3xx: Redirection
	MovedPermanently = 301,
	Found = 302,
	SeeOther = 303,
	NotModified = 304,
	TemporaryRedirect = 307,
	PermanentRedirect = 308,

	// 4xx: Client Error
	BadRequest = 400,
	Unauthorized = 401,
	Forbidden = 403,
	NotFound = 404,
	MethodNotAllowed = 405,
	RequestTimeout = 408,
	Conflict = 409,
	Gone = 410,
	LengthRequired = 411,
	PayloadTooLarge = 413,
	URITooLong = 414,
	UnsupportedMediaType = 415,
	ExpectationFailed = 417,
	UpgradeRequired = 426,

	// 5xx: Server Error
	InternalServerError = 500,
	NotImplemented = 501,
	BadGateway = 502,
	ServiceUnavailable = 503,
	GatewayTimeout = 504,
	HTTPVersionNotSupported = 505
};
