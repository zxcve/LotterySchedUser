#include <sys/time.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sched.h>

/* Length of argv string passed to task */
#define  BUF_LEN		200

/* Number of max tasks supported */
#define  LOTTERY_TASKS_NUM	4000

/* Array of structure of all the tasks */
static pid_t lottery_tasks [LOTTERY_TASKS_NUM];

/* Array for tickets for all the tasks */
static unsigned long long tickets[LOTTERY_TASKS_NUM];

/* Number of tasks passed from user */
static int lottery_tasks_num=0;

/**
 * @brief Starts the simulation by sending signal SIGUSR1
 */
static void start_simulation(void)
{
	int i;

	printf("Running the tasks\n\n");

	for(i=0;i<lottery_tasks_num;i++){
		kill(lottery_tasks[i],SIGUSR1);
	}

}

/**
 * @brief Ends the simulation by sending signal SIGUSR2
 *
 * @param signal  NOT_USED
 */
static void end_simulation(int signal)
{
	int i;

	printf("##############################################################\n");
	printf("Killing the tasks\n\n");

	for(i=0;i<lottery_tasks_num;i++){
		kill(lottery_tasks[i],SIGUSR2);
	}

}

/**
 * @brief Called when wrong number of arguments are passed
 *
 * @param name Name of the process running
 */
static void help(char* name)
{
	fprintf(stderr, "Usage: %s <config> <duration> \n",	name);
	exit(0);
}

/**
 * @brief Main function for test tool
 *
 * @param argc Number of arguments passed
 * @param argv[] Array of arguments passed
 *
 * @return 0 for success, -1 for failure
 */
int main(int argc, char *argv[])
{

	int duration,i=0,j=0,k=0,n=0;
	struct itimerval sim_time;
	char arg[2][BUF_LEN],*parg[3];
	char buffer[BUF_LEN];
	struct sched_param param;
	FILE *fd;

	/* Check if 2 arguments passed */
	if(argc!=3){
		help(argv[0]);
	}

	/* Parse the duration in seconds */
	duration = atoi(argv[2]);

	fd  = fopen(argv[1], "r");

	if (fd == NULL) {
		printf("file open failed\n");
		return -1;
	}

	/* Iterate across the config file and parse process & tickets */
	while( (fgets(buffer, BUF_LEN, fd))!=NULL) {
		if(strlen(buffer)>1){
			j = 0;
			k = atoi(buffer);
			while (buffer[j++] != ' '){};

			while (k--)
				tickets[i++] = atoll(buffer + j);
		}
	}

	fclose(fd);

	/* Bump the priority of test utility */
	param.sched_priority = 99;

	/* Bump the scheduler to real time */
	if (sched_setscheduler( 0, 1, (struct sched_param *)&param ) ==-1){
		perror("system could not become fifo");
	}

	lottery_tasks_num = i;

	fd  = fopen("/proc/lottery/stats", "wr");

	if (fd == NULL) {
		printf("proc/lottery/stats file open failed\n");
		return -1;
	}

	/* Flush the stats from previous run */
	putc(0, fd);

	fclose(fd);

	sim_time.it_interval.tv_sec = 0;
	sim_time.it_interval.tv_usec = 0;
	sim_time.it_value.tv_sec = duration;
	sim_time.it_value.tv_usec = 0;

	printf("###############################################################\n");
	printf("Executing lottery test benchmark with %d tasks for %d seconds \n",
	       lottery_tasks_num, duration);
	printf("###############################################################\n");

	for(i=0;i<lottery_tasks_num;i++){
		strcpy(arg[0],"lottery_task");
		sprintf(arg[1],"%llu",tickets[i]);
		n=2;
		for(k=0;k<n;k++){
			parg[k]=arg[k];
		}
		parg[k]=NULL;
		lottery_tasks[i]=fork();
		if(lottery_tasks[i]==0){
			execv("./lottery_task",parg);
			perror("Error: execv\n");
			exit(0);
		}
		/* Required to give enough time to each process to start
		 * initially */
		usleep(100);
	}

	/* Register for alarm after duration seconds to kill test */
	signal(SIGALRM, end_simulation);
	setitimer(ITIMER_REAL, &sim_time, NULL);

	/* Starts the simulation*/
	start_simulation();

	/* Wait for the SIGALRM */
	pause();

	/* Wait for all processes to complete */
	for(i=0;i<lottery_tasks_num;i++){
		wait(NULL);
	}

	printf("##############################################################\n");

	printf("Statistics\n\n");
	fd  = fopen("/proc/lottery/stats", "r");

	if (fd == NULL) {
		printf("proc file open failed\n");
		return -1;
	}

	/* Dump the statistics for the test */
	while( (fgets(buffer, BUF_LEN, fd))!=NULL) {
		printf("%s", buffer);
	}

	fclose(fd);

	printf("\nAll tasks have finished properly!!!\n");
	printf("##############################################################\n");

	return 0;
}
