#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include "seed.h"

// Adapted from: https://stackoverflow.com/a/323302/3025921
//          and: https://burtleburtle.net/bob/hash/doobs.html

unsigned long mix(unsigned long, unsigned long, unsigned long);

void seed_random() {
    unsigned long seed = mix(clock(), time(NULL), getpid());
    srand(seed);
}

unsigned long mix(unsigned long a, unsigned long b, unsigned long c)
{
    a=a-b;  a=a-c;  a=a^(c >> 13);
    b=b-c;  b=b-a;  b=b^(a << 8);
    c=c-a;  c=c-b;  c=c^(b >> 13);
    a=a-b;  a=a-c;  a=a^(c >> 12);
    b=b-c;  b=b-a;  b=b^(a << 16);
    c=c-a;  c=c-b;  c=c^(b >> 5);
    a=a-b;  a=a-c;  a=a^(c >> 3);
    b=b-c;  b=b-a;  b=b^(a << 10);
    c=c-a;  c=c-b;  c=c^(b >> 15);
    return c;
}
