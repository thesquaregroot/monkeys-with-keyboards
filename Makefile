
monkey : Makefile src/main.c src/monkey.c src/monkey.h src/seed.c src/seed.h
	gcc -o monkey src/main.c src/monkey.c src/seed.c

clean :
	rm -f monkey tmp/ out.log
