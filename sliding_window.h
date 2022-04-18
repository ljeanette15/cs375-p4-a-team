#include <poll.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>

#define TIMEOUT 5
#define SENDERPORT 4950
#define RECEIVERPORT 4951
#define MAXBUFLEN 1032
#define SENDERPORTSTRING "4950"
#define RECEIVERPORTSTRING "4951"

using namespace std;

void send(char *hostname)
{

    // Set up my socket to listen on

    int mysockfd = socket(PF_INET, SOCK_DGRAM, 0); // create a new UDP socket
    if (mysockfd == -1)
    {
        perror("receiver: socket");
        exit(1);
    }

    struct sockaddr_in my_addr;                           // socket address to listen on
    my_addr.sin_family = AF_INET;                         // IPv4
    my_addr.sin_port = htons(SENDERPORT);                 // my port in network byte order
    my_addr.sin_addr.s_addr = INADDR_ANY;                 // bind to local IP address
    memset(my_addr.sin_zero, 0, sizeof my_addr.sin_zero); // zero out sin_zero

    if (bind(mysockfd, (struct sockaddr *)&my_addr, sizeof my_addr) == -1)
    {
        close(mysockfd);
        perror("bind");
        exit(1);
    }

    // Set up other client socket to send to

    struct addrinfo hints, // input to getaddrinfo()
        *server_info;      // head of linked list of results from getaddrinfo()

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;      // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP socket

    int status = getaddrinfo(hostname, RECEIVERPORTSTRING, &hints, &server_info);
    if (status != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(1);
    }

    // loop through all the results and make a socket
    int sockfd;
    struct addrinfo *ptr = server_info; // pointer to current struct addrinfo
    while (ptr != NULL)
    {
        sockfd = socket(ptr->ai_family, ptr->ai_socktype, 0);
        if (sockfd != -1)
        {
            break;
        }
        ptr = ptr->ai_next;
    }

    if (ptr == NULL)
    {
        fprintf(stderr, "sender: failed to create socket\n");
        exit(2);
    }

    // Start Polling

    struct pollfd pfds[2];

    pfds[0].fd = 0; // stdin
    pfds[0].events = POLLIN;
    pfds[1].fd = mysockfd; // Socket of my device to listen for messages on
    pfds[1].events = POLLIN;

    // Send setup message to set initial seqnum

    unsigned char setup_message[MAXBUFLEN];

    memset(setup_message, 0, MAXBUFLEN);

    setup_message[0] = 0x00;            // seqnum1
    setup_message[1] = 0x00;            // seqnum2
    setup_message[2] = 0x00;            // seqnum3
    setup_message[3] = 0x00;            // seqnum4
    setup_message[4] = 0x00;            // ack
    setup_message[5] = 0x01;            // control
    setup_message[6] = 8 & 0xff;        // len1
    setup_message[7] = (8 >> 8) & 0xff; // len2

    srand(time(0));

    // Get random integer between 0 and 9
    int randInt = rand() % 10;
    int numbytes;

    // Randomly choose to not send sometimes
    if (randInt < 7)
    {
        cout << "sent setup message successfully" << endl;
        numbytes = sendto(sockfd, setup_message, 8, 0, ptr->ai_addr, ptr->ai_addrlen);
    }

    // Get time right after message is sent
    time_t now;
    time(&now);

    struct tm *setup_time = localtime(&now);
    int setup_seconds = setup_time->tm_sec;

    struct tm *sent_time;
    int sent_seconds;

    // wait for ack from receiver

    bool setup_ack_received = 0;

    unsigned char buffer[MAXBUFLEN];

    while (setup_ack_received != 1)
    {
        memset(buffer, 0, MAXBUFLEN);

        struct sockaddr_storage sender_addr;     // sender's address (may be IPv6)
        socklen_t addr_len = sizeof sender_addr; // length of this address

        int poll_count = poll(pfds, 2, 5);

        if (poll_count == -1)
        {
            perror("poll");
            exit(1);
        }

        for (int i = 0; i < 2; i++)
        {
            if (pfds[i].revents & POLLIN)
            {
                // Ack from receiver
                if (pfds[i].fd == mysockfd)
                {
                    int numbytes = recvfrom(mysockfd, buffer, MAXBUFLEN - 1, 0, (struct sockaddr *)&sender_addr, &addr_len);
                }
            }
        }

        // If message is setup and an ack:
        if ((buffer[4] == 0x01) && (buffer[5] == 0x01))
        {
            setup_ack_received = 1;
        }

        time(&now);

        struct tm *current_time = localtime(&now);
        int current_seconds = current_time->tm_sec;

        int difference = (current_seconds - setup_seconds < 0) ? current_seconds - setup_seconds + 60 : current_seconds - setup_seconds;

        // Sender timeout
        if (difference > TIMEOUT)
        {
            cout << "setup message timed out... trying again..." << endl;
            if ((rand() % 10) < 8)
            {
                cout << "sent setup message successfully" << endl;
                numbytes = sendto(sockfd, setup_message, 8, 0, ptr->ai_addr, ptr->ai_addrlen);
            }
            time(&now);

            setup_time = localtime(&now);
            setup_seconds = setup_time->tm_sec;
        }
    }

    printf("sender: setup complete. Enter message to send.\n");

    // Listen for inputs on stdin and also listen for acks from other receiver

    int last_seqnum_received = 0;
    int last_seqnum_sent = 0;
    int next_seqnum_to_send = 1;

    int ack_received[2] = {2, 2};

    unsigned char stored_message[MAXBUFLEN];
    char message_from_stdin[MAXBUFLEN];

    int message_from_stdin_len;

    memset(buffer, 0, MAXBUFLEN);
    memset(stored_message, 0, MAXBUFLEN);

    struct sockaddr_storage sender_addr;     // sender's address (may be IPv6)
    socklen_t addr_len = sizeof sender_addr; // length of this address

    while (1)
    {
        int poll_count = poll(pfds, 2, 100);

        if (poll_count == -1)
        {
            perror("poll");
            exit(1);
        }

        for (int i = 0; i < 2; i++)
        {
            if (pfds[i].revents & POLLIN)
            {
                // Ack from receiver
                if (pfds[i].fd == mysockfd)
                {
                    // Receive ack from receiver
                    numbytes = recvfrom(mysockfd, buffer, MAXBUFLEN - 1, 0, (struct sockaddr *)&sender_addr, &addr_len);

                    // Teardown message
                    if (buffer[5] == 0x02)
                    {
                        cout << "shutting down..." << endl;
                        break;
                    }

                    // Get sequence number of ack
                    last_seqnum_received = buffer[0] + (buffer[1] << 8) + (buffer[2] << 16) + (buffer[3] << 24);

                    // If sequence number received matches sequence number that was sent last
                    if (last_seqnum_sent == last_seqnum_received)
                    {
                        ack_received[last_seqnum_received] = 1;
                        next_seqnum_to_send = (last_seqnum_sent + 1) % 2;
                        memset(stored_message, 0, MAXBUFLEN);
                    }
                    else
                    {
                        cout << "seqnums don't match" << endl;
                    }

                    printf("ack received from receiver\n");
                }

                // Message from standard input
                else if (pfds[i].fd == 0)
                {
                    memset(buffer, 0, MAXBUFLEN);
                    memset(message_from_stdin, 0, MAXBUFLEN);

                    // get message from standard input

                    fgets(message_from_stdin, MAXBUFLEN, stdin);

                    message_from_stdin_len = strlen(message_from_stdin);

                    if ((strlen(message_from_stdin) > 0) && (message_from_stdin[strlen(message_from_stdin) - 1] == '\n'))
                        message_from_stdin[strlen(message_from_stdin) - 1] = '\0';

                    // Fill in byte array

                    buffer[0] = next_seqnum_to_send & 0xff;
                    buffer[1] = (next_seqnum_to_send >> 8) & 0xff;
                    buffer[2] = (next_seqnum_to_send >> 16) & 0xff;
                    buffer[3] = (next_seqnum_to_send >> 24) & 0xff;
                    buffer[4] = 0x00;
                    buffer[5] = 0x00;
                    buffer[6] = (message_from_stdin_len - 1) & 0xff;
                    buffer[7] = ((message_from_stdin_len - 1) >> 8) & 0xff;

                    for (int i = 0; i < message_from_stdin_len; i++)
                    {
                        buffer[i + 8] = message_from_stdin[i];
                    }

                    if ((rand() % 10) < 8)
                    {
                        numbytes = sendto(sockfd, buffer, message_from_stdin_len + 8, 0, ptr->ai_addr, ptr->ai_addrlen);
                        cout << "sent message with length:  " << buffer[6] + (buffer[7] << 8) << endl;
                    }

                    time_t now;
                    time(&now);

                    sent_time = localtime(&now);
                    sent_seconds = sent_time->tm_sec;

                    ack_received[next_seqnum_to_send] = 0;

                    memcpy(stored_message, buffer, MAXBUFLEN);

                    last_seqnum_sent = next_seqnum_to_send;
                }
            }
        }

        // Get current time
        time(&now);

        struct tm *current_time = localtime(&now);
        int current_seconds = current_time->tm_sec;

        int difference = (current_seconds - sent_seconds < 0) ? current_seconds - sent_seconds + 60 : current_seconds - sent_seconds;

        // See how long it has been since last message was sent
        if ((difference > TIMEOUT) && (ack_received[last_seqnum_sent] == 0))
        {
            cout << "sender: timeout" << endl;

            if ((rand() % 10) < 8)
            {
                numbytes = sendto(sockfd, stored_message, stored_message[6] + (stored_message[7] << 8), 0, ptr->ai_addr, ptr->ai_addrlen);
                cout << "resent message with length: " << stored_message[6] + (stored_message[7] << 8) << endl;
            }

            time(&now);

            sent_time = localtime(&now);
            sent_seconds = sent_time->tm_sec;
        }
    }
}

/**************************************************************************
 *
 *  Function: Receive
 *
 ***************************************************************************/

void receive(char *hostname)
{
    // Set up my socket to listen on

    int mysockfd = socket(PF_INET, SOCK_DGRAM, 0); // create a new UDP socket
    if (mysockfd == -1)
    {
        perror("receiver: socket");
        exit(1);
    }

    struct sockaddr_in my_addr;                           // socket address to listen on
    my_addr.sin_family = AF_INET;                         // IPv4
    my_addr.sin_port = htons(RECEIVERPORT);               // my port in network byte order
    my_addr.sin_addr.s_addr = INADDR_ANY;                 // bind to local IP address
    memset(my_addr.sin_zero, 0, sizeof my_addr.sin_zero); // zero out sin_zero

    if (bind(mysockfd, (struct sockaddr *)&my_addr, sizeof my_addr) == -1)
    {
        close(mysockfd);
        perror("bind");
        exit(1);
    }

    // Set up other client socket to send to

    struct addrinfo hints, // input to getaddrinfo()
        *server_info;      // head of linked list of results from getaddrinfo()

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;      // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP socket

    int status = getaddrinfo(hostname, SENDERPORTSTRING, &hints, &server_info);
    if (status != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(1);
    }

    // loop through all the results and make a socket
    int sockfd;
    struct addrinfo *ptr = server_info; // pointer to current struct addrinfo
    while (ptr != NULL)
    {
        sockfd = socket(ptr->ai_family, ptr->ai_socktype, 0);
        if (sockfd != -1)
        {
            break;
        }
        ptr = ptr->ai_next;
    }

    if (ptr == NULL)
    {
        fprintf(stderr, "receiver: failed to create socket\n");
        exit(2);
    }

    // Start Polling

    struct pollfd pfds[2];

    pfds[0].fd = mysockfd; // stdin
    pfds[0].events = POLLIN;

    char buffer[MAXBUFLEN];
    unsigned char ack[MAXBUFLEN];

    int last_seqnum_sent;
    int next_seqnum_to_send;

    struct sockaddr_storage sender_addr;     // sender's address (may be IPv6)
    socklen_t addr_len = sizeof sender_addr; // length of this address

    bool setup_ack_sent = 0;

    while (setup_ack_sent != 1)
    {
        memset(buffer, 0, MAXBUFLEN);

        int poll_count = poll(pfds, 1, -1);

        if (poll_count == -1)
        {
            perror("poll");
            exit(1);
        }

        if (pfds[0].revents & POLLIN)
        {
            // Ack from receiver
            if (pfds[0].fd == mysockfd)
            {
                int numbytes = recvfrom(mysockfd, buffer, MAXBUFLEN - 1, 0, (struct sockaddr *)&sender_addr, &addr_len);

                if (buffer[5] != 0x01)
                {
                    perror("not setup message");
                    exit(1);
                }

                char setup_ack[MAXBUFLEN];

                setup_ack[0] = buffer[0]; // Memcopy
                setup_ack[1] = buffer[1];
                setup_ack[2] = buffer[2];
                setup_ack[3] = buffer[3];
                setup_ack[4] = 0x01;
                setup_ack[5] = buffer[5];
                setup_ack[6] = buffer[6];
                setup_ack[7] = buffer[7];

                last_seqnum_sent = setup_ack[0] + (setup_ack[1] << 8) + (setup_ack[2] << 16) + (setup_ack[3] << 24);

                int numbytes1 = sendto(sockfd, setup_ack, 8, 0, ptr->ai_addr, ptr->ai_addrlen);

                setup_ack_sent = 1;
            }
        }
    }

    printf("receiver: setup complete\n");

    // Receive messages and send acks until teardown message is received
    while (1)
    {
        int poll_count = poll(pfds, 1, 5);

        if (poll_count == -1)
        {
            perror("poll");
            exit(1);
        }

        if (pfds[0].revents & POLLIN)
        {
            // message from sender
            if (pfds[0].fd == mysockfd)
            {
                cout << "message received from sender with length ";
                // get sequence number then send back an ack
                int numbytes = recvfrom(mysockfd, buffer, MAXBUFLEN - 1, 0, (struct sockaddr *)&sender_addr, &addr_len);

                cout << buffer[6] + (buffer[7] << 8) << ": ";

                for (int i = 0; i < (buffer[6] + (buffer[7] << 8)); i++)
                {
                    cout << buffer[i + 8];
                }

                cout << "\n";

                ack[0] = buffer[0];
                ack[1] = buffer[1];
                ack[2] = buffer[2];
                ack[3] = buffer[3];
                ack[4] = 0x01;
                ack[5] = 0x00;
                ack[6] = 8 & 0xff;
                ack[7] = (8 >> 8) & 0xff;

                int numbytes1 = sendto(sockfd, ack, 8, 0, ptr->ai_addr, ptr->ai_addrlen);

                next_seqnum_to_send = (last_seqnum_sent + 1) % 2;

                cout << "ack sent to sender" << endl;
            }
        }
    }
}
