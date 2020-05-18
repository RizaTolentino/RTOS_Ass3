/**********************************************************************************

           *************NOTE**************
This is a template for the subject of RTOS in University of Technology Sydney(UTS)
Please complete the code based on the assignment requirement.

Assignment 3 Program_2 template

**********************************************************************************/
/*
  To compile prog_1 ensure that gcc is installed and run the following command:
  gcc -Wall -O2 program_2.c -o prog_2 -lpthread -lrt

*/
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

	// reference number
#define REFERENCESTRINGLENGTH 24

//Number of pagefaults in the program
int pageFaults = 0;
char BoolPrint[2] = {'N', 'Y'};
//Function declaration
void SignalHandler(int signal);
void welcomeMessage(int framesize);
/**
 Main routine for the program. In charge of setting up threads and the FIFO.

 @param argc Number of arguments passed to the program.
 @param argv array of values passed to the program.
 @return returns 0 upon completion.
 */
int main(int argc, char* argv[])
{
	/* Verify the correct number of arguments were passed in */
	if (argc != 2) {
		fprintf(stderr, "\033[1;31mUSAGE: Must input valid frame size as argument\033[0m\n Exiting program...\n Try \'./Assignment3 4\' \n");
	}
	//Register Ctrl+c(SIGINT) signal and call the signal handler for the function.
	signal(SIGINT, SignalHandler);
	//counters for loops
    int i,j;
	//Argument from the user on the frame size, such as 4 frames in the document
	int frameSize = atoi(argv[1]);
	//Frame where we will be storing the references. -1 is equivalent to an empty value
	uint frame[frameSize];
	//Reference string from the assignment outline
	int referenceString[REFERENCESTRINGLENGTH] = {7,0,1,2,0,3,0,4,2,3,0,3,0,3,2,1,2,0,1,7,0,1,7,5};
	//Next position to write a new value to.
	int nextWritePosition = 0;
	//Boolean value for whether there is a match or not.
	bool match = false;

	//print welcome message
	welcomeMessage(frameSize);

	//Initialise the empty frame with -1 to simulate empty values.
	for(i = 0; i < frameSize; i++)
	{
		frame[i] = -1;
	}

	//Loop through the reference string values.
	for(i = 0; i < REFERENCESTRINGLENGTH; i++)
	{
		//loop through the frame to check for a match
		for(j = 0; j <frameSize; j++)
		{
			if (frame[j] == referenceString[i])
				match = true;		
		}

		//if there is no match, store the value in the frame
		if (match != true)
		{
			frame[nextWritePosition] = referenceString[i];
			//there was a page fault, increment value
			pageFaults ++;
			//increment to the next position
			nextWritePosition ++;
			//wrap around if FIFO is full
			if (nextWritePosition == frameSize)
				nextWritePosition = 0;
		}

		//print frame information
		printf("\033[1;34mIteration: \033[0m%02d, \033[0;34mPage Fault?: \033[0m%c\t", i, BoolPrint[!match]);
		for(j = 0; j <frameSize; j++)
		{
			if (match)
				printf("\033[0;36mFrame[%d]:\033[0m %d\t", j, frame[j]);
			else 
				printf("Frame[%d]: %d\t", j, frame[j]);
		}

		printf("\n");	

		//reset match to false		
		match = false;
	}

	printf("Algorithm is complete, waiting for ctrl+c signal....\n");
	//Sit here until the ctrl+c signal is given by the user.
	while(1)
	{
		sleep(1);
	}

	return 0;
}

/**
 Performs the final print when the signal is received by the program.

 @param signal An integer values for the signal passed to the function.
 */
void SignalHandler(int signal)
{
	printf("\nTotal page faults: %d\n", pageFaults);
	exit(0);
}

/**
 Prints the Welcome message for the user.

 @param frameSize An integer value that determines size of frame
 */
void welcomeMessage(int frameSize)
{
  char c = '\0';
  // Print message to mains identifying purpose of program 
  printf("\033[1;36mWelcome to Program 2: Memory management using FIFO\033[0m\n");
  printf("This program will simulate page replacement for virtual memory using a \033[1;37mFirst-In-First-Out\033[0m algorithm.\n");
  printf("\033[1;31mNote:\033[0m \n");
  printf("\t [1] You have chosen to have \033[1;34m%d\033[0m frames. \n\t [2] If a page fault has NOT occurred during any of the iterations, text will be in \033[0;36mCyan \033[0m.\n\t [3] When the algorithm ends, press Ctrl+C to exit\n\n", frameSize);
  printf("Would you like to continue? \33[1;31m[y/n]\033[0m\n");
 
  //if y and n is not entered, wait for a correct response.
  while ( c != 'y' && c != 'n')
	c = getchar();
	
  printf ("\n\n");

  //Exit program if n has been entered
  if(c == 'n')
  {
	printf("Exiting program...\n");
	exit(1);
  }	

}

