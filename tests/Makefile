all: test_self test_unco

test_self: test_self.o
	@gcc -Wall -o test_self test_self.o

test_self.o:
	@gcc -Wall -o test_self.o -c test_self.c

test_unco: test_unco.o
	@gcc -Wall -o $@ $<

test_unco.o:
	@gcc -Wall -o $@ -c test_unco.c

.PHONY: clean clear

clean: clear
	@rm -f *.o test_self test_unco

clear:
	@clear
