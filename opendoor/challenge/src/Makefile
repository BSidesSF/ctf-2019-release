CFLAGS = -Wall -std=c++11
LDFLAGS = -Wl,-Bstatic -lssl -lcrypto -pthread -Wl,-Bdynamic -ldl
CXX = g++
OBJS = crypter.o buffer.o conn.o message.o main.o state.o

opendoor: $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)
	mkdir -p ../../distfiles
	strip --strip-debug -o ../../distfiles/$@ $@

%.o: %.cc $(wildcard *.h) Makefile
	$(CXX) -c $(CFLAGS) $< -o $@

clean:
	rm -f opendoor $(OBJS)
