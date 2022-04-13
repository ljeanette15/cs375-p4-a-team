#include <poll.h>


int send(char* hostname)
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

    struct addrinfo hints, // input to getaddrinfo()
	*server_info;	   // head of linked list of results from getaddrinfo()

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;		// IPv4
	hints.ai_socktype = SOCK_DGRAM; // UDP socket

	int status = getaddrinfo(argv[1], SERVER_PORT, &hints, &server_info);
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

    // Exchange setup messages to agree on initial seqnum

    unsigned char setup_message[MAXBUFLEN];

    memset(setup_message, 0, MAXBUFLEN);

    int user_len = strlen(argv[2]);

    setup_message[0] = 0x00; //seqnum1
    setup_message[1] = 0x00; //seqnum2
    setup_message[2] = 0x00; //seqnum3
    setup_message[3] = 0x00; //seqnum4
    setup_message[4] = 0x00; //control
    setup_message[5] = 0x00; //ack
    setup_message[6] = user_len & 0xff; //len1
    setup_message[7] = (user_len >> 8) & 0xff; //len2

    // Send connection message


    // wait for ack and setup message from the other client

    bool setup_ack_received = 0;


    unsigned char buffer[MAXBUFLEN];

    while(setup_ack_received != 1)
    {
        memset(buffer, 0, MAXBUFLEN);

	    struct sockaddr_storage sender_addr;	 // sender's address (may be IPv6)
	    socklen_t addr_len = sizeof sender_addr; // length of this address

	    // wait for a message
	    int numbytes = recvfrom(sockfd, buffer, MAXBUFLEN - 1, 0, (struct sockaddr *)&sender_addr, &addr_len);

        // convert sender's address to a string
	    // char sender_ip_string[INET6_ADDRSTRLEN];
	    // inet_ntop(sender_addr.ss_family, &(((struct sockaddr_in *)&sender_addr)->sin_addr), sender_ip_string, INET6_ADDRSTRLEN);

        // If message is setup and an ack:
        if ((buffer[4] == 0x00) && (buffer[5] == 0x01))
        {
            setup_ack_received = 1;
        }
    }

    printf("setup complete\n");

    // Listen for inputs on stdin and also listen for acks from other receiver

    struct pollfd pfds[2];

    pfds[0].fd = 0; //stdin
    pfds[0].events = POLLIN; 
    pfds[1].fd = mysockfd; // Socket of my device to listen for messages on
    pfds[1].events = POLLIN;

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
