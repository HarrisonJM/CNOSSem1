#ifndef __MYSTRUCTS_H__
#define __MYSTRUCTS_H__

#include <stdio.h>
#include <stdlib.h>

#include <netinet/ip.h> //Declarations for IP header

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
//#define VNNUMBER 4
#define VNNUMBER 0b010
//#define CLIENTMODE 3
#define CLIENTMODE 0b110

#define NTPEPOCH ((uint64_t)((365 * 70) + 17) * 24 * 60 * 60)
//70 years + 17 leap days, convert to seconds. Needs casting otherwise int overrun

//datagram indexes for the char*
#define REFIND 16 //time system clock
#define ORIIND 24 //originate timesstamp, when timestamp was sent from client to server
#define SERIND 32 //Time Server received datagram
#define TRAIND 40 //Time datagram departed client/server

uint64_t CurrentTimems();
int CompileTimeStamp(unsigned char* s, uint64_t time);
int TimeSendOut( uint64_t* timeOfRequest, uint64_t* systemTimeOfRequest);
int BuildDataGram(unsigned char* datagram, uint64_t* timeOfRequest, uint64_t* systemTimeOfRequest);

int FixTimeStamp(char* DR, unsigned char* fixedTimeStamp);
int HandleDatagram(long long* Datagram, uint64_t timeOfRequest, uint64_t sysReqTime);
int SortTime();

struct timeStamps
{
	uint64_t _referenceTimeClient;
	uint64_t _originateTimeClient;
	uint64_t _transmitTimeClient;
    uint64_t _systemTimeSend; //NOT TO BE SENT USED FOR REFERENCE!

    uint64_t _systemTimeReceive; //NOT TO BE SENT USED FOR REFERENCE!
	uint64_t _referenceTimeServer; //maybe relevant for server?
	uint64_t _originateTimeServer; //Shouldn't change, must remove
	uint64_t _receivedTimeServer; 
	uint64_t _transmiteTimeServer;

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
	uint64_t _refTime;
	uint64_t _oriTime;
	uint64_t _recTime;
	uint64_t _traTime;
};

#endif/*__MYSTRUCTS_H__*/