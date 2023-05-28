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

    int idx = 0;

    while (idx < file_size) {
        if (code[idx] == '\0') break;
        if ((code[idx] & 0b11000000) == 0b10000000) {
            // TODO: Check if we accidentally index midway into a codepoint. If we 
            // do, we can work backwards until we find a valid start.
            printf("ERROR: indexed the middle of a codepoint. advancing...\n");
            idx += 1;
            continue;
        }
        int bytes = codepoint_bytes(code[idx]);
        uint32_t codepoint = compute_codepoint(code, idx, bytes);
        printf("U+%x (bytes = %d): '%lc'\n", codepoint, bytes, codepoint);
        idx += bytes;
    }

    fclose(file);
    printf("read %d bytes\n", file_size);
    free(code);

    return 0;
}
