#include "../include/download.h"


int checkURL(char *url) {
    char *pattern = "^ftp://([a-z0-9]+:[a-z0-9]+@)?([\\.a-z0-9-]+)/([\\./a-z0-9-]+)$";
    regex_t regex;
    int reti;
    reti = regcomp(&regex, pattern, REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");            
        exit(-1);
    }

    reti = regexec(&regex, url, 0, NULL, 0);
    if (reti == REG_NOMATCH) {
        fprintf(stderr, "Invalid format. The format should be: ftp://[<user>:<password>@]<host>/<url-path>\n");
        exit(-1);
    } else if (reti) {
        fprintf(stderr, "Regex match failed\n");
        exit(-1);
    }

    regfree(&regex);

    return 0;
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


int login(char * url, int sockfd) {
    char *user = malloc(1024);
    char *password = malloc(1024);
    char *host = malloc(1024);
    char *url_path = malloc(1024);

    sscanf(url, "ftp://%[^:]:%[^@]@%[^/]/%s", user, password, host, url_path);

    char *login = malloc(1024);
    sprintf(login, "user %s\n", user);
    write(sockfd, login, strlen(login));

    char *reply = getReply(sockfd);

    if (strncmp(reply, "4", 1) == 0 || strncmp(reply, "5", 1) == 0) {
        close(sockfd);
        return -1;
    }

    sprintf(login, "pass %s\n", password);
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

    sscanf(reply, "227 Entering Passive Mode (%[^,],%d,%d)", ip, &port[0], &port[1]);

    int port_pasv = port[0] * 256 + port[1];

    int sockRequest = connectSocket(ip, port_pasv);

    return sockRequest;
}



int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Invalid number of arguments. The format should be: download ftp://[<user>:<password>@]<host>/<url-path>\n");
        exit(-1);
    }

    // check if url is valid
    if (checkURL(argv[2]) < 0) {
        fprintf(stderr, "Invalid URL\n");
        exit(-1);
    }

    char *url = argv[2];

    // get ip address from host
    struct hostent *h;

    char *host = malloc(1024);
    sscanf(url, "ftp://%*[^:]:%*[^@]@%[^/]/%*s", host);

    if ((h = gethostbyname(argv[2])) == NULL) {
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
    if (login(url, sockfd) < 0) {
        fprintf(stderr, "Error logging in\n");
        exit(-1);
    }

    // pasv and connect to server B
    int passiveMode = pasv(sockfd);

    if (passiveMode < 0) {
        fprintf(stderr, "Error entering passive mode\n");
        exit(-1);
    }

    printf("Passive mode established\n");

    return 0;
}


