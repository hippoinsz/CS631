
CC = gcc
CFLAGS = -Wall -Werror -Wextra

ls:ls.o print.o
	$(CC)  ls.o print.o -o ls 

ls.o:ls.c
	$(CC) $(CFLAGS) -c ls.c -o ls.o

print.o:print.c
	$(CC) $(CFLAGS) -c print.c -o print.o

clean:
	rm -rf *.o ls
