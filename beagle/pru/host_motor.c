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

    uiLen = strlen(pcValue);
    if (uiLen != write(fd, pcValue, uiLen)) {
        fprintf(stderr, "Failed to write value %s in %s!\n", pcValue, pcFile);
        close(fd);
        return -1;
    }

    uiLen = read(fd, pcReadVal, READ_VAL_BUF_LEN-1);
    if (uiLen>0 && uiLen<READ_VAL_BUF_LEN-1) {
        pcReadVal[uiLen] = '\0';
        printf("ReadVal %u: '%s'\n", uiLen, pcReadVal);
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
    struct gpiod_line *lineReset = NULL;
    int i, ret;

    volatile char* gpio_map;

    ret = mlockall(MCL_FUTURE|MCL_CURRENT);
    if (ret < 0) {
        perror("Failed to lock memory\n");
        goto end;
    }


    int memfd = open("/dev/mem", O_RDWR | O_SYNC);
    if (memfd < 0) {
        printf("Failed to open /dev/mem (did you remember to run as root?)\n");
        exit(1);
    }

    gpio_map = mmap(
        NULL,             // Any adddress in our space will do
        4*1024,           // Map length (always 4K)
        PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
        MAP_SHARED,       // Shared with other processes
        memfd,            // File to map
        (off_t)0x9ED00000 // Adress to GPIO peripheral
    );

    if (gpio_map == MAP_FAILED) {
        printf("mmap error %d\n", (int)gpio_map); // errno also set!
        exit(-1);
    }

    *gpio_map = 'B';


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

    // Init current GPIO
    ret = WriteValue2File("/sys/devices/platform/ocp/ocp:P8_46_pinmux/state", "pruout");
    if (ret < 0) goto release_line;

    // Init step GPIO
    ret = WriteValue2File("/sys/devices/platform/ocp/ocp:P8_44_pinmux/state", "pruout");
    if (ret < 0) goto release_line;

    getchar();

    while(1) {
        ret = WriteValue2FileChecked("/dev/rpmsg_pru31", "ST");
        if (ret < 0)
            goto release_line;
        getchar();
        ret = WriteValue2FileChecked("/dev/rpmsg_pru31", "SP");
        if (ret < 0)
            goto release_line;
        getchar();
    }

release_line:
    if (lineReset != NULL)
        gpiod_line_release(lineReset);
close_chip:
    gpiod_chip_close(chip);
end:
    return 0;
}
