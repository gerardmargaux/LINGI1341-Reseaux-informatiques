main: clean lib sender receiver

sender: sender.o
	@gcc -Wall -o $@ src/sender.o src/lib.a

sender.o:
	@gcc -Wall -o src/sender.o -c src/sender.c -I src

receiver: receiver.o
	@gcc -Wall -o $@ src/receiver.o src/lib.a

receiver.o:
	@gcc -Wall -o src/receiver.o -c src/receiver.c -I src

lib: lib.o
	@ar r src/lib.a src/lib.o

lib.o:
	@gcc -Wall -o src/lib.o -c src/lib.c

.PHONY: clean tests

clean:
	@rm -f *.o sender receiver && clear && cd src && rm -f *.a *.o
