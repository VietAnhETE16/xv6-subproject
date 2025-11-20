#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int main(void)
{
  char *path = "/dev/fifo/mylog";
  char buf[100];

  printf("logreader: opening fifo for read...\n");
  // This open() will block until the writer opens its end
  int fd = open(path, O_RDONLY);
  if(fd < 0){
    fprintf(2, "logreader: open failed\n");
    exit(1);
  }

  printf("logreader: reading logs...\n");
  while(1){
    int n = read(fd, buf, sizeof(buf));
    if(n <= 0){
      break; // Writer closed
    }
    printf("logreader: read '%s'\n", buf);
  }

  close(fd);
  printf("logreader: done\n");
  exit(0);
}