CC=gcc
CFLAGS=-Wall
DEPS = tipo.h
OBJ = utility.o help.o vedi_vincite.o signup.o login.o invia_giocata.o vedi_giocate.o vedi_estrazione.o const.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

lottomake: lotto_server.o $(OBJ)
	$(CC) -o lotto_server lotto_server.o $(OBJ)

clean:
	rm *.o lotto_server
