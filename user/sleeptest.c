#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int start_ticks, end_ticks;

  printf("sleeptest: starting\n");
  
  // 50 ticks usually equals roughly 5 seconds in standard xv6 configuration
  printf("sleeptest: sleeping for 50 ticks (approx 5 seconds)...\n");
  
  // Record the time before sleeping
  start_ticks = uptime();
  if(start_ticks < 0){
    printf("sleeptest: uptime failed\n");
    exit(1);
  }
  
  // This calls the sys_sleep system call you implemented
  sleep(50); 
  
  // Record the time after waking up
  end_ticks = uptime();
  
  printf("sleeptest: woke up! (slept for %d ticks)\n", end_ticks - start_ticks);
  
  exit(0);
}