
monkey : Makefile src/monkey.c src/seed.c src/seed.h
	gcc -o monkey src/monkey.c src/seed.c

clean :
	rm -f monkey tmp/ out.log
