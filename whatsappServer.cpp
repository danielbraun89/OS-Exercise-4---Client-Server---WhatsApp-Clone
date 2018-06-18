//
// Created by danielbraun on 6/18/18.
//

#include "whatsappio.h"
#include "ex4_utils.h"

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
#include <algorithm>

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
//char message_buffer[WA_MAX_MESSAGE + 1];

fd_set clientsfds;
fd_set readfds;


void terminateServer()
{
    exit(0);
};

int connectNewClient(int server_socket_file_descriptor,std::vector<client>* client_vector ){
    std::cout << "a client want to connect! " << std::endl;
    int new_client_socket_file_descriptor = accept(server_socket_file_descriptor,NULL,NULL);
    if (new_client_socket_file_descriptor < 0)
    {
        print_fail_connection();
        return 0;
    }

    std::cout << "a new socket has been created, num: " << new_client_socket_file_descriptor << std::endl;

    struct client cur_client;
    cur_client.fid = new_client_socket_file_descriptor;
    client_vector->push_back(cur_client);

    int a = 6;

};

void serverStdInput(){
    std::pair<std::string,int> a = read_data_from_socket(STDIN_FILENO);
//    std::cout << "read from stdin: " << message_buffer << " with len: " << str1.length() << std::endl;
    std::string str2 ("EXIT\n");

    if ((a.first.compare(str2)) == 0)
    {
        terminateServer();
    }

};



void handleClientRequest(int fid, std::vector<client>* client_vector){
    std::vector<struct client> client_vector2 = *client_vector;
    std::pair<std::string,int> a= read_data_from_socket(fid);
    if (a.second)
    {
        std::cout << "server recognized disconnection \n";
        //TODO remove it from vector
    }
    std::cout << "server got: "<< a.first;
};


int main(int argc, char *argv[])
{

    std::vector<client> client_vector;

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


    int stillRunning = 1;
    while (stillRunning)
    {
        FD_ZERO(&clientsfds);
        FD_SET(server_socket_file_descriptor, &clientsfds);
        FD_SET(STDIN_FILENO, &clientsfds);
        int  a = client_vector.size();
        if (a){
            int b = client_vector[0].fid;
            int c = 7;
        }
        for (int i=0; i<client_vector.size(); i++)
        {
            FD_SET(client_vector[i].fid, &clientsfds);
        }
        readfds = clientsfds;
        if (select(MAX_CLIENTS+1, &readfds, NULL,NULL, NULL) < 0)
        {
            terminateServer();
            return -1;
        }

        if (FD_ISSET(server_socket_file_descriptor, &readfds))
        {
            //will also add the client to the clientsfds
            connectNewClient(server_socket_file_descriptor, &client_vector);
            continue;
        }

        if (FD_ISSET(STDIN_FILENO, &readfds))
        {
            serverStdInput();
        }

        for (int i=0; i<client_vector.size(); i++)
        {
            int client_fid = client_vector[i].fid;
            if(FD_ISSET(client_fid, &readfds))
            {
                handleClientRequest(client_fid, &client_vector);
            }
        }
    }

    return 0;
}

