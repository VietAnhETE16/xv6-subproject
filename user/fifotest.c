#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define FIFO_PATH "/dev/fifo/mylog"
#define NMSG 5

// Convert integer to string (simple, no snprintf)
void
itoa(int x, char *buf)
{ 
  static char digits[] = "0123456789";
  char tmp[16];
  int i, j;
  int neg = 0;

  if (x < 0) {
    neg = 1;
    x = -x;
  }

  i = 0;
  do {
    tmp[i++] = digits[x % 10];
    x /= 10;
  } while (x);

  if (neg)
    tmp[i++] = '-';
  
  // Reverse the string
  j = 0;
  while(--i >= 0)
    buf[j++] = tmp[i];
  buf[j] = '\0';
}

int
main(void)
{
    char buf[128];
    int i;

    printf("fifotest: creating fifo '%s'\n", FIFO_PATH);
    if (mkfifo(FIFO_PATH) < 0) {
        printf("fifotest: mkfifo failed\n");
        exit(1);
    }

    int pid = fork();
    if (pid < 0) {
        printf("fifotest: fork failed\n");
        exit(1);
    }

    if (pid == 0) {
        // -------------------------------------
        // CHILD = writer
        // -------------------------------------
        int wfd = open(FIFO_PATH, O_WRONLY);
        if (wfd < 0) {
            printf("child: open for write failed\n");
            exit(1);
        }

        for (i = 0; i < NMSG; i++) {
            // build "log message X\n" manually
            char msg[64];
            char num[16];
            int p = 0;

            // "log message "
            msg[p++] = 'l';
            msg[p++] = 'o';
            msg[p++] = 'g';
            msg[p++] = ' ';
            msg[p++] = 'm';
            msg[p++] = 'e';
            msg[p++] = 's';
            msg[p++] = 's';
            msg[p++] = 'a';
            msg[p++] = 'g';
            msg[p++] = 'e';
            msg[p++] = ' ';

            // convert number
            itoa(i, num);

            // copy number
            for (int k = 0; num[k]; k++)
                msg[p++] = num[k];

            // newline
            msg[p++] = '\n';
            msg[p] = 0;

            write(wfd, msg, p);
            sleep(20);
        }

        close(wfd);
        exit(0);
    }

    // -------------------------------------
    // PARENT = reader
    // -------------------------------------
    int rfd = open(FIFO_PATH, O_RDONLY);
    if (rfd < 0) {
        printf("parent: open for read failed\n");
        exit(1);
    }

    printf("parent: reading...\n");

    int n;
    while ((n = read(rfd, buf, sizeof(buf)-1)) > 0) {
        buf[n] = 0;
        printf("parent read: %s", buf);
    }

    printf("parent: done reading, waiting for child...\n");
    wait(0);

    printf("fifotest: all done.\n");
    exit(0);
}
