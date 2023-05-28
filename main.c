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

int main(int argc, char** argv) {
    setlocale(LC_CTYPE, "");

    FILE *file;
    int n; int c;

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

    }
    code[file_size - 1] = '\0';

    for (int i = 0; i < file_size; i++) {
        wchar_t codepoint = 0;
        char pot = code[i];
        int bts = codepoint_bytes((int) pot);

        switch (bts) {
            case 1:
                codepoint |= (code[i] & 0x7F);
                break;
            case 2:
                codepoint |= ((code[i] & 0x1F) << 0x6);
                codepoint |= ((code[i + 1] & 0x3F));
                break;
            case 3:
                codepoint |= ((code[i] & 0xF) << 0xc);
                codepoint |= ((code[i + 1] & 0x3F) << 0x6);
                codepoint |= ((code[i + 2] & 0x3F));
                break;
            case 4:
                codepoint |= ((code[i] & 0x7) << 0x12);
                codepoint |= ((code[i + 1] & 0x3F) << 0xc);
                codepoint |= ((code[i + 2] & 0x3F) << 0x6);
                codepoint |= ((code[i + 3] & 0x3F));
                break;
            default:
                printf("ERROR IN DECODING (bts = %d)", bts);
                exit(1);
        }

        i += bts - 1;

        if (pot != '\0') {
            printf("U+%x (bytes = %d): '%lc'\n", codepoint, bts, codepoint);
        }
    }

    fclose(file);
    printf("read %d bytes\n", file_size);
    free(code);

    return 0;
}
