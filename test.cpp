/*
 * test.cpp
 *
 *  Created on: 8 juin 2021
 *      Author: ber
 */

#include <cstdio>

#include "ipaddress.h"



int main(int argc, char* argv[], char **env) {
	char buffer[256];
	char buffer2[256];

	try {
		ipAddress_c* ip = new ipAddress_c((const char*)"192.168.3.36");
		printf((const char*)"%llx\n", ip->hash64());
		printf("%s\n", ip->toString(buffer));
		delete ip;

		ip = new ipAddress_c((const char*)"192.168.3.36/17");
		printf((const char*)"%llx\n", ip->hash64());
		printf("%s\n", ip->toString(buffer));
		delete ip;

		ip = new ipAddress_c((const uint8_t*)(const unsigned char[]){172,17,36,37}, 4);
		ip->setPrefixLen(16);
		printf((const char*)"%llx\n", ip->hash64());
		printf("%s\n", ip->toString(buffer));


		ipAddress_c* ip2 = new ipAddress_c((const uint8_t*)(const unsigned char[]){172,17,12,34}, 4);
		ip2->setPrefixLen(24);
		printf((const char*)"%llx\n", ip2->hash64());
		printf("%s\n", ip2->toString(buffer));

		if (ip2->isIncluded(ip)) {
			printf("%s isIncluded in %s\n", ip2->toString(buffer), ip->toString(buffer2));

		} else {
			printf("%s not included in %s\n", ip2->toString(buffer), ip->toString(buffer2));
		}

		delete ip;
		delete ip2;



	}
	catch (ipAddress_c::error const& e) {
		printf("Exception ipAddress::error = %s\n", e.what());
	}
}
