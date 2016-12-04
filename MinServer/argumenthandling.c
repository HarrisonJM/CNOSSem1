#include "definitions.h"

/***************************************************/
//Name: ArgHandler
//Parameters: int argc, char** args
//Returns: 
//Description:
/* 
 * 
 */
/***************************************************/
//TODO: getopt() is what I need to use. 
int ArgHandler(int argc, char** argv)
{
    //argc = number of arguments including program ("$talker localhost hello" would be 3)
	//argv = argumments, indexed by +1 because of program name
    if(argc < 2)
	{
		fprintf(stderr, "usage: $NTPClient <SNTP Server Address>\n");
		printf("./NTPClient -n<number> <NTP ADDRESSES> -t<seconds>\n");
		printf("-n<number> 		: tells the client how many servers to to expect\n");
		printf("<NTP ADDRESSES> : The NTP address to try. Either by name or IP. Seperated by a space.\n");
		printf("-t<seconds		: The amount of time in whole number seconds before moving onto the next server to try");
		getchar();
		exit(1);
	}
    else
    {
        return 0;
    }

    return 0;

}