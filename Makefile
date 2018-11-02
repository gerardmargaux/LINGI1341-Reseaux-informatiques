main: clean lib sender receiver linksim

sender: sender.o
	@gcc -Wall -g -o $@ src/sender.o src/lib.a -lz

sender.o:
	@gcc -Wall -o src/sender.o -c src/sender.c -I src

receiver: receiver.o
	@gcc -Wall -g -o $@ src/receiver.o src/lib.a -lz

receiver.o:
	@gcc -Wall -o src/receiver.o -c src/receiver.c -I src

lib: lib.o
	@ar r src/lib.a src/lib.o

lib.o:
	@gcc -Wall -o src/lib.o -c src/lib.c

linksim:
	@cd linksim && $(MAKE)

.PHONY: clean tests

clean:
	@rm -f *.o sender receiver test && clear && cd src && rm -f *.a *.o && cd ../tests && $(MAKE) clean

tests: clean lib sender receiver
	@cd tests && $(MAKE)
