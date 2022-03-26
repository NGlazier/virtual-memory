CFLAGS = -g -Wall -Werror -O3
CC = g++

PROGRAM = vm

 all: $(PROGRAM)
 
  $(PROGRAM): $(PROGRAM).cc
	$(CC) $(CFLAGS) -o $(PROGRAM) $(PROGRAM).cc
 
  clean:
	$(RM) $(PROGRAM)