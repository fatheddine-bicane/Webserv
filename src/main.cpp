#include "Core_modules/Webserv/Definitions/Webserv.hpp"
#include "HTTP_request_parser/Definitions/Request.hpp"
#include "Process_request/Definitions/ProcessRequest.hpp"
#include "./HTTP_response_processer/Exceptions/ResponseExceptions.hpp"


int main(int argc, char** argv) {
	Webserv	webserv;

	try {
		// initialise scanner
		Scanner scanner(argc, argv);
		std::vector<Token> tokens = scanner.scanTokens();

		// initialise parser
		Parser parser(tokens, *scanner.source);
		parser.scanTokens();
		webserv.setServers(parser.getServers());

		// Initialize EpollMultiplexer
		ServerMultiplexing multiplexer = ServerMultiplexing(parser.getAddresses(), webserv.epfd);
		multiplexer.bootstrapServerListeners();
		webserv.setSocketsMap(multiplexer.getSocketsMap());
	} catch (ParserException& e) {
		std::cout << e.what() << std::endl;
		return 2;
	} catch (SystemCallsFailedException& e) {
		std::cout << e.what() << std::endl;
		return 3;
	} catch (std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return 4;
	}

	while (true) {
		webserv.getReadySockets();

		for (int index = 0; index < webserv.events_size; index++) {
			Connection*	connection = webserv.getConnectionObject(index);

			if (connection->type == SERVER_S) {
				try {
					webserv.addNewClientConnection(connection);
				} catch (ConnectionException& e) {
					webserv.error_log << e.what() << std::endl;
				}
			}

			else if (connection->type == CLIENT_S) {
				ClientConnection* client_connection = dynamic_cast<ClientConnection*>(connection);

				// serve request if ready or malformed
				if (client_connection->request.isRequestState(READY_TO_SERVE)) {
					try {

						client_connection->response.sendResponse();
						if (client_connection->response.is_served) {
							// clear connection
                            webserv.removeClient(client_connection);
						}

					} catch (ClientSocketErrorException& e) {
						std::cerr << e.what() << '\n';
						return 3;
					}
				}


				// cgi pipe reading/sending
				else if (client_connection->request.isRequestState(CGI)) {
					// handle cgi pipe reading here
					continue;
				}

				// else keep on parsing the incoming request
				else {
					client_connection->request.attemptRequestParse();

					// request still incoming
					if (client_connection->request.isRequestState(INCOMPLETE)) {
						continue;
					}

					// else processes request
                    ProcessRequest process_request(client_connection->request,
                                                   *client_connection->server,
												   *client_connection,
												   webserv.epfd);

                    process_request.processRequest();

					if (client_connection->request.isRequestState(READY_TO_SERVE)) {

						// monitor the socker for output
						try {
							client_connection->monitorSockerForOutput(webserv.epfd);
						} catch (SystemCallsFailedException& e) {
							std::cerr << e.what() << '\n';
							return 3;
						}

						// initialize the response object
						client_connection->response.initializeResponseObject();
					}
				} // else keep parsing request

			} // if client connection
		} // for each ready socket
	} //while true

	return 0;
}
