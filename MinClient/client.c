/****************************************************************************/
/*********************************NTPClient**********************************/
/****************************************************************************/
/* Program Name: 		NTPClient											*/
/* Version: 			0.5													*/
/* Date started: 		15/11/16											*/
/* Date last edited: 	05/12/16											*/
/* Description:																*/
/*				The program, when supplied with the address for an NTP		*/ 
/*				server, prints the date, time, time offset, and roundtrip   */
/*				delay. to the console.										*/
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
	struct sockaddr_in their_addr; /* server address info */
	struct timeStamps ts; /*Holds time stamps*/
	struct datagram dataRec, dataSend; /* Our datagrams */
	struct timeval tv, timeout, offset, delay;

	int sockfd, numbytes;
	int portNo;

	//ArgHandler(argc, argv);

	if(argc < 3)
	{
		fprintf(stderr, "usage: $NTPClient <SNTP Server Address> <port (type 123 for default)>\n");
		getchar();
		exit(1);
	}

	/* resolve server host name or IP address */
	if((he = gethostbyname(argv[1])) == NULL)
	{
		perror("Client: Cannot get host by name");
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

	//sets up host address struct
	memset(&their_addr, 0, sizeof(their_addr)); 			/* zero struct*/
	their_addr.sin_family = AF_INET;						/*...host byte order*/
	their_addr.sin_port = htons(portNo);					/*...short, netwk byte order*/
	their_addr.sin_addr = *((struct in_addr *)he -> h_addr);

	// BuildDataGram(datagramBody, &timeOfRequest, &sysReqTime);
	memset(&dataSend, 0, sizeof(dataSend)); 		/* zero struct*/
	memset(&dataRec, 0, sizeof(dataRec)); 			/* zero struct*/

	//Sets up the timeout for recv
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));

	//initialises datagram to be sent to the server
	DatagramInit(&dataSend, &ts);

	//sends datagram
	if((numbytes = sendto(sockfd, &dataSend, sizeof(dataSend), 0, (struct sockaddr*)&their_addr, sizeof(struct sockaddr))) == -1)
	{ //sendto returns number of bytes sent
		perror("Client: Error Sending Datagram");
		getchar();
		exit(1);
	}

	//recv error handlling and timeout handling
	numbytes = recv(sockfd, &dataRec, sizeof(dataRec), 0);
	if(numbytes == -1)
	{
		perror("Client: Error Receiving Datagram");
		getchar();
		exit(1);
	}
	else if (numbytes == EAGAIN)
	{
		perror("Client: Timeout connecting to host");
		getchar();
		exit(1);
	}
	else if(numbytes == EWOULDBLOCK)
	{
		perror("Client: Timeout connecting to host");
		getchar();
		exit(1);
	}

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
