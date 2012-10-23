all: dirtail

dirtail: dirtail.c
	gcc -Wall -o $@ $<
