#include "sliding_window.h"

int main(int argc, char* argv[])
{

    if (argc != 2)
    {
        perror("usage: sliding_window hostname username");
    }

    // Set up connection

    char setup_message[MAXBUFLEN];
    memset(setup_message, 0, MAXBUFLEN);

    user_len = strlen(argv[2]);

    setup_message[0] = 0x00; //seqnum1
    setup_message[1] = 0x00; //seqnum2
    setup_message[2] = 0x00; //seqnum3
    setup_message[3] = 0x00; //seqnum4
    setup_message[4] = 0x00; //control
    setup_message[5] = 0x00; //ack
    setup_message[6] = user_len & 0xff; //len1
    setup_message[7] = (user_len >> 8) & 0xff; //len2

    for (int i = 0; i < user_len); i++)
    {
        setup_message[i + 8] = argv[2][i];
    }

    send(argv[1], setup_message);

    // wait for setup ack

    bool setup_ack_received = 0;
    bool setup_message_received = 0;

    char other_client[MAXBUFLEN];
    unsigned char buffer[MAXBUFLEN];

    memset(other_client, 0, MAXBUFLEN);

    while(setup_ack_received && setup_message_received != 1)
    {
        memset(buffer, 0, MAXBUFLEN);

        receive(buffer);

        // If message is setup and an ack:
        if ((buffer[4] == 0x00) && (buffer[5] == 0x01))
        {
            setup_ack_received = 1;
        }

        // If message is setup and a normal message
        if ((buffer[4] == 0x00) && (buffer[5] == 0x00))
        {
            int username_length = buffer[6] + (buffer[7] << 8);
            
            for (int i = 0; i < username_length; i++)
            {
                other_client[i] = buffer[i + 8];
            }

            setup_message_received = 1;
        }
    }

    printf("setup complete\n");

    // Listen for inputs on stdin and also listen for messages from other client

    pfds[0].fd = 0; //stdin
    pfds[0].events = POLLIN; 
    pfds[1].fd = mysockfd; // Socket of my device to listen for messages on
    pfds[1].events = POLLIN;


    
}
