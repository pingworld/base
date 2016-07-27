#include "b1_socket.h"
#include <string.h>
#include <stdio.h>

int 
b1_sock_addr_str_to_ulong(const std::string &str_addr, uint64_t &ulong_addr)
{
    struct in_addr ret_addr;
    int ret = inet_aton(str_addr.c_str(), &ret_addr);
    if (ret != 0) {
        ulong_addr = htonl(ret_addr.s_addr);
        return 0;
    }
    return ret;
}

int 
b1_sock_addr_str_to_in_addr(const std::string &str_addr, 
                            struct in_addr *in_addr_out)
{
    if (!in_addr_out) return -1;
    int ret = inet_aton(str_addr.c_str(), in_addr_out);
    return ret;
}

int 
b1_sock_addr_ulong_to_str(uint64_t ulong_addr, std::string &str_addr)
{
    struct in_addr input_addr;
    input_addr.s_addr = ntohl(ulong_addr);
    char *str = inet_ntoa(input_addr);
    str_addr = std::string(str);
    return 0;
}

struct addrinfo *
b1_sock_get_addrinfo(int sfamily, int sport, int stype, const std::string &saddr)
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = (sfamily != 0) ? sfamily : PF_UNSPEC;
    hints.ai_socktype = stype;
    // It means to bind to INADDR_ANY.
    if (saddr.empty()) hints.ai_flags = AI_PASSIVE;

    int rc = 0;
    const int port_str_max_len = 11;
    char port_str[port_str_max_len] = {0};
    snprintf(port_str, port_str_max_len-1, "%d", sport);    
    struct addrinfo *res = NULL;
    rc = getaddrinfo(saddr.c_str(), port_str, &hints, &res);
    if (rc != 0) return NULL;

#if 0
    for (struct addrinfo *node = res; node; node = node->ai_next) {
        char host[NI_MAXHOST] = {0};
        char serv[NI_MAXSERV] = {0};
        rc = getnameinfo(res->ai_addr, res->ai_addrlen, host, sizeof(host), 
            serv, sizeof(serv), NI_NUMERICHOST|NI_NUMERICSERV);
        assert(rc == 0);
        if (rc < 0) {
            freeaddrinfo(res);
        }
    }
#endif

    return res;
}

