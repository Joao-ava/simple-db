#ifndef SIMPLE_DB_REPL_H
#define SIMPLE_DB_REPL_H

#include "preprocessor.h"

InputBuffer* new_input_buffer();
void print_prompt();
void read_input(InputBuffer* input_buffer);
void close_input_buffer(InputBuffer* input_buffer);
void print_parse_error(const char* sql_query, ParseError* error);

#endif //SIMPLE_DB_REPL_H