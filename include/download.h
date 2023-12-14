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

#define MAX_SIZE 1024

#define SERVER 21

#define h_addr h_addr_list[0]

typedef struct {
    char user[1024];
    char password[1024];
    char host[1024];
    char url_path[1024];
} url;

// This function converts a string to a url struct
url convertToURL(const char* urlStr);

// This function connects to the server
int connectSocket(char *ip_addr, int port);

// This function activates passive mode
int pasv(int sockfd);

// This function logs into the server
int login(url* urlObj, int sockfd);

// This function retrieves the response from the server
char * getReply(int sockfd);

// This function downloads the file from the server
int retrieveResource(int sockfd, int sockfd2, const char* url_path);

#endif // DOWNLOAD_H
