all: y86emul

y86emul:
	gcc -Wall -g -o y86emul y86emul.c 

clean:
	rm y86emul

