#make file 

CC = gcc
CFLAGS = -ansi -Wall -g -O0 -Wwrite-strings -Wshadow -pedantic-errors -fstack-protector-all 
PROGS = d8sh 

all: $(PROGS)


d8sh: d8sh.o executor.o parser.tab.o lexer.o
	$(CC) $(CFLAGS) -o d8sh d8sh.o lexer.o parser.tab.o executor.o -lreadline

lexer.o: lexer.c parser.tab.h
	$(CC) $(CFLAGS) -c lexer.c 

parser.tab.o: parser.tab.c command.h
	$(CC) $(CFLAGS) -c parser.tab.c

d8sh.o: d8sh.c lexer.h executor.h
	$(CC) $(CFLAGS) -c d8sh.c 

executor.o: executor.c command.h executor.h
	$(CC) $(CFLAGS) -c executor.c



clean:
	@rm -f *.o $(PROGS) a.out