#include "Core_modules/Webserv/Definitions/Webserv.hpp"


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

				client_connection->request.attemptRequestParse();

				if (client_connection->request.isRequestState(COMPLETE)) {
					// process request
				}

				if (client_connection->request.isRequestState(MALFORMED)) {
					// if client is still connected serv erorr code page
					// if client droped connection close socket connection
				}

				// request still incoming
				if (!client_connection->request.isRequestState(COMPLETE)) {
					continue;
				}



				// processes request

				// serve request

			} // if client connection
		} // for each ready socket
	} //while true

	return 0;
}
