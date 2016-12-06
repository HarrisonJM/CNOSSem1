#include "definitions.h"

/***************************************************/
//Name: gettimeofdaysmall
//Parameters: struct timeval *tv
//Returns: 0
//Description:
/* 
 * gettimeofday() but instead of returning -1
 * just exits. Saves lines
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
