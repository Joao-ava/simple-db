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

void execute_statement(const Statement *statement) {
    switch (statement->type) {
        case STATEMENT_INSERT:
            printf("Insert statement.\n");
            break;
        case STATEMENT_SELECT:
            printf("Select statement.\n");
            printf("Table %s\n", statement->data.select.table_name);
            for (int i = 0; i < statement->data.select.column_count; i++) {
                printf("Column %d: %s\n", i, statement->data.select.column_names[i]);
            }
            break;
        case STATEMENT_CREATE:
            printf("Create statement.\n");
            printf("Table %s\n", statement->data.create_table.table_name);
            for (int i = 0; i < statement->data.create_table.column_count; i++) {
                printf("Column %d: %s %d\n", i, statement->data.create_table.columns[i].name, statement->data.create_table.columns[i].type);
            }
            break;
        case STATEMENT_INVALID:
            printf("Invalid statement.\n");
            break;
    }
}

int main(void) {
    InputBuffer *input_buffer = new_input_buffer();
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

        Statement statement = {0};

        ParseError error = { .code = ERROR_NONE, .cursor_position = 0 };
        parse_sql(input_buffer, &statement, &error);
        print_parse_error(input_buffer->buffer, &error);

        if (error.code != ERROR_NONE) {
            continue;
        }

        execute_statement(&statement);
        if (statement.type == STATEMENT_INVALID) continue;
        printf("Executed.\n");
    }
}