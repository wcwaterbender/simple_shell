CC = gcc
ARGS = -Wall

all: simple_shell

simple_shell: simple_shell.cpp
	$(CC) $(ARGS) -o simple_shell simple_shell.cpp

clean:
	rm -f *.o simple_shell *~
