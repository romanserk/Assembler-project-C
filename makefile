assembler : main.o internalfunc.o operations.o symbol.o opservice.o
	gcc -g -Wall -ansi -pedantic main.o internalfunc.o operations.o opservice.o symbol.o -o assembler

main.o :: main.c
	gcc -c -Wall -ansi -pedantic main.c -o main.o 

internalfunc.o :: internalfunc.c
	gcc -c -Wall -ansi -pedantic internalfunc.c -o internalfunc.o 

operations.o :: operations.c
	gcc -c -Wall -ansi -pedantic operations.c -o operations.o 

symbol.o :: symbol.c
	gcc -c -Wall -ansi -pedantic symbol.c -o symbol.o 

operations.o :: operations.c
	gcc -c -Wall -ansi -pedantic operations.c -o operations.o 

symbol.o :: symbol.c
	gcc -c -Wall -ansi -pedantic symbol.c -o symbol.o 

opservice.o :: opservice.c
	gcc -c -Wall -ansi -pedantic opservice.c -o opservice.o



