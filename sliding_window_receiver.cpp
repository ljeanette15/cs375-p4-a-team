#include <iostream>
#include "send_receive.h"

#define MAXBUFLEN 100

using namespace std;

int main()
{

    unsigned char buffer[MAXBUFLEN];

    // receive message and store in buffer
    receive(buffer);

    // separate bytes of the buffer into their corresponding variables
    int length = buffer[6] + (buffer[7] << 8);
    int seqnum = buffer[0] + (buffer[1] << 8) + (buffer[2] << 16) + (buffer[3] << 24);
    char ack = buffer[4];
    char control = buffer[5];

    // print message and seqnum and length
    for (int i = 0; i < length; i++)
    {
        printf("%c", buffer[i + 8]);
    }
    printf("\n");

    printf("seqnum received: %d\n", seqnum);
    printf("length received: %d\n", length);
}