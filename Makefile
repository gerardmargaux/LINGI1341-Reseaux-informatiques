main: clean lib sender receiver

sender: sender.o
	@gcc -Wall -o $@ src/sender.o src/lib.a -lz

sender.o:
	@gcc -Wall -o src/sender.o -c src/sender.c -lz -I src

receiver: receiver.o
	@gcc -Wall -o $@ src/receiver.o src/lib.a -lz

receiver.o:
	@gcc -Wall -o src/receiver.o -c src/receiver.c -lz -I src

lib: lib.o
	@ar r src/lib.a src/lib.o

lib.o:
	@gcc -Wall -o src/lib.o -c src/lib.c -lz

.PHONY: clean tests

clean:
	@rm -f *.o sender receiver && clear && cd src && rm -f *.a *.o
