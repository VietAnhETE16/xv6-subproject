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

    // Ensure the FIFO exists on the filesystem
    // If it already exists, mkfifo returns -1, which we can safely ignore
    mkfifo(FIFO_PATH);

    printf("Reader: Opening pipe... (Waiting for Writer to connect)\n");

    // Open for READING - This blocks in the kernel until a Writer connects
    fd = open(FIFO_PATH, O_RDONLY);
    if(fd < 0){
        printf("Reader: Error - Could not open pipe %s\n", FIFO_PATH);
        exit(1);
    }

    printf("Reader: Connected! Listening for data...\n");

    // Continuous read loop
    // read() returns 0 when the Writer closes their end of the pipe
    while((n = read(fd, buf, sizeof(buf)-1)) > 0){
        buf[n] = 0; // Null-terminate string for printf
        printf("Reader: Received -> %s", buf);
    }

    if(n < 0) {
        printf("Reader: Error while reading data\n");
    }

    close(fd);
    printf("Reader: Writer disconnected. Exiting.\n");
    exit(0);
}