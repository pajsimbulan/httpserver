CC	=	clang
CFLAGS	=	-Wall	-Wextra	-Wpedantic

all  :  httpserver

httpserver	:	httpserver.o 	bind.o	queue.o
	$(CC)	-o	httpserver	httpserver.o	bind.o	queue.o -pthread

httpserver.o	:	httpserver.c  bind.h	queue.h
	$(CC)	$(CFLAGS)	-c	httpserver.c

bind.o	:	bind.c  bind.h
	$(CC)	$(CFLAGS)  -c	bind.c

queue.o	:  queue.c	queue.h
	$(CC)	$(CFLAGS)	-c	queue.c

clean	:	
	rm	-f	httpserver	*.o
