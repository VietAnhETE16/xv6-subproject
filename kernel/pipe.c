#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"

//
// struct pipe, PIPESIZE, and NPIPE should all be defined in kernel/file.h now.
//

// Define the global array of pipes.
// "extern struct pipe pipes[NPIPE];" in file.h is the declaration.
// This is the actual definition.
struct pipe pipes[NPIPE];

// NEW FUNCTION: pipeinit()
// This must be called from main() in kernel/main.c at boot.
void
pipeinit(void)
{
  struct pipe *p;
  for(p = pipes; p < &pipes[NPIPE]; p++){
    initlock(&p->lock, "pipe");
    p->readopen = 0;
    p->writeopen = 0;
  }
}

int
pipealloc(struct file **f0, struct file **f1)
{
  struct pipe *p;

  p = 0;
  *f0 = *f1 = 0;

  // Allocate two file structures
  if((*f0 = filealloc()) == 0 || (*f1 = filealloc()) == 0)
    goto bad;

  // MODIFIED: Find a free pipe in the global array instead of kalloc'ing
  p = 0;
  for(p = pipes; p < &pipes[NPIPE]; p++){
    acquire(&p->lock);
    if(p->readopen == 0 && p->writeopen == 0){
      // Found a free pipe
      p->readopen = 1;
      p->writeopen = 1;
      p->nwrite = 0;
      p->nread = 0;
      release(&p->lock);
      break; // Success
    }
    release(&p->lock);
  }

  if(p == &pipes[NPIPE]){ // No free pipe found
    goto bad;
  }
  
  // Set up the file descriptors
  (*f0)->type = FD_PIPE;
  (*f0)->readable = 1;
  (*f0)->writable = 0;
  (*f0)->pipe = p;
  (*f0)->ip = 0; // Unnamed pipe has no inode
  
  (*f1)->type = FD_PIPE;
  (*f1)->readable = 0;
  (*f1)->writable = 1;
  (*f1)->pipe = p;
  (*f1)->ip = 0; // Unnamed pipe has no inode
  
  return 0;

 bad:
  // if(p) kfree((char*)p); // MODIFIED: No kfree
  if(*f0)
    fileclose(*f0);
  if(*f1)
    fileclose(*f1);
  return -1;
}

void
pipeclose(struct pipe *pi, int writable)
{
  acquire(&pi->lock);
  if(writable){
    pi->writeopen = 0;
    wakeup(&pi->nread);
  } else {
    pi->readopen = 0;
    wakeup(&pi->nwrite);
  }

  // FIXED: Reset counters when pipe is fully closed so it can be reused cleanly.
  // This prevents stale counts from interfering with future named pipe sessions.
  if(pi->readopen == 0 && pi->writeopen == 0){
    pi->nread = 0;
    pi->nwrite = 0;
  }
  
  release(&pi->lock);
}

//
// pipewrite() and piperead() do not need any changes.
// Their logic is independent of how the pipe was allocated.
//

int
pipewrite(struct pipe *p, uint64 addr, int n)
{
  int i = 0;
  struct proc *pr = myproc();

  acquire(&p->lock);
  while(i < n){
    if(p->readopen == 0 || killed(pr)){
      release(&p->lock);
      return -1;
    }
    if(p->nwrite == p->nread + PIPESIZE){ //DOC: pipewrite-full
      wakeup(&p->nread);
      sleep(&p->nwrite, &p->lock);
    } else {
      char ch;
      if(copyin(pr->pagetable, &ch, addr + i, 1) == -1)
        break;
      p->data[p->nwrite++ % PIPESIZE] = ch;
      i++;
    }
  }
  wakeup(&p->nread);
  release(&p->lock);

  return i;
}

int
piperead(struct pipe *p, uint64 addr, int n)
{
  int i;
  struct proc *pr = myproc();
  char ch;

  acquire(&p->lock);
  while(p->nread == p->nwrite && p->writeopen){  //DOC: pipe-empty
    if(killed(pr)){
      release(&p->lock);
      return -1;
    }
    sleep(&p->nread, &p->lock); //DOC: piperead-sleep
  }
  for(i = 0; i < n; i++){  //DOC: piperead-copy
    if(p->nread == p->nwrite)
      break;
    ch = p->data[p->nread % PIPESIZE];
    if(copyout(pr->pagetable, addr + i, &ch, 1) == -1) {
      if(i == 0)
        i = -1;
      break;
    }
    p->nread++;
  }
  wakeup(&p->nwrite);  //DOC: piperead-wakeup
  release(&p->lock);
  return i;
}