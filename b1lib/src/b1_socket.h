#ifndef _B1_SOCKET_H_
#define _B1_SOCKET_H_

#include <string>
#include <stdint.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// - IP address helper
// 1. convert ip address from char * into ulonglong;
// 2. convert ip address from ulonglong into char *;
// 3. some kindof address type:
//  - struct sockaddr {}, The common socket address.
//  - struct sockaddr_in, The special socket address for internet.
//  - struct in_addr, The 32 bit ip address used in sockaddr_in
// 

// All The Structs listed on bellow:
// struct sockaddr {
//     sa_family_t sa_family;
//     char        sa_data[14];
// };
// struct sockaddr_in {
//     sa_family_t    sin_family; /* address family: AF_INET */
//     in_port_t      sin_port;   /* port in network byte order */
//     struct in_addr sin_addr;   /* internet address */
//     unsigned char  sin_zero[8]; /* Same size as struct sockaddr */
// };
// /* Internet address. */
// struct in_addr {
//     uint32_t       s_addr;     /* address in network byte order */
// };
//
// ------------------------------------------
// retrieves the locally-bound name of the specified socket
// int getsockname(int sockfd, struct sockaddr * localaddr, socken_t * addrlen);
// retrieves the peer address of the specified socket
// int getpeername(int sockfd, struct sockaddr * peeraddr, socken_t * addrlen);
//
// ------------------------------------------
// struct addrinfo {
//     int              ai_flags;
//     int              ai_family;
//     int              ai_socktype;
//     int              ai_protocol;
//     socklen_t        ai_addrlen;
//     struct sockaddr *ai_addr;
//     char            *ai_canonname;
//     struct addrinfo *ai_next;
// };
// int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);
// void freeaddrinfo(struct addrinfo *res);
//
// The getaddrinfo() function combines the functionality provided by the 
// gethostbyname(3) and getservbyname(3) functions into a single interface, 
// but unlike the latter functions, getaddrinfo() is reentrant and 
// allows programs to eliminate IPv4-versus-IPv6 dependencies.
//
//
// **NOTE**
// The gethostbyname*() and gethostbyaddr*() functions are **obsolete(·ÏÆú)**. 
// Applications should use getaddrinfo(3) and getnameinfo(3) instead.
//

// ==> inet_addr, eg: 192.168.x.x --> 123xxxxxx
int b1_sock_addr_str_to_ulong(const std::string &str_addr, uint64_t &ulong_addr);
// ==> inet_aton||inet_pton, eg: 192.168.x.x --> { 123xxxxxx }
int b1_sock_addr_str_to_in_addr(const std::string &str_addr, struct in_addr *in_addr_out);
// ==> inet_ntoa||inet_ntop, eg: 123xxxxxx --> 192.168.x.x
int b1_sock_addr_ulong_to_str(uint64_t ulong_addr, std::string &str_addr);

struct addrinfo *b1_sock_get_addrinfo(int sfamily, int sport, int stype, const std::string &saddr);
inline void b1_sock_get_addrinfo(struct addrinfo *af) { freeaddrinfo(af); }

#endif
