#pragma once

#include <map>
#include <string>
#include <vector>

class Server;

typedef int SOCKET; // socket file discriptor
typedef int EP_INSTANCE; // epoll instance file discriptor
typedef std::string String;
// map to index the server block info
typedef  std::map<String, std::map<String, Server> > Servers;
// map to index the SOCKET IP-PORT combination
typedef std::map<SOCKET, String> SocketsMap;
// sockets addresses
typedef std::vector<std::pair<String, String> > Addresses;
typedef std::map<String, String> Headers; // http headers

#define IsValidSocket(s) ((s) >= 0)
#define CloseSocket(s) (close(s))
#define SocketAdded(s) ((s) == 0)
