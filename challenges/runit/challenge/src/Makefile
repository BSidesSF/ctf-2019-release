CC?=gcc
OBJS=runit.o

runit: ${OBJS}
	${CC} -m32 ${CFLAGS} -o runit runit.c

clean:
	rm -fv *.o core core.*
