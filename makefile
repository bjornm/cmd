all: dirtail

dirtail: dirtail.c
	gcc -Wall -o $@ $<

clean: 
	rm dirtail
