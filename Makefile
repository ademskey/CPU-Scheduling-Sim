CC = gcc
CFLAGS = -g

scheduler: scheduler.c
	$(CC) scheduler.c -o scheduler

test01:
	./scheduler sample_io/input/input-1

test02:
	./scheduler sample_io/input/input-2

test03:
	./scheduler sample_io/input/input-3

clean:
	rm -f scheduler *.o *~