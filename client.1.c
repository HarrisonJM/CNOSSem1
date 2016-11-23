/* client.c - a datagram 'client' (talker)
 * ned to supply host name/IPO and one word message,
 * e.g. talker localhost hello
 */


#include <stdio.h>
#include <stdlib.h>

#include <string.h> 	//memeset
#include <time.h>		//Need to know the time

#include <unistd.h> 	//read()
#include <errno.h> 		//errors

#include <sys/types.h> //u_int*_t
#include <sys/socket.h>	//socket(), connect()
#include <sys/ioctl.h>

#include <netinet/in.h>	//struct sockaddr_in, htons()
#include <netinet/udp.h>//Declarations for UDP header
#include <netinet/ip.h> //Declarations for IP header
#include <net/if.h>

#include <arpa/inet.h> 	//inet_aton() - should be inet_pton()?
#include <netdb.h> 		//for gethostbyname()


//For more see http://www.timetools.co.uk/2013/07/25/ntp-server-uk/
#define NTPIPHOME1 "194.35.252.7"
#define NTPIPHOME2 "158.43.192.66"
#define NTPIPHOME3 "91.148.192.49"
#define NTPIPHOME4 "33.117.170.50"
#define NTPIPHOME5 "81.168.77.149"

//#define SNTPPort 4950 //old
#define SNTPPort 123

#define VNNUMBER 4

struct PSEUDOHeader //RFC 768
{
	u_int32_t	source;
	u_int32_t	destination;
	u_int8_t	zero;
	u_int8_t	protocol;
	u_int16_t	packetSize;
};

struct UDPHeader //RFC 768
{
	u_int16_t	sourcePort;
	u_int16_t	destPort;
	u_int16_t	length;
	u_int16_t	checksum;

};

struct UDPBody
{
	unsigned int LI			: 2;//Leap indicator
	unsigned int VN			: 3;//version number, currently 4 
	unsigned int mode		: 3;//client = 3 then server sets to 4. In broadcast mode server sets to 5
	signed int stratum;  		//0 = KOD, 1 = prim. ref., 2-15 = secondary ref., 15-255 = reserved
	unsigned char poll		: 8;//Exponent of 2 is max interval between successive messages sent, only significant on SNTP servers
	signed	 char prec; 		//exponent of two where the resulting value is the precision of the system clock in seconds. only significant on servers
	u_int32_t rootDelay;		//total roundtrip delay to primary reference source, in seconds. RFC 4330. Used for server messages
	u_int32_t rootDispers;		//max error due to clock frequency toelrance
	u_int32_t refIdent;			//used for stratum, only for 0 and 1. Value is 4 char ASCII string, left justified, 0 padded to 32 bits. value = 32bit IPv4 addr of sync source

	u_int64_t refTimeStamp;		//The last time the system clock was set or corrected, 64bit timestamp format
	u_int64_t originTimeStamp;	//Time Request was sent from client
	u_int64_t recTimeStamp;		//Time request reached server 
	u_int64_t transTimeStamp;	//Time Request departed client/server
	
	u_int32_t keyIdent;			//Key Identifier; For sake of completeness
	u_int64_t messDig1;			//
	u_int64_t messdig2;			//Message Digest, for completness

};

struct Datagram
{
	struct UDPHeader header;
	struct UDPBody body;
};

struct Datagram BuildDataGram(struct Datagram dg)

unsigned short CalcChecksum(unsigned short *ptr, int nbytes);

int main(int argc, char * argv[])
{
	int sockfd, numbytes, addr_len;
	struct hostent *he;
	struct sockaddr_in their_addr; /* server address info */

	struct Datagram datagram;

	printf("Size of datagram is: %d\n\n", sizeof(datagram));
	printf("Size of header is: %d\n\n", sizeof(datagram.header));
	printf("Size of body is: %d\n\n", sizeof(datagram.body));

	//argc = number of arguments including program ("$talker localhost hello" would be 3)
	//argv = argumments, indexed by +1 because of program name
	// if(argc < 2) 
	// {
	// 	fprintf(stderr, "usage: $NTPClient <SNTP Server Address>\n");
	// 	exit(1);
	// }
	
	/* resolve server host name or IP address */
	//if((he = gethostbyname(argv[1])) == NULL)
	if((he = gethostbyname(NTPIPHOME2)) == NULL)
	{
		perror("Client: Cannot get host by name");
		exit(1);
	}

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("Client: Socket Creation Failure");
		exit(1);
	}

	memset(&their_addr, 0, sizeof(their_addr)); 			/* zero struct*/
	their_addr.sin_family = AF_INET;						/*...host byte order*/
	their_addr.sin_port = htons(SNTPPort);					/*...short, netwk byte order*/
	their_addr.sin_addr = *((struct in_addr *)he -> h_addr); 

	//sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
	if((numbytes = sendto(sockfd, &datagram, sizeof(datagram), 0, (struct sockaddr*)&their_addr, sizeof(struct sockaddr))) == -1)
	{
		perror("Client: Error Sending Datagram");
		exit(1);
	}

	printf("Sent %d bytes to %s:%d\n", numbytes, inet_ntoa(their_addr.sin_addr), SNTPPort);

	addr_len = sizeof(struct sockaddr);
	

	/* receives echoed content*/
	if((numbytes = recvfrom(sockfd, argv[2], strlen(argv[2]), 0, (struct sockaddr *)&their_addr, &addr_len)) == -1)
	{
		perror("Client: Error Receiving Datagram");
		exit(1);
	}

	printf("A datagram has been received!");
	//printf("Time is: %s\n", argv[2]);
		

	close(sockfd);
	return 0;
}

/***************************************************/
//Name: CalcChecksum
//Description: Calculates checksum for a UDP Header
//Parameters: 
//Returns:
/***************************************************/
unsigned short CalcChecksum(unsigned short *ptr, int nbytes)
{
	long sum;
    unsigned short oddbyte;
    short answer;
 
    sum=0;

    while(nbytes>1) 
	{
        sum+=*ptr++;
        nbytes-=2;
    }

    if(nbytes==1) 
	{
        oddbyte=0;
        *((u_char*)&oddbyte)=*(u_char*)ptr;
        sum+=oddbyte;
    }
 
    sum = (sum>>16) + (sum & 0xffff);
    sum = sum + (sum>>16);
    answer=(short)~sum;
     
    return(answer);
}

struct Datagram BuildDataGram(struct Datagram dg)
{
	dg.header.sourcePort = htons(SNTPPort);
	dg.header.destPort = htons(SNTPPort);
	dg.header.length = sizeof(dg);
	dg.header.checksum = 0;

	dg.body.LI = 0;
	dg.body.VN = VNNUMBER; 
	dg.body.mode = 3;
	dg.body.stratum = 0;
	dg.body.poll = 0;
	dg.body.prec = 0;
	dg.body.rootDelay = 0;
	dg.body.rootDispers = 0;
	dg.body.refIdent = 0;			//used for stratum, only for 0 and 1. Value is 4 char ASCII string, left justified, 0 padded to 32 bits. value = 32bit IPv4 addr of sync source

	dg.body.refTimeStamp;		//The last time the system clock was set or corrected, 64bit timestamp format
	dg.body.originTimeStamp;	//Time Request was sent from client
	dg.body.recTimeStamp;		//Time request reached server 
	dg.body.transTimeStamp;		//Time Request departed client/server
	
	dg.body.keyIdent;			//Key Identifier; For sake of completeness
	dg.body.messDig1;			//
	dg.body.messdig2;			//Message Digest, for completness

}














