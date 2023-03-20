#include <gpiod.h>
#include <stdio.h>
#include <unistd.h> // usleep

#ifndef CONSUMER
#define CONSUMER "mygpiod"
#endif

int main(void)
{
    char *chipname = "gpiochip1";
    char *chipname2 = "gpiochip2";
    unsigned int line_num = 28; // P9_12 - GPIO_60
    unsigned int line_num_step = 9;   // LCD_DATA3 / P8_44 / GPIO73
    struct timespec ts = { 1, 0 };
    struct gpiod_line_event event;
    struct gpiod_chip *chip;
    struct gpiod_chip *chip2;
    struct gpiod_line *line;
    struct gpiod_line *lineStep;
    int i, ret = 0;
    int val;

    chip = gpiod_chip_open_by_name(chipname);
    if (!chip) {
        perror("Open chip failed\n");
        goto end;
    }

    chip2 = gpiod_chip_open_by_name(chipname2);
    if (!chip) {
        perror("Open chip failed\n");
        goto end;
    }

    line = gpiod_chip_get_line(chip, line_num);
    if (!line) {
        perror("Get line failed\n");
        ret = -1;
        goto close_chip;
    }

    ret = gpiod_line_request_rising_edge_events(line, CONSUMER);
    if (ret < 0) {
        perror("Request line as input failed\n");
        ret = -1;
        goto release_line;
    }

    lineStep = gpiod_chip_get_line(chip2, line_num_step);
    if (!lineStep) {
        perror("Get lineStep failed\n");
        goto close_chip;
    }

    ret = gpiod_line_request_output(lineStep, CONSUMER, 0);
    if (ret < 0) {
        perror("Request lineStep as output failed\n");
        goto release_line;
    }

    /* Notify event up to 20 times */
    i = 0;
    while (i <= 20) {
        ret = gpiod_line_event_wait(line, &ts);
        if (ret < 0) {
            perror("Wait event notification failed\n");
            ret = -1;
            goto release_line;
        } else if (ret == 0) {
            ret = gpiod_line_set_value(lineStep, 0);
            if (ret < 0) {
                perror("Set lineStep failed\n");
                goto release_line;
            }
            printf("Wait event notification on line #%u timeout\n", line_num);
            continue;
        }

        ret = gpiod_line_event_read(line, &event);
        printf("Get event notification on line #%u %d times\n", line_num, i);
        if (ret < 0) {
            perror("Read last event notification failed\n");
            ret = -1;
            goto release_line;
        }

        ret = gpiod_line_set_value(lineStep, 1);
        if (ret < 0) {
            perror("Set lineStep failed\n");
            goto release_line;
        }

        i++;
    }

    ret = 0;

release_line:
    gpiod_line_release(line);
    gpiod_line_release(lineStep);
close_chip:
    gpiod_chip_close(chip);
    gpiod_chip_close(chip2);
end:
    return ret;
}
