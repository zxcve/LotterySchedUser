#include <stdlib.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

/**
 * @brief Re-defining sched_param from sched.h for removing compilation
 * dependency on the sched.h
 */
struct sched_param_t {
	int sched_priority;
	unsigned long long tickets;
};

/**
 * @brief Coarse Counter
 */
unsigned long long high_counter = 0;

/**
 * @brief Fine Counter
 */
unsigned long long low_counter = 0;

/**
 * @brief Holds the params for this task
 */
struct sched_param_t param;

/**
 * @brief Performs counter increment in infinite loop
 *
 * @param s NOT_USED
 */
void start_task(int s)
{
	printf("<START> PID->(%d) TICKETS->%llu counter->%llu\n",
	       getpid(), param.tickets, high_counter);

	if (sched_setscheduler( 0, 6 , (struct sched_param *)&param ) ==-1){
		perror("Changing scheduler policy to LOTTERY_SCHEDULER failed");
	}

	while(1){
		low_counter++;
		/* For every 10k increment of fine grain counter we increase
		 * coarse grain counter by 1.
		 */
		if (low_counter == 10000) {
			low_counter = 0;
			high_counter++;
		}
	}
}

/**
 * @brief Kills the current task
 *
 * @param s NOT_USED
 */
void end_task(int s)
{
	printf("<END> PID->(%d) TICKETS->%llu counter->%llu\n",
	       getpid(), param.tickets, high_counter);
	exit(0);
}

/**
 * @brief Main function for the current task
 *
 * @param argc Number of arguments passed from outside including the task name
 * @param argv Array of string for the arguments
 *
 * @return Always 0
 */
int main(int argc, char** argv) {

	/* We dont care about real-time priority and setting it to 1. */
	param.sched_priority = 1;

	/* We assign the tickets passed from the caller. */
	param.tickets=atoll(argv[1]);

	/* SIGUSR1 will start the execution for the workload */
	signal(SIGUSR1, start_task);

	/* SIGUSR2 will terminate the execution for the workload */
	signal(SIGUSR2, end_task);

	/* Wait infinitely for the signals */
	while(1){
		pause();
	}

	/* Never reach here as end_task calls exit(0) */
	return 0;
}
