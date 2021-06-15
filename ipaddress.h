/*
 * ipaddress.h
 *
 *  Created on: 6 juin 2021
 *      Author: ber
 */

#ifndef IPADDRESS_H_
#define IPADDRESS_H_

#include <cstdint>
#include <cstdlib>
#include <exception>




class ipAddress_c {


public:
	ipAddress_c(uint32_t);          /* v4, d'après uint32 network order */
	ipAddress_c(const uint8_t*, size_t);  /* v4 ou v6, d'après pointeur vers adresse en network order */
	ipAddress_c(const char*);             /* v4 ou v6, d'après chaine de caractère notations habituelles */

	void setPrefixLen(uint8_t);     /* Si n'a pas été fixé par le constructeur */

	uint8_t getVersion();

	char*   toString(char*);
	char*   reverseDnsPtr(char*);

	uint8_t getPrefixLen();
	bool    isHost();
	bool	isMulicast();   /* RFC 3171 et RFC2373 */
	bool	isGlobal();
	bool	isLinkLocal();  /* 169.254/16 et FE80 */
	bool	isPrivate();    /* RFC1918 et 100.64 */
	bool	isLoopback();   /* 127/8 et ::1/128 */
	bool	isV6Tunnel();   /* 6to4, teredo ... */

	class   error;

	static constexpr uint8_t scopeUnspecified =0;
	static constexpr uint8_t scopeLinkLocal   =1; /* plus sympa qu'un enum en namespace global non ? */
	static constexpr uint8_t scopeSite        =2;
	static constexpr uint8_t scopeGlobal      =3;
	static constexpr uint8_t scopeLoopback    =4;

	uint8_t getScope();

	uint64_t hash64();      /* renvoie un haché convenable pour des structures genre Bloom ou HyperLogLog */

	bool    isIncluded(ipAddress_c*); /* indique si est inclus dans l'autre adresse fournie en paramètre (subnets bien sûr) */

private:
	union {
		uint8_t  bytes[16]; /* tjs représenté comme IPv6 network order */
		uint32_t words[4];  /* utilisé dans la fonction de hash */
		uint8_t  ipv6[16];
		struct {
			uint8_t   padding64[12];
			uint32_t  ipv4;
		};
	};

	uint64_t hash;      /* haché 64 bits */
	bool     hashOk;
	uint8_t  scope;     /* si unspecified, getScope() essaie de le redéterminer mieux (espèce de cache) */
	uint8_t  prefixLen; /* 0..32 ou 0..128 */
	uint8_t  version;

};


class ipAddress_c::error : public std::exception {
public:
	error(int value);
	~error();
	int getValue();
	/*   0 = no error
	 *   1 = paramètre incorrect
	 *   2 = objet pas encore totalement initialisé
	 */

	const char* what() const noexcept;

private:
	int value;
};



#endif /* IPADDRESS_H_ */
