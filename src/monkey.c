#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "seed.h"
#include "monkey.h"

volatile bool KEEP_MONKEYING = true;

const size_t MAX_COMMAND_LENGTH = 256;

const char * COMPILE_COMMAND_FORMAT = "/usr/bin/gcc -Wfatal-errors -o \"%s\" \"%s\"";
const char * COMPILE_COMMAND_ERROR_REDIRECT = " > /dev/null 2>&1";

int monkey(const char * source_file, const char * output_file, int read_max, bool verbose, bool silent) {
    char compile_command[MAX_COMMAND_LENGTH];
    bool good_command = buildCommand(compile_command, MAX_COMMAND_LENGTH, verbose, source_file, output_file);
    if (!good_command) {
        fprintf(stderr, "Unable to built command.  Built [%s].", compile_command);
        return 1;
    }

    seed_random();

    unsigned long attempt = 1;
    while (KEEP_MONKEYING) {
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
            if (!silent && verbose) {
                printf("Failed.\n");
                printf("--------------------\n");
            }
            attempt++;
        }
    }

    return 0;
}

bool doesOutputFileExist(const char * output_file);

bool buildCommand(char * compile_command, size_t max_command_length, bool verbose, const char * source_file, const char * output_file) {
    int command_length = snprintf(compile_command, max_command_length, COMPILE_COMMAND_FORMAT, output_file, source_file);
    if (!verbose) {
        strncat(compile_command, COMPILE_COMMAND_ERROR_REDIRECT, max_command_length - command_length);
        command_length += strlen(COMPILE_COMMAND_ERROR_REDIRECT);
    }
    return command_length > 0 && command_length < max_command_length;
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
        exit(1);
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

