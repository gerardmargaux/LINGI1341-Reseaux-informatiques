select: clean select.o
	@gcc -Wall -o select_test select.o

select.o:
	@gcc -Wall -o select.o -c select.c

showip: showip.o
	@gcc -o showip $<

showip.o: showip.c
	@gcc -o $@ -c $<

.PHONY: clean

clean:
	@rm -f *.o select_test showip && clear
