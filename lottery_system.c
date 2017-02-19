
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

#define  BUF_LEN			200
#define  LOTTERY_TASKS_NUM		20	


struct lottery_tasks_config {
	int pid;
	unsigned long long tickets;
};

pid_t lottery_tasks_pid [LOTTERY_TASKS_NUM];
int lottery_tasks_num=0;

int get_int_val (char* str) 
{
	char* s = str;
	int val;
	for (s = str;*s!='\t';s++);
	*s='\0';
	val=atoi(str);
	return val;
}
void print_lottery_tasks_config(struct lottery_tasks_config *tasks, int num)
{
	int i;
	printf("\nLOTTERY TASKS CONFIG\n");
	printf("pid\ttickets\n");
	for(i=0;i<num;i++){
		printf("%d\t%llu\n",
		tasks[i].pid,
		tasks[i].tickets
		);
	}
}

void clear_lottery_tasks_config_info(struct lottery_tasks_config *tasks, int num)
{
	int i;
	
	for(i=0;i<num;i++){
		tasks[i].pid=0;
		tasks[i].tickets=0;
	}
}
void get_lottery_task_config_info(char * str, struct lottery_tasks_config *tasks,int *n)
{
	char *s ,*s1;
	int i=0;
	s = s1=str;
	while(i<1){
		if(*s=='\t'){
			*s='\0';
			switch(i){
				case 0:
					tasks[*n].pid = atoi(s1);
					s1=s+1;
					tasks[*n].tickets = atoll(s1);
					i++;

			}

		}
		s++;
	}
	(*n)++;
}

void get_lottery_tasks_config_info(char *file, int *duration, struct lottery_tasks_config *tasks,int *n)
{
	char buffer[BUF_LEN];
	int count=0;
	FILE* fd  = fopen(file, "r");
	*n=0;
	buffer[0]='\0';
	while( (fgets(buffer, BUF_LEN, fd))!=NULL) {
		if(strlen(buffer)>1){
			printf("%s\n", buffer);
			switch(count){
				case 0:
					*duration=get_int_val(buffer);
					count++;
				break;
				default:
					get_lottery_task_config_info(buffer, tasks,n);
			}
		}
		buffer[0]='\0';
	}
	fclose(fd);

}


void start_simulation()
{
	int i;
	printf("I will send a SIGUSR1 signal to start all tasks\n");
	for(i=0;i<lottery_tasks_num;i++){
		kill(lottery_tasks_pid[i],SIGUSR1);
	}

}
void end_simulation(int signal)
{
	int i;
	printf("I will send a SIGUSR2 signal to finish all tasks\n");
	for(i=0;i<lottery_tasks_num;i++){
		kill(lottery_tasks_pid[i],SIGUSR2);
	}

}
void help(char* name)
{
	fprintf(stderr, "Usage: %s file_name (system configuration)\n",	name);
	exit(0);
}

int main(int argc, char *argv[])
{

	int duration,i,j,k,n;
	struct lottery_tasks_config lottery_tasks_config[LOTTERY_TASKS_NUM];
	struct itimerval sim_time;
	char arg[LOTTERY_TASKS_NUM][BUF_LEN],*parg[LOTTERY_TASKS_NUM];

	srand(time(NULL));

	if(argc!=2){
		help(argv[0]);
	}

	clear_lottery_tasks_config_info(lottery_tasks_config, LOTTERY_TASKS_NUM);


	get_lottery_tasks_config_info(argv[1],&duration,lottery_tasks_config,&lottery_tasks_num);

	sim_time.it_interval.tv_sec = 0;
	sim_time.it_interval.tv_usec = 0;
	sim_time.it_value.tv_sec = duration;
	sim_time.it_value.tv_usec = 0;

	signal(SIGALRM, end_simulation);
	setitimer(ITIMER_REAL, &sim_time, NULL);

	for(i=0;i<lottery_tasks_num;i++){
		strcpy(arg[0],"lottery_task");

		sprintf(arg[1],"%d",lottery_tasks_config[i].pid);
		sprintf(arg[2],"%d",lottery_tasks_config[i].tickets);
		sprintf(arg[3],"%ld",rand());
		n=4;
		for(k=0;k<n;k++){
			parg[k]=arg[k];
		}
		parg[k]=NULL;
		lottery_tasks_pid[i]=fork();
		if(lottery_tasks_pid[i]==0){
			execv("./lottery_task",parg);
			perror("Error: execv\n");
			exit(0);
		}
		sleep(1);
	}

	start_simulation();  //time zero of the execution
	//waiting for the end of the simulation
	pause();

	for(i=0;i<lottery_tasks_num;i++){
		wait(NULL);
	}

	printf("All tasks have finished properly!!!\n");
	return 0;

}
