#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include <alchemy/task.h>

RT_TASK hello_task;

// function to be executed by task
void hello_task_func(void *arg)
{
    RT_TASK_INFO curtaskinfo;

    rt_printf("Hello World!\n");

    // inquire current task
    rt_task_inquire(NULL, &curtaskinfo);

    // print task name
    rt_printf("Task name : %s \n", curtaskinfo.name);
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

    return 0;
}

