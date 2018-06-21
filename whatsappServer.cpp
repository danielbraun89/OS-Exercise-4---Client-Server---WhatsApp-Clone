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
#include<set>
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
        return EXIT_FAILURE;
    }
    else
    {
        std::map<std::string, int> ::iterator it = client_maps->find(a.first);
        if(it != client_maps->end())
        {
            //element found;
            write_data_to_socket(new_client_socket_fid, "1");
            close(new_client_socket_fid);
            return EXIT_FAILURE;

        }
        else
        {
            client_maps->insert({ a.first, new_client_socket_fid});
            print_connection_server(a.first);
            write_data_to_socket(new_client_socket_fid, "0");
            return EXIT_SUCCESS;
        }
    }

};

void serverStdInput(std::map<std::string, int>  * client_maps){
    std::pair<std::string,int> a = read_data_from_socket(STDIN_FILENO);
//    std::cout << "read from stdin: " << message_buffer << " with len: " << str1.length() << std::endl;
    std::string str2 ("EXIT");
    if ((a.first.compare(str2)) == 0)
    {
        print_exit();
        close_all_sockets(client_maps);
        exit(EXIT_SUCCESS);
    }
    else
    {
        print_invalid_input();
    }

};

int handleSend(std::string& sender_name, std::string& message, std::string& recieving_name, std::map<std::string, int>  * client_map,
                std::map<std::string, std::set<std::string>> * group_map)
{
    int sending_fid = client_map->operator[](sender_name);

    // check if recieving name is valid
    if (sender_name == recieving_name)
    {
        print_send(true, false, sender_name, recieving_name, message);
        write_data_to_socket(sending_fid, "1");
        return EXIT_FAILURE;
    }

    if (client_map->find(recieving_name) !=  client_map->end())
    {
        int recieving_fid = client_map->operator[](recieving_name);
        //sending the message
        int rt = write_data_to_socket(recieving_fid, sender_name + std::string(": ") +message);
        //check success
        if (rt == -1)
        {
            print_send(true, false, sender_name, recieving_name, message);
            write_data_to_socket(sending_fid, "1");
            return EXIT_FAILURE;
        }
        else
        {
            print_send(true, true, sender_name, recieving_name, message);
            write_data_to_socket(sending_fid, "0");
            return EXIT_SUCCESS;
        }
    }

    bool success = false;
    //check if the group exists
    if (group_map->find(recieving_name) != group_map->end())
    {
        //check if sender is inside the group
        if (group_map->at(recieving_name).find(sender_name) != group_map->at(recieving_name).end())
        {
            for (auto const& x : group_map->at(recieving_name))
            {
                //make sure you dont send to the sender as well
                if (x != sender_name)
                {
                    write_data_to_socket(client_map->operator[](x), sender_name + std::string(": ") + message);
                    success = true;
                }
            }
        }
    }
    //print and send the correct feedback
    if (success)
    {
        print_send(true, true, sender_name, recieving_name, message);
        write_data_to_socket(sending_fid, "0");
        return EXIT_SUCCESS;
    }
    else
    {
        print_send(true, false, sender_name, recieving_name, message);
        write_data_to_socket(sending_fid, "1");
        return EXIT_FAILURE;
    }
}


int handleCreateGroup(std::string& sender_name, std::vector<std::string>& clients_to_include, std::string& group_name, std::map<std::string, int>  * client_map,
                       std::map<std::string, std::set<std::string>> * group_map)
{
    int sending_fid = client_map->operator[](sender_name);
    // check if group name is not taken as client name
    if (client_map->find(group_name) != client_map->end())
    {
        print_create_group(true, false, sender_name, group_name);
        write_data_to_socket(sending_fid, "1");
        return EXIT_FAILURE;
    }
    // check if group name is not taken as other group name
    if (group_map->find(group_name) != group_map->end())
    {
        print_create_group(true, false, sender_name, group_name);
        write_data_to_socket(sending_fid, "1");
        return EXIT_FAILURE;
    }

    //make a set out of everyone
    std::set<std::string> group_client_set;
    for (auto const& x : clients_to_include)
    {
        group_client_set.insert(x);
    }
    group_client_set.insert(sender_name);
    //make sure there are at least 2 clients
    if (group_client_set.size() < 2)
    {
        print_create_group(true, false, sender_name, group_name);
        write_data_to_socket(sending_fid, "1");
        return EXIT_FAILURE;
    }
    // make sure they all valid clients
    for (auto const& x : group_client_set)
    {
        if (client_map->find(x) == client_map->end())
        {
            print_create_group(true, false, sender_name, group_name);
            write_data_to_socket(sending_fid, "1");
            return EXIT_FAILURE;
        }
    }

    group_map->insert({group_name, group_client_set});
    print_create_group(true, true, sender_name, group_name);
    write_data_to_socket(sending_fid, "0");
    return EXIT_FAILURE;
}

void handleClientRequest(std::string client_name, std::map<std::string, int>  * client_map, std::map<std::string, std::set<std::string>> *group_map)
{
    int fid = client_map->operator[](client_name);
    std::pair<std::string,int> a = read_data_from_socket(fid);
    if (a.second)
    {
//        std::cout << "server recognized disconnection \n";
        terminateClient(client_name, client_map);
    }
    else
    {
        command_type cur_command_type;
        std::string cur_message;
        std::string name;
        std::vector<std::string> clients;
        parse_command(a.first, cur_command_type, name, cur_message, clients);

        if (cur_command_type == EXIT)
        {
            print_exit(true, client_name);
            terminateClient(client_name, client_map);
        }
        else if (cur_command_type == SEND)
        {
            handleSend(client_name, cur_message, name, client_map, group_map);
        }
        else if (cur_command_type == CREATE_GROUP)
        {
            handleCreateGroup(client_name, clients, name, client_map, group_map);
        }
        else if (cur_command_type == WHO)
        {
            std::string client_str;
            for (auto const& x : *client_map)
            {
                client_str += x.first + ",";
            }
            write_data_to_socket(fid, client_str);
            print_who_server(client_name);
        }
    }
};

void main_loop(int listener_socket, std::map<std::string, int> * client_map, std::map<std::string, std::set<std::string>> *group_map )
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
                handleClientRequest(client_name, client_map, group_map);
            }
        }
    }
}

void validate_arguments(int argc, char *argv[])
{
    //input validation
    if ((argc != 2) )
    {
        print_server_usage();
        exit(1);
    }
    char* p;
    strtol(argv[argc-1], &p, 10);
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
    std::map<std::string, std::set<std::string>> group_map;
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

    /* create socket */
    int server_socket_fid = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_fid < 0) {print_error("socket", errno); exit(1);}
    //bind it
    int bind_return = bind(server_socket_fid , (struct sockaddr *)&server_sockaddr_in , sizeof(struct sockaddr_in));
    if (bind_return < 0){print_error("bind", errno); exit(1);}

    int listen_return = listen(server_socket_fid, MAX_PENDING_CONNECTIONS);
    if (listen_return < 0){print_error("listen", errno); exit(1);}

    main_loop(server_socket_fid, &client_map, &group_map);
    return 0;
}

