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
#include <sstream>

char message_buffer[WA_MAX_MESSAGE];

void clientStdInput(int server_socket, std::string client_name)
{
    std::pair<std::string,int> a = read_data_from_socket(STDIN_FILENO);
    std::string str2 ("exit");
    if ((a.first.compare(str2)) == 0)
    {
        write_data_to_socket(server_socket,"exit");
        print_exit(false , "");
        close(server_socket);
        exit(EXIT_SUCCESS);
    }
    else
    {
        command_type cur_command_type;
        std::string cur_message;
        std::string cur_send_to_name;
        std::vector<std::string> clients;
        parse_command(a.first, cur_command_type, cur_send_to_name, cur_message, clients);


        if (cur_command_type==SEND)
        {
            write_data_to_socket(server_socket, a.first);
            std::pair<std::string, int> rt = read_data_from_socket(server_socket);
            if (rt.second)
            {
                print_send(false, false, client_name, cur_send_to_name, cur_message);
            }
            else
            {
                if (rt.first == "1")
                {
                    print_send(false, false, client_name, cur_send_to_name, cur_message);
                }
                else if (rt.first == "0")
                {
                    print_send(false, true, client_name, cur_send_to_name, cur_message);
                }
            }
        }
        else if (cur_command_type==CREATE_GROUP)
        {
            write_data_to_socket(server_socket, a.first);
            std::pair<std::string, int> rt = read_data_from_socket(server_socket);
            if (rt.second)
            {
                print_create_group(false, false, client_name, cur_send_to_name);
            }
            else
            {
                if (rt.first == "1")
                {
                    print_create_group(false, false, client_name, cur_send_to_name);
                }
                else if (rt.first == "0")
                {
                    print_create_group(false, true, client_name, cur_send_to_name);
                }
            }
        }
        else if (cur_command_type==INVALID)
        {
            print_invalid_input();
        }
        else if (cur_command_type==WHO)
        {
            write_data_to_socket(server_socket, a.first);
            std::pair<std::string, int> rt = read_data_from_socket(server_socket);
            if (rt.second)
            {
                std::vector<std::string> aa;
                print_who_client(false, aa);
            }
            else
            {
                if (rt.first == "1")
                {
                    std::vector<std::string> aa;
                    print_who_client(false, aa);
                }
                else
                {
                    std::stringstream ss(rt.first);
                    std::vector<std::string> result;
                    while( ss.good() )
                    {
                        std::string substr;
                        getline( ss, substr, ',' );
                        if (!(substr.empty()))
                        {
                            result.push_back( substr );
                        }

                    }
                    print_who_client(true, result);
                }
            }
//            void print_who_client(bool success, const std::vector<std::string>& clients);

        }
    }
};




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

//    std::cout << "server_socket_address_object initialized, with address: "<< inet_ntoa(server_socket_address_object.sin_addr)<< " port: "<< ntohs(server_socket_address_object.sin_port)  << std::endl;

    if ((client_socket_file_descriptor = socket(hp->h_addrtype, SOCK_STREAM, 0)) < 0) {
        return(-1);
    }

    if (connect(client_socket_file_descriptor, (struct sockaddr *)&server_socket_address_object , sizeof(server_socket_address_object)) < 0)
    {
        close(client_socket_file_descriptor);
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
        std::cout << a.first << std::endl;
    }
}

void main_loop(int server_socket, std::string client_name)
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
            print_fail_connection();
            exit(1);
        }

        if (FD_ISSET(server_socket, &readfds))
        {
            handleServerRequest(server_socket);
            continue;
        }

        if (FD_ISSET(STDIN_FILENO, &readfds))
        {
            clientStdInput(server_socket, client_name);
        }
    }
}

void validate_arguments(int argc, char *argv[])
{
    //input validation
    if ((argc != 4) )
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

    int ret = inet_aton(argv[2] ,nullptr);
    if (ret == 0) {
        print_server_usage();
        exit(1);
    }
}



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
    //parsing arguments
    std::string client_name (argv[1]);
    char* ip_address = argv[2];
    auto port_num = (unsigned short)strtol(argv[3], nullptr, 10);

    //hostent initialization
    struct hostent *h = gethostbyname(ip_address);
    if (h == nullptr) { print_fail_connection(); exit(1);}

    //connect to server socket
    int client_socket_fid = call_socket(h->h_name, port_num);
    if (client_socket_fid < 0){print_error("call_socket", errno); exit(1);}

    register_with_server(client_socket_fid, client_name);
    //send data to server about registration (my client name)
    //handle errors like name in use

    // starting main loop
    main_loop(client_socket_fid, client_name);

    return 0;
}