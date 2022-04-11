/*
** talker.c -- a datagram "client" demo
** Modified by JTH
*/

#include <stdio.h>	// for printf, fprintf, perror
#include <stdlib.h> // for exit
#include <unistd.h> // for close
//#include <errno.h>
#include <string.h> // for memset
//#include <sys/types.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
#include <netdb.h>	   // for getaddrinfo
#include <arpa/inet.h> // inet_ntop

#define SERVER_PORT "4950" // the port users will be connecting to
#define MYPORT 4950		   // the port users will be connecting to
#define MAXBUFLEN 100

int send(char *hostname, unsigned char *message)
{
	// use getaddrinfo to get the IP address of the hostname

	struct addrinfo hints, // input to getaddrinfo()
		*server_info;	   // head of linked list of results from getaddrinfo()

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;		// IPv4
	hints.ai_socktype = SOCK_DGRAM; // UDP socket

	int status = getaddrinfo(hostname, SERVER_PORT, &hints, &server_info);
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

	// we don't need to call bind() to bind our socket to a port because
	// we are only sending a message and do not expect to receive a response

	// can't use strlen because there can be 0s causing it to nul terminate

	int length = message[6] + (message[7] << 8);

	int numbytes = sendto(sockfd, message, length + 8, 0, ptr->ai_addr, ptr->ai_addrlen);
	if ((numbytes) == -1)
	{
		perror("sender: sendto");
		exit(1);
	}

	freeaddrinfo(server_info);

	close(sockfd);

	return 0;
}

int receive(unsigned char *buffer)
{
	int sockfd = socket(PF_INET, SOCK_DGRAM, 0); // create a new UDP socket
	if (sockfd == -1)
	{
		perror("receiver: socket");
		exit(1);
	}

	struct sockaddr_in my_addr;							  // socket address to listen on
	my_addr.sin_family = AF_INET;						  // IPv4
	my_addr.sin_port = htons(MYPORT);					  // my port in network byte order
	my_addr.sin_addr.s_addr = INADDR_ANY;				  // bind to local IP address
	memset(my_addr.sin_zero, 0, sizeof my_addr.sin_zero); // zero out sin_zero

	if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof my_addr) == -1)
	{
		close(sockfd);
		perror("receiver: bind");
		exit(1);
	}

	printf("receiver: waiting to recvfrom...\n");

	// char buffer[MAXBUFLEN];                   // buffer to hold received message
	struct sockaddr_storage sender_addr;	 // sender's address (may be IPv6)
	socklen_t addr_len = sizeof sender_addr; // length of this address

	// wait for a message
	int numbytes = recvfrom(sockfd, buffer, MAXBUFLEN - 1, 0,
							(struct sockaddr *)&sender_addr, &addr_len);
	if (numbytes == -1)
	{
		perror("recvfrom");
		exit(1);
	}

	buffer[numbytes] = '\0'; // add null terminator for printing

	// convert sender's address to a string
	char sender_ip_string[INET6_ADDRSTRLEN];
	inet_ntop(sender_addr.ss_family, &(((struct sockaddr_in *)&sender_addr)->sin_addr), sender_ip_string, INET6_ADDRSTRLEN);

	close(sockfd);

	return 0;
}