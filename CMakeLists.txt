cmake_minimum_required(VERSION 3.8)
project(OS-Exercise-4---Client-Server---WhatsApp-Clone)

set(CMAKE_CXX_STANDARD 11)
find_package (Threads)
set(SOURCE_FILES whatsappio.cpp whatsappio.h ex4_utils.cpp ex4_utils.h whatsappServer.cpp)
#whatsappClient.cpp
#whatsappServer.cpp
#sineen 132.65.126.21
add_executable(OS-Exercise-4---Client-Server---WhatsApp-Clone ${SOURCE_FILES})
target_link_libraries (OS-Exercise-4---Client-Server---WhatsApp-Clone ${CMAKE_THREAD_LIBS_INIT})
