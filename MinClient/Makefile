CC=gcc
CFLAGS=-I.

DEPS = definitions.h
OBJ =  client.o argumenthandling.o datagramFunctions.o timeFunctions.o 

#builds objects with %.o:
%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLGAS)

NTPClient: $(OBJ)
	gcc -g -o $@ $^ $(CFLAGS)

clean:
	rm -f *.o

