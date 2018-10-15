main: clean sender receiver

sender:
	@gcc -o $@ src/sender.c

receiver:
	@gcc -o $@ src/receiver.c

.PHONY: clean

clean:
	@rm -f *.o sender receiver && clear
