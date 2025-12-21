#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define FIFO_PATH "/dev/fifo/mylog"

int
main(void)
{
    int fd;
    char buf[128];
    int n;

    // 1. Create the named pipe
    mkfifo(FIFO_PATH); 

    printf("Sender: Waiting for a receiver to connect...\n");
    
    // 2. Open the pipe (Block until Receiver is ready)
    fd = open(FIFO_PATH, O_WRONLY);
    if(fd < 0){
        printf("Sender: open failed\n");
        exit(1);
    }
    
    // Sleep briefly to let the Receiver print "Connected" first
    sleep(10);
    printf("Sender: Connected! Type your messages (Ctrl-D to stop)...\n");

    // 3. Interactive Loop
    while(1){
        // Print a prompt
        printf("Sender >> ");
        
        // Read from Standard Input (Keyboard)
        n = read(0, buf, sizeof(buf));
        
        if(n <= 0){
            break; // EOF (Ctrl+D) or error
        }

        // Write the input directly to the named pipe
        if(write(fd, buf, n) != n){
            printf("Sender: write error\n");
            break;
        }

        // Sleep briefly to allow the Receiver (running in background)
        // to print its output before we print the next "Sender >>" prompt.
        sleep(5);
    } 

    close(fd);
    printf("\nSender: Done.\n");
    exit(0);
}