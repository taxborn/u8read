#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>

int codepoint_bytes(uint8_t c) {
    // first check if we are multi-byte at all, if not, just a single byte
    // code point. this just checks if the MSB is 1.
    if ((c & 0x80) == 0) {
        return 1;
    }

    // select the upper 4 bits of c
    c &= 0xF0;

    int count = 0;

    // count the number of 1's
    while (c) {
        // increment if there is a 1 in the LSB
        count += c & 1;
        c >>= 1;
    }

    // the number of ones is equal to how many bytes the codepoint has
    return count;
}

wchar_t compute_codepoint(char *buffer, int index, int length) {
    if (length == 1) {
        return       ((buffer[index]     & 0x7F) << 0);
    }
    if (length == 2) {
            return   ((buffer[index]     & 0x1F) << 6)
                   | ((buffer[index + 1] & 0x3F));
    }
    if (length == 3) {
            return   ((buffer[index]     & 0x0F) << 12)
                   | ((buffer[index + 1] & 0x3F) << 6)
                   | ((buffer[index + 2] & 0x3F));
    }
    if (length == 4) {
            return   ((buffer[index]     & 0x07) << 18)
                   | ((buffer[index + 1] & 0x3F) << 12)
                   | ((buffer[index + 2] & 0x3F) << 6)
                   | ((buffer[index + 3] & 0x3F));
    }

    // if we don't get one of those byte lengths, there's an issue.
    printf("ERROR IN DECODING (bytes = %d)", length);

    exit(EXIT_FAILURE);
}

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
        wchar_t codepoint = compute_codepoint(code, idx, bytes);
        printf("U+%x (bytes = %d): '%lc'\n", codepoint, bytes, codepoint);
        idx += bytes;
    }

    fclose(file);
    printf("read %d bytes\n", file_size);
    free(code);

    return 0;
}
