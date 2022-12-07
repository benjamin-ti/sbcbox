#include <stdio.h>
#include <unistd.h> // usleep
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <errno.h>
#include <sys/mman.h>
#include <gpiod.h>

#ifndef CONSUMER
#define CONSUMER "pwmtest"
#endif

static struct gpiod_line *lineFail = NULL;

typedef struct {
    struct gpiod_line* line;
    unsigned int time_high_us;
    unsigned int time_low_us;
    unsigned char duty_cyle_percentage;
    int print_2high_jitter_gap;
    unsigned short jitter_gap_border_us;
} pwm_args;

static void pwm(pwm_args args)
{
    int ret;
    int b = 1;

    long long diff_ns;
    long diff_us;

    struct timespec ts;
    struct timespec ts2;

    unsigned int time_us = args.time_low_us;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    while (1)
    {
        ts.tv_nsec += time_us * 1000;
        if(ts.tv_nsec >= 1000*1000*1000){
            ts.tv_nsec -= 1000*1000*1000;
            ts.tv_sec++;
        }
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts,  NULL);

        if (args.print_2high_jitter_gap) {
            clock_gettime(CLOCK_MONOTONIC, &ts2);
            diff_ns = 1000*1000*1000 * (ts2.tv_sec - ts.tv_sec)
                              + (ts2.tv_nsec - ts.tv_nsec);
            diff_us = diff_ns / 1000;
            if (diff_us > args.jitter_gap_border_us) {
                // printf("CurPWMDiff: %lu us\n", diff_us);
                static int b;
                gpiod_line_set_value(lineFail, b);
                b = !b;
            }
        }

        if (b)
        {
            if (args.duty_cyle_percentage > 0) {
                ret = gpiod_line_set_value(args.line, 1);
                if (ret < 0) {
                    perror("Set line failed\n");
                    pthread_exit(NULL);
                }
                time_us = args.time_high_us;
            }
        }
        else
        {
            if (args.duty_cyle_percentage < 100) {
                ret = gpiod_line_set_value(args.line, 0);
                if (ret < 0) {
                    perror("Set line failed\n");
                    pthread_exit(NULL);
                }
                time_us = args.time_low_us;
            }
        }
        b = !b;
    }
}

static int WriteValue2File(char* pcFile, char* pcValue)
{
    int fd;
    unsigned int uiLen;

    if (NULL==pcFile || NULL==pcValue) {
        fprintf(stderr, "NULL Pointer error!\n");
        return -1;
    }

    fd = open(pcFile, O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open %s!\n", pcFile);
        return -1;
    }

    uiLen = strlen(pcValue);
    if (uiLen != write(fd, pcValue, uiLen)) {
        fprintf(stderr, "Failed to write value %s in %s!\n", pcValue, pcFile);
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

int main(void)
{
    pthread_t pwm_thread_id;
    struct sched_param paramCurr;
    struct sched_param paramStep;

    char *chipname = "gpiochip2";
    unsigned int line_num_step = 9;   // LCD_DATA3 / P8_44 / GPIO73
    unsigned int line_num_fail = 7; // LCD_DATA1 / P8_46 / GPIO71
    struct gpiod_chip *chip;
    struct gpiod_line *lineStep = NULL;
    int i, ret;

    pwm_args args;

    ret = mlockall(MCL_FUTURE|MCL_CURRENT);
    if (ret < 0) {
        perror("Failed to lock memory\n");
        goto end;
    }

    chip = gpiod_chip_open_by_name(chipname);
    if (!chip) {
        perror("Open chip failed\n");
        goto end;
    }

    lineFail = gpiod_chip_get_line(chip, line_num_fail);
    if (!lineFail) {
        perror("Get lineFail failed\n");
        goto close_chip;
    }
    ret = gpiod_line_request_output(lineFail, CONSUMER, 0);
    if (ret < 0) {
        perror("Request lineFail as output failed\n");
        goto release_line;
    }
    WriteValue2File("/sys/devices/platform/ocp/ocp:P8_46_pinmux/state", "gpio");

    lineStep = gpiod_chip_get_line(chip, line_num_step);
    if (!lineStep) {
        perror("Get lineStep failed\n");
        goto close_chip;
    }
    ret = gpiod_line_request_output(lineStep, CONSUMER, 0);
    if (ret < 0) {
        perror("Request lineStep as output failed\n");
        goto release_line;
    }
    WriteValue2File("/sys/devices/platform/ocp/ocp:P8_44_pinmux/state", "gpio");

    // Step-"Thread"
    paramStep.sched_priority = 80;
    ret = sched_setscheduler(0, SCHED_FIFO, &paramStep);
    if (ret != 0)
    {
        perror("sched_setscheduler failed\n");
        goto release_line;
    }

    args.line = lineStep;
    args.time_high_us = 100;
    args.time_low_us = 100;
    args.duty_cyle_percentage = 50;
    args.print_2high_jitter_gap = 1;
    args.jitter_gap_border_us = 50;

    pwm(args);

release_line:
    if (lineStep != NULL)
        gpiod_line_release(lineStep);
    if (lineFail != NULL)
        gpiod_line_release(lineFail);
close_chip:
    gpiod_chip_close(chip);
end:
    return 0;
}
