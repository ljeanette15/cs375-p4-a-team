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

#define TIMEOUT 10
#define MYPORT 4950
#define MAXBUFLEN 1032
#define MYPORTSTRING "4950"

using namespace std;

void send(char* hostname)
{

    // Set up my socket to listen on

	int mysockfd = socket(PF_INET, SOCK_DGRAM, 0); // create a new UDP socket
	if (mysockfd == -1)
	{
		perror("receiver: socket");
		exit(1);
	}

	struct sockaddr_in my_addr;							  // socket address to listen on
	my_addr.sin_family = AF_INET;						  // IPv4
	my_addr.sin_port = htons(MYPORT);					  // my port in network byte order
	my_addr.sin_addr.s_addr = INADDR_ANY;				  // bind to local IP address
	memset(my_addr.sin_zero, 0, sizeof my_addr.sin_zero); // zero out sin_zero

	if (bind(mysockfd, (struct sockaddr *)&my_addr, sizeof my_addr) == -1)
	{
		close(mysockfd);
		perror("bind");
		exit(1);
	}


    // Set up other client socket to send to

    struct addrinfo hints,     // input to getaddrinfo()
	*server_info;	           // head of linked list of results from getaddrinfo()

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;		// IPv4
	hints.ai_socktype = SOCK_DGRAM; // UDP socket

	int status = getaddrinfo(hostname, MYPORTSTRING, &hints, &server_info);
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

    pfds[0].fd = 0; //stdin
    pfds[0].events = POLLIN; 
    pfds[1].fd = mysockfd; // Socket of my device to listen for messages on
    pfds[1].events = POLLIN;


    // Send setup message to set initial seqnum

    unsigned char setup_message[MAXBUFLEN];

    memset(setup_message, 0, MAXBUFLEN);

    setup_message[0] = 0x00; //seqnum1
    setup_message[1] = 0x00; //seqnum2
    setup_message[2] = 0x00; //seqnum3
    setup_message[3] = 0x00; //seqnum4
    setup_message[4] = 0x00; //ack
    setup_message[5] = 0x01; //control
    setup_message[6] = 8 & 0xff; //len1
    setup_message[7] = (8 >> 8) & 0xff; //len2

    int randInt = rand() % 10;

    int numbytes = sendto(sockfd, setup_message, 8, 0, ptr->ai_addr, ptr->ai_addrlen);

    // Get time right after message is sent
    time_t now;
    time(&now);

    struct tm *local1 = localtime(&now);
    int seconds1 = local1->tm_sec;

    // wait for ack from receiver

    bool setup_ack_received = 0;

    unsigned char buffer[MAXBUFLEN];

    while(setup_ack_received != 1)
    {
        memset(buffer, 0, MAXBUFLEN);

        struct sockaddr_storage sender_addr;	 // sender's address (may be IPv6)
        socklen_t addr_len = sizeof sender_addr; // length of this address

        cout << "before poll count" << endl;
        int poll_count = poll(pfds, 2, 10); 
        cout << "after poll count" << endl;

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
                    cout << "entered where it shouldn't" << endl;
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

        struct tm *local2 = localtime(&now);
        int seconds2 = local2->tm_sec;

        int difference = (seconds2 - seconds1 < 0) ? seconds2 - seconds1 + 60 : seconds2 - seconds1; 

        cout << "difference: " << difference << endl;

        if (difference > TIMEOUT)
        {
            int numbytes = sendto(sockfd, setup_message, 8, 0, ptr->ai_addr, ptr->ai_addrlen);
            
            time(&now);

            struct tm *local1 = localtime(&now);
            seconds1 = local1->tm_sec;
        }
        
    }

    printf("setup complete\n");

    // Listen for inputs on stdin and also listen for acks from other receiver

    while (1)
    {
        int poll_count = poll(pfds, 2, -1); 

        if (poll_count == -1)
        {
            perror("poll");
            exit(1);
        }

        for (int i = 0; i < 2; i++)
        {
            if (pfds[i].revents & POLLIN)
            {
                // Message from other client
                if (pfds[i].fd == mysockfd)
                {
                    // Receive from other client
                    // Check seqnum
                    // If seqnum is not previous seqnum, send new ack and update latest seqnum
                    // If seqnum is same as before, send same ack 
                    printf("message received from other client");
                }
                    
                // Message from standard input
                else if (pfds[i].fd == 0)
                {
                    // Choose new seqnum for message
                    // Fill in byte array
                    // Send buffer
                    // If no ack received in certain time, send again
                    printf("message received from stdin");
                }
            }
        }
    }
}

void receive(char* hostname)
{
    // Set up my socket to listen on

	int mysockfd = socket(PF_INET, SOCK_DGRAM, 0); // create a new UDP socket
	if (mysockfd == -1)
	{
		perror("receiver: socket");
		exit(1);
	}

	struct sockaddr_in my_addr;							  // socket address to listen on
	my_addr.sin_family = AF_INET;						  // IPv4
	my_addr.sin_port = htons(MYPORT);					  // my port in network byte order
	my_addr.sin_addr.s_addr = INADDR_ANY;				  // bind to local IP address
	memset(my_addr.sin_zero, 0, sizeof my_addr.sin_zero); // zero out sin_zero

	if (bind(mysockfd, (struct sockaddr *)&my_addr, sizeof my_addr) == -1)
	{
		close(mysockfd);
		perror("bind");
		exit(1);
	}

    // Set up other client socket to send to

    struct addrinfo hints,     // input to getaddrinfo()
	*server_info;	           // head of linked list of results from getaddrinfo()

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;		// IPv4
	hints.ai_socktype = SOCK_DGRAM; // UDP socket

	int status = getaddrinfo(hostname, MYPORTSTRING, &hints, &server_info);
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

    pfds[0].fd = mysockfd; //stdin
    pfds[0].events = POLLIN; 

    char buffer[MAXBUFLEN];

    struct sockaddr_storage sender_addr;	 // sender's address (may be IPv6)
	socklen_t addr_len = sizeof sender_addr; // length of this address

    while(1)
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

                printf("Receiver: received setup message. Sending ack...\n");

                char setup_ack[MAXBUFLEN];

                setup_ack[0] = buffer[0];    //Memcopy
                setup_ack[1] = buffer[1];
                setup_ack[2] = buffer[2];
                setup_ack[3] = buffer[3];
                setup_ack[4] = 0x01;
                setup_ack[5] = buffer[5];
                setup_ack[6] = buffer[6];
                setup_ack[7] = buffer[7];

                int numbytes1 = sendto(sockfd, setup_ack, 8, 0, ptr->ai_addr, ptr->ai_addrlen);
            }
        }
    }    

    printf("receiver: setup complete\n");

}
