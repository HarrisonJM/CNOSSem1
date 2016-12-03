/* client.c - a datagram 'client' (talker)
 * ned to supply host name/IPO and one word message,
 * e.g. talker localhost hello
 */
// https://github.com/magicmonty/bash-git-prompt


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
	
	//argc = number of arguments including program ("$talker localhost hello" would be 3)
	//argv = argumments, indexed by +1 because of program name
	if(argc < 2)
	{
		fprintf(stderr, "usage: $NTPClient <SNTP Server Address>\n");
		printf("./NTPClient <NTP ADDRESSES (seperated by spaces)> -n<numberofserverstotry> -t<seconds>");
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

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) //sockfd = file decriptor
	{
		perror("Client: Socket Creation Failure");
		getchar();
		exit(1);
	}

	memset(&their_addr, 0, sizeof(their_addr)); 			/* zero struct*/
	their_addr.sin_family = AF_INET;						/*...host byte order*/
	their_addr.sin_port = htons(SNTPPort);					/*...short, netwk byte order*/
	their_addr.sin_addr = *((struct in_addr *)he -> h_addr);

	// BuildDataGram(datagramBody, &timeOfRequest, &sysReqTime);
	memset(&dataSend, 0, sizeof(dataSend)); 		/* zero struct*/
	memset(&dataRec, 0, sizeof(dataRec)); 			/* zero struct*/
	DatagramInit(&dataSend, &ts);

	if((numbytes = sendto(sockfd, &dataSend, sizeof(dataSend), 0, (struct sockaddr*)&their_addr, sizeof(struct sockaddr))) == -1)
	{ //sendto returns number of bytes sent
		perror("Client: Error Sending Datagram");
		getchar();
		exit(1);
	}

	if((numbytes = recv(sockfd, &dataRec, sizeof(dataRec), 0)) == -1) //TODO: recv not block with 'MSG_DONTWAIT' instead of 0. decide timeout
	{
		perror("Client: Error Receiving Datagram");
		getchar();
		exit(1);
	}

	gettimeofday(&tv, NULL);

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
