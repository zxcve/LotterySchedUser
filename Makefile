CC=/usr/bin/gcc

all: lottery_system  lottery_task

lottery_system: lottery_system.c 
	${CC} -ggdb -lrt -lm lottery_system.c -o lottery_system


lottery_task: lottery_task.c 
	${CC} -ggdb -lrt -lm lottery_task.c -o lottery_task

clean:
	rm -f *.o *.a *~ lottery_task  lottery_task
