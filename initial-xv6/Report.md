# xv6 Report

## System Calls

### Gotta Count 'em All
- The underlying architecture is an `array` which stores the frequency of each system call and is reset before each run.
- The user program `syscount` takes in a mask and a command to run and prints the number of times the system call was called.
- The system call is implemented in the kernel by incrementing the frequency array whenever a system call is made.

### Wake Me Up When My Timer Ends
- The `proc` struct is modified to include fields for the alarm interval, alarm handler function, ticks passed, saved trapframe, and handling alarm flag.
- The `sigalarm` system call sets the alarm interval and handler function, while the `sigreturn` system call resets the handling alarm flag.
- The `usertrap()` function in `trap.c` increments the ticks passed field and calls the alarm handler function when the alarm interval is reached.


## Scheduling Policies
- Apart from the default Round Robin scheduling policy, two additional scheduling policies are implemented: Lottery Based Scheduling (LBS) and Multi-Level Feedback Queue (MLFQ).
- The `proc` struct is modified to include fields required for the specific scheduling policy.
- The `scheduler()` function is modified to include the scheduling policy logic.

### Default Round Robin (RR)

The performance observed using `schedulertest` is as follows:
- Wait Time : [108, 108, 108, 107, 107, 109, 108]
- Run Time : [8, 8, 8, 8, 8, 8, 9]

### Lottery Based Scheduling (LBS)

The performance observed using `schedulertest` is as follows:
- Wait Time : [110, 109, 109, 112, 110, 113, 115]
- Run Time : [6, 7, 7, 4, 7, 6, 6]

### Multi-Level Feedback Queue (MLFQ)

The performance observed using `schedulertest` is as follows:
- Wait Time : [110, 109, 111, 110, 109, 109, 111]
- Run Time : [8, 9, 8, 8, 9, 9, 6]

### Performance Comparison
- The average running time for the default RR policy is the lowest, followed by MLFQ and then LBS.
- The average waiting time for the LBS policy is the highest, followed by MLFQ policy and the default RR policy.
- However, the LBS policy has a lot of variance, which can be a disadvantage in real-world scenarios. While other policies are relatively stable and consistent in their performance.

## Questions

1. What is the implication of adding the arrival time in the lottery based scheduling policy?
<br>
--> Adding the arrival time in the lottery based scheduling policy ensures that the process that arrived the earliest is executed first. This prevents the condition of starvation and ensures that all processes are executed in a fair manner (by preventing randomness).

2. Are there any pitfalls to watch out for?
<br>
--> This may add some complexity to the scheduling policy (been there, done that, won't recommend) and may require additional overhead to maintain the arrival time of each process. It might also affect the response time of the system due to the additional overhead. Also, in case of multi-core systems, the (chance of) same arrival time for multiple processes may lead to a deadlock.

3. What happens if all processes have the same number of tickets?
<br>
--> In such a case, the scheduling will rely entirely on arrival times, ie, the order in which processes were created will dictate their chances of running. Loss of randomness and possibility of starvation would also be a concern.

## MLFQ Analysis

- Please refer to [this plot](./plot.png) for the MLFQ scheduling policy analysis.
- The graph is based on the `usertests2` test program and shows the change in queues over time.
- Note that a subset of time ticks is used to plot the graph for better visualization.
