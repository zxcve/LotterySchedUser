#include <sys/time.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sched.h>

#define  BUF_LEN				200
#define  LOTTERY_TASKS_NUM		2000

pid_t lottery_tasks [LOTTERY_TASKS_NUM];
unsigned long long tickets[LOTTERY_TASKS_NUM];
int lottery_tasks_num=0;

void start_simulation()
{
	int i;
	printf("Running the tasks\n\n");
	for(i=0;i<lottery_tasks_num;i++){
		kill(lottery_tasks[i],SIGUSR1);
	}

}

void end_simulation(int signal)
{
	int i;
	printf("##############################################################\n");
	printf("Killing the tasks\n\n");
	for(i=0;i<lottery_tasks_num;i++){
		kill(lottery_tasks[i],SIGUSR2);
	}

}

void help(char* name)
{
	fprintf(stderr, "Usage: %s <config> <duration> \n",	name);
	exit(0);
}

int main(int argc, char *argv[])
{

	int duration,i=0,j,k,n;
	struct itimerval sim_time;
	char arg[2][BUF_LEN],*parg[3];
	char buffer[BUF_LEN];
	struct sched_param param;
	FILE *fd;

	if(argc!=3){
		help(argv[0]);
	}

	duration = atoi(argv[2]);

	fd  = fopen(argv[1], "r");

	if (fd == NULL) {
		printf("file open failed\n");
		return -1;
	}

	while( (fgets(buffer, BUF_LEN, fd))!=NULL) {
		if(strlen(buffer)>1){
			j = 0;
			k = atoi(buffer);
			while (buffer[j++] != ' '){};

			while (k--)
				tickets[i++] = atoll(buffer + j);
		}
	}

	param.sched_priority = 99;

	if (sched_setscheduler( 0, 1, (struct sched_param *)&param ) ==-1){
		perror("system could not become fifo");
	}

	lottery_tasks_num = i;

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
		sleep(1);
	}

	signal(SIGALRM, end_simulation);
	setitimer(ITIMER_REAL, &sim_time, NULL);

	start_simulation();
	pause();

	for(i=0;i<lottery_tasks_num;i++){
		wait(NULL);
	}

	printf("All tasks have finished properly!!!\n");
	printf("##############################################################\n");
	return 0;
}
