/* client.c - a datagram 'client' (talker)
 * ned to supply host name/IPO and one word message,
 * e.g. talker localhost hello
 */
// https://github.com/magicmonty/bash-git-prompt

#include <stdio.h>
#include <stdlib.h>

#include <string.h> 	//memeset
#include <time.h>		//Need to know the time
#include <sys/time.h>	//gettimeofday(struct timevall *tv, struct timezone *tz)
#include <sys/sysinfo.h>//sysinfo.uptime = seconds since boot

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
//stratum 1
#define NTPIPHOME1 "194.35.252.7"
#define NTPIPHOME2 "158.43.192.66"
#define NTPIPHOME3 "91.148.192.49"
#define NTPIPHOME4 "33.117.170.50"
#define NTPIPHOME5 "81.168.77.149"
//stratum 2
#define NTPIPHOME6 "129.6.15.28"
#define UWENTP "ntp.uwe.ac.uk"

//#define SNTPPort 4950 //old
#define SNTPPort 123

//DataGram Information
#define MAXDATASIZE 48 //bytes
#define VNNUMBER 4
#define CLIENTMODE 3

#define NTPEPOCH ((uint64_t)((365 * 70) + 17) * 24 * 60 * 60)
//70 years + 17 leap days, convert to seconds. Needs casting otherwise int overrun

//TODO: WRITE MY OWN!!
unsigned short CalcChecksum(unsigned short *ptr, int nbytes);

u_int64_t CurrentTimems();
int CompileTimeStamp(unsigned char* s, u_int64_t time);
int BuildDataGram(unsigned char* datagram, u_int64_t* timeOfRequest, u_int64_t* systemTimeOfRequest);

//int Checksum(struct PH, int *checksum);

int main(int argc, char * argv[])
{
	int sockfd, numbytes, addr_len;
	u_int64_t timeOfRequest, sysReqTime; //clock of time made, computer uptime fo reference
	struct hostent *he;
	struct sockaddr_in their_addr; /* server address info */

	unsigned char datagramBody[MAXDATASIZE]; /* This is the body of the Datagram*/
	char datagramRec[MAXDATASIZE];

	// printf("Size of datagram is: %d\n\n", sizeof(*datagram));
	// printf("Size of header is: %d\n\n", sizeof(datagram->header));
	// printf("Size of body is: %d\n\n", sizeof(datagram->body));

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

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) //sockfd = file decriptor
	{
		perror("Client: Socket Creation Failure");
		exit(1);
	}

	memset(&their_addr, 0, sizeof(their_addr)); 			/* zero struct*/
	their_addr.sin_family = AF_INET;						/*...host byte order*/
	their_addr.sin_port = htons(SNTPPort);					/*...short, netwk byte order*/
	their_addr.sin_addr = *((struct in_addr *)he -> h_addr);

	BuildDataGram(datagramBody, &timeOfRequest, &sysReqTime);

	//sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
	if((numbytes = sendto(sockfd, datagramBody, sizeof(datagramBody), 0, (struct sockaddr*)&their_addr, sizeof(struct sockaddr))) == -1)
	{ //sendto returns number of bytes sent
		perror("Client: Error Sending Datagram");
		exit(1);
	}

	printf("Sent %d bytes to %s:%d\n", numbytes, inet_ntoa(their_addr.sin_addr), SNTPPort);

	addr_len = sizeof(struct sockaddr);

	/* receives Datagram*/
	if((numbytes = recvfrom(sockfd, datagramRec, sizeof(datagramRec), 0, (struct sockaddr *)&their_addr, &addr_len)) == -1)
	{
		perror("Client: Error Receiving Datagram");
		exit(1);
	}

	printf("A datagram has been received!");
	//printf("Time is: %s\n", argv[2]);

	close(sockfd);
	getchar();
	return 0;
}

/***************************************************/
//Name: CalcChecksum
//Description: Calculates checksum for a UDP Header
//Parameters: MAKE MY OWN ONE!!!
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

/***************************************************/
//Name: currentTimems()
//Parameters: -
//Returns: u_int64_t milliseconds since EPOCH
//Description:
/*
 * Retrieves system time since EPOCH and converts
 * into milliseconds
 */
/***************************************************/
u_int64_t CurrentTimems()
{
	struct timeval tv;
	u_int64_t milliseconds;
	u_int64_t seconds;
	u_int64_t microseconds;

	if(gettimeofday(&tv, NULL) == -1) //TZ obselote. should just be NULL
	{
		perror("Client: Error in retrieveing time of day.");
		exit(1);
	}

	/*TV conatains both seconds an micro seconds since EPOCH.
	  So both must be converted into a proper format*/
	seconds = (u_int64_t) tv.tv_sec * 1000;
	microseconds  = tv.tv_usec/1000; //divison creates remainders
	milliseconds = seconds + microseconds;

	return milliseconds;
}

/***************************************************/
//Name:
//Parameters: char* s, u_int64_t time
//Returns: -
//Description:
/*
 * Writes the timestamp from the system into the
 * message to send to the server. Also deals with
 * EPOCH and little -> big endian
 */
/***************************************************/
int CompileTimeStamp(unsigned char* s, u_int64_t time)
{
	int index = 40; //How many bytes into the packet we need to start
	
	u_int64_t seconds = time / 1000;
	u_int64_t milliseconds = time - seconds * 1000;  //for fractions of a second
	u_int64_t fraction = milliseconds * 0x1000000000 / 1000;

	seconds += NTPEPOCH; //ADD SECONDS SINCE 1900

	//network order is big endian, x86 is little endian
	s[index++] = (char)seconds >> 24; //S is a char*, each element is a char
	s[index++] = (char)seconds >> 16;
	s[index++] = (char)seconds >> 8;
	s[index++] = (char)seconds >> 0;

	//need to write fractions of a second in aswell
	s[index++] = (char)fraction >> 24;
	s[index++] = (char)fraction >> 16;
	s[index++] = (char)fraction >> 8;

	s[index++] = 0;

	return 0;
}

/***************************************************/
//Name: BuildDataGram()
//Parameters: char* datagram, int* timeOfRequest, int* systemTimeOfRequest
//Returns: 0 on success
//Description:
/* Build the data gram to be sent to the NTP server.
 * Requires a char* to write the message into and must
 * also be passed the IP of the server to send to along
 * with a port (if required)
 */
/***************************************************/
int BuildDataGram(unsigned char* datagram, u_int64_t* timeOfRequest, u_int64_t* systemTimeOfRequest)
{
	struct sysinfo si;
	
	if(sysinfo(&si) == -1) //on error
	{
		perror("Client: Failure retrieving system information");
		exit(1);
	}

	*timeOfRequest = CurrentTimems();
	*systemTimeOfRequest = (u_int64_t)(si.uptime >> 16);

	u_int64_t systemTimeOfReceive;

	memset(datagram, 0, sizeof(datagram));
	datagram[0] = CLIENTMODE | (VNNUMBER << 3);

	CompileTimeStamp(datagram, *timeOfRequest);

	return 0;
}
