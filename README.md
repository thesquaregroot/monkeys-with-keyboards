# Monkeys with Keyboards
Monkeys with Keyboards is a project that attempts to compile programs it builds
out of random strings of printable ASCII characters.  It uses the following
algorithm:

- Get a random number, N, under some limit.
- Get random characters until we find N printable ASCII characters.
- Write this data to a file.
- Run a C compiler on the file:
  - If it compiles, exit.
  - If it doesn't, try again.

You can compile _monkey_ using the provided _Makefile_ by running _make_.
The script _run.sh_ provides a simple way to quietly run the program, writing
the output to a log file.

