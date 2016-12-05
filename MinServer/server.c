/****************************************************************************/
/*********************************NTPServer**********************************/
/****************************************************************************/
/* Program Name: 		NTPServer											*/
/* Version: 			0.5													*/
/* Date started: 		15/11/16											*/
/* Date last edited: 	03/12/16											*/
/* Description:																*/
/*				The program, when supplied with A Port to run on, opens a	*/ 
/*				socket and waits for connections							*/
/* 																			*/
/****************************************************************************/
/****************************************************************************/


#include <time.h>		//Need to know the time
#include <endian.h>

#include <unistd.h> 	//read()
#include <errno.h> 		//errors

#include <sys/types.h>  //u_int*_t
#include <sys/socket.h>	//socket(), connect()
#include <sys/ioctl.h>

#include <netinet/in.h>	//struct sockaddr_in, htons()
#include <netinet/udp.h>//Declarations for UDP header
#include <net/if.h>

#include <arpa/inet.h> 	//inet_aton() - should be inet_pton()?
#include <netdb.h> 		//for gethostbyname()

#include "definitions.h"


int main(int argc, char * argv[])
{
	struct hostent *he; /* host entity */
	struct sockaddr_in their_addr my_addr; /* server address info */
	struct timeStamps ts; /*Holds time stamps*/
	struct datagram dataRec, dataSend; /* Our datagrams */
	struct timeval tv, timeout, offset, delay;

	int sockfd, numbytes;
	int portNo;

	//ArgHandler(argc, argv);

	if(argc < 3)
	{
		fprintf(stderr, "usage: $NTPServer <port>\n");
		getchar();
		exit(1);
	}

	//convert command line argument into an integer
	portNo = atoi(argv[2]);

	//Open our socket
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) //sockfd = file descriptor
	{
		perror("Client: Socket Creation Failure");
		getchar();
		exit(1);
	}

	/* resolve server host name or IP address */
	if((he = gethostbyname("127.0.0.1")) == NULL)
	{
			perror("Client: Cannot get host by name");
			getchar();
			exit(1);
	}

    memset(&my_addr, 0, sizeof(my_addr));   /* zero struct*/
    my_addr.sin_family = AF_INET;           /* host byte order...*/
    my_addr.sin_port = htons(MYPORT);       /* ...short, net. byte order*/
    my_addr.sin_addr.s_addr = INADDR_ANY;   /* any of server IP addrs*/

	memset(&their_addr, 0, sizeof(their_addr));                     /* zero struct*/
	their_addr.sin_family = AF_INET;                                /*...host byte order*/
	their_addr.sin_port = htons(portNo);                            /*...short, netwk byte order*/
	their_addr.sin_addr = *((struct in_addr *)he -> h_addr);


	//binds server to a port
    if(bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
	{
        perror("Listener bind");
    	exit(1);
	}

	//Sets up the timeout for recv
	//timeout.tv_sec = 5;
	//timeout.tv_usec = 0;
	//setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));

	do
	{	
		memset(&dataSend, 0, sizeof(dataSend)); 		/* zero struct */
		memset(&dataRec, 0, sizeof(dataRec)); 			/* zero struct */

		//receives datagram from client
		//if((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1)
		if((numbytes = recv(sockfd, &dataRec, sizeof(dataRec), 0)) == -1)
		{
			perror("SERVER: Error Receiving Datagram");
			getchar();
			exit(1);
		}

		//sends datagram to a server
		if((numbytes = sendto(sockfd, &dataRec, sizeof(dataRec), 0, (struct sockaddr*)&their_addr, sizeof(struct sockaddr))) == -1)
		{ //sendto returns number of bytes sent
			perror("SERVER: Error Sending Datagram");
			getchar();
			exit(1);
		}

		printf("sent datagram to personal SNTP");

		//Fills outs datagram to send back to client 
		DatagramInit(&dataRec);

		//sends datagram
		if((numbytes = sendto(sockfd, &dataRec, sizeof(dataRec), 0, (struct sockaddr*)&their_addr, sizeof(struct sockaddr))) == -1)
		{ //sendto returns number of bytes sent
			perror("SERVER: Error Sending Datagram");
			getchar();
			exit(1);
		}

	} while(1)

	gettimeofdaysmall(&tv);
	ts._systemTimeReceive = tv.tv_sec;

	HandleDatagram(&ts, &dataRec, &offset, &delay);

	printf("NTP Server: ");
	printf("%s %s Stratum: %d \n", he->h_name, inet_ntoa(their_addr.sin_addr), dataRec._stratum);

	printf("Time is: ");
	PrintDateAndTime(&ts, &dataRec, offset, delay);
	if(dataRec._LI)
	{
		printf("Leap\n");
	}
	else
	{
		printf("No-leap\n");
	}

	close(sockfd);
	getchar();
	return 0;
}
