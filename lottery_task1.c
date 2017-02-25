#include <sys/time.h>
#include <stdlib.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>
// !!!!!! This value is CPU-dependent !!!!!!

//#define LOOP_ITERATIONS_PER_MILLISEC 155000

#define LOOP_ITERATIONS_PER_MILLISEC 178250
//#define LOOP_ITERATIONS_PER_MILLISEC 193750


#define MILLISEC 	1000
#define MICROSEC	1000000
#define NANOSEC 	1000000000



/*   This is how LOTTERY tasks are specified 

In the file linux-2.6.24-lottery/include/linux/sched.h

also in the file /usr/include/bits/sched.h (the user space scheduler file)

*/
struct sched_param_t {
	int sched_priority;
	int lottery_id;
	unsigned long long tickets;
};

double min_offset,max_offset; //seconds
double min_exec_time,max_exec_time; //seconds
double min_inter_arrival_time,max_inter_arrival_time; //seconds

unsigned int lottery_id,jid=1;
struct itimerval inter_arrival_time;


void burn_1millisecs() {
	unsigned long long i;
	for(i=0; i<LOOP_ITERATIONS_PER_MILLISEC; i++);
}

void burn_cpu(long milliseconds){
	long i;
	for(i=0; i<milliseconds; i++)
		burn_1millisecs();
}

void clear_sched_param_t(struct sched_param_t *param)
{
	param->lottery_id=-1;
	param->tickets=0;
}
void print_task_param(struct sched_param_t *param)
{
    	printf("\npid[%d]\n",param->lottery_id);
	printf("tickets[%llu]\n",param->tickets);
}


void clear_signal_timer(struct itimerval *t)
{
	t->it_interval.tv_sec = 0;
	t->it_interval.tv_usec = 0;
	t->it_value.tv_sec = 0;
	t->it_value.tv_usec = 0;
}
void set_signal_timer(struct itimerval *t,double secs)
{
	t->it_interval.tv_sec = 0;
	t->it_interval.tv_usec =0 ;
	t->it_value.tv_sec = (int)secs;
	t->it_value.tv_usec = (secs-t->it_value.tv_sec)*MICROSEC;
}
void print_signal_timer(struct itimerval *t)
{
	printf("Interval: secs [%ld] usecs [%ld] Value: secs [%ld] usecs [%ld]\n",
		t->it_interval.tv_sec,
		t->it_interval.tv_usec,
		t->it_value.tv_sec,
		t->it_value.tv_usec);
}
double get_time_value(double min, double max)
{
	if(min==max)
		return min;
	return (min + (((double)rand()/RAND_MAX)*(max-min)));
}

void start_task(int s)
{
	printf("\nTask(%d) has just started\n",lottery_id);
	set_signal_timer(&inter_arrival_time,get_time_value(min_offset,max_offset));
	setitimer(ITIMER_REAL, &inter_arrival_time, NULL);
}

void do_work(int s)
{
	volatile int i = 1;
	while(i == 1){
		;
	}
}

void end_task(int s)
{
	printf("\nTask(%d) has finished\n",lottery_id);
	exit(0);
}

int main(int argc, char** argv) {

	struct sched_param_t param;

	unsigned long long seed;
	int i;


	param.sched_priority = 1;

	lottery_id=param.lottery_id=atoi(argv[1]);
	param.tickets=atoll(argv[6]);

	signal(SIGUSR1, start_task);
	signal(SIGALRM, do_work);
	signal(SIGUSR2, end_task);

	if ( sched_setscheduler( 0, 6 , (struct sched_param *)&param ) ==-1){
		perror("ERROR");
	}

	printf("After sched_setscheduler:priority %d\n",sched_getscheduler(0));

	//Wait for signals while the task is idle
	
	volatile int k = 1;
	while(k == 1){
		;
	}

	return 0;
}

