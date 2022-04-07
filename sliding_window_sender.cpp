#include <iostream>
#include "send_receive.h"


using namespace std;

int main(int argc, char *argv[])
{
    char buffer[MAXBUFLEN];
    char message[MAXBUFLEN];

    memset(message, 0, MAXBUFLEN);
    
    if (argc != 2)
    {
        printf("usage: sliding_window_sender hostname");
    }

    while (true)
    {
        fgets(message, MAXBUFLEN, stdin);

        if ((strlen(message) > 0) && (message[strlen (message) - 1] == '\n'))
                        message[strlen (message) - 1] = '\0';
        
        int length = strlen(message);

        printf("length: %d\n", length);

        //decompose length into bytes

        char lenbyte1 = length & 0xff;
        char lenbyte2 = (length >> 8) & 0xff;

        int seqnum = 1;

        // decompose seqnum into its bytes

        char seqbyte1 = seqnum & 0xff;
        char seqbyte2 = (seqnum >> 8) & 0xff;
        char seqbyte3 = (seqnum >> 16) & 0xff;
        char seqbyte4 = (seqnum >> 24) & 0xff;

        buffer[0] = 0x00;
        buffer[1] = 0x00;
        buffer[2] = 0x00;
        buffer[3] = 0x00;

        buffer[4] = 0x00;

        buffer[5] = 0x00;

        buffer[6] = 0x00;
        buffer[7] = 0x00;

        for (int i = 0; i < length; i++)
        {
            buffer[i + 8] = message[i];
        }

        buffer[length + 8] = '\0';

        send(argv[1], buffer);

        cout << "sent buffer with last byte of %s\n" << buffer[length + 7] << endl;

    }



    return 0;
}