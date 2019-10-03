sshell : sshell.o
	gcc -Wall -Werror -o sshell sshell.o

sshell.o : sshell.c job.h
	gcc -Wall -Werror -c sshell.c

clean :
	rm -f sshell.o sshell
