
monkey : monkey.c
	gcc -o monkey monkey.c

clean :
	rm -f monkey _a.out _test.c out.log

