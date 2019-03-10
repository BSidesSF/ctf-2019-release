CFLAGS?=-Wall -fno-stack-protector -m32
LIBS?=-lpthread -lm
all: genius loader

clean:
	rm -f genius *.o core core.*

genius: genius.o
	${CC} ${CFLAGS} -o genius genius.o ${LIBS}
	strip genius

loader: loader.o
	${CC} ${CFLAGS} -o loader loader.o ${LIBS}
	strip loader
