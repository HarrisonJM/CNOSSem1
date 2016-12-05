#ifndef __MYSTRUCTS_H__
#define __MYSTRUCTS_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 	//memeset

#include <netinet/ip.h> //Declarations for IP header
#include <sys/sysinfo.h>//sysinfo.uptime = seconds since boot
#include <sys/time.h>	//gettimeofday(struct timevall *tv, struct timezone *tz)
//#include <sys/timex.h> //Seems to include values for deciding whether to set the system clock

#include <math.h>

//For more see http://www.timetools.co.uk/2013/07/25/ntp-server-uk/
//stratum 1
#define NTPIPHOME1 "194.35.252.7"
#define NTPIPHOME2 "158.43.192.66"
#define NTPIPHOME2NAMME "ntp2.pipex.net"
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
#define VNNUMBER 0b010
#define CLIENTMODE 0b110
#define SERVERMODE 0b010

#define NTPEPOCH ((uint64_t)((365 * 70) + 17) * 24 * 60 * 60)
#define NTPEPOCH2 (uint64_t)2208988800LL

//70 years + 17 leap days, convert to seconds. Needs casting otherwise int overrun

//datagram indexes for the char*
#define REFIND 16 //time system clock
#define ORIIND 24 //originate timesstamp, when timestamp was sent from client to server
#define SERIND 32 //Time Server received datagram
#define TRAIND 40 //Time datagram departed client/server

#define DATETIMESIZE 26

struct timeStamps
{
	uint32_t _transmitTimeClientSec;
	uint32_t _transmitTimeClientMic;

	uint32_t _referenceTimeServerSec;
	uint32_t _referenceTimeServerMic;

	uint32_t _receivedTimeServerSec;
	uint32_t _receivedTimeServerMic;  

	uint32_t _transmitTimeServerSec;
	uint32_t _transmitTimeServerMic;

	uint32_t _systemTimeReceive;
};

struct datagram
{
	uint8_t _LI : 2;
	uint8_t _VN : 3;
	uint8_t _mode : 3;
	uint8_t _stratum;
	uint8_t _poll;
	uint8_t _precision;
	uint32_t _rootDelay;
	uint32_t _rootDispersion;
	uint32_t _refIdent;

	//Time system clock last set
	uint32_t _refTimeSeconds;
	uint32_t _refTimeMicro; //fraction

	//Time client sent request to server
	uint32_t _oriTimeSeconds;
	uint32_t _oriTimeMicro;

	//Time server received reqeust
	uint32_t _recTimeSeconds;
	uint32_t _recTimeMicro;

	//time packet was sent (client/server)
	uint32_t _traTimeSeconds;
	uint32_t _traTimeMicro;
};

struct offset
{
	uint32_t _seconds;
	uint32_t _fraction;
};

uint64_t CurrentTimems();
int CompileTimeStamp(unsigned char* s, uint64_t time);
int TimeSendOut( uint64_t* timeOfRequest, uint64_t* systemTimeOfRequest);
int BuildDataGram(unsigned char* datagram, uint64_t* timeOfRequest, uint64_t* systemTimeOfRequest);
int FixTimeStamp(char* DR, unsigned char* fixedTimeStamp);

uint64_t htonll(uint64_t temp64);
uint64_t ntohll(uint64_t temp64);

int DatagramInit(struct datagram *dataSend, struct timeStamps *ts);

void gettimeofdaysmall(struct timeval *tv);

int HandleDatagram(struct timeStamps *ts, struct datagram *ds, 
					struct timeval *offset, struct timeval *delay);
int TimeStampsReceived(struct timeStamps *ts, struct datagram *dg);
int CalculateOffset(struct timeStamps *ts, struct timeval *offset);
int CaluclateDelay(struct timeStamps *ts, struct timeval *delay);

void PrintDateAndTime(struct timeStamps *ts, struct datagram *ds,
						 struct timeval offset, struct timeval delay);

#endif/*__MYSTRUCTS_H__*/