#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  if(argc < 3){
    fprintf(2, "Oopsie Woopsie : Invalid argument length!\n");
    exit(1);
  }

  int mask = atoi(argv[1]);
  
  int pid = fork();
  if (pid < 0){
    fprintf(2, "Oopsie Woopsie : fork() failed!\n");
    exit(1);
  }
  
  if (pid == 0){
    // Child process
    exec(argv[2], &argv[2]);
    fprintf(2, "Oopsie Woopsie : exec() %s failed!\n", argv[2]);
    exit(1);
  }
  else {
    // Parent process
    wait(0);
    int count = getsyscount(mask);
    
    // Find the syscall name
    char *syscall_name = "unknown";
    for (int i = 0; i < 32; i++) {
      if (mask == (1 << i)) {
        switch(i) {
          case 1: syscall_name = "fork"; break;
          case 2: syscall_name = "exit"; break;
          case 3: syscall_name = "wait"; break;
          case 4: syscall_name = "pipe"; break;
          case 5: syscall_name = "read"; break;
          case 6: syscall_name = "kill"; break;
          case 7: syscall_name = "exec"; break;
          case 8: syscall_name = "fstat"; break;
          case 9: syscall_name = "chdir"; break;
          case 10: syscall_name = "dup"; break;
          case 11: syscall_name = "getpid"; break;
          case 12: syscall_name = "sbrk"; break;
          case 13: syscall_name = "sleep"; break;
          case 14: syscall_name = "uptime"; break;
          case 15: syscall_name = "open"; break;
          case 16: syscall_name = "write"; break;
          case 17: syscall_name = "mknod"; break;
          case 18: syscall_name = "unlink"; break;
          case 19: syscall_name = "link"; break;
          case 20: syscall_name = "mkdir"; break;
          case 21: syscall_name = "close"; break;
          case 22: syscall_name = "getsyscount"; break;
          case 23: syscall_name = "sigalarm"; break;
          case 24: syscall_name = "sigreturn"; break;
          case 25: syscall_name = "settickets"; break;
          #ifdef LBS
          case 26: syscall_name = "settickets"; break;
          #endif
        }
        break;
      }
    }
    
    
    printf("PID %d called %s %d times\n", pid, syscall_name, count);
  }
  
  exit(0);
}