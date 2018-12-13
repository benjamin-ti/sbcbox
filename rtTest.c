#include <sys/mman.h> //for mmap
#include <fcntl.h> //for file opening
#include <unistd.h> //  for sleep
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

// BCM-Manual 0x7Ennnn-Bus-Adresses go to 0x3Fnnnn-Phys-Adresses
#define BCM2835_GPIO_BASE        0x3F200000

int lk_iSig;
sigset_t lk_alarm_sig;
timer_t lk_timer_id;
int lk_wakeups_missed = 0;
pthread_t lk_tid;

volatile unsigned* lk_gpset0;
volatile unsigned* lk_gpclr0;

static void TimerValueSet(unsigned int uiTimerValue_us)
{
    int iErr;
    unsigned int iNs;
    unsigned int iSec;
    struct itimerspec sItval;

    iSec = uiTimerValue_us/1000000;
    iNs = (uiTimerValue_us - (iSec * 1000000)) * 1000;
    sItval.it_interval.tv_sec = iSec;
    sItval.it_interval.tv_nsec = iNs;
    sItval.it_value.tv_sec = iSec;
    sItval.it_value.tv_nsec = iNs;
    if (timer_settime(lk_timer_id, 0, &sItval, NULL) == -1)
    {
        iErr = errno;
        printf(("%s\n", strerror(iErr)));
    }
}


static void* StepTimerThread(void* pv)
{
	static int b;
    int sig;

    while (1)
    {
        sigwait (&(lk_alarm_sig), &sig);
        lk_wakeups_missed += timer_getoverrun (lk_timer_id);
        if (b)
        	*lk_gpset0 = 0x00040000;
        else
        	*lk_gpclr0 = 0x00040000;
        b = !b;
    }

    return NULL;
}

int main(void)
{
	sigset_t alarm_sig;

    volatile char* gpio_map;
    volatile unsigned* gpfsel1;

    struct timespec tim, tim2;
    tim.tv_sec = 0;
    tim.tv_nsec = 100*1000;

    struct sigevent sSigev;
    int iRet;
    int i;

    struct sched_param params;
    int policy = 0;

    /* Block all real time signals so they can be used for the timers.
       Note: this has to be done in main() before any threads are created
       so they all inherit the same mask. Doing it later is subject to
       race conditions */
    sigemptyset (&alarm_sig);
    for (i=SIGRTMIN; i<=SIGRTMAX; i++)
        sigaddset (&alarm_sig, i);
    sigprocmask (SIG_BLOCK, &alarm_sig, NULL);

    int memfd = open("/dev/mem", O_RDWR | O_SYNC);
    if (memfd < 0)
    {
        printf("Failed to open /dev/mem (did you remember to run as root?)\n");
        exit(1);
    }

    gpio_map = mmap(
        NULL,             // Any adddress in our space will do
		4*1024,           // Map length (always 4K)
		PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
		MAP_SHARED,       // Shared with other processes
		memfd,            // File to map
		(off_t)BCM2835_GPIO_BASE // Adress to GPIO peripheral
    );

    if (gpio_map == MAP_FAILED)
    {
    	printf("mmap error %d\n", (int)gpio_map); // errno also set!
    	exit(-1);
    }

    gpfsel1 = (volatile unsigned*)(gpio_map+0x0004);
    lk_gpset0  = (volatile unsigned*)(gpio_map+0x001C);
    lk_gpclr0  = (volatile unsigned*)(gpio_map+0x0028);

    *gpfsel1 &= ~0x07000000;
    *gpfsel1 |= 0x01000000; // OUTPUT

    /* Create the signal mask that will be used in wait_period */
    lk_iSig = SIGRTMIN;
    sigemptyset(&lk_alarm_sig);
    sigaddset(&lk_alarm_sig, lk_iSig);
    pthread_sigmask (SIG_BLOCK, &lk_alarm_sig, NULL);

    /* Create a timer that will generate the signal we have chosen */
    sSigev.sigev_notify = SIGEV_SIGNAL;
    sSigev.sigev_signo = lk_iSig;
    sSigev.sigev_value.sival_ptr = (void *)&lk_timer_id;
    iRet = timer_create (CLOCK_MONOTONIC, &sSigev, &lk_timer_id);
    if (iRet == -1)
    {
    	printf(("%s\n", strerror(iRet)));
        return 1;
    }

    iRet = pthread_create(&lk_tid, NULL, &StepTimerThread, NULL);
    if (iRet != 0)
    {
    	printf(("%s\n", strerror(iRet)));
        return 1;
    }

    // We'll set the priority to the maximum.
    params.sched_priority = sched_get_priority_max(SCHED_FIFO);
    // Attempt to set thread real-time priority to the SCHED_FIFO policy
    iRet = pthread_setschedparam(lk_tid, SCHED_FIFO, &params);
    if (iRet != 0) {
    	printf(("%s\n", strerror(iRet)));
        return 1;
    }

    // Now verify the change in thread priority
    iRet = pthread_getschedparam(lk_tid, &policy, &params);
    if (iRet != 0) {
    	printf(("%s\n", strerror(iRet)));
        return 1;
    }

     // Check the correct policy was applied
	if (policy != SCHED_FIFO) {
		printf("Scheduling is NOT SCHED_FIFO!\n");
	} else {
		 printf("SCHED_FIFO OK\n");
	}

	// Print thread scheduling priority
	printf("Thread priority is %i\n", params.sched_priority);

    TimerValueSet(200);

/*
    while(1)
    {
    	*lk_gpset0 = 0x00040000;
    	nanosleep(&tim, &tim2);
    	*lk_gpclr0 = 0x00040000;
    	nanosleep(&tim, &tim2);
    }
*/
    sleep(30);

    printf("%i\n", lk_wakeups_missed);

    return 0;
}

