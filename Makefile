main: clean lib sender receiver

sender: local_sender.o
	@gcc -Wall -o $@ src/local_sender.o src/lib.a -lz

local_sender.o:
	@gcc -Wall -o src/local_sender.o -c src/local_sender.c -I src

receiver: local_receiver.o
	@gcc -Wall -o $@ src/local_receiver.o src/lib.a -lz

local_receiver.o:
	@gcc -Wall -o src/local_receiver.o -c src/local_receiver.c -I src

lib: lib.o
	@ar r src/lib.a src/lib.o

lib.o:
	@gcc -Wall -o src/lib.o -c src/lib.c

.PHONY: clean tests

clean:
	@rm -f *.o sender receiver test && clear && cd src && rm -f *.a *.o && cd ../tests && $(MAKE) clean

tests:
	@cd tests && $(MAKE)
