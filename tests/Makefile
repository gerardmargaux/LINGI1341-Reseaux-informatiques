main: clean lib sender_test_stdin receiver_test_stdout sender_test_fichier receiver_test_fichier test_link

test_link:
	./link_sim -p 12345 -P 1236 & ../receiver :: 1236

sender_test_fichier: sender_test_fichier.o
	@gcc -Wall -o $@ sender_test_fichier.o lib.a -lz

sender_test_fichier.o:
	@gcc -Wall -o sender_test_fichier.o -c sender_test_fichier.c -lz

receiver_test_fichier: receiver_test_fichier.o
	@gcc -Wall -o $@ receiver_test_fichier.o lib.a -lz

receiver_test_fichier.o:
	@gcc -Wall -o receiver_test_fichier.o -c receiver_test_fichier.c -lz

sender_test_stdin: sender_test_stdin.o
	@gcc -Wall -o $@ sender_test_stdin.o lib.a -lz

sender_test_stdin.o:
	@gcc -Wall -o sender_test_stdin.o -c sender_test_stdin.c -lz

receiver_test_stdout: receiver_test_stdout.o
	@gcc -Wall -o $@ receiver_test_stdout.o lib.a -lz

receiver_test_stdout.o:
	@gcc -Wall -o receiver_test_stdout.o -c receiver_test_stdout.c -lz

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
	@rm -f *.o *.a sender_test_fichier sender_test_stdin receiver_test_fichier receiver_test_stdout && clear
