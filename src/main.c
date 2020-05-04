#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include "seed.h"
#include "monkey.h"

void interruptHandler(int signal) {
    KEEP_MONKEYING = false;
}

const size_t DEFAULT_READ_MAX = 1024;
const char * DEFAULT_OUTPUT_DIRECTORY = "tmp";
const char * DEFAULT_SOURCE_FILE_NAME = "_test.c";
const char * DEFAULT_OUTPUT_FILE_NAME = "_a.out";

int createOrEmptyTmpDirectory(const char * output_directory);

int main(int argc, char * argv[]) {
    bool verbose = false;
    bool silent = false;
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
            silent = true;
            verbose = false;
            break;
        case 'v':
            verbose = true;
            silent = false;
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

    signal(SIGINT, interruptHandler);
    signal(SIGTERM, interruptHandler);

    int successCode = createOrEmptyTmpDirectory(output_directory);
    if (successCode != 0) {
        fprintf(stderr, "Unable to clear or create directory: %s", output_directory);
        return 1;
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

    int result = monkey(source_file, output_file, read_max, verbose, silent);
    return result;
}

int createOrEmptyTmpDirectory(const char * output_directory) {
    char * command = malloc(100 * sizeof(char));
    snprintf(command, 100, "rm -rf %s && mkdir %s", output_directory, output_directory);
    return system(command);
}

