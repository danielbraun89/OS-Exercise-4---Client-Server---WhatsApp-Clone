//
// Created by danielbraun on 6/18/18.
//
#include "whatsappio.h"
#include "ex4_utils.h"
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

//int read_data(int s, char *buf, int n) {
//    int bcount;
///* counts bytes read */
//    int br;
///* bytes read this pass */
//    bcount= 0; br= 0;
//    while (bcount < n) { /* loop until full buffer */
//        br = (int)read(s, buf, n-bcount);
//        if (br > 0) {
//            bcount += br;
//            if (buf[strlen(buf) - 1] == '\n')
//            {
//                buf += br;
//                return(bcount);
//            }
//            buf += br;
//        }
//        if (br < 1) {
//            return(-1);
//        }
//    }
//    return(bcount);
//}


void clientStdInput(int server_socket)
{
    std::pair<std::string,int> a = read_data_from_socket(STDIN_FILENO);
    std::string str2 ("exit\n");
    if ((a.first.compare(str2)) == 0)
    {
        write_data_to_socket(server_socket,"exit");
        print_exit(false , "");
        close(server_socket);
        exit(EXIT_SUCCESS);
    }
    else
    {
        write_data_to_socket(server_socket,a.first);
    }
};

//int write_data(int s,const char *buf, int n)
//
//{ int bcount,          /* counts bytes read */
//        br;              /* bytes read this pass */
//
//    bcount= 0;
//    br= 0;
//    while (bcount < n) {             /* loop until full buffer */
//        if ((br= write(s,buf,n-bcount)) > 0)
//        {
//            std::cout << "write_data!\n";
//            bcount += br;                /* increment byte counter */
//            buf += br;                   /* move buffer ptr for next read */
//        }
//        if (br < 0) {
//            perror("write_data");        /* signal an error to the caller */
//            return(-1);
//        }
//    }
//    return(bcount);
//}



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


void handleServerRequest(int server_socket)
{
    std::pair<std::string, int> a = read_data_from_socket(server_socket);

    if (a.second)
    {
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    else
    {
        //todo parse message
    }
}

void main_loop(int server_socket)
{
    int stillRunning = 1;
    while (stillRunning)
    {
        fd_set clientsfds;
        fd_set readfds;

        FD_ZERO(&clientsfds);
        FD_SET(server_socket, &clientsfds);
        FD_SET(STDIN_FILENO, &clientsfds);

        readfds = clientsfds;
        if (select(MAX_CLIENTS+1, &readfds, NULL,NULL, NULL) < 0)
        {
            std::cout << "client terminating on error: " << std::endl << std::flush;
            exit(1);
        }

        if (FD_ISSET(server_socket, &readfds))
        {
            handleServerRequest(server_socket);
            continue;
        }

        if (FD_ISSET(STDIN_FILENO, &readfds))
        {
            clientStdInput(server_socket);
        }
    }
}

bool validate_arguments(int argc, char *argv[])
{
    //input validation
    if ((argc != 4) )
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

    int ret = inet_aton(argv[2] ,nullptr);
    if (ret == 0) {
        print_server_usage();
        exit(1);
    }
}


std::pair<unsigned short,char*> parse_arguments(int argc, char *argv[])
{
    return {(unsigned short)strtol(argv[argc-1], nullptr, 10), argv[2]};
};

void register_with_server(int server_fid, std::string name)
{
    write_data_to_socket(server_fid, name);
    std::pair<std::string,int> answer = read_data_from_socket(server_fid);
    if (answer.second)
    {
        print_fail_connection();
        exit(1);
    }
    else
    {
        if(answer.first == "1")
        {
            print_dup_connection();
            exit(1);
        }
        if(answer.first != "0")
        {
            print_fail_connection();
            exit(1);
        }
        else
        {
            print_connection();
        }
    }

}

int main(int argc, char *argv[])
{
    validate_arguments(argc, argv); // make sure usage is valid
    std::pair<unsigned short,char*> arguments = parse_arguments(argc, argv); // <port num, string of ip>
    //parsing arguments
    std::string client_name (argv[1]);
    char* ip_address = argv[2];
    auto port_num = (unsigned short)strtol(argv[3], nullptr, 10);

    //hostent initialization
    struct hostent *h = gethostbyname(ip_address);
    if (h == nullptr) { print_error("gethostbyname", errno); exit(1);}

    //connect to server socket
    int client_socket_fid = call_socket(h->h_name, port_num);
    if (client_socket_fid < 0){print_error("call_socket", errno); exit(1);}

    register_with_server(client_socket_fid, client_name);
    //send data to server about registration (my client name)
    //handle errors like name in use

    // starting main loop
    main_loop(client_socket_fid);

    return 0;
}