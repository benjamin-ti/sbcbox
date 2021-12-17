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
#define CONSUMER "motor"
#endif

static struct gpiod_line *lineCurr = NULL;

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

        if (args.print_2high_jitter_gap) {
            clock_gettime(CLOCK_MONOTONIC, &ts2);
            diff_ns = 1000*1000*1000 * (ts2.tv_sec - ts.tv_sec)
                              + (ts2.tv_nsec - ts.tv_nsec);
            diff_us = diff_ns / 1000;
            if (diff_us > args.jitter_gap_border_us) {
                printf("CurPWMDiff: %lu us\n", diff_us);
            }
        }
    }
}

static void* pwmThread(void *argv)
{
    pwm_args args;

    args.line = lineCurr;
    args.time_high_us = 1*1000;
    args.time_low_us = 1*1000;
    args.duty_cyle_percentage = 50;
    args.print_2high_jitter_gap = 0;
    args.jitter_gap_border_us = 500;

    pwm(args);

    pthread_exit(NULL);
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

static int WriteValue2FileChecked(char* pcFile, char* pcValue)
{
#define READ_VAL_BUF_LEN 20
    char pcReadVal[READ_VAL_BUF_LEN];
    int fd;
    unsigned int uiLen;

    if (NULL==pcFile || NULL==pcValue) {
        fprintf(stderr, "NULL Pointer error!\n");
        return -1;
    }

    fd = open(pcFile, O_RDWR);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open %s!\n", pcFile);
        return -1;
    }

    uiLen = read(fd, pcReadVal, READ_VAL_BUF_LEN-1);
    if (uiLen > 0) {
        printf("ReadVal %u: '%s'\n", uiLen, pcReadVal);
        if (strcmp(pcReadVal, pcValue) == 0) {
            close(fd);
            return 0;
        }
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
    unsigned int line_num_curr = 7;   // LCD_DATA1 / P8_46 / GPIO71
    unsigned int line_num_step = 9;   // LCD_DATA3 / P8_44 / GPIO73
    unsigned int line_num_reset = 12; // LCD_DATA6 / P8_39 / GPIO76
    struct gpiod_chip *chip;
    struct gpiod_line *lineStep = NULL;
    struct gpiod_line *lineReset = NULL;
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

    // Init reset GPIO
    lineReset = gpiod_chip_get_line(chip, line_num_reset);
    if (!lineReset) {
        perror("Get lineReset failed\n");
        goto close_chip;
    }

    ret = gpiod_line_request_output(lineReset, CONSUMER, 0);
    if (ret < 0) {
        perror("Request lineReset as output failed\n");
        goto release_line;
    }

    ret = gpiod_line_set_value(lineReset, 1);
    if (ret < 0) {
        perror("Set lineReset failed\n");
        goto release_line;
    }

/*
    // Init current GPIO
    int iPWMDirExist = 0;
    DIR* dir = opendir("/sys/class/pwm/pwmchip7/pwm-7:1/");
    if (dir) {
        iPWMDirExist = 1;
        closedir(dir);
    } else if (ENOENT == errno) {
        // Directory does not exist
    } else {
        // opendir() failed for some other reason
    }

    ret = WriteValue2File("/sys/devices/platform/ocp/ocp:P8_46_pinmux/state", "pwm");
    if (ret < 0) goto release_line;
    if (!iPWMDirExist) {
        ret = WriteValue2File("/sys/class/pwm/pwmchip7/export", "1");
        if (ret < 0) goto release_line;
    }
    ret = WriteValue2FileChecked("/sys/class/pwm/pwmchip7/pwm-7:1/enable", "0");
    if (ret < 0) goto release_line;
    ret = WriteValue2FileChecked("/sys/class/pwm/pwmchip7/pwm-7:1/duty_cycle", "0");
    if (ret < 0) goto release_line;
    ret = WriteValue2File("/sys/class/pwm/pwmchip7/pwm-7:1/period", "4600");
    if (ret < 0) goto release_line;
    ret = WriteValue2File("/sys/class/pwm/pwmchip7/pwm-7:1/duty_cycle", "2300");
    if (ret < 0) goto release_line;
    ret = WriteValue2File("/sys/class/pwm/pwmchip7/pwm-7:1/enable", "1");
    if (ret < 0) goto release_line;
*/

    WriteValue2File("/sys/devices/platform/ocp/ocp:P8_46_pinmux/state", "gpio");

    lineCurr = gpiod_chip_get_line(chip, line_num_curr);
    if (!lineCurr) {
        perror("Get lineCurr failed\n");
        goto close_chip;
    }

    ret = gpiod_line_request_output(lineCurr, CONSUMER, 0);
    if (ret < 0) {
        perror("Request lineCurr as output failed\n");
        goto release_line;
    }

    // PWM-Current Thread
    ret = pthread_create(&pwm_thread_id, NULL, pwmThread, 0);
    if (ret != 0)
    {
        perror("pthread_create failed\n");
        goto release_line;
    }

    paramCurr.sched_priority = 40;
    ret = pthread_setschedparam(pwm_thread_id, SCHED_FIFO, &paramCurr);
    if (ret != 0)
    {
        perror("pthread_setschedparam failed\n");
        goto release_line;
    }

    ret = pthread_detach(pwm_thread_id);
    if (ret)
    {
        perror("pthread_detach failed\n");
        goto release_line;
    }

    // Init step GPIO
    WriteValue2File("/sys/devices/platform/ocp/ocp:P8_44_pinmux/state", "gpio");

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

    // Step-"Thread"
    paramStep.sched_priority = 80;
    ret = sched_setscheduler(0, SCHED_FIFO, &paramStep);
    if (ret != 0)
    {
        perror("sched_setscheduler failed\n");
        goto release_line;
    }

    args.line = lineStep;
    args.time_high_us = 500;
    args.time_low_us = 500;
    args.duty_cyle_percentage = 50;
    args.print_2high_jitter_gap = 1;
    args.jitter_gap_border_us = 200;

    pwm(args);

/*
    while(1) {
        ret = gpiod_line_set_value(lineStep, 0);
        if (ret < 0) {
            perror("Set lineStep failed\n");
            goto release_line;
        }
        usleep(500*1000);
        ret = gpiod_line_set_value(lineStep, 1);
        if (ret < 0) {
            perror("Set lineStep failed\n");
            goto release_line;
        }
        usleep(500*1000);
    }
*/

release_line:
    if (lineCurr != NULL)
        gpiod_line_release(lineCurr);
    if (lineStep != NULL)
        gpiod_line_release(lineStep);
    if (lineReset != NULL)
        gpiod_line_release(lineReset);
close_chip:
    gpiod_chip_close(chip);
end:
    return 0;
}
