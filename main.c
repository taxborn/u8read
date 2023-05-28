#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>

int codepoint_bytes(uint8_t c) {
    // first check if we are multi-byte at all, if not, just a single byte
    // code point
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

wchar_t compute_codepoint(char *buffer, int index, int bytes) {
    wchar_t codepoint = 0;

    switch (bytes) {
        case 1:
            codepoint |= ((buffer[index]     & 0x7F) << 0x00);
            break;
        case 2:
            codepoint |= ((buffer[index]     & 0x1F) << 0x06)
                      |  ((buffer[index + 1] & 0x3F) << 0x00);
            break;
        case 3:
            codepoint |= ((buffer[index]     & 0x0F) << 0x0c)
                      |  ((buffer[index + 1] & 0x3F) << 0x06)
                      |  ((buffer[index + 2] & 0x3F) << 0x00);
            break;
        case 4:
            codepoint |= ((buffer[index]     & 0x07) << 0x12)
                      |  ((buffer[index + 1] & 0x3F) << 0x0c)
                      |  ((buffer[index + 2] & 0x3F) << 0x06)
                      |  ((buffer[index + 3] & 0x3F) << 0x00);
            break;
        default:
            printf("ERROR IN DECODING (bytes = %d)", bytes);
            exit(1);
    }

    return codepoint;
}

int main(int argc, char** argv) {
    setlocale(LC_CTYPE, "");

    FILE *file;
    int c;

    if ((file = fopen("read.txt", "r")) == NULL) {
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
        char chr = code[idx];
        if (chr == '\0') break;
        int bytes = codepoint_bytes((int) chr);

        wchar_t codepoint = compute_codepoint(code, idx, bytes);


        printf("U+%x (bytes = %d): '%lc'\n", codepoint, bytes, codepoint);
        idx += bytes;
    }

    fclose(file);
    printf("read %d bytes\n", file_size);
    free(code);

    return 0;
}
