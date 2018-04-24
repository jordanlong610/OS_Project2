/*
 * Project 2 - CPU Scheduling
 * Programmer: Jordan Long
 * Instructor: S. Lee
 * Course: SMPE 320
 * Section 1
 */


#include <semaphore.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>


sem_t timer;
sem_t ready;
int threadsCreated;
int threadsFinished = 0;
struct thread *threadInfo;
int programExecutionTime;

int globalTime = 0;

struct thread{
    int executionTime;
    int period;
    int elapsedTime;
    int isComplete;
};


/*
 * Function for quicksort
 */
int compare (const void * a, const void * b)
{
	 struct thread *threadA = (struct thread *)a;
	 struct thread *threadB = (struct thread *)b;

	 return (threadA->period - threadB->period);
}

/*
 * Runner for each spawned thread. Will wait for its turn, then run for allotted execution time until finished.
 */
void *threadRunner(void *number)
{
  int threadNumber = *((int *) number);
  while(threadInfo[threadNumber].elapsedTime < threadInfo[threadNumber].period)
  {
	  sem_wait(&timer);
	  printf("Thread %d now being executed.\n", threadNumber);
	  int i = 0;
	  while(i < threadInfo[threadNumber].executionTime)
	  {
	  	 sleep(1);
	  	 threadInfo[threadNumber].elapsedTime++;
	  	 i++;
	  }
  }
  printf("CPU is idling now.\n");
  threadInfo[threadNumber].isComplete = 1;	//Mark thread as complete
  threadsFinished++;
  pthread_exit(0);
}

/*
 * Timer will call sem post every time the time reaches the timeQuantum
 * When all threads finish, timer thread will exit.
 */
void *timerFunction()
{
	globalTime = 0;
	int interval = 0;

	while(threadsFinished != threadsCreated || !(globalTime>programExecutionTime))
	{
		for(int i=0; i<threadsCreated;i++)
		{

			if(interval == threadInfo[i].executionTime)
			{
				interval = 0;
				sem_post(&timer);
			}
			else
			{
				printf("%d\n", globalTime);
				sleep(1);
				globalTime++;
				interval++;
			}


		}


    }
	printf("Killed");
	pthread_exit(0);
}


/*
 * Main method for the thread scheduling program.
 * Will check inputs, request burst times for each thread, and then spawn the threads.
 */
int main(int argc, char **argv)
{
	sem_init(&ready, 0, 0);
	sem_init(&timer, 0, 1);

	/*
	 * If integer is not included when the program is run, throws error
	 * ./a.out 5
	 */
	if(argc!=2)
	{
		fprintf(stderr,"Arguments for threads must be included.\n");
		return -1;
	}

	/*
	 * If threads to be created is greater than 10, reject and quit.
	 */
	if(atoi(argv[1]) > 10)
	{
		fprintf(stderr,"Cannot create more than 10 threads!\n");
		return -1;
	}

	threadsCreated = atoi(argv[1]);
	struct thread TI[threadsCreated];
	threadInfo = TI;

	/*
	 * Prints out requests for execution times of each thread, and stores value in  executionTime array.
	 */
	for(int i=0; i<threadsCreated; i++)
	{
		printf("\nExecution time for Thread %d: ", i);
		scanf("%d", &threadInfo[i].executionTime);
		threadInfo[i].isComplete = 0;
		threadInfo[i].elapsedTime = 0;
	}

	/*
	 * Prints out requests for thread period times, and stores value in  period array.
	 */
	for(int i=0; i<threadsCreated; i++)
	{
		printf("\nPeriod for Thread %d: ", i);
		scanf("%d", &threadInfo[i].period);
	}

	/*
	 * Request how long the program should execute.
	 */
	printf("\nHow long do you want to execute this program (sec): ");
	scanf("%d", &programExecutionTime);

	/*
	 * Sort periods least to greatest for EDF algorithm
	 */
	qsort (TI, threadsCreated, sizeof(struct thread), compare);


	/*
	 * Create Timer thread
	 */
	pthread_t timer_thread;
	pthread_attr_t timer_attr;
	pthread_attr_init(&timer_attr);
	pthread_create(&timer_thread, &timer_attr, timerFunction, NULL);

	/*
	 * Loop and create requested threads.
	 */
	pthread_t threads[threadsCreated];
	pthread_attr_t attr[threadsCreated];
	for(int i=0; i<threadsCreated; i++)
	{
		int *arg = malloc(sizeof(*arg)); //Convert int to a pointer  possibly pass struct in
		*arg = i;
		pthread_attr_init(&attr[i]);
		pthread_attr_setscope(&attr[i], PTHREAD_SCOPE_SYSTEM);
		pthread_attr_setschedpolicy(&attr[i], SCHED_RR);
		pthread_create(&threads[i], &attr[i], threadRunner, arg);
	}
	pthread_join(timer_thread, NULL);
}

