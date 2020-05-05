# Monkeys with Keyboards

## So...  What does it do?

Monkeys with Keyboards is a project that attempts to prove a variant of the [infinite monkey theorem](https://en.wikipedia.org/wiki/Infinite_monkey_theorem),
with a psuedo-random number generator instead of a monkey and the goal of creating a single, valid C program
instead of a specific existing text, such as the complete works of William Shakespeare.
That is, it attempts to compile programs it builds out of random strings of printable ASCII characters.
It uses the following algorithm:

- Get a random number, N, under some limit.
- Get random characters until we find N printable ASCII characters.
- Write this data to a file.
- Run a C compiler on the file:
  - If it compiles, exit.
  - If it doesn't, try again.

## But... Why?

You know, I try not to think too hard about that.
But when I do, I think of Monkeys with Keyboards as a kind of art project or novelty, like a [useless box](https://en.wikipedia.org/wiki/Useless_machine).
It's something I don't really ever expect anyone to run (though please do if you're considering it!) but
that I want people to see and think about nonetheless.

Initially, though, it was an excuse to write something in C.
This idea in particular offered an interesting set of challenges: generating randomness (and generating random strings), shelling out to gcc, file management, etc.
And doing all of this in a loop means any memory leaks will compound quickly.
So if my artsy explanation wasn't good enough, think of this as a kind of on-going [kata](https://en.wikipedia.org/wiki/Kata).

## Great!  How do I get started?

You can compile _monkey_ using the provided _Makefile_ by running _make_.
The script _run.sh_ provides a simple way to quietly run the program, writing the output to a log file.

## But what if one monkey isn't enough?

Not to worry, Monkeys with Keyboards is multithreaded!
Just use the `-n N` argument to run it with `N` threads.
There are other options you can change too if you like, use `./monkey -h` or `./run.sh -h` to learn more.
