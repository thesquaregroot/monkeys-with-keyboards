#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <stdbool.h>
#include "seed.h"
#include "monkey.h"

void interruptHandler(int signal) {
    stopMonkeying();
}

const size_t DEFAULT_READ_MAX = 1024;
const char * DEFAULT_OUTPUT_DIRECTORY = "tmp";
const char * DEFAULT_SOURCE_FILE_NAME = "_test.c";
const char * DEFAULT_OUTPUT_FILE_NAME = "_a.out";

int createOrEmptyTmpDirectory(const char * output_directory);
bool createFilePath(char * file_path, size_t max_length, const char * output_directory, const char * prefix, const char * base_file_name);
int startMonkeying(const char * output_directory, int index, const char * output_file_name, const char * source_file_name, int read_max, bool verbose, bool silent);

struct MonkeyThreadArguments {
    const char * output_directory;
    int index;
    const char * output_file_name;
    const char * source_file_name;
    int read_max;
    bool verbose;
    bool silent;
};
void * monkeyThread(void * vargp);

int main(int argc, char * argv[]) {
    bool verbose = false;
    bool silent = false;
    int parallelism = 0;
    size_t read_max = DEFAULT_READ_MAX;
    const char * output_file_name = DEFAULT_OUTPUT_FILE_NAME;
    const char * source_file_name = DEFAULT_SOURCE_FILE_NAME;
    // consider impacts if this made configurable, since it is blindly deleted
    const char * const output_directory = DEFAULT_OUTPUT_DIRECTORY;

    int c;
    while ((c = getopt(argc, argv, "hi:p:n:o:sv")) != -1) {
        switch (c) {
        case 'h':
            printf("Usage: %s [-h] [-v] [-s] [-n max_size] [-i source_file] [-o output_file]\n", argv[0]);
            printf("\n");
            printf(" -h         Display this help content\n");
            printf(" -v         Enable verbose mode\n");
            printf(" -s         Enable silent mode\n");
            printf(" -p [N]     Degree of parallelism: create N monkey threads\n");
            printf(" -n [N]     Set the maximum number of characters to add to the input file\n");
            printf(" -i [file]  Override the default source file name of %s\n", DEFAULT_SOURCE_FILE_NAME);
            printf(" -o [file]  Override the default output file name of %s\n", DEFAULT_OUTPUT_FILE_NAME);
            printf("\n");
            return 0;
        case 'i':
            source_file_name = optarg;
            break;
        case 'p':
            parallelism = atoi(optarg);
            if (parallelism < 1 || parallelism > 255) {
                fprintf(stderr, "Parallelism must be between 1 and 255.\n");
                return 1;
            }
            break;
        case 'n':
            read_max = atoi(optarg);
            if (read_max <= 0) {
                fprintf(stderr, "Character maximum must be an integer greater than zero.\n");
                return 1;
            }
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
        fprintf(stderr, "Unable to clear or create directory: %s\n", output_directory);
        return 1;
    }

    if (parallelism == 0) {
        struct MonkeyThreadArguments args;
        args.output_directory = output_directory;
        args.index = 0;
        args.output_file_name = output_file_name;
        args.source_file_name = source_file_name;
        args.read_max = read_max;
        args.verbose = verbose;
        args.silent = silent;

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, monkeyThread, &args);
        pthread_join(thread_id, NULL);
    } else {
        struct MonkeyThreadArguments * args = malloc( (parallelism + 1) * sizeof(struct MonkeyThreadArguments));
        struct MonkeyThreadArguments * args_iterator = args;
        pthread_t * threads = malloc((parallelism + 1) * sizeof(pthread_t));
        pthread_t * thread_iterator = threads;

        for (int i=1; i <= parallelism; i++) {
            args->output_directory = output_directory;
            args->index = i;
            args->output_file_name = output_file_name;
            args->source_file_name = source_file_name;
            args->read_max = read_max;
            args->verbose = verbose;
            args->silent = silent;

            pthread_t thread_id;
            pthread_create(&thread_id, NULL, monkeyThread, args);

            *threads = thread_id;
            threads++;
            args++;
        }
        *threads = 0;

        args = args_iterator;
        threads = thread_iterator;

        while (*thread_iterator != 0) {
            pthread_t thread_id = *thread_iterator;
            pthread_join(thread_id, NULL);
            printf("Joined thread %ld\n", thread_id);
            thread_iterator++;
            args_iterator++;
        }
        free(args);
        free(threads);
    }

    return 0;
}

int createOrEmptyTmpDirectory(const char * output_directory) {
    char * command = malloc(100 * sizeof(char));
    snprintf(command, 100, "rm -rf %s && mkdir %s", output_directory, output_directory);
    int result = system(command);
    free(command);
    return result;
}

bool createFilePath(char * file_path, size_t max_length, const char * output_directory, const char * prefix, const char * base_file_name) {
    int length = snprintf(file_path, max_length, "%s/%s%s", output_directory, prefix, base_file_name);
    if (length > max_length) {
        fprintf(stderr, "Output file path too long: %s", file_path);
        return false;
    }
    return true;
}

int startMonkeying(const char * output_directory, int index, const char * output_file_name, const char * source_file_name, int read_max, bool verbose, bool silent) {
    char * prefix = malloc(5 * sizeof(char));
    if (index == 0) {
        prefix[0] = '\0';
    } else {
        snprintf(prefix, 5, "_%d", index);
    }

    char * output_file = malloc(255 * sizeof(char));
    if (! createFilePath(output_file, 255, output_directory, prefix, output_file_name)) {
        return 1;
    }
    char * source_file = malloc(255 * sizeof(char));
    if (! createFilePath(source_file, 255, output_directory, prefix, source_file_name)) {
        return 1;
    }
    free(prefix);

    int result = monkey(source_file, output_file, read_max, verbose, silent);
    free(output_file);
    free(source_file);
    return result;
}

void * monkeyThread(void * vargp) {
    struct MonkeyThreadArguments args = * (struct MonkeyThreadArguments *) vargp;

    int result = startMonkeying(args.output_directory, args.index, args.output_file_name, args.source_file_name, args.read_max, args.verbose, args.silent);
    return NULL;
}

