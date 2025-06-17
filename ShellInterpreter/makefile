LDFLAGS = -lreadline
ssi: ssi.o
	$(CC) -o ssi ssi.o $(LDFLAGS)
ssi.o: ssi.c
	$(CC) -c ssi.c
clean:
	rm -f ssi ssi.o
