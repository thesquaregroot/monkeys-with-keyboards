#ifndef MONKEY_H
#define MONKEY_H 1

#include <stdbool.h>

extern volatile bool KEEP_MONKEYING;

int monkey(const char * source_file, const char * output_file, int read_max, bool verbose, bool silent);

bool buildCommand(char * compile_command, size_t max_command_length, bool verbose, const char * source_file, const char * output_file);
bool tryNextRandomProgram(size_t size, const char * compile_command, const char * source_file, const char * output_file);
char * getPrintableCharacters(size_t size);

#endif
