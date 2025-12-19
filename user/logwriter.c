#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define FIFO_PATH "/dev/fifo/mylog"

// Helper to convert int to string
void
itoa(int x, char *buf)
{
    int i = 0, j;
    char tmp[16];
    if (x == 0) { buf[0] = '0'; buf[1] = 0; return; }
    while (x > 0) { tmp[i++] = '0' + (x % 10); x /= 10; }
    for (j = 0; j < i; j++) buf[j] = tmp[i - j - 1];
    buf[i] = 0;
}

int
main(void)
{
    int i;
    int fd;

    // 1. Create the named pipe
    // We ignore error if it already exists
    mkfifo(FIFO_PATH); 

    printf("Sender: Waiting for a receiver to connect...\n");
    
    // 2. Open the pipe (Block until Receiver is ready)
    fd = open(FIFO_PATH, O_WRONLY);
    if(fd < 0){
        printf("Sender: open failed\n");
        exit(1);
    }
    
    printf("Sender: Connected! Starting to send logs...\n");

    // 3. Send 5 messages
    for(i = 0; i < 5; i++){
        char msg[64] = "log message ";
        char num[16];
        int p = 12; // length of "log message "

        itoa(i, num);
        for(int k=0; num[k]; k++) msg[p++] = num[k];
        msg[p++] = '\n';
        msg[p] = 0;

        write(fd, msg, p);
        printf("Sender: Sent 'log message %d'\n", i);
        
        // Sleep to simulate live streaming and let Receiver print cleanly
        sleep(20); 
    }

    close(fd);
    printf("Sender: Done.\n");
    exit(0);
}