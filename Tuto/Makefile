main: clean select_a select_b

select_a: select_a.o
	@gcc -Wall -o select_a select_a.o

select_a.o:
	@gcc -Wall -o select_a.o -c select_a.c

select_b: select_b.o
	@gcc -Wall -o select_b select_b.o

select_b.o:
	@gcc -Wall -o select_b.o -c select_b.c

showip: showip.o
	@gcc -o showip $<

showip.o: showip.c
	@gcc -o $@ -c $<

.PHONY: clean

clean:
	@rm -f *.o select_a select_b showip && clear
