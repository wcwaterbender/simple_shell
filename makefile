CC = g++
ARGS = -Wall

all: simple_shell simple_shell2

simple_shell: simple_shell.cpp
	$(CC) $(ARGS) -o simple_shell simple_shell.cpp

simple_shell2:
	$(CC) $(ARGS) -o simple_shell2 simple_shell2.cpp
clean:
	rm -f *.o simple_shell *~
