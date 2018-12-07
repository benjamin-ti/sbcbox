#include <wiringPi.h>

#include <stdio.h>
#include <stdlib.h>

const int PWM_pin = 1; // GPIO 1 as per WiringPi, GPIO18 as per BCM

int main(void)
{
    if (wiringPiSetup() == -1)
        return 1;
        
    pinMode(PWM_pin, PWM_OUTPUT);
    
    pwmSetMode(PWM_MODE_MS);
    //pwmSetMode(PWM_MODE_BAL);
    pwmSetRange(1024);
    // 19,2 MHz Clock
    pwmSetClock(7);
    pwmWrite(PWM_pin, 100);
    delay(1000);
        
    return 0;
}
