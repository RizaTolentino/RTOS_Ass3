/**********************************************************************************

           *************NOTE**************
This is a template for the subject of RTOS in University of Technology Sydney(UTS)
Please complete the code based on the assignment requirement.

Assignment 3 Program_2 template

**********************************************************************************/
/*
  To compile prog_1 ensure that gcc is installed and run the following command:
  gcc Assignment3_12876254_Prg_1.c -o Prg1.out -lpthread -lrt -Wall -O2


*/

#include <pthread.h> /* pthread functions and data structures for pipe */
#include <unistd.h>	 /* for POSIX API */
#include <stdlib.h>	 /* for exit() function */
#include <stdio.h>	 /* standard I/O routines */
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
#define MAX_STR_LENGTH 256

typedef struct SRTF_Params
{
	//add your variables here
	int pid;
	int arrive_t, wait_t, burst_t, turnaround_t, remain_t;

} Process_Params;

/*---------------------------------- Variables -------------------------------*/
//Array of proceses with 1 extra for placeholder remain_t
Process_Params processes[PROCESSNUM + 1];
//Index variable
int i;
//Calculated averages
float avg_wait_t;
float avg_turnaround_t;
//Semaphore
sem_t sem_SRTF;
//Pthreads
pthread_t thread1, thread2;
//file Name
char outputFilename[MAX_STR_LENGTH];
//fifo file descriptor
char *myfifo = "/tmp/myfifo1";
/*---------------------------------- Functions -------------------------------*/
//Initialise values for arrive time and burst times for processes
void initialisePrcoesses();
//Print welcome message and request continuing input from user
void welcomeMessage();
//Schedule processes according to SRTF rule
void orderSRTF();
//Simple calculate average wait time and turnaround time function
void calculateAverage();
//Send and write average wait time and turnaround time to fifo
void sendFIFO();
// Read average wait time and turnaround time from fifo then write to output.txt
void readFIFO();
//Print results, taken from sample
void print_results();
//this function calculates CPU SRTF scheduling, writes waiting time and turn-around time to th FIFO
void *worker1_thread();
//reads the waiting time and turn-around time through the FIFO and writes to text file
void *worker2_thread();

/*---------------------------------- Implementation -------------------------------*/

/**
 Main routine for the program. this main function creates threads, semaphores and the fifo .

 @param argc Number of arguments passed to the program.
 @param argv array of values passed to the program, expects an output file name .
 @return returns 0 upon completion, negative value if an error is incurred.
 */
int main(int argc, char *argv[])
{
	//Verify the correct number of arguments were passed in
	if (argc != 2)
	{
		fprintf(stderr, "\033[1;31mUSAGE: Must input ONE filename as a commandline argument\033[0m\n Exiting program...\n Try \'./Assignment3_P2 output.txt\' \n");
		return (-11);
	}

	//asign output file name
	strcpy(outputFilename, argv[1]);

	//display welcome messsage
	welcomeMessage();

	// initialize the parameters, check file for fifo is available for use
	initialisePrcoesses();

	//Initialise semaphore
	if (sem_init(&sem_SRTF, 0, 0) != 0)
	{
		printf("semaphore initialize error\n");
		return (-10);
	}

	//make fifo
	if (mkfifo(myfifo, 0777) < 0)
	{
		perror("Error making file");
		//remove the fifo such that the program can run next time if there has been an error
		remove(myfifo);
		return (-9);
	}

	//create thread 1
	if (pthread_create(&thread1, NULL, &worker1_thread, NULL) != 0)
	{
		printf("Thread 1 created error\n");
		return -1;
	}

	//create thread 2
	if (pthread_create(&thread2, NULL, &worker2_thread, NULL) != 0)
	{
		printf("Thread 2 created error\n");
		return -2;
	}

	//wait for the thread to exit
	if (pthread_join(thread1, NULL) != 0)
	{
		printf("join thread 1 error\n");
		return -3;
	}

	//wait for the thread to exit
	if (pthread_join(thread2, NULL) != 0)
	{
		printf("join thread 2 error\n");
		return -4;
	}

	//wait for the thread to exit
	if (sem_destroy(&sem_SRTF) != 0)
	{
		printf("Semaphore destroy error\n");
		return -5;
	}

	return 0;
}

/**
 this thread calculates CPU SRTF scheduling, writes waiting time and turn-around time to th FIFO

 @return void.
 */
void *worker1_thread()
{
	// add your code here
	orderSRTF();
	calculateAverage();
	sendFIFO();
	print_results();
}

/**
 this thread reads the waiting time and turn-around time through the FIFO and writes to text file 

 @return void.
 */
void *worker2_thread()
{
	sem_wait(&sem_SRTF);
	readFIFO();
}

/**
 Prints welcome message, notifies users of purpose of program, requests [y/n] to continue .

 @return void.
 */
void welcomeMessage()
{
	char c = '\0';

	// Print message to mains identifying purpose of program - in red bold text
	printf("This program will implement a SRTF algorithm and will calculate average wait time \nand turnaround time for processes then print this to an output file which you specify.\n");
	printf("\033[1;31mNote:\033[0m If you selected an output file that already exists in your directory it will overridden.\n\n");
	printf("\033[1;33mWriting to:\033[0m %s\n\n", outputFilename);
	printf("Would you like to continue? \33[1;31m[y/n]\033[0m\n");

	//if y and n is not entered, wait for a correct response.
	while (c != 'y' && c != 'n' && c != 'Y' && c != 'N')
		c = getchar();

	printf("\n\n");

	//Exit program if n has been entered
	if (c == 'n')
	{
		printf("Exiting program...\n");
		exit(EXIT_FAILURE);
	}
}

/**
 Initialise values for arrive time and burst times for processes, Ensure file "/tmp/myfifo1" does not exist

 @return void.
 */
void initialisePrcoesses()
{
	// The input data of the cpu scheduling algorithm is:
	// --------------------------------------------------------
	// Process ID           Arrive time          Burst time
	//     1					8		    		10
	//     2                   10                  3
	//     3                   14                  7
	//     4                   9                   5
	//     5                   16                  4
	//     6                   21                  6
	//     7                   26                  2
	// --------------------------------------------------------

	int k;
	int arrive_time_array[PROCESSNUM] = {8, 10, 14, 9, 16, 21, 26};
	int burst_time_array[PROCESSNUM] = {10, 3, 7, 5, 4, 6, 2};

	for (k = 0; k < PROCESSNUM; k++)
	{
		processes[k].pid = k + 1;
		processes[k].arrive_t = arrive_time_array[k];
		processes[k].burst_t = burst_time_array[k];
		processes[k].remain_t = burst_time_array[k];
	}

	int notExist; //indicator if file exists
	struct stat buffer;
	char c = '\n';

	//Ensure this file does not exist
	notExist = stat("/tmp/myfifo1", &buffer);
	//If it does exist, ask the user if the file can be deleted
	if (!notExist)
	{
		//check if the user is okay with deleting the file
		printf("A file /tmp/myfifo1 exists but must be deleted to run this program.");
		printf("The file will be deleted. \nWould you like to continue? [y/n]\n");
		//if y and n is not entered, wait for a correct response.
		while (c != 'y' && c != 'n' && c != 'Y' && c != 'N')
			c = getchar();

		printf("\n\n");

		//Exit program if n has been entered
		if (c == 'n')
		{
			printf("Exiting program...\n");
			exit(EXIT_FAILURE);
		}
		//otherwise, remove the file
		else
			remove("/tmp/myfifo1");
	}
}

/**
 Simple calculate average wait time and turnaround time function

 @return void.
 */
void calculateAverage()
{
	avg_wait_t /= PROCESSNUM;
	avg_turnaround_t /= PROCESSNUM;
}

/**
 Schedule processes according to SRTF rule

 @return void.
 */
void orderSRTF()
{

	int endTime, smallest, time, remain = 0;

	//Placeholder remaining time to be replaced, choose large number such that it is overridden
	processes[PROCESSNUM].remain_t = 9999;

	//Run function until remain is equal to number of processes
	for (time = 0; remain != PROCESSNUM; time++)
	{

		//Assign placeholder remaining time as smallest
		smallest = PROCESSNUM;

		//Check all processes that have arrived for lowest remaining time then set the lowest to be smallest
		for (i = 0; i < PROCESSNUM; i++)
		{
			//if the arrive time of process i and its remain time is less than the last recorded smallest remaining time and has not already been executed, it now has the smallest remaining time
			if (processes[i].arrive_t <= time && processes[i].remain_t < processes[smallest].remain_t && processes[i].remain_t > 0)
				smallest = i;
		}

		//Decrease remaining time as time increases
		processes[smallest].remain_t--;

		//If process is finished, save time information, add to average totals and increase remain
		if (processes[smallest].remain_t == 0)
		{
			//remain increments, indicating another process has been completed
			remain++;
			//end time is the counted time + 1
			endTime = time + 1;
			//turnaround time is calulated as the difference between the arrive time and the end time
			processes[smallest].turnaround_t = endTime - processes[smallest].arrive_t;
			//wait time is calculated by subtracting the burst time and arrive time from the end time
			processes[smallest].wait_t = endTime - processes[smallest].burst_t - processes[smallest].arrive_t;
			//accumulated average times
			avg_wait_t += processes[smallest].wait_t;
			//accumulated turnaround time
			avg_turnaround_t += processes[smallest].turnaround_t;
		}
	}
}

/**
 Print time results including arrival time, burst time, turnaround time, wait time, average times.

 @return void.
 */
void print_results()
{

	printf("\033[1;36mProcess Schedule Table: \033[0m\n");

	printf("\033[0;36m\tProcess ID\tArrival Time\tBurst Time\tWait Time\tTurnaround Time\n\033[0m");

	for (i = 0; i < PROCESSNUM; i++)
	{
		printf("\t%d\t\t%d\t\t%d\t\t%d\t\t%d\n", processes[i].pid, processes[i].arrive_t, processes[i].burst_t, processes[i].wait_t, processes[i].turnaround_t);
	}

	printf("\nAverage wait time: %fs\n", avg_wait_t);

	printf("\nAverage turnaround time: %fs\n", avg_turnaround_t);
}

/**
 Send and write average wait time and turnaround time to fifo

 @return void.
 */
void sendFIFO()
{
	int resWrite; //write result

	sem_post(&sem_SRTF);

	//open fifo
	int fifofd;
	fifofd = open(myfifo, O_WRONLY);

	//if there is an error, exit
	if (fifofd < 0)
	{
		printf("fifo open send error\n");
		//remove the fifo such that the program can run next time if there has been an error
		remove(myfifo);
		exit(EXIT_FAILURE);
	}

	//write average wait time to the fifo
	resWrite = write(fifofd, &avg_wait_t, sizeof(avg_wait_t));

	//if there is an error, exit
	if (resWrite < 0)
	{
		perror("Error writing to fifo");
		//remove the fifo such that the program can run next time if there has been an error
		remove(myfifo);
		exit(EXIT_FAILURE);
	}
	//write average wait time to the fifo
	resWrite = write(fifofd, &avg_turnaround_t, sizeof(avg_turnaround_t));

	//if there is an error, exit
	if (resWrite < 0)
	{
		perror("Error writing to fifo");
		//remove the fifo such that the program can run next time if there has been an error
		remove(myfifo);
		exit(EXIT_FAILURE);
	}

	//close fifo
	close(fifofd);

	//unlink fifo
	unlink(myfifo);
}

/**
 Read average wait time and turnaround time from fifo then write to output.txt

 @return void.
 */
void readFIFO()
{
	//result for read
	int resRead;
	//file descriptor
	int fifofd;

	//variables for average turnaround time and average wait time
	float fifo_avg_turnaround_t,
		fifo_avg_wait_t;

	//Ensure this file path does not exist

	FILE *file_to_write;
	file_to_write = fopen(outputFilename, "w");

	if (file_to_write == NULL)
	{
		perror("Error opening file");
		//remove the fifo such that the program can run next time if there has been an error
		remove(myfifo);
		exit(1);
	}

	fifofd = open(myfifo, O_RDONLY);

	if (fifofd < 0)
	{
		printf("fifo open read error\n");
		remove(myfifo);
		exit(EXIT_FAILURE);
	}

	//Read average wait time to the fifo
	resRead = read(fifofd, &fifo_avg_wait_t, sizeof(int));

	//if there is an error, exit
	if (resRead < 0)
	{
		perror("Error reading fifo");
		//remove the fifo such that the program can run next time if there has been an error
		remove(myfifo);
		exit(EXIT_FAILURE);
	}

	//Read average wait time to the fifo
	resRead = read(fifofd, &fifo_avg_turnaround_t, sizeof(int));

	//if there is an error, exit
	if (resRead < 0)
	{
		perror("Error reading fifo");
		//remove the fifo such that the program can run next time if there has been an error
		remove(myfifo);
		exit(EXIT_FAILURE);
	}

	printf("\nRead from FIFO: %fs Average wait time\n", fifo_avg_wait_t);
	printf("\nRead from FIFO: %fs Average turnaround time\n", fifo_avg_turnaround_t);

	fprintf(file_to_write, "%s %.4f\n", "Average wait time was: ", fifo_avg_wait_t);
	fprintf(file_to_write, "%s %.4f", "Average turnaround time was: ", fifo_avg_turnaround_t);

	close(fifofd);

	remove(myfifo);
}
