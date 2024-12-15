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

.PHONY: run-default run-debug run-test clean run-client run-multiclient

run-default:
	./server.out -k 8888 -w 5 -j 10 -e 1 -b dictAM.txt

run-debug:
	./server.out -k 9999 -w 3 -j 5 -e 1 -b dictAM.txt

run-test:
	./server.out -k 7777 -w 8 -j 15 -e 2 -b dictAM.txt

run-custom:
	./server.out -k $(PORT) -w $(THREADS) -j $(BUFFER) -e $(PRIO) -b $(DICT)

run-client:
	./client.out

run-multiclient:
	./client.out & ./client.out & ./client.out & ./client.out & ./client.out

clean:
	rm -f *.out *.o