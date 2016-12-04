#include "definitions.h"

/***************************************************/
//Name: DatagramInit
//Parameters: struct datagram*, struct timeStamps*
//Returns: 0
//Description:
/* 
 * Compiles datagram to send to NTP server
 * also populates beginning of the timmestamp struct
 */
/***************************************************/
int DatagramInit(struct datagram *dataSend, struct timeStamps *ts)
{
	struct timeval tv;
	gettimeofdaysmall(&tv);

	dataSend->_VN = dataSend->_VN | VNNUMBER;
	dataSend->_mode = dataSend->_mode | CLIENTMODE;
	
	ts->_transmitTimeClientSec = tv.tv_sec;
	ts->_transmitTimeClientMic = tv.tv_usec;

	dataSend->_traTimeSeconds = htonll((NTPEPOCH + ts->_transmitTimeClientSec)); //seconds
	dataSend->_traTimeMicro = htonll(ts->_transmitTimeClientMic); //micro seconds

	return 0;
}

/***************************************************/
//Name: HandleDatagram
//Parameters: struct timeStamps*, struct datagram*, struct timeval*, struct timeval*
//Returns: 0
//Description:
/* 
 * Handles the received datagram by making sense of
 * the received timestamps
 */
/***************************************************/
int HandleDatagram(struct timeStamps *ts, struct datagram *ds, struct timeval *offset, struct timeval *delay)
{
	TimeStampsReceived(ts, ds);	
	CalculateOffset(ts, offset);
	CaluclateDelay(ts, delay);

	return 0;
}

/***************************************************/
//Name: ntohll
//Parameters: uint64_t
//Returns: 
//Description:
/*
 * takes a uint64_t and converts from network order 
 * to little endian
 */
/***************************************************/
uint64_t ntohll(uint64_t temp64)
{
	uint32_t temp32[3];
	
	temp32[0] = ntohl((temp64 & 0xffffffff)); //lsb -> msb
	temp32[1] = ntohl((temp64 >> 32)); //msb -> lsb
	temp32[2] = temp32[0];
	temp32[1] = temp32[0];
	temp32[1] = temp32[2];
	temp64 = (uint64_t) temp32[0] << 32 | temp32[1];

	return temp64;
}

/***************************************************/
//Name: htonll
//Parameters: uint64_t
//Returns: -
//Description:
/*
 * takes a unit64_t and converts it from little endian
 * to netwrok order
 */
/***************************************************/
uint64_t htonll(uint64_t temp64)
{
	uint32_t temp32[3];
	
	temp32[0] = htonl((temp64 & 0xffffffff)); //lsb
	temp32[1] = htonl((temp64 >> 32)); //msb
	temp32[2] = temp32[0];
	temp32[1] = temp32[0];
	temp32[1] = temp32[2];
	temp64 = (uint64_t) temp32[0] << 32 | temp32[1];

	return temp64;
}