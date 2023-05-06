CFLAGS = -Wall -g -lm

build: server subscriber

subscriber: subscriber.c
	gcc subscriber.c -o subscriber $(CFLAGS)

server: server.c
	gcc server.c -o server $(CFLAGS)

clean:
	rm -f server subscriber