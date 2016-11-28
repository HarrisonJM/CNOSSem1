#include "definitions.h"
#include <math.h>

/*

	CLOCK SYNC ALGORITHMS:
	theta, offset 
	theta = ((T1 - T0) + (T2 - T3)) / 2
	t0 = time sent out (sysinfo()) _systemTimeSend
	t1 = server received _recTime, _receivedTimeServer
	t2 = server sent _traTime, _transmiteTimeServer
	t3 = client reception, _systemTimeReceive

	delta, tound trip delay
	delta = (t3 - t0) - (t2 - t1)



*/

/***************************************************/
//Name:
//Parameters: char* s, uint64_t time
//Returns: -
//Description:
/*
 * Writes the timestamp from the system into the
 * message to send to the server. Also deals with
 * EPOCH and little -> big endian
 */
/***************************************************/
int CompileTimeStamp(unsigned char* s, uint64_t time)
{
	int index = 40; //How many bytes into the packet we need to start
	
	uint64_t seconds = time / 1000;
	uint64_t milliseconds = time - seconds * 1000;  //for fractions of a second
	uint64_t fraction = milliseconds * 0x1000000000 / 1000;

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
//Name: currentTimems()
//Parameters: -
//Returns: uint64_t milliseconds since EPOCH
//Description:
/*
 * Retrives time, calculates time in microseconds
 */
/***************************************************/
uint64_t CurrentTimeus()
{
	struct timeval tv;
	uint64_t seconds;
	uint64_t millis;
	uint64_t microseconds;

	if(gettimeofday(&tv, NULL) == -1) //TZ obselote. should just be NULL
	{
		perror("Client: Error in retrieveing time of day.");
		exit(1);
	}

	millis = (tv.tv_sec  + NTPEPOCH) * 1000;
	microseconds = tv.tv_usec + millis * 1000;
	/*TV conatains both seconds an micro seconds since UNIX.
	  So both must be converted into a proper format*/

	return microseconds;
}


/***************************************************/
//Name: TimeStampsReceived
//Parameters: struct timeStamps, struct datagram*
//Returns: int
//Description:
/* 
 * Converts received 64bit timestamps into little format.
 * 
 */
/***************************************************/
//int FixTimeStamp(char* DR, unsigned char* fixedTimeStamp)
int TimeStampsReceived(struct timeStamps *ts, struct datagram *dg)
{ //TODO: WE ARE NOW HERE!!!!
	
	struct sysinfo *si;
	sysinfo(si);	
	ts->_systemTimeReceive = si->uptime;

	uint64_t _referenceTimeServer; //maybe relevant for server?
	uint64_t _originateTimeServer; //Shouldn't change, must remove
	uint64_t _receivedTimeServer; 
	uint64_t _transmiteTimeServer;

	ts->_referenceTimeServer = ntohll(dg->_refTime);
	ts->_receivedTimeServer = ntohll(dg->_oriTime);
	ts->_transmiteTimeServer = ntohll(dg->_traTime);


}

/***************************************************/
//Name:
//Parameters: 
//Returns:
//Description:
/* 
 * TODO: WRITE MY OWN!!!
 */
/***************************************************/
uint32_t read32(char* buffer, int offset)
{
    char b0 = buffer[offset];
    char b1 = buffer[offset+1];
    char b2 = buffer[offset+2];
    char b3 = buffer[offset+3];

    // convert signed bytes to unsigned values
    uint32_t i0 = ((b0 & 0x80) == 0x80 ? (b0 & 0x7F) + 0x80 : b0);
    uint32_t i1 = ((b1 & 0x80) == 0x80 ? (b1 & 0x7F) + 0x80 : b1);
    uint32_t i2 = ((b2 & 0x80) == 0x80 ? (b2 & 0x7F) + 0x80 : b2);
    uint32_t i3 = ((b3 & 0x80) == 0x80 ? (b3 & 0x7F) + 0x80 : b3);

    
    uint32_t v = (i0 << 24) + (i1 << 16) + (i2 << 8) + i3;
    return v;
}

/***************************************************/
//Name: 
//Parameters: 
//Returns:
//Description:
/* 
 * 
 */
/***************************************************/
uint64_t readTimeStamp(char *buffer, int offset)
{
    uint32_t seconds  = read32(buffer, offset);
    uint32_t fraction = read32(buffer, offset + 4);
    uint64_t v = ((int64_t)seconds - NTPEPOCH) * 1000 +
    (int64_t) fraction * 1000 / (int64_t) 0x100000000;

    return v;
}

/***************************************************/
//Name: 
//Parameters: 
//Returns:
//Description:
/* 
 * 
 */
/***************************************************/