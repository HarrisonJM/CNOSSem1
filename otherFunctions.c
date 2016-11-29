#include "definitions.h"
//#include <>

/***************************************************/
//Name: BuildDataGram()
//Parameters: char* datagram, int* timeOfRequest, int* systemTimeOfRequest
//Returns: 0 on success
//Description:
/* Builds the data gram to be sent
 * Requires a char* to write the message into
 */
/***************************************************/
int BuildDataGram(unsigned char* datagram, uint64_t *timeOfRequest, uint64_t *systemTimeOfRequest)
{
	struct sysinfo si;
	
	if(sysinfo(&si) == -1) //on error
	{
		perror("Client: Failure retrieving system information");
		getchar();
		exit(1);
	}

	//*timeOfRequest = CurrentTimems();
	*systemTimeOfRequest = (uint64_t)(si.uptime >> 16);

	memset(datagram, 0, sizeof(datagram));
	datagram[0] = CLIENTMODE | (VNNUMBER << 3);

	//CompileTimeStamp(datagram, *timeOfRequest);

	return 0;
}

/***************************************************/
//Name: DatagramInit
//Parameters: struct datagram*, struct timeStamps*
//Returns: 0
//Description:
/* 
 * Compiles datagram to send to NTP server
 * also populates beginning of the tiemmstamp struct
 */
/***************************************************/
int DatagramInit(struct datagram *dataSend, struct timeStamps *ts)
{
	struct sysinfo *si;
	struct timeval tv;

	//dataSend->_VN = dataSend._VN | VNNUMBER;
	dataSend->_VN = dataSend->_VN | VNNUMBER;
	dataSend->_mode = dataSend->_mode | CLIENTMODE;
	//1482341983394	

	//ts->_transmitTimeClientSec = ts->_originateTimeClientSec = CurrentTimems();
	//dataSend->_traTime = dataSend->_oriTime = htonll(ts->_transmitTimeClientSec);
	
	ts->_transmitTimeClientSec = ts->_originateTimeClientSec = tv.uptime;
	dataSend->_traTimeSeconds = dataSend->_oriTimeMicro = htonll(ts->_transmitTimeClientSec);

	sysinfo(si);
	ts->_systemTimeSendSec = si->uptime;

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