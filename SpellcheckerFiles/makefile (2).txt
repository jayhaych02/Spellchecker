CC = gcc
CFLAGS = -I. -Wall -Werror -lpthread
DEPS = network.h dictionary.h
OBJ = server.o client.o network.o dictionary.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

server.out: server.o network.o dictionary.o
	$(CC) -o $@ $^ $(CFLAGS)

client.out: client.o network.o dictionary.o
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean run runclient

run: server.out
	./server.out -k $(PORT) -w $(THREADS) -j $(BUFFER) -e $(PRIO) -b $(DICT)

runclient: client.out
	./client.out
	
runmanyclient:
	./client.out & ./client.out & ./client.out & ./client.out & ./client.out

clean:
	rm -rf *.out *.o 
