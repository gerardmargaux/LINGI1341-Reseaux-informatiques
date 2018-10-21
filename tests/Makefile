sender_test: sender_test.o
	@gcc -Wall -o $@ sender_test.o lib.a -lz

sender_test.o:
	@gcc -Wall -o sender_test.o -c sender_test.c -lz

receiver_test: receiver_test.o
	@gcc -Wall -o $@ receiver_test.o lib.a -lz

receiver_test.o:
	@gcc -Wall -o receiver_test.o -c receiver_test.c -lz

lib: lib.o
	@ar r lib.a lib.o

lib.o:
	@gcc -Wall -o lib.o -c lib.c -lz

test: clean lib test.o
	@gcc -Wall -o $@ test.o lib.a -lz

test.o:
	@gcc -Wall -o test.o -c test.c -lz

.PHONY: tests

clean:
	@rm -f *.o *.a sender_test receiver_test test && clear
