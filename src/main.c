#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "u8read.h"

void print_usage(char *executable) {
    printf("USAGE: %s <file> [options]\n", executable);
    printf("OPTIONS:\n");
    printf("  --print-bits   - Print the bits of the codepoint\n");
}

char *open_file(char *input) {
    FILE *file;

    if ((file = fopen(input, "r")) == NULL) {
        free(file);
        printf("err = %d", errno);
        exit(EXIT_FAILURE);
    }

    // get the file size
    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    rewind(file);

    char *buffer = (char *)malloc(file_size);

    // read the file into the buffer
    int ret = fread(buffer, sizeof(*buffer), file_size, file);
    fclose(file);
    if (ret != file_size) {
        printf("file read error: %d\n", ret);
        exit(EXIT_FAILURE);
    }
    buffer[file_size] = '\0';
    return buffer;
}

int main(int argc, char** argv) {
    setlocale(LC_CTYPE, "");

    if (argc == 1 || argc > 3) {
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    // check if the 3rd argument passed is '--print-bits'
    bool print = argc == 3 && strcmp(argv[2], "--print-bits") == 0;

    // create a buffer
    char *buffer = open_file(argv[1]);
    // print the codepoints of a buffer
    uint8_t size = print_codepoints(buffer, print);
    printf("read %d codepoints\n", size);
    free(buffer);

    return 0;
}
