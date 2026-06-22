#include <stdio.h>
#include <stdlib.h>
#include "repl.h"

InputBuffer* new_input_buffer() {
    InputBuffer* input_buffer = (InputBuffer*) malloc(sizeof(InputBuffer));
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;

    return input_buffer;
}

void print_prompt() {
    printf("db > ");
}

ssize_t portable_get_line(char **line_pointer, size_t *read_size, FILE *stream) {
    if (*line_pointer == NULL || *read_size == 0) {
        *read_size = 128;
        *line_pointer = malloc(*read_size);
        if (*line_pointer == NULL) return -1;
    }

    ssize_t pos = 0;
    int c;

    while ((c = fgetc(stream)) != EOF) {
        if (pos + 1 >= *read_size) {
            *read_size *= 2;
            char *new_ptr = realloc(*line_pointer, *read_size);
            if (!new_ptr) return -1;

            *line_pointer = new_ptr;
        }
        (*line_pointer)[pos++] = (char) c;
        if (c == '\n') break;
    }
    if (pos == 0 && c == EOF) return -1;

    (*line_pointer)[pos] = '\0';
    return pos;
}

void read_input(InputBuffer* input_buffer) {
    ssize_t bytes_read = portable_get_line(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);
    if (bytes_read <= 0) {
        printf("Error reading input\n");
        exit(EXIT_FAILURE);
    }

    input_buffer->input_length = bytes_read - 1;
    input_buffer->buffer[bytes_read - 1] = 0;
}

void close_input_buffer(InputBuffer* input_buffer) {
    free(input_buffer->buffer);
    free(input_buffer);
}
