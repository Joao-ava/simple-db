#include <string.h>
#include <ctype.h>

#include "preprocessor.h"

#include <stdio.h>
#include <stdlib.h>

int is_keyword(const char *keyword) {
    if (strncasecmp(keyword, "select", 6) == 0) return 1;
    if (strncasecmp(keyword, "insert", 6) == 0) return 1;
    if (strncasecmp(keyword, "from", 4) == 0) return 1;
    if (strncasecmp(keyword, "create", 5) == 0) return 1;
    if (strncasecmp(keyword, "table", 5) == 0) return 1;
    if (strncasecmp(keyword, "int", 3) == 0) return 1;
    if (strncasecmp(keyword, "text", 4) == 0) return 1;
    return 0;
}

int is_symbol(const char symbol) {
    if (symbol == '(' || symbol == ')') return 1;
    if (symbol == '*' || symbol == '=') return 1;
    return symbol == ',' || symbol == ';';
}

void ignore_whitespace(InputBuffer *input) {
    while (input->cursor < input->input_length && isspace(input->buffer[input->cursor])) {
        input->cursor++;
    }
}

Token next_token(InputBuffer *input) {
    ignore_whitespace(input);

    if (input->cursor >= input->input_length) {
        const Token token = { .type = TOKEN_EOF, .length = 0 };
        return token;
    }

    Token token;
    token.start = &input->buffer[input->cursor];

    if (input->buffer[input->cursor] == '\0') {
        token.type = TOKEN_EOF;
        token.length = 0;
    }

    if (is_symbol(input->buffer[input->cursor])) {
        token.type = TOKEN_SYMBOL;
        token.length = 1;
        input->cursor++;
        return token;
    }

    // words and identifiers
    if (isalpha((input->buffer[input->cursor]))) {
        while (isalnum((input->buffer[input->cursor]))) {
            input->cursor++;
        }
        token.length = (int) (&input->buffer[input->cursor] - token.start);
        token.type = is_keyword(token.start) ? TOKEN_KEYWORD : TOKEN_IDENTIFIER;
        return token;
    }
    token.type = TOKEN_ERROR;
    token.length = 0;
    input->cursor++;
    return token;
}

int expect_keyword(InputBuffer *input_buffer, const char *keyword, ParseError* error) {
    const Token token = next_token(input_buffer);
    if (token.type == TOKEN_KEYWORD && strncasecmp(token.start, keyword, token.length) == 0) return 1;

    error->code = ERROR_SYNTAX_EXPECTED_KEYWORD;
    snprintf(error->message, sizeof(error->message),
        "Syntax error: Expected keyword '%s', but found '%.*s'",
        keyword, token.length, token.start);
    error->cursor_position = input_buffer->cursor;

    return 0;
}

int expect_symbol(InputBuffer *input_buffer, const char symbol, ParseError* error) {
    const Token token = next_token(input_buffer);
    if (token.type == TOKEN_SYMBOL && token.start[0] == symbol) return 1;

    error->code = ERROR_SYNTAX_EXPECTED_SYMBOL;
    snprintf(error->message, sizeof(error->message),
        "Syntax error: Expected symbol '%c', but found '%.*s'",
        symbol, token.length, token.start);
    error->cursor_position = input_buffer->cursor;
    return 0;
}

int parse_create_table(InputBuffer *input_buffer, Statement *statement, ParseError* error) {
    statement->type = STATEMENT_CREATE;
    CreateTableStatementData *args = &statement->data.create_table;
    args->column_count = 0;

    if (!expect_keyword(input_buffer, "table", error)) return 0;

    Token token = next_token(input_buffer);
    if (token.type != TOKEN_IDENTIFIER) {
        error->code = ERROR_SYNTAX_EXPECTED_IDENTIFIER;
        snprintf(error->message, sizeof(error->message),
            "Syntax error: Expected identifier, but found '%.*s'",
            token.length, token.start);
        error->cursor_position = input_buffer->cursor;
        return 0;
    }
    snprintf(args->table_name, token.length + 1, "%s", token.start);

    if (!expect_symbol(input_buffer, '(', error)) return 0;

    while (1) {
        if (args->column_count >= MAX_COLUMNS) {
            error->code = ERROR_SYNTAX_TOO_MANY_COLUMNS;
            error->cursor_position = input_buffer->cursor;
            snprintf(error->message, sizeof(error->message), "Syntax error: Too many columns in create statement");
            return 0;
        }

        token = next_token(input_buffer);

        if (token.type != TOKEN_IDENTIFIER) {
            error->code = ERROR_SYNTAX_EXPECTED_IDENTIFIER;
            error->cursor_position = input_buffer->cursor;
            snprintf(error->message, sizeof(error->message),
                "Syntax error: Expected identifier, but found '%.*s'",
                token.length, token.start);
            return 0;
        }

        ColumnDefinition *column = &args->columns[args->column_count];
        snprintf(column->name, token.length + 1, "%s", token.start);

        token = next_token(input_buffer);
        if (token.type == TOKEN_KEYWORD && strncasecmp(token.start, "INT", token.length) == 0) {
            column->type = TYPE_INT;
        } else if (token.type == TOKEN_KEYWORD && strncasecmp(token.start, "TEXT", token.length) == 0) {
            column->type = TYPE_TEXT;
        } else {
            error->code = ERROR_SYNTAX_EXPECTED_TYPE;
            error->cursor_position = input_buffer->cursor;
            snprintf(error->message, sizeof(error->message),
                "Syntax error: Expected type, but found '%.*s'", token.length, token.start);
            return 0;
        }

        args->column_count++;

        token = next_token(input_buffer);
        if (token.type != TOKEN_SYMBOL) return 0;
        if (token.start[0] == ',') continue;
        if (token.start[0] == ')') break;
    }

    return 1;
}

int parse_select(InputBuffer *input_buffer, Statement *statement, ParseError* error) {
    statement->type = STATEMENT_SELECT;
    SelectStatementData *args = &statement->data.select;
    args->column_count = 0;
    Token token;

    // get columns
    while (1) {
        if (args->column_count >= MAX_COLUMNS) {
            error->code = ERROR_SYNTAX_TOO_MANY_COLUMNS;
            error->cursor_position = input_buffer->cursor;
            snprintf(error->message, sizeof(error->message), "Syntax error: Too many columns in select statement");
            return 0;
        }
        token = next_token(input_buffer);
        if (token.type != TOKEN_IDENTIFIER && token.start[0] != '*') {
            error->code = ERROR_SYNTAX_EXPECTED_IDENTIFIER;
            error->cursor_position = input_buffer->cursor - token.length;
            snprintf(error->message, sizeof(error->message), "Syntax error: Expected identifier or '*' after 'select' keyword");
            return 0;
        }

        snprintf(args->column_names[args->column_count], token.length + 1, "%.*s", token.length, token.start);
        args->column_count++;

        token = next_token(input_buffer);
        if (token.type == TOKEN_SYMBOL && token.start[0] == ',') continue;
        if (token.type == TOKEN_KEYWORD && strncasecmp(token.start, "from", 4) == 0) break;

        error->code = ERROR_SYNTAX_EXPECTED_KEYWORD;
        error->cursor_position = input_buffer->cursor;
        snprintf(error->message, sizeof(error->message), "Syntax error: Expected comma or 'from' keyword after column name");
        return 0;
    }

    token = next_token(input_buffer);
    if (token.type != TOKEN_IDENTIFIER) return 0;

    snprintf(args->table_name, token.length + 1, "%s", token.start);

    token = next_token(input_buffer);
    if (token.type == TOKEN_EOF || (token.type == TOKEN_SYMBOL && token.start[0] == ';')) {
        return  1;
    }

    if (!expect_keyword(input_buffer, "where", error)) return 0;

    // TODO: implement where

    return 0;
}

int parse_sql(InputBuffer *input_buffer, Statement *statement, ParseError* error) {
    const Token token = next_token(input_buffer);
    if (token.type != TOKEN_KEYWORD) {
        statement->type = STATEMENT_INVALID;
        return 0;
    }
    if (strncasecmp(token.start, "create", 5) == 0) {
        return parse_create_table(input_buffer, statement, error);
    }
    if (strncasecmp(token.start, "select", 6) == 0) {
        return parse_select(input_buffer, statement, error);
    }
    return 1;
}

PrepareResult prepare_statement(
    const InputBuffer *input_buffer, Statement* statement) {
    if (strncmp(input_buffer->buffer, "insert", 6) == 0) {
        statement->type = STATEMENT_INSERT;
        return PREPARE_SUCCESS;
    }
    if (strncmp(input_buffer->buffer, "select", 6) == 0) {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }
    return PREPARE_UNRECOGNIZED_STATEMENT;
}