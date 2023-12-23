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

/**
 * @brief Structure representing a URL.
 * 
 * This structure holds the components of a URL, including the user, password, host, and URL path.
 * Each component is represented by a character array with a maximum length of 1024 characters.
 */
typedef struct {
    char user[1024];
    char password[1024];
    char host[1024];
    char url_path[1024];
} url;

/**
 * @brief Converts a string representing a URL to a URL object.
 *
 * This function takes a string `urlStr` as input and converts it to a URL object.
 * The URL object represents the URL in a structured format, allowing easy manipulation and retrieval of its components.
 *
 * @param urlStr The string representing the URL.
 * @return The URL object representing the converted URL.
 */
url convertToURL(const char* urlStr);

/**
 * @brief Establishes a connection to a server using the specified IP address and port number.
 *
 * This function creates a socket and connects it to the server specified by the given IP address and port number.
 *
 * @param ip_addr The IP address of the server to connect to.
 * @param port The port number of the server to connect to.
 * @return The file descriptor of the connected socket, or -1 if an error occurred.
 */
int connectSocket(char *ip_addr, int port);

/**
 * @brief Sets the FTP server to passive mode.
 *
 * This function sends the PASV command to the FTP server, which instructs the server to enter passive mode.
 * In passive mode, the server opens a random port and waits for the client to establish a connection.
 *
 * @param sockfd The socket file descriptor connected to the FTP server.
 * @return Returns 0 on success, or -1 if an error occurred.
 */
int pasv(int sockfd);

/**
 * @brief Performs the login operation for the given URL object and socket file descriptor.
 *
 * This function is responsible for performing the login operation using the provided URL object
 * and socket file descriptor. It establishes a connection with the server and sends the necessary
 * login credentials. The function returns an integer value indicating the success or failure of
 * the login operation.
 *
 * @param urlObj A pointer to the URL object containing the necessary login information.
 * @param sockfd The socket file descriptor used for communication with the server.
 * @return An integer value indicating the success or failure of the login operation.
 */
int login(url* urlObj, int sockfd);

/**
 * @brief Receives a reply from the server through the specified socket.
 *
 * This function reads the server's response from the socket and returns it as a null-terminated string.
 *
 * @param sockfd The socket descriptor.
 * @return A pointer to the received reply as a null-terminated string. The caller is responsible for freeing the memory allocated for the string.
 */
char * getReply(int sockfd);

/**
 * @brief Retrieves a resource from a given URL path using two socket descriptors.
 *
 * This function establishes a connection with the server using the provided socket descriptors
 * and retrieves the resource specified by the URL path. The retrieved resource is then stored
 * in the local file system.
 *
 * @param sockfd The socket descriptor for the control connection.
 * @param sockfd2 The socket descriptor for the data connection.
 * @param url_path The URL path of the resource to be retrieved.
 * @return Returns 0 on success, or a negative value on failure.
 */
int retrieveResource(int sockfd, int sockfd2, const char* url_path);

#endif // DOWNLOAD_H
