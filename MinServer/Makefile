CC=gcc
CFLAGS=-I.

DEPS = definitions.h
OBJ =  server.o argumenthandling.o datagramFunctions.o timeFunctions.o 

#builds objects with %.o:
%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLGAS)

NTPServer: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

clean:
	rm -f *.o

