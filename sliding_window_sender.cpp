#include <iostream>
#include "send_receive.h"
#include "message.h"

using namespace std;

int main(int argc, char *argv[])
{
    char buffer[MAXBUFLEN];
    struct Message message1;

    memset(message1, 0, MAXBUFLEN);
    memset(buffer, 0, MAXBUFLEN);

    if (argc != 2)
    {
        printf("usage: sliding_window_sender hostname");
    }

    while (true)
    {
        fgets(buffer, MAXBUFLEN, stdin);

        if ((strlen(buffer) > 0) && (buffer[strlen(buffer) - 1] == '\n'))
            buffer[strlen(buffer) - 1] = '\0';

        int length = strlen(buffer);

        printf("length: %d\n", buffer);

        // decompose length into bytes

        char lenbyte1 = length & 0xff;
        char lenbyte2 = (length >> 8) & 0xff;

        int seqnum = 1;

        // decompose seqnum into its bytes

        char seqbyte1 = seqnum & 0xff;
        char seqbyte2 = (seqnum >> 8) & 0xff;
        char seqbyte3 = (seqnum >> 16) & 0xff;
        char seqbyte4 = (seqnum >> 24) & 0xff;

        message1.seqnum1 = seqbyte1;
        message1.seqnum2 = seqbyte2;
        message1.seqnum3 = seqbyte3;
        message1.seqnum4 = seqbyte4;

        message1.ack = 0x00;

        message1.control = 0x00;

        message1.len1 = lenbyte1;
        message1.len2 = lenbyte2;

        for (int i = 0; i < length; i++)
        {
            (message1.body)[i] = buffer[i];
        }

        (message1.body)[length] = '\0';

        send(argv[1], message1);

        return 0;
    }