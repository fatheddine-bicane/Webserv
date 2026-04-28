#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <iostream>
#include <stdlib.h>


int	main() {
	struct ifaddrs* addresses;
	if (getifaddrs(&addresses) == -1) {
		std::cerr << "Error: getifaddrs() failed\n";
	}

	struct ifaddrs* address = addresses;
	while (address) {
		int	family = address->ifa_addr->sa_family;
		if (family == AF_INET || family == AF_INET6) {
			std::cout << "Address name: ";
			std::cout << address->ifa_name << "  |  ";
			std::cout << "IP version: ";
			std::cout << (family == AF_INET ? "IPv4" : "IPv6");

			char ip[100];
			const int	family_size = family == AF_INET ?
				sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
			getnameinfo(address->ifa_addr, family_size, ip, sizeof(ip), 0, 0, NI_NUMERICHOST);
			std::cout << "  |  -> " << ip;

			std::cout << "\n---------------------------------\n";
		}

		address = address->ifa_next;
	}

	free(addresses);
}
