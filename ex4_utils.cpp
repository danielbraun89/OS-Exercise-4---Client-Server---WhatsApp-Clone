//
// Created by danielbraun on 6/18/18.
//

#include "ex4_utils.h"
#include "whatsappio.h"


std::pair<std::string,int> read_data_from_socket(int fid)
{
    char message_buffer[WA_MAX_MESSAGE + 1];
    bzero(message_buffer,WA_MAX_MESSAGE +1);
    int byteCount = 0;
    int byteRead = 0;
    char* buf = message_buffer;
    while (byteCount < WA_MAX_MESSAGE)
    { /* loop until full buffer */
        byteRead = (int) read(fid, buf, (WA_MAX_MESSAGE - byteCount));
        if (byteRead > 0)
        {
            byteCount += byteRead;
            buf += byteRead;
        }
        if (byteRead < 1)
        {
            return {std::string (""), 1};
        }
        if (message_buffer[byteCount] =='\0')
        {
            break;
        }
    }
    message_buffer[byteCount+1] = '\0';
    return {std::string (message_buffer), 0};
}



int write_data_to_socket(int fid, std::string input_str)
{
    const char *buf = input_str.c_str();
    auto n = (int)input_str.length();
    std::cout << "writing data\n";
    int bcount,          /* counts bytes read */
            br;          /* bytes read this pass */

    bcount= 0;
    br= 0;
    while (bcount < n) {             /* loop until full buffer */
        if ((br = write(fid,buf,n-bcount)) > 0)
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