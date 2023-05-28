#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "u8read.h"

void print_usage(char *executable) {
    printf("USAGE: %s <file>\n", executable);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    setlocale(LC_CTYPE, "");

    FILE *file;

    if ((file = fopen(argv[1], "r")) == NULL) {
        free(file);
        printf("err = %d", errno);
        return EXIT_FAILURE;
    }

    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    rewind(file);

    char *code = (char *)malloc(file_size);

    int ret = fread(code, sizeof(*code), file_size, file);
    if (ret != file_size) {
        printf("file read error: %d\n", ret); 
    }
    code[file_size - 1] = '\0';

    // print the codepoints of a buffer
    print_codepoints(code, file_size);

    fclose(file);
    printf("read %d bytes\n", file_size);
    free(code);

    return 0;
}
