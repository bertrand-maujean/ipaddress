/*
 * ipaddress.cpp
 *
 *  Created on: 6 juin 2021
 *      Author: ber
 */


/* syscall */


/* libc */
//#include <arpa/inet.h>   /* ntohs() et compagnie */
//#include <netinet/in.h>  /* structure entête L3 */
//#include <netinet/ip.h>
//#include <netinet/ip6.h>
#include <cassert>

/* lib C++ */
//#include <vector>
#include <cstdlib>
#include <cstdint>
#include <cstdio> /* sscanf() */
//#include <cerrno>
#include <cstring>

/* libs spécifiques */

/* Local */
#include "ipaddress.h"


/********************************************************
 * implémentation de la classe ipAddress_c:error
 */
ipAddress_c::error::error(int value_) {
	value = value_;
}

ipAddress_c::error::~error() {

}

int ipAddress_c::error::getValue() {
	/*   0 = no error
	 *   1 = paramètre incorrect
	 *   2 = objet pas encore totalement initialisé
	 *   3 = erreur inconnue
	 */
	return value;
}

const char* ipAddress_c::error::what() const noexcept {
	static char msg[][32] = {"Pas d'erreur", "paramètre incorrect", "objet pas initialisé", "erreur inconnue"};
	if ((value<0)||(value>3)) {
		return msg[3];
	}
	return msg[value];
}




/********************************************************
 * implémentation de la classe ipAddress_c
 */

ipAddress_c::ipAddress_c(uint32_t ipv4_) { /* v4, d'après uint32 network order */
	scope       = scopeUnspecified;
	version     = 4;
	memset(bytes, 0, 16);
	ipv4        = ipv4_;
	prefixLen   = 32;
	hashOk      = false;
	hash        = 0;
}
ipAddress_c::ipAddress_c(const uint8_t* addr, size_t len) { /* v4 ou v6, d'après pointeur vers adresse en network order */
	scope       = scopeUnspecified;
	memset(bytes, 0, 16);
	hashOk      = false;
	hash        = 0;

	if (len == 4) {
		version     = 4;
		prefixLen   = 32;
		ipv4 = *(uint32_t*)addr;
	} else if (len == 16) {
		version     = 4;
		prefixLen   = 32;
		memcpy(bytes, addr, 16);
	} else {
		throw error(1); /* paramètre incorrect */
	}
}
ipAddress_c::ipAddress_c(const char* str) { /* v4 ou v6, d'après chaine de caractère notations habituelles, gère cidr /xx  */ // @suppress("Class members should be properly initialized")
	scope       = scopeUnspecified;
	memset(bytes, 0, 16);
	hashOk      = false;
	hash        = 0;
	prefixLen   = 253;

	/* petite stat des caractères présents pour détecter v4/v6 et si CIDR */
	size_t nbCars[5] = {0,0,0,0,0}; /* 0->chiffres, 1->a..f, 2->:, 3->., 4->/ */
	for (size_t n=0; str[n]; n++) {
		if ((str[n] >= '0') && (str[n]<='9')) {
			nbCars[0]++;
		} else if ((str[n] >= 'a') && (str[n]<='f')) {
			nbCars[1]++;
		} else if ((str[n] >= 'A') && (str[n]<='F')) {
			nbCars[1]++;
		} else if (str[n] == ':') {
			nbCars[2]++;
		} else if (str[n] == '.') {
			nbCars[3]++;
		} else if (str[n] == '/') {
			nbCars[4]++;
		}
	}

	bool avecPrefixLen;
	if (nbCars[4] >1) {
		/* param incorrect */
		throw error(1);
	} else if (nbCars[4] == 1) {
		/* Prefixlen présent */
		char* s=(char*)str;
		while ((*s)&&(*s!='/')) s++;
		if (*s == 0) throw error(1);
		while ((*s)&&((*s<'0')||(*s>'9'))) s++; /* saute les caractères non chiffre au début */
		if (*s == 0) throw error(1);
		sscanf(s, "%d", &prefixLen);
		avecPrefixLen = true;
	} else {
		avecPrefixLen = false;
	}

	if (nbCars[3] == 3) {
		/* Cas IPv4 */
		version=4;
		if (!avecPrefixLen) {
			prefixLen=32;
		} else {
			if ((prefixLen<0)||(prefixLen>32)) throw error(1); /* param incorrect */
		}
		char* s=(char*)str;
		while ((*s)&&(*s<'0')&&(*s>'9')) s++; /* saute les caractères non chiffre au début */
		if (*s) {
			sscanf(s, "%d.%d.%d.%d", &bytes[12], &bytes[13], &bytes[14], &bytes[15]);
		} else {
			throw error(1);
		}

	} else if ((nbCars[3]==0)&&(nbCars[2]>1)) {
		/* Cas IPv6 */
		version=6;
		if (!avecPrefixLen) {
			prefixLen=128;
		} else {
			if ((prefixLen<0)||(prefixLen>128)) throw error(1); /* param incorrect */
		}
		assert(0); /* todo !!! */
	} else {
		throw error(1); /* param incorrect */
	}
}

void ipAddress_c::setPrefixLen(uint8_t len) { /* Si n'a pas été fixé par le constructeur */
	hashOk      = false;

	if (version == 4) {
		if ((len >=0)&&(len<=32)) {
			prefixLen = len;
		} else {
			/* erreur à gérer */
		}
	} else if (version == 6) {
		if ((len >=0)&&(len<=128)) {
			prefixLen = len;
		} else {
			/* erreur à gérer */
		}
	} else {
		throw(error(3)); /* erreur inconune */
	}
}

uint8_t ipAddress_c::getVersion() {
	if (version == 4) {
		return 4;
	} else if (version == 6) {
		return 6;
	} else {
		throw(error(3)); /* erreur inconune */
	}
	return 0;
}

char*   ipAddress_c::toString(char* result) {
	if (version == 4) {
		if (prefixLen == 32) {
			sprintf(result, "%d.%d.%d.%d", bytes[12], bytes[13], bytes[14], bytes[15]);
		} else {
			sprintf(result, "%d.%d.%d.%d/%d", bytes[12], bytes[13], bytes[14], bytes[15], prefixLen);
		}
	} else if (version == 6) {
		assert(0); /* todo */
	} else {
		throw(error(3)); /* erreur inconune */
	}
	return result;
}

char*   ipAddress_c::reverseDnsPtr(char* result) {
	/* todo */
	return result;
}

uint8_t ipAddress_c::getPrefixLen() {
	return prefixLen;
}

bool ipAddress_c::isHost() {
	if (version == 4) {
		return (prefixLen == 32);
	} else if (version == 6) {
		return (prefixLen == 128);
	} else {
		throw(error(3)); /* erreur inconune */
	}
	return false;
}

bool	ipAddress_c::isMulicast() { /* RFC 3171 et RFC2373 */
	if (version == 4) {

	} else if (version == 6) {

	} else {
		assert(0);
	}
	return false;
}

uint8_t ipAddress_c::getScope() {
	if (scope != scopeUnspecified) {
		return scope;
	} else {
		if (version == 4) {
			/* todo */
			assert(0);
		} else if (version==6) {
			/* todo */
			assert(0);
		} else {
			assert(0);
		}
		return scope;
	}
}

uint64_t ipAddress_c::hash64() { /* renvoie un haché convenable pour des structures genre Bloom ou HyperLogLog */
	const uint64_t piHex[] = { 0x243F6A8885A308D3, 0x13198A2E03707344, 0xA4093822299F31D0,	0x082EFA98EC4E6C89 }; /* décimales de pi en hexa */
	if (hashOk) {
		return hash;
	} else {
		uint64_t h = (words[3]) + ((uint64_t)words[3] << 32) + ((uint64_t)words[3] << 16); /* plus de diffusion pour les IPv4 */
		h = h + (prefixLen) + ((uint64_t)prefixLen<<32) + ((uint64_t)prefixLen<<16);                           /* prend en compte le prefix len */
		for (int n=0; n<4; n++) {
			h = h + (words[n]) + ((uint64_t)words[n] << 32) + ((uint64_t)words[n] << 16);
			h = h + piHex[n];
			__asm__("movq    %1, %%rax     ;"  /* middleSquare plus pratique en asm car 128 bits dans rdx:rax */
					"mulq    %%rax         ;"
					"shlq    $32, %%rdx    ;"
					"shrq    $32, %%rax    ;"
					"orq     %%rdx, %%rax  ;"
					"movq    %%rax, %0     ;"
					: "=m" (h)
					: "m"  (h)
			);
		}
		return h;
	}
}


uint64_t ipAddress_c::hash64(int lgMask) { /* renvoie un haché convenable pour des structures genre Bloom ou HyperLogLog, en masquant les bits de poids faible */
	const uint64_t piHex[] = { 0x243F6A8885A308D3, 0x13198A2E03707344, 0xA4093822299F31D0,	0x082EFA98EC4E6C89 }; /* décimales de pi en hexa */

	uint32_t words_[4] = { words[0], words[1], words[2], words[3] };
	uint8_t*  bytes_ = (uint8_t*)&words_[0];

	if (version == 4) {
		lgMask = 32-lgMask;
	} else if (version == 6) {
		lgMask = 128-lgMask;
		assert(0); /* implémentation pas testée */
	} else {
		throw error(2); /* pas initialisé */
	}


	int n=15;
	do {
		if (lgMask == 0) {
			break; /* si pas dès le départ, traité par le while en fait */
		} else if (lgMask < 8) {
			bytes_[n] &= 0xff<<lgMask; /* rappel : big endian */
			break;
		} else {
			bytes_[n] = 0;
			lgMask -= 8;
			n--;
		}
	} while (lgMask > 0);


	/*printf("(%d.%d.%d.%d)", bytes_[12], bytes_[13], bytes_[14], bytes_[15]);*/


	if (hashOk) {
		return hash;
	} else {
		uint64_t h = (words_[3]) + ((uint64_t)words_[3] << 32) + ((uint64_t)words_[3] << 16); /* plus de diffusion pour les IPv4 */
		h = h + (prefixLen) + ((uint64_t)prefixLen<<32) + ((uint64_t)prefixLen<<16);                           /* prend en compte le prefix len */
		for (int n=0; n<4; n++) {
			h = h + (words_[n]) + ((uint64_t)words_[n] << 32) + ((uint64_t)words_[n] << 16);
			h = h + piHex[n];
			__asm__("movq    %1, %%rax     ;"  /* middleSquare plus pratique en asm car 128 bits dans rdx:rax */
					"mulq    %%rax         ;"
					"shlq    $32, %%rdx    ;"
					"shrq    $32, %%rax    ;"
					"orq     %%rdx, %%rax  ;"
					"movq    %%rax, %0     ;"
					: "=m" (h)
					: "m"  (h)
			);
		}
		return h;
	}
}



bool ipAddress_c::isIncluded(ipAddress_c* other) { /* indique si 'this' est inclus dans 'other' fourni en paramètre (subnets bien sûr) */
	uint8_t otherPrefixLen = other->getPrefixLen();
	uint8_t thisPrefixLen  = prefixLen;

	/* Ajustement prerfix len ipv4->ipv6
	 * Note : comparaison entre v4 et v6 possible, ça a du sens avec la notation ::192.168.1.1
	 * */
	if (version == 4)              thisPrefixLen += 96;
	if (other->getVersion() == 4) otherPrefixLen += 96;

	int octetsComplets = otherPrefixLen & ~7;

	if (prefixLen < otherPrefixLen) return false; /* this est plus large que other */

	for (int n=0; n<octetsComplets; n++) {
		if (bytes[n] != other->bytes[n]) {
			printf("Difference sur octet %d != %d\n", bytes[n] != other->bytes[n]);
			return false;
		}
	}

	if ((otherPrefixLen & 7) ==0) return true; /* le masque était aligné sur une fontière d'octet */

	int octetsFraction = (otherPrefixLen >>3);   /* l'octet qui est à moitié dans le n° de réseau et le n° d'hôte */
	int bitsFraction   = (otherPrefixLen & 7);   /* nb de bits qui sont dans le n° de réseau */
	uint8_t mask       = ((1<<(8-bitsFraction))-1);  /* mask pour n° d'hote */
	        mask       = ~mask;                  /* mask pour n° de réseau */

	return ((bytes[octetsFraction]&mask) == (other->bytes[octetsFraction]&mask));
}
