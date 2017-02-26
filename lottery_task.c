#include <stdlib.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

struct sched_param_t {
	int sched_priority;
	unsigned long long tickets;
};

unsigned long long high_counter = 0;
unsigned long long low_counter = 0;

struct sched_param_t param;

void start_task(int s)
{
	printf("<START> PID->(%d) TICKETS->%llu counter->%llu\n", getpid(), param.tickets, high_counter);
	if (sched_setscheduler( 0, 6 , (struct sched_param *)&param ) ==-1){
		perror("ERROR");
	}

	while(1){
		low_counter++;
		if (low_counter == 10000) {
			low_counter = 0;
			high_counter++;
		}
	};
}

void end_task(int s)
{
	printf("<END> PID->(%d) TICKETS->%llu counter->%llu\n", getpid(), param.tickets, high_counter);
	exit(0);
}

int main(int argc, char** argv) {

	param.sched_priority = 1;

	param.tickets=atoll(argv[1]);

	//printf("task %d tickets %d \n", getpid(), param.tickets);
	signal(SIGUSR1, start_task);
	signal(SIGUSR2, end_task);

	while(1){
		pause();
	}

	return 0;
}
