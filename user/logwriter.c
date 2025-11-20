#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

//
// 'sprintf', 'strcpy', and 'strcat' do not exist in the xv6 user library.
// We must build the string manually.
//

// A simple itoa (integer to ASCII) function
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
  char *path = "/dev/fifo/mylog";

  printf("logwriter: creating fifo...\n");
  if(mkfifo(path) < 0){
    fprintf(2, "logwriter: mkfifo failed\n");
  }

  printf("logwriter: opening fifo for write...\n");
  int fd = open(path, O_WRONLY);
  if(fd < 0){
    fprintf(2, "logwriter: open failed\n");
    exit(1);
  }

  printf("logwriter: writing logs...\n");
  for(int i = 0; i < 5; i++){
    char buf[100];
    char num_buf[16];
    char *base_str = "log message ";
    char *p;
    
    // Manually build the string
    
    // 1. Manual strcpy: Copy the base string
    p = buf;
    char *s = base_str;
    while(*s){
      *p++ = *s++;
    }
    
    // 2. Convert the integer to a string
    itoa(i, num_buf);
    
    // 3. Manual strcat: Concatenate the number string
    s = num_buf;
    while(*s){
      *p++ = *s++;
    }
    
    // 4. Null-terminate the final string
    *p = '\0';
    
    // strlen() is available in ulib.c, so this is safe
    write(fd, buf, strlen(buf) + 1); // +1 to include the null terminator
    printf("logwriter: wrote '%s'\n", buf);
  }

  close(fd);
  printf("logwriter: done\n");
  exit(0);
}