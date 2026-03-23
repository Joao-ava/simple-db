#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "repl.h"
#include "preprocessor.h"

MetaCommandResult do_meta_command(InputBuffer *input_buffer) {
    if (strcmp(input_buffer->buffer, ".exit") == 0) {
        close_input_buffer(input_buffer);
        exit(EXIT_SUCCESS);
    }
    return META_COMMAND_UNRECOGNIZED_COMMAND;
}

void execute_statement(Statement *statement) {
    switch (statement->type) {
        case STATEMENT_INSERT:
            printf("Insert statement.\n");
            break;
        case STATEMENT_SELECT:
            printf("Select statement.\n");
            break;
    }
}

int main(void) {
    InputBuffer* input_buffer = new_input_buffer();
    while (true) {
        print_prompt();
        read_input(input_buffer);

        if (input_buffer->buffer[0] == '.') {
            switch (do_meta_command(input_buffer)) {
                case META_COMMAND_SUCCESS:
                    continue;
                case META_COMMAND_UNRECOGNIZED_COMMAND:
                    printf("Unrecognized command `%s'\n", input_buffer->buffer);
                    continue;
            }
        }

        Statement statement;
        switch (prepare_statement(input_buffer, &statement)) {
            case PREPARE_SUCCESS:
                break;
            case PREPARE_UNRECOGNIZED_STATEMENT:
                printf("Unrecognized keyword at start of `%s'.\n", input_buffer->buffer);
                continue;
        }
        execute_statement(&statement);
        printf("Executed.\n");
    }
}