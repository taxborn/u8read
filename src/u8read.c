#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "u8read.h"

// check to see if the input is a valid unicode byte 1 format.
static uint8_t unicode_codepoint_length(uint8_t input) {
    // 1-byte case, 0b0xxx'xxxx
    if ((input & 0x80) == 0x00) return 1;
    // 2-byte case, 0b110x'xxxx
    if ((input & 0xE0) == 0xC0) return 2;
    // 3-byte case, 0b1110'xxxx
    if ((input & 0xF0) == 0xE0) return 3;
    // 4-byte case, 0b1111'0xxx
    if ((input & 0xF8) == 0xF0) return 4;

    // not valid
    return 0;
}

// compute the 32-bit codepoint
static uint32_t unicode_codepoint_of(char *buffer, int index, int length) {
    switch (length) {
        // 0b0xxxxxxx [U+0000 - U+007F]
        case 1:
            return   (buffer[index]     & 0x7F) << 0;
        // 0b110xxxxx 0b10xxxxxx [U+0080 - U+07FF]
        case 2:
            return   (buffer[index]     & 0x1F) << 6
                   | (buffer[index + 1] & 0x3F) << 0;
        // 0b1110xxxx 0b10xxxxxx 0b10xxxxxx [U+0800 - U+FFFF]
        case 3:
            return   (buffer[index]     & 0x0F) << 12
                   | (buffer[index + 1] & 0x3F) << 6
                   | (buffer[index + 2] & 0x3F) << 0;
        // 0b11110xxx 0b10xxxxxx 0b10xxxxxx 0b10xxxxxx [U+10000 - U+10FFFF]
        case 4:
            return   (buffer[index]     & 0x07) << 18
                   | (buffer[index + 1] & 0x3F) << 12
                   | (buffer[index + 2] & 0x3F) << 6
                   | (buffer[index + 3] & 0x3F) << 0;
        // we should not get here, but exit if we do.
        default:
            // if we don't get one of those byte lengths, there's an issue.
            printf("ERROR IN DECODING (bytes = %d). expected at most 4.\n", length);

            exit(EXIT_FAILURE);
    }
}

// print the utf-8 codepoints of a buffer. returns the number of bytes
// read.
uint8_t print_codepoints(char* buffer) {
    uint8_t index = 0;
    uint8_t byte = 0;
    int bytes_read = 0;

    while ((byte = buffer[index]) != '\0') {
        // TODO: Check for BOM (U+FEFF, 0xEFBBBF) and skip if present.
        // check if we indexed into a continuation byte
        if ((byte & 0xc0) == 0x80) {
            // TODO if we do this, we can backtrack until we found a valid codepoint
            // but for now, we keep advancind until we are not in a continuation byte
            printf("ERROR: indexed into a continuation byte (0b10xxxxxx). advancing...\n");
            index += 1;
            continue;
        }

        // TODO: Check for overlong encodings. https://en.wikipedia.org/wiki/UTF-8#Overlong_encodings

        uint8_t bytes = unicode_codepoint_length(byte);
        bytes_read += bytes;
        if (bytes == 0) {
            printf("unable to determine length of codepoint: %.8b", byte);
            exit(EXIT_FAILURE);
        }
        uint32_t codepoint = unicode_codepoint_of(buffer, index, bytes);

        if (bytes <= 3) {
            printf("U-%.4X: '%lc' (%d bytes)\n", codepoint, codepoint, bytes);
        } else {
            printf("U-%.6X: '%lc' (%d bytes)\n", codepoint, codepoint, bytes);
        }
        // printf("  -> bits: 0b%.32b\n", codepoint);

        index += bytes;
    }

    return bytes_read;
}
