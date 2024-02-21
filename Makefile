Job.o: Job.h Job.c
	gcc -c Job.c
node.o: Job.h node.h node.c
	gcc -c node.c
JobLL.o: Job.h node.h JobLL.h JobLL.c
	gcc -c JobLL.c
mysh: Job.o node.o JobLL.o mysh.c
	gcc -o mysh Job.o node.o JobLL.o mysh.c -lreadline
clean:
	rm *.o
