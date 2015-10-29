#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

typedef int bool; // for sanity

const size_t DEFAULT_READ_MAX = 1024;
const char * DEFAULT_SOURCE_FILE = "_test.c";
const char * DEFAULT_OUTPUT_FILE = "_a.out";

const size_t MAX_COMMAND_SIZE = 256;
const char * COMPILE_COMMAND_FORMAT = "gcc -o \"%s\" \"%s\"";
const char * COMPILE_COMMAND_ERROR_REDIRECT = "&> /dev/null";

bool buildCommand(char * compile_command, bool verbose, const char * output_file, const char * source_file);
bool tryNextRandomProgram(size_t size, const char * compile_command, const char * output_file);
char * getPrintableCharacters(size_t size);

int main(int argc, char * argv[]) {
    bool verbose = 0;
    size_t read_max = DEFAULT_READ_MAX;
    const char * output_file = DEFAULT_OUTPUT_FILE;
    const char * source_file = DEFAULT_SOURCE_FILE;

    int c;
    while ((c = getopt (argc, argv, "i:hn:o:v")) != -1) {
        switch (c) {
        case 'h':
            printf("Usage: %s [-h] [-v] [-n max_size] [-i source_file] [-o output_file]\n", argv[0]);
            return 0;
        case 'v':
            verbose = 1;
            break;
        case 'o':
            output_file = optarg;
            break;
        case 'i':
            source_file = optarg;
            break;
        case 'n':
            read_max = atoi(optarg);
            break;
        case '?':
            if (optopt == 'o' || optopt == 'i' || optopt == 'n') {
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            } else if (isprint (optopt)) {
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            } else {
                fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
            }
            return 1;
        default:
            fprintf(stderr, "Unrecognized argument '%c'.\n", c);
            return 1;
        }
    }

    time_t now;
    srand((unsigned) time(&now));

    setbuf(stdout, NULL); // disable stdout buffering

    char compile_command[MAX_COMMAND_SIZE];
    bool good_command = buildCommand(compile_command, verbose, output_file, source_file);
    if (!good_command) {
        printf("Unable to built command.  Built [%s].", compile_command);
        return 1;
    }

    unsigned int attempt = 1;
    while (1) {
        const size_t size = rand() % read_max;
        printf("Attempt %d (%d characters).\n", attempt, size);
        bool success = tryNextRandomProgram(size, compile_command, source_file);
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

bool buildCommand(char * compile_command, bool verbose, const char * output_file, const char * source_file) {
    int command_length = snprintf(compile_command, MAX_COMMAND_SIZE, COMPILE_COMMAND_FORMAT, output_file, source_file);
    if (!verbose) {
        strncat(compile_command, COMPILE_COMMAND_ERROR_REDIRECT, MAX_COMMAND_SIZE);
        command_length += strlen(COMPILE_COMMAND_ERROR_REDIRECT);
    }
    return command_length > 0 && command_length < MAX_COMMAND_SIZE;
}

bool tryNextRandomProgram(size_t size, const char * compile_command, const char * source_file) {
    const char * text = getPrintableCharacters(size);

    FILE * f = fopen(source_file, "w");
    fputs(text, f);
    fclose(f);

    int ret = system(compile_command);
    return ret == 0;
}

char getRandomCharacter() {
    return rand() & 0x7F; // get bits in ASCII range
}

char * getPrintableCharacters(size_t size) {
    char * text = malloc(size+1);
    for (int i=0; i<size; i++) {
        // random character in ASCII range
        char c = getRandomCharacter();
        while (!(isprint(c) || c == '\n')) {
            c = getRandomCharacter();
        }
        text[i] = c;
    }
    text[size] = '\0'; // null-terminate
    return text;
}

