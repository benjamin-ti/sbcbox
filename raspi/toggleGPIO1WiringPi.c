#include <stdio.h>
#include <wiringPi.h>
 
int main (void)
{
	if (wiringPiSetup () == -1)
		return 1 ;
 
	pinMode (1, OUTPUT) ;
 
	for (;;)
	{
		digitalWrite(1, 1) ;
		delay (500);
		digitalWrite(1, 0);
		delay (500);
	}
	
	return 0 ;
}
