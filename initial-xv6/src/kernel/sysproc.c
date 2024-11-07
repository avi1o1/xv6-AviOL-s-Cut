#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n)
  {
    if (killed(myproc()))
    {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_waitx(void)
{
  uint64 addr, addr1, addr2;
  uint wtime, rtime;
  argaddr(0, &addr);
  argaddr(1, &addr1); // user virtual memory
  argaddr(2, &addr2);
  int ret = waitx(addr, &wtime, &rtime);
  struct proc *p = myproc();
  if (copyout(p->pagetable, addr1, (char *)&wtime, sizeof(int)) < 0)
    return -1;
  if (copyout(p->pagetable, addr2, (char *)&rtime, sizeof(int)) < 0)
    return -1;
  return ret;
}

uint64 syscall_counts[32] = {0};
char* syscall_names[] = {
  "fork", "exit", "wait", "pipe", "read",
  "kill", "exec", "fstat", "chdir", "dup",
  "getpid", "sbrk", "sleep", "uptime", "open",
  "write", "mknod", "unlink", "link", "mkdir",
  "close", "getsyscount"
};

// Clear the syscall count
void clear_syscall_count() {
  for (int i = 0; i < 32; i++) {
    syscall_counts[i] = 0;
  }
}

// Increment the syscall count
void increment_syscall_count(int syscall_num) {
  if(syscall_num >= 0 && syscall_num < 32) {
    syscall_counts[syscall_num]++;
  }
}

// Function to implement the getSysCount system call
uint64 sys_getsyscount(void) {
  int mask;
  argint(0, &mask);
  if(mask < 0) {
    return -1;
  }
  
  // Find the syscall number from the mask
  int syscall_num = -1;
  for(int i = 0; i < 32; i++) {
    if(mask == (1 << i)) {
      syscall_num = i;
      break;
    }
  }
  
  // Check for invalid syscall number
  if(syscall_num == -1 || syscall_num >= 32) {
    return -1;
  }
  
  // Return the count of the syscall
  int ret = syscall_counts[syscall_num];
  clear_syscall_count(syscall_num);
  return ret;
}

uint64 sys_sigalarm(void) {
  int interval;
  uint64 handler;

  argint(0, &interval);
  argaddr(1, &handler);
  if (interval < 0 || handler < 0) {
    return -1;
  }

  struct proc* p = myproc();
  p->alarm_interval = interval;
  p->alarm_handler = (void(*)())handler;
  p->ticks_count = 0;
  p->alarm_on = (interval > 0);
  p->handling_alarm = 0;

  return 0;
}

uint64 sys_sigreturn(void) {
  struct proc *p = myproc();
  if (p->alarm_tf == 0 || !p->handling_alarm) {
    return -1;
  }

  // Restore the trapframe
  memmove(p->trapframe, p->alarm_tf, sizeof(struct trapframe));
  kfree(p->alarm_tf);
  p->alarm_tf = 0;
  p->handling_alarm = 0;
  p->ticks_count = 0;

  return 0;
}

#ifdef LBS
uint64 sys_settickets(void) {
  int tickets;
  argint(0, &tickets);
  if(tickets < 1) {
    return -1;
  }
  
  struct proc *p = myproc();
  p->tickets = tickets;
  return p->tickets;
}
#endif
