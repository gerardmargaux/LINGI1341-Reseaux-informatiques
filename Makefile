main: main.o
	@gcc -o $@ $<
	
main.o: main.c
	@gcc -o $@ -c $<