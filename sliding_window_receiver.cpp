#include <iostream>
#include "send_receive.h"

#define MAXBUFLEN 100

using namespace std;

int main()
{
    char buffer[MAXBUFLEN];
    char message[MAXBUFLEN];

    while(true)
    {
        memset(buffer, 0, MAXBUFLEN);
        memset(message, 0, MAXBUFLEN);

        receive(buffer);

        char seqnum1 = buffer[0];
        char seqnum2 = buffer[1];
        char seqnum3 = buffer[2];
        char seqnum4 = buffer[3];

        char ack = buffer[4];

        char control = buffer[5];

        char lenbyte1 = buffer[6];
        char lenbyte2 = buffer[7];

        int length = lenbyte1 + (lenbyte2 << 8);

        for (int i = 0; i < 13; i++)
        {
            cout << buffer[i] << endl;
            message[i] = buffer[i];
        }

        printf("Message: %s\n", message);
    }
}