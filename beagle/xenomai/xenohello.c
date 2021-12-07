#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <gpiod.h>

#include <alchemy/task.h>

#ifndef CONSUMER
#define CONSUMER "mygpiod"
#endif

RT_TASK hello_task;

// function to be executed by task
void hello_task_func(void *arg)
{
    RT_TASK_INFO curtaskinfo;

    char *chipname = "gpiochip1";
    unsigned int line_num = 28; // P9_12 - GPIO_60
    struct gpiod_chip *chip;
    struct gpiod_line *line;
    int i, ret;

    rt_printf("Hello World!\n");

    // inquire current task
    rt_task_inquire(NULL, &curtaskinfo);

    // print task name
    rt_printf("Task name : %s \n", curtaskinfo.name);

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
    return;
}

int main(int argc, char *argv[])
{
    char  str[10];

    rt_printf("Start RT Task\n");

    sprintf(str, "hello");
    /* Create task
     * Arguments: &task,
     *            name,
     *            stack size (0=default),
     *            priority,
     *            mode (FPU, start suspended, ...)
     */
    rt_task_create(&hello_task, str, 0, 50, 0);

    /*  Start task
     * Arguments: &task,
     *            task function,
     *            function argument
     */
    rt_task_start(&hello_task, &hello_task_func, 0);

    sleep(5);

    return 0;
}
