main : test_wait_for_client_send_message.o
	@gcc -o exec test_wait_for_client_send_message.o

test_wait_for_client.o: test_wait_for_client_send_message.c
	@gcc -o $@ -c $<
