//
// Created by danielbraun on 6/18/18.
//
#include "whatsappio.h"
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
#define MAXHOSTNAME 30

char message_buffer[WA_MAX_MESSAGE];

int read_data(int s, char *buf, int n) {
    int bcount;
/* counts bytes read */
    int br;
/* bytes read this pass */
    bcount= 0; br= 0;
    while (bcount < n) { /* loop until full buffer */
        br = (int)read(s, buf, n-bcount);
        if (br > 0) {
            bcount += br;
            if (buf[strlen(buf) - 1] == '\n')
            {
                buf += br;
                return(bcount);
            }
            buf += br;
        }
        if (br < 1) {
            return(-1);
        }
    }
    return(bcount);
}


int write_data(int s,const char *buf, int n)

{ int bcount,          /* counts bytes read */
        br;              /* bytes read this pass */

    bcount= 0;
    br= 0;
    while (bcount < n) {             /* loop until full buffer */
        if ((br= write(s,buf,n-bcount)) > 0)
        {
            std::cout << "write_data!\n";
            bcount += br;                /* increment byte counter */
            buf += br;                   /* move buffer ptr for next read */
        }
        if (br < 0) {
            perror("write_data");        /* signal an error to the caller */
            return(-1);
        }
    }
    return(bcount);
}



int call_socket(char *hostname, unsigned short portnum) {
    struct sockaddr_in server_socket_address_object;
    struct hostent *hp;
    int client_socket_file_descriptor;

    if ((hp= gethostbyname (hostname)) == nullptr)
    {
        return(-1);
    }

    memset(&server_socket_address_object,0,sizeof(struct sockaddr_in));
    server_socket_address_object.sin_family = (sa_family_t)hp->h_addrtype;
    memcpy(&server_socket_address_object.sin_addr , hp->h_addr , (size_t)hp->h_length);
    server_socket_address_object.sin_port = htons((u_short)portnum);

    std::cout << "server_socket_address_object initialized, with address: "<< inet_ntoa(server_socket_address_object.sin_addr)<< " port: "<< ntohs(server_socket_address_object.sin_port)  << std::endl;

    if ((client_socket_file_descriptor = socket(hp->h_addrtype, SOCK_STREAM, 0)) < 0) {
        return(-1);
    }

    if (connect(client_socket_file_descriptor, (struct sockaddr *)&server_socket_address_object , sizeof(server_socket_address_object)) < 0)
    {
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
        print_client_usage();
        return -1;
    }

    char* p;
    long converted = strtol(argv[argc-1], &p, 10);
    if (*p) {
        print_client_usage();
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

    std::string client_name (argv[1]);
//    send(client_socket_file_descriptor, (const void *)(&client_name), client_name.length() );
    while(1){
        getchar();
        write_data(client_socket_file_descriptor, client_name.c_str(), client_name.length());
        write_data(client_socket_file_descriptor, "\n", 1);
        printf("written client name");
        std::cout << argv[1];
        getchar();
        read_data(client_socket_file_descriptor, message_buffer, WA_MAX_MESSAGE-1);
        message_buffer[WA_MAX_MESSAGE-1] = 0;
        std::cout << "got response from server: " << message_buffer;
    }

    return 0;
}