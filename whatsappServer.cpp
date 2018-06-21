//
// Created by danielbraun on 6/18/18.
//

#include "whatsappio.h"
#include "ex4_utils.h"

#define MAXHOSTNAME 30
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
#include <map>
int server_socket_fid = -1;
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
            return(EXIT_FAILURE);
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


void debug_print_clients_map( std::map<std::string, int> * client_maps)
{
    for (std::map<std::string, int>::iterator it=client_maps->begin(); it!=client_maps->end(); ++it)
        std::cout << it->first << " => " << it->second << '\n';
}

void terminateClient(std::string client_name, std::map<std::string, int>  * client_maps)
{
    int client_fid = client_maps->at(client_name);
    for( std::map<std::string, int>::iterator i = client_maps->begin(); i != client_maps->end(); ) {
        if( i->second == client_fid )
        {
            client_maps->erase( i++ ); // advance before iterator become invalid
        }
        else
        {
            ++i;
        }
    }
    close(client_fid);
}

void close_all_sockets(std::map<std::string, int> *client_maps)
{
    for (auto const& x : *client_maps)
    {
        close(x.second);
    }
    close(server_socket_fid);
};

int connectNewClient(int server_socket_fid, std::map<std::string, int>  * client_maps ){
    int new_client_socket_fid = accept(server_socket_fid,NULL,NULL);
    if (new_client_socket_fid < 0)
    {
        print_fail_connection();
        return EXIT_FAILURE;
    }
    std::pair<std::string,int> a = read_data_from_socket(new_client_socket_fid);
    if (a.second)
    {
        close(new_client_socket_fid);
    }
    else
    {
        std::map<std::string, int> ::iterator it = client_maps->find(a.first);
        if(it != client_maps->end())
        {
            //element found;
            write_data_to_socket(new_client_socket_fid, "1");
            close(new_client_socket_fid);
        }
        else
        {
            client_maps->insert({ a.first, new_client_socket_fid});
            print_connection_server(a.first);
            write_data_to_socket(new_client_socket_fid, "0");
        }
    }

};

void serverStdInput(std::map<std::string, int>  * client_maps){
    std::pair<std::string,int> a = read_data_from_socket(STDIN_FILENO);
//    std::cout << "read from stdin: " << message_buffer << " with len: " << str1.length() << std::endl;
    std::string str2 ("EXIT\n");

    if ((a.first.compare(str2)) == 0)
    {
//        for (auto const& x : *client_maps)
//        {
//            int client_fid = x.second;
//            write_data_to_socket(client_fid, "EXIT");
//        }
        print_exit();
        close_all_sockets(client_maps);
        exit(EXIT_SUCCESS);
    }

};


void handleClientRequest(std::string client_name, std::map<std::string, int>  * client_map)
{
    int fid = client_map->at(client_name);
    std::pair<std::string,int> a = read_data_from_socket(fid);
    if (a.second)
    {
        std::cout << "server recognized disconnection \n";
        terminateClient(client_name, client_map);
    }
    else
    {
        if (a.first == "exit")
        {
            print_exit(true, client_name);
            terminateClient(client_name, client_map);
        }
        else
        {
            std::string aa = a.first;
            std::cout << "server got: " << std::flush;
            std::cout << aa << std::endl << std::flush;
        }

    }
};

void main_loop(int listener_socket, std::map<std::string, int> * client_map)
{
    int stillRunning = 1;
    while (stillRunning)
    {
        fd_set clientsfds;
        fd_set readfds;

        FD_ZERO(&clientsfds);
        FD_SET(listener_socket, &clientsfds);
        FD_SET(STDIN_FILENO, &clientsfds);

        for (auto const& x : *client_map)
        {
            FD_SET(x.second, &clientsfds);
        }
        readfds = clientsfds;
        if (select(MAX_CLIENTS+1, &readfds, NULL,NULL, NULL) < 0)
        {
            close_all_sockets(client_map);
            exit(1);
        }

        if (FD_ISSET(listener_socket, &readfds))
        {
            //will also add the client to the clientsfds
            connectNewClient(listener_socket, client_map);
            continue;
        }

        if (FD_ISSET(STDIN_FILENO, &readfds))
        {
            serverStdInput(client_map);
        }

        for (auto const& x : *client_map)
        {
            int client_fid = x.second;
            std::string client_name = x.first;
            if(FD_ISSET(client_fid, &readfds))
            {
                handleClientRequest(client_name, client_map);
            }
        }
    }
}

bool validate_arguments(int argc, char *argv[])
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
}

uint16_t parse_arguments(int argc, char *argv[])
{
    char* p;
    long converted = strtol(argv[argc-1], &p, 10);
    return (uint16_t)converted;
}


int main(int argc, char *argv[])
{
    validate_arguments(argc, argv);  // make sure usage is valid
    uint16_t portnum = parse_arguments(argc, argv); // port num
    std::map<std::string, int> client_map;  // dynamic map to keep track of clients num and name

    //hostent initialization
    struct hostent* hp;
    char myname[MAXHOSTNAME+1];
    gethostname(myname, MAXHOSTNAME);
    hp = gethostbyname(myname);
    if (hp == nullptr) { print_error("gethostbyname", errno); exit(1); }

    //sockaddrr_in initlization
    struct sockaddr_in server_sockaddr_in;
    memset(&server_sockaddr_in, 0, sizeof(struct sockaddr_in));
    server_sockaddr_in.sin_family = (sa_family_t)hp->h_addrtype;
    memcpy(&server_sockaddr_in.sin_addr, hp->h_addr, (size_t)hp->h_length);
    server_sockaddr_in.sin_port= htons(portnum);
    server_sockaddr_in.sin_addr.s_addr = INADDR_ANY;

    std::cout << "server_socket_address_object initialized, with address: "<< inet_ntoa(server_sockaddr_in.sin_addr)<< " port: "<< ntohs(server_sockaddr_in.sin_port)  << std::endl;

    /* create socket */
    int server_socket_fid = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_fid < 0) {print_error("socket", errno); exit(1);}
    //bind it
    int bind_return = bind(server_socket_fid , (struct sockaddr *)&server_sockaddr_in , sizeof(struct sockaddr_in));
    if (bind_return < 0){print_error("bind", errno); exit(1);}

    int listen_return = listen(server_socket_fid, MAX_PENDING_CONNECTIONS);
    if (listen_return < 0){print_error("listen", errno); exit(1);}

    main_loop(server_socket_fid, &client_map);
    return 0;
}

