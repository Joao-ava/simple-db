#ifndef SIMPLE_DB_PREPROCESSOR_H
#define SIMPLE_DB_PREPROCESSOR_H

#define MAX_NAME_LEN 64
#define MAX_COLUMNS 16

typedef struct {
    char* buffer;
    size_t buffer_length;
    ssize_t input_length;
    int cursor;
} InputBuffer;

typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT,
    STATEMENT_CREATE,
    STATEMENT_INVALID,
} StatementType;

typedef enum {
    TYPE_INT,
    TYPE_TEXT
} DataType;

typedef struct {
    char name[MAX_NAME_LEN];
    DataType type;
} ColumnDefinition;

typedef struct {
    char table_name[MAX_NAME_LEN];
    ColumnDefinition columns[MAX_COLUMNS];
    int column_count;
} CreateTableStatementData;

typedef enum {
    OPERATOR_EQUAL,
    OPERATOR_NOT_EQUAL,
    OPERATOR_LESS_THAN,
    OPERATOR_LESS_THAN_OR_EQUAL,
    OPERATOR_GREATER_THAN,
    OPERATOR_GREATER_THAN_OR_EQUAL,
} WhereOperator;

typedef struct {
    char column_name[MAX_NAME_LEN];
    WhereOperator operator;
    char value[MAX_NAME_LEN];
} WhereCondition;

typedef struct {
    char table_name[MAX_NAME_LEN];
    char column_names[MAX_COLUMNS][MAX_NAME_LEN];
    int column_count;
    WhereCondition where;
} SelectStatementData;

typedef struct {
    StatementType type;
    union {
        CreateTableStatementData create_table;
        SelectStatementData select;
    } data;
} Statement;

typedef enum {
    PREPARE_SUCCESS,
    PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult;

typedef enum {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

PrepareResult prepare_statement(const InputBuffer* input_buffer, Statement* statement);

typedef enum {
    TOKEN_KEYWORD,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_SYMBOL,
    TOKEN_EOF,
    TOKEN_ERROR
} TokenType;

typedef struct {
    TokenType type;
    int length;
    const char *start;
} Token;

typedef enum {
    ERROR_NONE,
    ERROR_SYNTAX_EXPECTED_KEYWORD,
    ERROR_SYNTAX_EXPECTED_SYMBOL,
    ERROR_SYNTAX_EXPECTED_IDENTIFIER,
    ERROR_SYNTAX_EXPECTED_TYPE,
    ERROR_SYNTAX_TOO_MANY_COLUMNS,
    ERROR_UNKNOWN_COMMAND,
} ErrorCode;

typedef struct {
    ErrorCode code;
    char message[256];
    int cursor_position;
} ParseError;

Token next_token(InputBuffer *input_buffer);
int parse_sql(InputBuffer *input_buffer, Statement *statement, ParseError* error);

#endif //SIMPLE_DB_PREPROCESSOR_H