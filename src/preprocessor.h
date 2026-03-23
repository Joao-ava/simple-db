#ifndef SIMPLE_DB_PREPROCESSOR_H
#define SIMPLE_DB_PREPROCESSOR_H

#include "repl.h"

typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT
} StatementType;

typedef struct {
    StatementType type;
} Statement;

typedef enum {
    PREPARE_SUCCESS,
    PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult;

typedef enum {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement);
#endif //SIMPLE_DB_PREPROCESSOR_H