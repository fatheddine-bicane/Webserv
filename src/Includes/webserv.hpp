#pragma once


#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>
#include <sys/epoll.h>

#include "../config_parser/Scanner/Definitions/Scanner.hpp"
#include "../Server_socket_multiplexing/Exceptions/SystemCallsException.hpp"
#include "../config_parser/Includes/ParserExceptions.hpp"
#include "../config_parser/Parser/Definitions/Parser.hpp"
#include "../config_parser/Scanner/Definitions/Token.hpp"
#include "../Server_socket_multiplexing/Definitions/ServerSocketsMultiplexing.hpp"
#include "ServerConnection.hpp"
#include "ClientConnection.hpp"
