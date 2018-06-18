//
// Created by danielbraun on 6/18/18.
//

#ifndef OS_EXERCISE_4_CLIENT_SERVER_WHATSAPP_CLONE_EX4_UTILS_H
#define OS_EXERCISE_4_CLIENT_SERVER_WHATSAPP_CLONE_EX4_UTILS_H
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


std::pair<std::string,int> read_data_from_socket(int fid);
int write_data_to_socket(int fid, std::string input_str);

#endif //OS_EXERCISE_4_CLIENT_SERVER_WHATSAPP_CLONE_EX4_UTILS_H
