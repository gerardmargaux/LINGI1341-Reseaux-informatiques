main: clean packet_implem_test.o
	@gcc -o exec packet_implem_test.o -lz

packet_implem_test.o: packet_implem_test.c
	@gcc -o $@ -c $< 

clean:
	@rm -f *.o exec && clear
