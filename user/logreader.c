#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define FIFO_PATH "/dev/fifo/mylog"

int
main(void)
{
    char buf[128];
    int fd;
    int n;

    printf("Receiver: Connecting to pipe...\n");

    // 1. Open the pipe (Block until Sender is ready)
    fd = open(FIFO_PATH, O_RDONLY);
    if(fd < 0){
        printf("Receiver: Error - Did you run logwriter?\n");
        exit(1);
    }

    printf("Receiver: Connected! Listening for logs...\n");

    // 2. Read loop
    while((n = read(fd, buf, sizeof(buf)-1)) > 0){
        buf[n] = 0; // Null terminate
        printf("Receiver: Received -> %s", buf);
    }

    close(fd);
    printf("Receiver: Pipe closed. Exiting.\n");
    exit(0);
}