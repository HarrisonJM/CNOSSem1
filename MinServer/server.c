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

int ClientDatagram(struct datagram* client, struct datagram* server, struct timeval *tv);

int main(int argc, char * argv[])
{
	struct hostent *he; /* host entity */
	struct sockaddr_in server_addr, client_addr, my_addr;  /* server address info */
	//struct timeStamps ts; /*Holds time stamps*/
	struct datagram dataRec, dataSend; /* Our datagrams */
	struct timeval tv;

	int sockfd, numbytes;
	socklen_t addrlen = sizeof(client_addr);
	int portNo;

	//ArgHandler(argc, argv);

	if(argc < 2)
	{
		fprintf(stderr, "usage: $NTPServer <port>\n");
		getchar();
		exit(1);
	}

	//convert command line argument into an integer
	portNo = atoi(argv[1]);

	//Open our socket
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) //sockfd = file descriptor
	{
		perror("Client: Socket Creation Failure");
		getchar();
		exit(1);
	}

	/* resolve server host name or IP address */
	if((he = gethostbyname("ntp.uwe.ac.uk")) == NULL)
	{
			perror("Client: Cannot get host by name");
			getchar();
			exit(1);
	}
	
	//server address information 
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;                             
	server_addr.sin_port = htons(123);                        
	server_addr.sin_addr = *((struct in_addr *)he -> h_addr); //FIX ME PLZ

	memset(&my_addr, 0, sizeof(my_addr)); 	/* zero struct*/
	my_addr.sin_family = AF_INET;		/* host byte order...*/
	my_addr.sin_port = htons(portNo);	/* ...short, net. byte order*/
	my_addr.sin_addr.s_addr = INADDR_ANY;	/* any of server IP addrs*/

	//binds server to a port
	if(bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
	{
        perror("Listener bind");
    	exit(1);
	}

	do
	{	
		memset(&dataSend, 0, sizeof(dataSend)); 		/* zero struct */
		memset(&dataRec, 0, sizeof(dataRec)); 			/* zero struct */

		printf("Waiting for packet\n");

		//receives datagram from client
		if ((numbytes = recvfrom(sockfd, &dataSend, sizeof(dataSend), 0, (struct sockaddr*) &client_addr, &addrlen)) == -1)
		{
			perror("SERVER: Error Receiving Datagram");
			getchar();
			exit(1);
		}

		gettimeofday(&tv, NULL);

		//sends datagram to a server
		if((numbytes = sendto(sockfd, &dataRec, sizeof(dataRec), 0, (struct sockaddr*)&server_addr, sizeof(struct sockaddr))) == -1)
		{ 
			perror("SERVER: Error Sending Datagram to server");
			getchar();
			exit(1);
		}
			
		printf("sent datagram to personal SNTP\n");
	
		//receive datagram back from server	
		numbytes = recv(sockfd, &dataRec, sizeof(dataRec), 0);
	    if(numbytes == -1)
	    {
				perror("Client: Error Receiving Datagram");
				getchar();
				exit(1);
		}

		
		ClientDatagram(&dataSend, &dataRec, &tv);

		//Fills outs datagram to send back to client 
		//DatagramInit(&dataRec);

		//sends datagram back to client
		if((numbytes = sendto(sockfd, &dataSend, sizeof(dataSend), 0, (struct sockaddr*)&client_addr, sizeof(struct sockaddr))) == -1)
		{ //sendto returns number of bytes sent
			perror("SERVER: Error Sending Datagram");
			getchar();
			exit(1);
		}

	} while(1);

	close(sockfd);
	return 0;
}
