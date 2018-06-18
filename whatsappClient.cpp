//
// Created by danielbraun on 6/18/18.
//

#include <iostream>
#include <netdb.h>
#include <unistd.h>
#include <cstdio>
//#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cerrno>
#include <clocale>
#include <cstring>

int call_socket(char *hostname, unsigned short portnum) {
    struct sockaddr_in server_socket_address;
    struct hostent *hp;
    int client_socket_file_descriptor;
    if ((hp= gethostbyname (hostname)) == NULL) {
        return(-1);
    }
    memset(&server_socket_address,0,sizeof(server_socket_address));
    memcpy((char *)&server_socket_address.sin_addr , hp->h_addr , (size_t)hp->h_length);
    server_socket_address.sin_family = (sa_family_t)hp->h_addrtype;
    server_socket_address.sin_port = htons((u_short)portnum);

    if ((client_socket_file_descriptor = socket(hp->h_addrtype, SOCK_STREAM, 0)) < 0) {
        return(-1);
    }

    if (connect(client_socket_file_descriptor, (struct sockaddr *)&server_socket_address , sizeof(server_socket_address)) < 0) {
        close(client_socket_file_descriptor);
        std::cout << std::strerror(errno) <<std::endl;
        return(-1);
    }

    return(client_socket_file_descriptor);
}

int main(int argc, char *argv[])
{
    if ((argc != 4) )
    {
        std::cout << "Usage: whatsappClient clientName serverAddress serverPort" << std::endl;
        return -1;
    }

    char* p;
    long converted = strtol(argv[argc-1], &p, 10);
    if (*p) {
        std::cout << "Usage: whatsappClient clientName serverAddress serverPort" << std::endl;
        return -1;
    }

    struct hostent *h;
    if ((h=gethostbyname(argv[2])) == nullptr) {
        fprintf(stderr, "gethostbyname error");
        exit(1);
    }

    printf("Host name : %s\n", h->h_name);
    printf("IP Address : %s\n", inet_ntoa(*((struct in_addr *)h->h_addr)));

    int client_socket_file_descriptor = call_socket(h->h_name, (unsigned short)converted);
    return 0;
}