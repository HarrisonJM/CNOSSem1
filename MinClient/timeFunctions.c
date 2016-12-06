#include "definitions.h"


int FindMonth(int *day, int leap);
int FindDayInMonth(int month, int day, int dayinyear, int leap);

/***************************************************/
//Name: gettimeofdaysmall
//Parameters: struct timeval *tv
//Returns: 0
//Description:
/* 
 * gettimeofday() but with error handling  
 */
/***************************************************/
void gettimeofdaysmall(struct timeval *tv)
{
	if( (gettimeofday(tv, NULL)) == -1)
	{
		perror("Client: Error getting time of day");
		getchar();
		exit(1);
	}
}

/***************************************************/
//Name: TimeStampsReceived
//Parameters: struct timeStamps *ts, struct datagram *dg
//Returns: 0
//Description:
/* 
 * moves received timestamps into the timestamps struct
 * and converts back into little endian
 */
/***************************************************/
int TimeStampsReceived(struct timeStamps *ts, struct datagram *dg)
{ 

	ts->_referenceTimeServerSec = ntohl(dg->_refTimeSeconds)  - NTPEPOCH;
	ts->_referenceTimeServerMic = ntohl(dg->_refTimeMicro);

	ts->_receivedTimeServerSec = ntohl(dg->_recTimeSeconds)  - NTPEPOCH;
	ts->_receivedTimeServerMic = ntohl(dg->_recTimeMicro);

	ts->_transmitTimeServerSec = ntohl(dg->_traTimeSeconds) - NTPEPOCH;
	ts->_transmitTimeServerMic = ntohl(dg->_traTimeMicro);

	//originate time left out as previously stored
	
	return 0;
}

/***************************************************/
//Name: CalculateOffset
//Parameters: struct timeStamps *ts, struct timeval *offset
//Returns: 0
//Description:
/* 
 * Calulates the offset of the system clock based on
 * time received from the NTP server 
 */
/***************************************************/
int CalculateOffset(struct timeStamps *ts, struct timeval *offset)
{
	uint32_t T1T0;
	uint32_t T2T3;

	T1T0 = ts->_transmitTimeClientSec - ts->_receivedTimeServerSec;
	T2T3 = ts->_transmitTimeServerSec - ts->_systemTimeReceive;

	offset->tv_sec = (T1T0 + T2T3) / 2;
	//off = ((double)T1T0 + (double)T2T3) / 2.0;

	T1T0 = ts->_transmitTimeClientMic - ts->_receivedTimeServerMic;
	T2T3 = ts->_transmitTimeServerMic; //uptime doesn't return microseconds, so 0 is assumed

	offset->tv_usec = (T1T0 + T2T3) / 2;

	return 0;
}

/***************************************************/
//Name: CaluclateDelay
//Parameters: struct timeStamps *ts, struct timeval *delay
//Returns: 0
//Description:
/* 
 * Calulates the possible Delay from sending and receiving
 * from the server
 */
/***************************************************/
int CaluclateDelay(struct timeStamps *ts, struct timeval *delay)
{
	int32_t T3T0;
	int32_t T2T1;

	//All numbers have EPOCH and endian accounted for
	T3T0 = ts->_systemTimeReceive - ts->_transmitTimeClientSec;
	T2T1 = ts->_transmitTimeServerSec - ts->_receivedTimeServerSec;

	delay->tv_sec = T3T0 - T2T1;

	T3T0 = 0 - ts->_transmitTimeClientMic;
	T2T1 = ts->_transmitTimeServerMic - ts->_receivedTimeServerMic;

	delay->tv_usec = abs(T3T0 - T2T1);
	
	return 0;
}

/***************************************************/
//Name: PrintDateAndTime
//Parameters: struct timeStamps *ts, struct datagram *ds, struct timeval offset, struct timeval delay
//Returns: 0
//Description:
/* 
 * Calculates the date and time and then formats it and prints it
 */
/***************************************************/
void PrintDateAndTime(struct timeStamps *ts, struct datagram *ds, struct timeval offset, struct timeval delay)
{
 	uint32_t microseconds = ts->_transmitTimeServerMic;
	uint32_t milliseconds = microseconds / 1000;
	uint32_t seconds = (ts->_transmitTimeServerSec);
	uint32_t minutes = seconds/60;
	uint32_t hours = minutes/60;
 	uint32_t days = hours/24;
 	uint32_t years = days/365; //current year

	int leapyear = (1970 + years) % 4; //if 0 leap year 
	int numLeapYears = ((1970 + years) / 4) - (1970 /4) - 2; //number of leap years

	int dayinyear = (days - (years* 365)) - numLeapYears; // - numLeapYears; //the current day in the year
	int temp = dayinyear;
	int month = FindMonth(&dayinyear, leapyear);
	if(month == -1)
	{
		printf("Error finding month number. Exiting...");
		exit(1);
	}

	int dayinmonth = FindDayInMonth(month, abs(dayinyear), temp, leapyear);

	uint32_t hourstoprint = hours - (days * 24);
	uint32_t minutestoprint = minutes - (hours * 60);
	uint32_t secondstoprint = seconds - (minutes * 60);
	uint32_t millisecondstoprint = milliseconds - (seconds * 1000);

	//      yy-mm-dd       hh:mm:ss.ms
	printf("%04d-%02d-%02d ", 1970+years, month, dayinmonth); 
	printf("%02d:%02d:%02d.%.6d ", (int)hourstoprint, (int)minutestoprint, (int)secondstoprint, (int)millisecondstoprint);
	
	printf("%lu.%lu +/-%lu.%lu ", offset.tv_sec, offset.tv_usec/1000, delay.tv_sec, delay.tv_usec/1000);
	//      offset   errorbound

	return;
}

/***************************************************/
//Name: FindMonth
//Parameters: int *day, int leap
//Returns: month number on success -1 on error;
//Description:
/* 
 * after being given the day in the year, and whether 
 * it's a leap year or not (0 if it is). Finds and returns 
 * the month number it currently is (march = 3) 
 */
/***************************************************/
int FindMonth(int *day, int leap)
{
	int month = 0;
	int i;

	for(i = 1; i < 13; ++i)
	{
		switch(i)
		{
			case 1:
				*day -= 31; //january
				month++;
				break;
			case 2:
				if(leap == 0)
				{
					*day -= 29;
				}
				else
				{
					*day -= 28; //February
				}
				month++;
				break;
			case 3:
				*day -= 31; //March
				month++;
				break;
			case 4:
				*day -= 30; //April
				month++;
				break;
			case 5:
				*day -= 31; //may
				month++;
				break;
			case 6:
				*day -= 30; //june
				month++;
				break;
			case 7:
				*day -= 31; //july
				month++;
				break;				
			case 8:
				*day -= 31; //August
				month++;
				break;
			case 9:
				*day -= 30; //September
				month++;
				break;
			case 10:
				*day -= 31; //October
				month++;
				break;
			case 11:
				*day -= 30; //November
				month++;
				break;
			case 12:
				*day -= 31; //December
				month++;
				break;
		}

		if(*day <= 0)
		{
			return month;
		}
	}

	return -1; //if this is reached, error
}

/***************************************************/
//Name: FindDayInMonth
//Parameters: int month, int day, int dayinyear, int leap
//Returns: 
//Description:
/* 
 * Finds what day in the month it is (347th day of the year is 
 * 2/12 if it's a leap year) and returns that number of
 * the month that it is
 */
/***************************************************/
int FindDayInMonth(int month, int day, int dayinyear, int leap)
{
	/*
		29 = day
		337 = dayinyear
		366
	*/
	int days;

	switch(month)
	{
		case 1: //January
		days = 31 - day; 
		break;
		case 2: //February
			if(leap == 0)
			{
				days = 29 - day;
			}
			else
			{
				days = 28 - day;
			}
		break;
		case 3: //march
		days = 31 - day;
		break;
		case 4: //april
		days = 30 - day;
		break;
		case 5: //may
		days = 31 - day;
		break;
		case 6: //june
		days = 30 - day;
		break;
		case 7: //july
		days = 31 - day;
		break;
		case 8: //August
		days = 31 - day;
		break;
		case 9: //September
		days = 30 - day;
		break;
		case 10: //October
		days = 31 - day;
		break;
		case 11: //November
		days = 30 - day;
		break;
		case 12: //December
		days = 31 - day;
		break;
	}

	if (days == 0)
	{
		days = 1;
	}

	return days;
}