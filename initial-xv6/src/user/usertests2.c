#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/syscall.h"
#include "kernel/memlayout.h"
#include "kernel/riscv.h"

#define NUM_PROCESSES 7
#define RUNTIME 30000000  // Set a shorter runtime (30 seconds)
#define PRINT_INTERVAL 5000000  // Interval for printing process status

// Function for CPU-bound process
void cpu_bound(int id) {
    int start = uptime();
    while (uptime() - start < RUNTIME) {
        for (volatile int i = 0; i < 100000; i++) {} // Reduced CPU-intensive work
        if (uptime() % PRINT_INTERVAL == 0) 
            printf("%d: CPU-bound\n", id);
    }
}

// Function for I/O-bound process
void io_bound(int id) {
    int start = uptime();
    while (uptime() - start < RUNTIME) {
        sleep(1); // Simulate I/O wait
        if (uptime() % PRINT_INTERVAL == 0) 
            printf("%d: I/O-bound\n", id);
    }
}

// Function for mixed workload process
void mixed(int id) {
    int start = uptime();
    while (uptime() - start < RUNTIME) {
        for (volatile int i = 0; i < 250000; i++) {} // Some CPU work
        sleep(1); // Simulate I/O wait
        if (uptime() % PRINT_INTERVAL == 0) 
            printf("%d: Mixed\n", id);
    }
}

// Function for short bursts of CPU work
void short_bursts(int id) {
    int start = uptime();
    while (uptime() - start < RUNTIME) {
        for (volatile int i = 0; i < 50000; i++) {} // Short CPU burst
        // No yield, just continue the loop
        if (uptime() % PRINT_INTERVAL == 0) 
            printf("%d: Short bursts\n", id);
    }
}

// Function for long bursts of CPU work
void long_bursts(int id) {
    int start = uptime();
    while (uptime() - start < RUNTIME) {
        for (volatile int i = 0; i < 1000000; i++) {} // Long CPU burst
        // No yield, just continue the loop
        if (uptime() % PRINT_INTERVAL == 0) 
            printf("%d: Long bursts\n", id);
    }
}

// Function for priority hogging process
void priority_hog(int id) {
    int start = uptime();
    while (uptime() - start < RUNTIME) {
        for (volatile int i = 0; i < 1500000; i++) {} // Very long CPU burst
        if (uptime() % PRINT_INTERVAL == 0) 
            printf("%d: Priority hog\n", id);
    }
}

// Function for frequent yielding process simulation without yield()
void frequent_yield(int id) {
    int start = uptime();
    while (uptime() - start < RUNTIME) {
        for (volatile int i = 0; i < 5000; i++) {} // Very short CPU burst
        // No yield, just continue the loop
        if (uptime() % PRINT_INTERVAL == 0) 
            printf("%d: Frequent yield\n", id);
    }
}

// Main function to create and manage processes
int main(int argc, char *argv[]) {
    int pids[NUM_PROCESSES];
    
    // Fork processes based on their types
    for (int i = 0; i < NUM_PROCESSES; i++) {
        pids[i] = fork();
        
        if (pids[i] < 0) {
            printf("Fork failed\n");
            exit(1);
        } else if (pids[i] == 0) { // Child process
            switch(i) {
                case 0: cpu_bound(i); break;
                case 1: io_bound(i); break;
                case 2: mixed(i); break;
                case 3: short_bursts(i); break;
                case 4: long_bursts(i); break;
                case 5: priority_hog(i); break;
                case 6: frequent_yield(i); break;
            }
            exit(0); // Exit child process after execution
        }
    }

    // Parent process waits for all child processes to finish
    for (int i = 0; i < NUM_PROCESSES; i++) {
        wait(0);
    }

    exit(0); // Exit the parent process
}