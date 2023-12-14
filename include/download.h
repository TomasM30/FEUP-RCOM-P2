#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>

#include <netdb.h>

#define SERVER_PORT 6000
#define SERVER_ADDR "192.168.28.96"

#define MAX_SIZE 1024

#define SERVER 21

#define h_addr h_addr_list[0]

typedef struct {
    char user[1024];
    char password[1024];
    char host[1024];
    char url_path[1024];
} url;


/*
struct hostent {
    char *h_name;    // Official name of the host.
    char **h_aliases;    // A NULL-terminated array of alternate names for the host.
    int h_addrtype;    // The type of address being returned; usually AF_INET.
    int h_length;    // The length of the address in bytes.
    char **h_addr_list;    // A zero-terminated array of network addresses for the host.
    // Host addresses are in Network Byte Order.
};*/

#endif // DOWNLOAD_H
