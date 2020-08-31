#include "contiki.h"
#include "lib/random.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
/*---------------------------------------------------------------------------*/
PROCESS(hello_world_process, "Hello world process");
PROCESS(hello_world_process2, "Hello world process2");
AUTOSTART_PROCESSES(&hello_world_process);
/*---------------------------------------------------------------------------*/


char* newTrafficGenerator(void);
static char previoudTrafficType[] = "Type A";
static void generateInfiniteRandomNumber(void)
{
	
}




/*---------------------------------------------------------------------------*/

char* newTrafficGenerator(void)
{
	//random_init(50);
	int i;
	int count =0;
	static char currentTrafficType[6];
	i = random_rand();
	//printf("Current random no is: %d\n",i);
	if (i>0 && i<32767)
	{
		strncpy(currentTrafficType,"Type A",sizeof(currentTrafficType));
	}
	else if(i>32767 && i<55705)
	{
		strncpy(currentTrafficType,"Type B",sizeof(currentTrafficType));
	}
	else
	{
		strncpy(currentTrafficType,"Type C",sizeof(currentTrafficType));
	}
	
	return currentTrafficType;
}


/*---------------------------------------------------------------------------*/




/*void randomTrafficGenrator(char *ptr)
{
	int i;
	int count =0;
	
	i = random_rand();
	if (i>0 && i<32767)
	{
		strncpy(setCurrentTrafficType,"Type A",sizeof(setCurrentTrafficType));
	}
	else if(i>32767 && i<55705)
	{
		strncpy(setCurrentTrafficType,"Type B",sizeof(setCurrentTrafficType));
	}
	else
	{
		strncpy(setCurrentTrafficType,"Type C",sizeof(setCurrentTrafficType));
	}
	for (count =0;count<10;count++)
	{
		*(ptr+count) = setCurrentTrafficType[count];
	}
}

*/

void justPrinting(void)
{

for (int i=0;i<100;i++)
{
	printf("%d message from second process\n");
}

}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(hello_world_process, ev, data)
{ 

  PROCESS_BEGIN();
  generateInfiniteRandomNumber();
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(hello_world_process2, ev, data)
{ 

  PROCESS_BEGIN();
  justPrinting();
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

