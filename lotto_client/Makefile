CC=gcc
CFLAGS=-Wall

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

lottomake: lotto_client.o
	$(CC) -o lotto_client lotto_client.o

clean:
	rm *.o lotto_client
