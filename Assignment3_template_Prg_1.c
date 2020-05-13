/**********************************************************************************

           *************NOTE**************
This is a template for the subject of RTOS in University of Technology Sydney(UTS)
Please complete the code based on the assignment requirement.

Assignment 3 Program_2 template

**********************************************************************************/
/*
  To compile prog_1 ensure that gcc is installed and run the following command:
  gcc -Wall -O2 program_1.c -o prog_1 -lpthread -lrt

*/

#include <pthread.h> 	/* pthread functions and data structures for pipe */
#include <unistd.h> 	/* for POSIX API */
#include <stdlib.h> 	/* for exit() function */
#include <stdio.h>	/* standard I/O routines */
#include <stdbool.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>

#define PROCESSNUM 7

typedef struct SRTF_Params 
{
  //add your variables here
  int pid;
  int arrive_t, wait_t, burst_t, turnaround_t, remain_t;
  
} Process_Params;

/*---------------------------------- Variables -------------------------------*/
//Array of proceses with 1 extra for placeholder remain_t
Process_Params processes[PROCESSNUM+1];
//Index variable
int i;
//Calculated averages
float avg_wait_t;
float avg_turnaround_t;
//Semaphore 
sem_t sem_SRTF;
//Pthreads
pthread_t thread1, thread2;

/*---------------------------------- Functions -------------------------------*/
//Create process arrive times and burst times, taken from assignment details
void input_processes();
//Schedule processes according to SRTF rule
void process_SRTF();
//Simple calculate average wait time and turnaround time function
void calculate_average();
//Print results, taken from sample
void print_results();
// this function calculates CPU SRTF scheduling, writes waiting time and turn-around time to th FIFO 
void *worker1_thread(void *params);
// reads the waiting time and turn-around time through the FIFO and writes to text file
void *worker2_thread();

*---------------------------------- Implementation -------------------------------*/
/* this main function creates named pipe and threads */
int main(void)
{
	/* creating a named pipe(FIFO) with read/write permission */
	// add your code 

	/* initialize the parameters */
	 // add your code 
	
	/* create threads */
	 // add your code
	
	/* wait for the thread to exit */
	//add your code
	
	return 0;
}


void calculate_average( float avg_wait_t, float avg_turnaround_t) 
{
	avg_wait_t /= PROCESSNUM;
	avg_turnaround_t /= PROCESSNUM;
}

//Schedule processes according to SRTF rule
void process_SRTF() {
	
    int endTime, smallest, time, remain = 0;
	
    //Placeholder remaining time to be replaced
    processes[8].remain_t=9999;
	
    //Run function until remain is equal to number of processes
    for (time = 0; remain != PROCESSNUM; time++) {
		
	//Assign placeholder remaining time as smallest
        smallest = 8;
		
	//Check all processes that have arrived for lowest remain time then set the lowest to be smallest
        for (i=0;i<PROCESSNUM;i++) {
            if (processes[i].arrive_t <= time && processes[i].remain_t < processes[smallest].remain_t && processes[i].remain_t > 0) {
                smallest = i;
            }
        }
		
	//Decrease remaining time as time increases
        processes[smallest].remain_t--;
		
	//If process is finished, save time information, add to average totals and increase remain
        if (processes[smallest].remain_t == 0) {
			
            remain++;
			
            endTime=time+1;
			
	    processes[smallest].turnaround_t = endTime-processes[smallest].arrive_t;
			
	    processes[smallest].wait_t = endTime-processes[smallest].burst_t-processes[smallest].arrive_t;
			
	    avg_wait_t += processes[smallest].wait_t;
			
	    avg_turnaround_t += processes[smallest].turnaround_t;
        }
    }
	
}

/* this function calculates CPU SRTF scheduling, writes waiting time and turn-around time to th FIFO */
void *worker1_thread(void *params)
{
   // add your code here

}

/* reads the waiting time and turn-around time through the FIFO and writes to text file */
void *worker2_thread()
{
   // add your code here
}

