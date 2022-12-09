#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(void)
{
#define READ_VAL_BUF_LEN 50
    char pcReadVal[READ_VAL_BUF_LEN];

    char* pcFile = "/dev/rpmsg_pru31";
    char* line;
    size_t len;
    ssize_t lineSize;
    int fd;

    printf("Enter String:\n");
    line = NULL;
    len = 0;
    lineSize = getline(&line, &len, stdin);
    len = lineSize-1;
    line[len] = '\0';
    printf("You entered '%s', which has %zu chars.\n", line, len);

    fd = open(pcFile, O_RDWR);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open %s!\n", pcFile);
        return -1;
    }

    if (len != write(fd, line, len)) {
        fprintf(stderr, "Failed to write value %s in %s!\n", line, pcFile);
        close(fd);
        return -1;
    }

    len = read(fd, pcReadVal, READ_VAL_BUF_LEN-1);
    pcReadVal[len] = '\0';
    if (len > 0) {
        printf("Echo(%u): '%s'\n", len, pcReadVal);
    }

    free(line);

    close(fd);

    return 0;
}
