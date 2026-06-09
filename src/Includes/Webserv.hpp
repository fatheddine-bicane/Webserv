#pragma once

#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "../config_parser/Scanner/Definitions/Scanner.hpp"
#include "../Server_socket_multiplexing/Definitions/ServerMultiplexing.hpp"
#include "../config_parser/Parser/Definitions/Parser.hpp"
#include "../config_parser/Scanner/Definitions/Token.hpp"
#include "../Server_socket_multiplexing/Definitions/ServerMultiplexing.hpp"
#include "Connection.hpp"
#include "ServerConnection.hpp"
#include "ClientConnection.hpp"

typedef int EP_INSTANCE;
