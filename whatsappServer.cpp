//
// Created by danielbraun on 6/18/18.
//
#define MAXHOSTNAME 30
#define MAX_CLIENTS 200
#define MAX_MESSAGE_LENGTH 256
#define MAX_PENDING_CONNECTIONS 10
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



void terminateServer(){};
void connectNewClient(){
    std::cout << "a client want to connect! " << std::endl;

//    int get_connection(int s) {
//        int t; /* socket of connection */
//        if ((t = accept(s,NULL,NULL)) < 0)
//            return -1;
//        return t;
//    }
};

void serverStdInput(){
    char buf[MAX_MESSAGE_LENGTH];
    read_data(STDIN_FILENO, buf, MAX_MESSAGE_LENGTH-1);
    buf[MAX_MESSAGE_LENGTH] = 0;
    std::cout << "read from stdin: " << buf << " with len: " << strlen(buf) << std::endl;

};


void handleClientRequest(){};
int main(int argc, char *argv[])
{
    //input validation
    if ((argc != 2) )
    {
        std::cout << "Usage: whatsappServer portNum" << std::endl;
        return -1;
    }
    char* p;
    long converted = strtol(argv[argc-1], &p, 10);
    if (*p) {
        std::cout << "Usage: whatsappServer portNum" << std::endl;
        return -1;
    }

    uint16_t portnum = (uint16_t)converted;
    char myname[MAXHOSTNAME+1];
    int server_socket_file_descriptor;
    struct sockaddr_in server_socket_address_object;
    struct hostent* hp;

    //hostnet initialization
    gethostname(myname, MAXHOSTNAME);
    hp = gethostbyname(myname);  //TODO CHECK RETURN VALUE
    if (hp == nullptr)
        return(-1);

    //sockaddrr_in initlization
    memset(&server_socket_address_object, 0, sizeof(struct sockaddr_in));
    server_socket_address_object.sin_family = (sa_family_t)hp->h_addrtype;
    memcpy(&server_socket_address_object.sin_addr, hp->h_addr, (size_t)hp->h_length);
    server_socket_address_object.sin_port= htons(portnum);

    std::cout << "server_socket_address_object initialized, with address: "<< inet_ntoa(server_socket_address_object.sin_addr)<< " port: "<< ntohs(server_socket_address_object.sin_port)  << std::endl;

    /* create socket */
    if ((server_socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return(-1);

    if (bind(server_socket_file_descriptor , (struct sockaddr *)&server_socket_address_object , sizeof(struct
            sockaddr_in)) < 0)
    {
        std::cout << std::strerror(errno) << "\nport number must lower than 1024 LOL!" <<std::endl;
        close(server_socket_file_descriptor);
        return(-1);
    }

    listen(server_socket_file_descriptor, MAX_PENDING_CONNECTIONS);

    fd_set clientsfds;
    fd_set readfds;

    FD_ZERO(&clientsfds);
    FD_SET(server_socket_file_descriptor, &clientsfds);
    FD_SET(STDIN_FILENO, &clientsfds);
    int stillRunning = 1;
    while (stillRunning) {
        readfds = clientsfds;
        if (select(MAX_CLIENTS+1, &readfds, NULL,
                   NULL, NULL) < 0) {
            terminateServer();
            return -1;
        }

        if (FD_ISSET(server_socket_file_descriptor, &readfds)) {
            //will also add the client to the clientsfds
            connectNewClient();
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            serverStdInput();
        }

        else {
            //will check each client if it’s in readfds
            //and then receive a message from him
            handleClientRequest();
        }
    }

    return 0;
}

