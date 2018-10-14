# See gcc/clang manual to understand all flags
CFLAGS += -std=c99 # Define which version of the C standard to use
CFLAGS += -Wall # Enable the 'all' set of warnings
CFLAGS += -Werror # Treat all warnings as error
CFLAGS += -Wshadow # Warn when shadowing variables
CFLAGS += -Wextra # Enable additional warnings
CFLAGS += -O2 -D_FORTIFY_SOURCE=2 # Add canary code, i.e. detect buffer overflows
CFLAGS += -fstack-protector-all # Add canary code to detect stack smashing
CFLAGS += -D_POSIX_C_SOURCE=201112L -D_XOPEN_SOURCE # feature_test_macros for getpot and getaddrinfo

# We have no libraries to link against except libc, but we want to keep
# the symbols for debugging
LDFLAGS= -rdynamic

# Default target
all: clean chat

# If we run `make debug` instead, keep the debug symbols for gdb
# and define the DEBUG macro.
debug: CFLAGS += -g -DDEBUG -Wno-unused-parameter -fno-omit-frame-pointer
debug: clean chat

# We use an implicit rule to build an executable named 'chat'
chat: chat.o read_write_loop.o create_socket.o real_address.o wait_for_client.o

.PHONY: clean test_real_address test_create_socket test_gethostname

clean:
	@rm -f chat test_real_address test_create_socket *.o

test_gethostname: test_gethostname.o
	@gcc -Wall -o test_gethostname test_gethostname.o

test_gethostname.o:
	@gcc -Wall -o test_gethostname.o -c test_gethostname.c

test_real_address: test_real_address.o
	@gcc -Wall -o test_real_address test_real_address.o

test_real_address.o:
	@gcc -Wall -o test_real_address.o -c test_real_address.c

test_create_socket: test_create_socket.o
	@gcc -Wall -o test_create_socket test_create_socket.o

test_create_socket.o:
	@gcc -Wall -o test_create_socket.o -c test_create_socket.c
