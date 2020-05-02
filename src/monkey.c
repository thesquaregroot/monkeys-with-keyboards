#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "seed.h"

typedef int bool; // for sanity

static volatile bool keepRunning = 1;

void interruptHandler(int signal) {
    keepRunning = 0;
}

const size_t DEFAULT_READ_MAX = 1024;
const char * DEFAULT_OUTPUT_DIRECTORY = "tmp";
const char * DEFAULT_SOURCE_FILE_NAME = "_test.c";
const char * DEFAULT_OUTPUT_FILE_NAME = "_a.out";

const size_t MAX_COMMAND_SIZE = 256;
const char * COMPILE_COMMAND_FORMAT = "/usr/bin/gcc -Wfatal-errors -o \"%s\" \"%s\"";
const char * COMPILE_COMMAND_ERROR_REDIRECT = " > /dev/null 2>&1";

int createOrEmptyTmpDirectory(const char * output_directory);
bool doesOutputFileExist(const char * output_file);
bool buildCommand(char * compile_command, bool verbose, const char * source_file, const char * output_file);
bool tryNextRandomProgram(size_t size, const char * compile_command, const char * source_file, const char * output_file);
char * getPrintableCharacters(size_t size);

int main(int argc, char * argv[]) {
    bool verbose = 0;
    bool silent = 0;
    size_t read_max = DEFAULT_READ_MAX;
    const char * output_file_name = DEFAULT_OUTPUT_FILE_NAME;
    const char * source_file_name = DEFAULT_SOURCE_FILE_NAME;
    // consider impacts if this made configurable, since it is blindly deleted
    const char * const output_directory = DEFAULT_OUTPUT_DIRECTORY;

    int c;
    while ((c = getopt(argc, argv, "hi:n:o:sv")) != -1) {
        switch (c) {
        case 'h':
            printf("Usage: %s [-h] [-v] [-s] [-n max_size] [-i source_file] [-o output_file]\n", argv[0]);
            printf("\n");
            printf(" -h         Display this help content\n");
            printf(" -v         Enable verbose mode\n");
            printf(" -s         Enable silent mode\n");
            printf(" -n [N]     Set the maximum number of characters to add to the input file\n");
            printf(" -i [file]  Override the default source file name of %s\n", DEFAULT_SOURCE_FILE_NAME);
            printf(" -o [file]  Override the default output file name of %s\n", DEFAULT_OUTPUT_FILE_NAME);
            printf("\n");
            return 0;
        case 'i':
            source_file_name = optarg;
            break;
        case 'n':
            read_max = atoi(optarg);
            break;
        case 'o':
            output_file_name = optarg;
            break;
        case 's':
            silent = 1;
            verbose = 0;
            break;
        case 'v':
            verbose = 1;
            silent = 0;
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

    int length;
    char * output_file = malloc(255 * sizeof(char));
    length = snprintf(output_file, 255, "%s/%s", output_directory, output_file_name);
    if (length > 255) {
        fprintf(stderr, "Output file path too long: %s", output_file);
        return 1;
    }
    char * source_file = malloc(255 * sizeof(char));
    length = snprintf(source_file, 255, "%s/%s", output_directory, source_file_name);
    if (length > 255) {
        fprintf(stderr, "Source file path too long: %s", source_file);
        return 1;
    }

    signal(SIGINT, interruptHandler);
    signal(SIGTERM, interruptHandler);

    char compile_command[MAX_COMMAND_SIZE];
    bool good_command = buildCommand(compile_command, verbose, source_file, output_file);
    if (!good_command) {
        fprintf(stderr, "Unable to built command.  Built [%s].", compile_command);
        return 1;
    }

    int successCode = createOrEmptyTmpDirectory(output_directory);
    if (successCode != 0) {
        fprintf(stderr, "Unable to clear or create directory: %s", output_directory);
        return 1;
    }

    seed_random();

    unsigned long attempt = 1;
    while (keepRunning) {
        const size_t size = rand() % (read_max+1);
        if (!silent) {
            printf("Attempt %ld (%ld characters).\n", attempt, size);
        }
        bool success = tryNextRandomProgram(size, compile_command, source_file, output_file);
        if (success) {
            printf("Success after %ld attempts!  See %s for the valid program!\n", attempt, source_file);
            printf("  WARNING: You probably don't want to run %s!\n", output_file);
            break;
        } else {
            if (verbose) {
                printf("Failed.\n");
                printf("--------------------\n");
            }
            attempt++;
        }
    }
}

bool createOrEmptyTmpDirectory(const char * output_directory) {
    char * command = malloc(100 * sizeof(char));
    snprintf(command, 100, "rm -rf %s && mkdir %s", output_directory, output_directory);
    return system(command);
}

bool buildCommand(char * compile_command, bool verbose, const char * source_file, const char * output_file) {
    int command_length = snprintf(compile_command, MAX_COMMAND_SIZE, COMPILE_COMMAND_FORMAT, output_file, source_file);
    if (!verbose) {
        strncat(compile_command, COMPILE_COMMAND_ERROR_REDIRECT, MAX_COMMAND_SIZE - command_length);
        command_length += strlen(COMPILE_COMMAND_ERROR_REDIRECT);
    }
    return command_length > 0 && command_length < MAX_COMMAND_SIZE;
}

bool tryNextRandomProgram(size_t size, const char * compile_command, const char * source_file, const char * output_file) {
    const char * text = getPrintableCharacters(size);

    FILE * f = fopen(source_file, "w");
    fputs(text, f);
    fclose(f);

    int ret = system(compile_command);
    if (ret == -1) {
        fprintf(stderr, "Failed system call on command, bailing out.\n");
        fprintf(stderr, "COMMAND: %s\n", compile_command);
        keepRunning = 0;
    }

    int executable_created = doesOutputFileExist(output_file);
    return (ret == 0) && executable_created;
}

char getRandomCharacter() {
    return rand() & 0x7F; // get bits in ASCII range
}

char * getPrintableCharacters(size_t size) {
    char * text = malloc((size+1) * sizeof(char));
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

bool doesOutputFileExist(const char * output_file) {
    return access(output_file, F_OK) != -1;
}

