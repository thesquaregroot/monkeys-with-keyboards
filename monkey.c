#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef int bool; // for sanity

const size_t DEFAULT_READ_MAX = 1024;
const char * DEFAULT_SOURCE_FILE = "_test.c";
const char * DEFAULT_OUTPUT_FILE = "_a.out";

const size_t MAX_COMMAND_SIZE = 128;
const char * COMPILE_COMMAND_FORMAT = "gcc -o \"%s\" \"%s\" > /dev/null";

bool tryNextRandomProgram(size_t read_max, const char * source_file, const char * output_file);
char * getPrintableCharacters(size_t size);

int main(int argc, char * argv[]) {
    // TODO: override with command-line arguments
    const size_t read_max = DEFAULT_READ_MAX;
    const char * output_file = DEFAULT_OUTPUT_FILE;
    const char * source_file = DEFAULT_SOURCE_FILE;

    time_t now;
    srand((unsigned) time(&now));

    setbuf(stdout, NULL); // disable stdout buffering

    char compile_command[MAX_COMMAND_SIZE];
    int command_length = snprintf(compile_command, MAX_COMMAND_SIZE, COMPILE_COMMAND_FORMAT, output_file, source_file);
    if (command_length < 0 || command_length >= MAX_COMMAND_SIZE) {
        printf("Unable to built command.  Built [%s].", compile_command);
        return 1;
    }

    unsigned int attempt = 1;
    while (1) {
        printf("Attempt %d.\n", attempt);
        bool success = tryNextRandomProgram(read_max, compile_command, source_file);
        if (success) {
            printf("Success!  See %s for the valid program!\n", source_file);
            printf("  WARNING: You probably don't want to run %s!\n", output_file);
            break;
        } else {
            printf("Failed.\n");
            printf("--------------------\n");
            attempt++;
        }
    }
}

bool tryNextRandomProgram(const size_t read_max, const char * compile_command, const char * source_file) {
    const size_t size = rand() % read_max;

    printf("Reading %d printable characters.\n", size);
    const char * text = getPrintableCharacters(size);

    FILE * f = fopen(source_file, "w");
    fputs(text, f);
    fclose(f);

    int ret = system(compile_command);
    return ret == 0;
}

char * getPrintableCharacters(size_t size) {
    char * text = malloc(size+1);
    for (int i=0; i<size; i++) {
        char c = rand();
        while (!isprint(c)) {
            c = rand();
        }
        text[i] = c;
    }
    text[size] = '\0'; // null-terminate
    return text;
}

