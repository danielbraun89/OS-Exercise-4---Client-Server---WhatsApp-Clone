//
// Created by danielbraun on 6/18/18.
//

#include "whatsappio.h"
#define MAXHOSTNAME 30
#define MAX_CLIENTS 200
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
#include <vector>

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

struct client {
    int fid;
    std::string str_name;
    char name[WA_MAX_NAME];
};
char message_buffer[WA_MAX_MESSAGE];

fd_set clientsfds;
fd_set readfds;
std::vector<client> client_vector;

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


int get_connection(int server_socket_file_descriptor) {
    int t; /* socket of connection */
    if ((t = accept(server_socket_file_descriptor,NULL,NULL)) < 0)
        exit(1);
    return t;
}

void terminateServer()
{
    exit(0);
};
void connectNewClient(int server_socket_file_descriptor){
    std::cout << "a client want to connect! " << std::endl;
    int new_client_socket_file_descriptor = get_connection(server_socket_file_descriptor);
    std::cout << "a new socket has been created, num: " << new_client_socket_file_descriptor << std::endl;

    struct client cur_client;
    cur_client.fid = new_client_socket_file_descriptor;
    client_vector.push_back(cur_client);

    read_data(cur_client.fid, message_buffer, WA_MAX_MESSAGE-1);
    message_buffer[WA_MAX_MESSAGE-1] = 0;
    std::string str1 (message_buffer);
    std::cout << "fresh client read: " << str1 <<std::endl;
    FD_SET(cur_client.fid, &clientsfds);
};

void serverStdInput(){
    read_data(STDIN_FILENO, message_buffer, WA_MAX_MESSAGE-1);
    message_buffer[WA_MAX_MESSAGE-1] = 0;
    std::string str1 (message_buffer);
//    std::cout << "read from stdin: " << message_buffer << " with len: " << str1.length() << std::endl;
    std::string str2 ("EXIT\n");

    if ((str1.compare(str2)) == 0)
    {
        terminateServer();
    }

};


void handleClientRequest(){
    std::vector<struct client> client_vector2 = client_vector;
    for (int i=0; i<client_vector.size(); i++)
    {
        if (FD_ISSET(client_vector[i].fid, &readfds))
        {
            int fid = client_vector[i].fid;
            read_data(fid, message_buffer, WA_MAX_MESSAGE-1);
            message_buffer[WA_MAX_MESSAGE-1] = 0;
            std::string str1 (message_buffer);
            std::cout << "request from client: " << str1 << " with len: " << str1.length() << " client num: " <<client_vector[i].fid << std::endl;
            FD_SET(client_vector[i].fid, &clientsfds);
            std::string str2 ("got it");
            write_data(client_vector[i].fid, str2.c_str(), str2.length());

        }
    }
};
int main(int argc, char *argv[])
{
    //input validation
    if ((argc != 2) )
    {
        print_server_usage();
        exit(1);
    }
    char* p;
    long converted = strtol(argv[argc-1], &p, 10);
    if (*p) {
        print_server_usage();
        exit(1);
    }

    uint16_t portnum = (uint16_t)converted;

    int server_socket_file_descriptor;
    struct sockaddr_in server_socket_address_object;
    struct hostent* hp;

    //hostnet initialization
    char myname[MAXHOSTNAME+1];
    gethostname(myname, MAXHOSTNAME);
    hp = gethostbyname(myname);  //TODO CHECK RETURN VALUE
    if (hp == nullptr)
        return(-1);

    //sockaddrr_in initlization
    memset(&server_socket_address_object, 0, sizeof(struct sockaddr_in));
    server_socket_address_object.sin_family = (sa_family_t)hp->h_addrtype;
    memcpy(&server_socket_address_object.sin_addr, hp->h_addr, (size_t)hp->h_length);
    server_socket_address_object.sin_port= htons(portnum);
    server_socket_address_object.sin_addr.s_addr = INADDR_ANY;

    std::cout << "server_socket_address_object initialized, with address: "<< inet_ntoa(server_socket_address_object.sin_addr)<< " port: "<< ntohs(server_socket_address_object.sin_port)  << std::endl;

    /* create socket */
    if ((server_socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        exit(1);

    if (bind(server_socket_file_descriptor , (struct sockaddr *)&server_socket_address_object , sizeof(struct
            sockaddr_in)) < 0)
    {
        std::cout << std::strerror(errno) << "\nport number must lower than 1024 LOL!" <<std::endl;
        close(server_socket_file_descriptor);
        return(-1);
    }

    listen(server_socket_file_descriptor, MAX_PENDING_CONNECTIONS);


    FD_ZERO(&clientsfds);
    FD_SET(server_socket_file_descriptor, &clientsfds);
    FD_SET(STDIN_FILENO, &clientsfds);
    int stillRunning = 1;
    while (stillRunning)
    {
        readfds = clientsfds;
        if (select(MAX_CLIENTS+1, &readfds, NULL,NULL, NULL) < 0)
        {
            terminateServer();
            return -1;
        }

        else if (FD_ISSET(server_socket_file_descriptor, &readfds))
        {
            //will also add the client to the clientsfds
            connectNewClient(server_socket_file_descriptor);
        }

        else if (FD_ISSET(STDIN_FILENO, &readfds))
        {
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

