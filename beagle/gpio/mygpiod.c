#include <gpiod.h>
#include <stdio.h>
#include <unistd.h> // usleep

#ifndef CONSUMER
#define CONSUMER "mygpiod"
#endif

int main(void)
{
    char *chipname = "gpiochip1";
    unsigned int line_num = 11; // GPIO Pin #43
    struct gpiod_chip *chip;
    struct gpiod_line *line;
    int i, ret;

    chip = gpiod_chip_open_by_name(chipname);
    if (!chip) {
        perror("Open chip failed\n");
        goto end;
    }

    line = gpiod_chip_get_line(chip, line_num);
    if (!line) {
        perror("Get line failed\n");
        goto close_chip;
    }

    ret = gpiod_line_request_output(line, CONSUMER, 0);
    if (ret < 0) {
        perror("Request line as output failed\n");
        goto release_line;
    }

    while(1) {
        ret = gpiod_line_set_value(line, 0);
        if (ret < 0) {
            perror("Request line as output failed\n");
            goto release_line;
        }
        usleep(500*1000);
        ret = gpiod_line_set_value(line, 1);
        if (ret < 0) {
            perror("Request line as output failed\n");
            goto release_line;
        }
        usleep(500*1000);
    }


release_line:
    gpiod_line_release(line);
close_chip:
    gpiod_chip_close(chip);
end:
    return 0;
}
