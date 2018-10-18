main: clean lib sender receiver

sender: sender.o
	@gcc -Wall -o $@ sender.o lib.a -lz

sender.o:
	@gcc -Wall -o sender.o -c sender.c -lz

receiver: receiver.o
	@gcc -Wall -o $@ receiver.o lib.a -lz

receiver.o:
	@gcc -Wall -o receiver.o -c receiver.c -lz

lib: lib.o
	@ar r lib.a lib.o

lib.o:
	@gcc -Wall -o lib.o -c lib.c -lz

test: clean lib test.o
	@gcc -Wall -o $@ test.o lib.a -lz

test.o:
	@gcc -Wall -o test.o -c test.c -lz

.PHONY: clean tests

clean:
	@rm -f *.o *.a sender receiver test && clear

