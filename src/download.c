#include "../include/download.h"

url convertToURL(const char* urlStr) {
    url urlObj;
    memset(&urlObj, 0, sizeof(urlObj));

    // Check if the URL starts with "ftp://"
    const char* ftpProtocol = "ftp://";
    if (strncmp(urlStr, ftpProtocol, strlen(ftpProtocol)) != 0) {
        fprintf(stderr, "Invalid URL format. The URL should start with 'ftp://'.\n");
        return urlObj;
    }

    // Skip the "ftp://" part
    urlStr += strlen(ftpProtocol);

    // Parse the user info if present
    const char* userInfoEnd = strchr(urlStr, '@');
    if (userInfoEnd != NULL) {
        size_t userInfoLen = userInfoEnd - urlStr;

        // Check for the presence of ":" to separate username and password
        const char* passwordSeparator = strchr(urlStr, ':');
        if (passwordSeparator != NULL && passwordSeparator < userInfoEnd) {
            size_t usernameLen = passwordSeparator - urlStr;
            size_t passwordLen = userInfoEnd - passwordSeparator - 1;

            strncpy(urlObj.user, urlStr, usernameLen);
            urlObj.user[usernameLen] = '\0';

            strncpy(urlObj.password, passwordSeparator + 1, passwordLen);
            urlObj.password[passwordLen] = '\0';
        } else {
            // If no ":", the entire user info is the username
            strncpy(urlObj.user, urlStr, userInfoLen);
            urlObj.user[userInfoLen] = '\0';
        }

        // Move the pointer past the "@" symbol
        urlStr = userInfoEnd + 1;
    }

    // Parse the host
    const char* hostEnd = strchr(urlStr, '/');
    if (hostEnd != NULL && hostEnd > urlStr) {
        size_t hostLen = hostEnd - urlStr;
        strncpy(urlObj.host, urlStr, hostLen);
        urlObj.host[hostLen] = '\0';

        urlStr = hostEnd + 1;
    } else {
        // If there's no "/", the entire remaining string is the host
        strcpy(urlObj.host, urlStr);
        urlStr += strlen(urlStr);
    }

    // The remaining part is the URL path
    strncpy(urlObj.url_path, urlStr, sizeof(urlObj.url_path) - 1);
    urlObj.url_path[sizeof(urlObj.url_path) - 1] = '\0';

    return urlObj;
}




int connectSocket(char *ip_addr, int port) {
    int sockfd;
    struct sockaddr_in server_addr;

    /*server address handling*/
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_addr);    /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(port);        /*server TCP port must be network byte ordered */

    /*open a TCP socket*/
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(-1);
    }
    /*connect to the server*/
    if (connect(sockfd,
                (struct sockaddr *) &server_addr,
                sizeof(server_addr)) < 0) {
        perror("connect()");
        exit(-1);
    }

    return sockfd;
}

char * getReply(int sockfd){
    char *reply = malloc(1024);
	
	size_t n = 0;
	ssize_t read;

	FILE* fp = fdopen(sockfd, "r");
	while((read = getline(&reply, &n, fp)) != -1) {
		if(reply[3] == ' ') break;
	}

	reply[1023] = '\0';
	printf("Reply: %s\n", reply);

	return reply;

}


int login(url* urlObj, int sockfd) {
    char login[1024];

    sprintf(login, "user %s\n", urlObj->user);
    write(sockfd, login, strlen(login));

    char* reply = getReply(sockfd);

    if (strncmp(reply, "4", 1) == 0 || strncmp(reply, "5", 1) == 0) {
        close(sockfd);
        return -1;
    }

    sprintf(login, "pass %s\n", urlObj->password);
    write(sockfd, login, strlen(login));

    reply = getReply(sockfd);

    if (strncmp(reply, "4", 1) == 0 || strncmp(reply, "5", 1) == 0) {
        close(sockfd);
        return -1;
    }

    return 0;
}

int pasv(int sockfd) {
    char *pasv = malloc(1024);
    sprintf(pasv, "pasv\n");
    write(sockfd, pasv, strlen(pasv));

    char *reply = getReply(sockfd);

    if (strncmp(reply, "4", 1) == 0 || strncmp(reply, "5", 1) == 0) {
        close(sockfd);
        return -1;
    }

    char *ip = malloc(1024);
    int port[2];

    sscanf(reply, "227 Entering Passive Mode (%*[^,],%*d,%*d,%*d,%d,%d)", &port[0], &port[1]);

    int port_pasv = port[0] * 256 + port[1];

    printf("port[0] is %d\n", port[0]);
    printf("port[1] is %d\n", port[1]);

    printf("port: %d\n", port_pasv);

    return port_pasv;
}

int retrieveResource(int sockfd, int sockfd2, const char* url_path) {
    char request[1024];
    sprintf(request, "retr %s\n", url_path);

    write(sockfd, request, strlen(request));

    char* reply = getReply(sockfd);

    if (strncmp(reply, "4", 1) == 0 || strncmp(reply, "5", 1) == 0) {
        close(sockfd);
        return -1;
    }

    const char* lastSlash = strrchr(url_path, '/');
    const char* filename = (lastSlash != NULL) ? lastSlash + 1 : url_path;

    printf("Filename: %s\n", filename);

	FILE *f = fopen(filename, "wb");
    if (f == NULL) {
        fprintf(stderr, "Error opening file for writing\n");
        close(sockfd);
        return -1;
    }

    char buf[1024];
    ssize_t bytesRead;

    long int cont;

    int bytes;

    while ((bytes = recv(sockfd2, buf, 1024, 0)) > 0) {
        if (f == NULL) {
            fprintf(stderr, "Error: File pointer is NULL.\n");
            break; 
        }

        size_t bytes_written = fwrite(buf, 1, bytes, f);

        if (bytes_written != bytes) {
            fprintf(stderr, "Error writing to the file.\n");
            break;  
        }
        cont+=bytes;
        printf("Bytes written: %ld\n", cont);
    }

    fclose(f);

    return 0;
}




int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Invalid number of arguments. The format should be: download ftp://[<user>:<password>@]<host>/<url-path>\n");
        exit(-1);
    }

    // check if url is valid and convert to url object
    url urlObj = convertToURL(argv[2]);
    if (strlen(urlObj.host) == 0) {
        fprintf(stderr, "Invalid URL\n");
        exit(-1);
    }

    if (strlen(urlObj.user) == 0) {
        strcpy(urlObj.user, "anonymous");
    }
    if (strlen(urlObj.password) == 0) {
        strcpy(urlObj.password, "anonymous");
    }


    printf("Host: %s\n", urlObj.host);
    printf("URL path: %s\n", urlObj.url_path);

    // get ip address from host
    struct hostent *h;

    if ((h = gethostbyname(urlObj.host)) == NULL) {
        herror("gethostbyname()");
        exit(-1);
    }

    printf("Host name  : %s\n", h->h_name);
    printf("IP Address : %s\n", inet_ntoa(*((struct in_addr *) h->h_addr)));

    char *ip_addr = inet_ntoa(*((struct in_addr *) h->h_addr));

    int sockfd = connectSocket(ip_addr, SERVER);

    // connect to server A
    if (sockfd < 0) {
        fprintf(stderr, "Error connecting to server\n");
        exit(-1);
    }

    char * reply = getReply(sockfd);
    
    if (strncmp(reply, "4", 1) == 0 || strncmp(reply, "5", 1) == 0) {
        close(sockfd);
        return -1;
    }

    // login
    if (login(&urlObj, sockfd) < 0) {
        fprintf(stderr, "Error logging in\n");
        exit(-1);
    }

    // pasv and connect to server B
    int passive_port = pasv(sockfd);

    if (passive_port < 0) {
        fprintf(stderr, "Error entering passive mode\n");
        exit(-1);
    }

    printf("Passive mode established\n");


    // request resource

    int sockfd2 = connectSocket(ip_addr, passive_port);

    int command = retrieveResource(sockfd, sockfd2, urlObj.url_path);

    if (command < 0){
        fprintf(stderr, "Error requesting resource\n");
        exit(-1);
    }


    printf("Resource downloaded\n");

    if(close(sockfd)!=0 || close(sockfd2)!=0) {
        printf("Error: Couldn't close sockets.\n");
        exit(-1);
    }

    return 0;
}


