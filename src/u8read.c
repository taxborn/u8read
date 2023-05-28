#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "u8read.h"

// check to see if the input is a valid unicode byte 1 format.
static bool is_valid_unicode_start(uint8_t input) {
    // 1-byte case, 0b0xxx'xxxx
    if ((input & 0x80) == 0x00) return true;
    // 2-byte case, 0b110x'xxxx
    if ((input & 0xE0) == 0xC0) return true;
    // 3-byte case, 0b1110'xxxx
    if ((input & 0xF0) == 0xE0) return true;
    // 4-byte case, 0b1111'0xxx
    if ((input & 0xF8) == 0xF0) return true;

    return false;
}

// count the number of bytes the unicode codepoint should be
static uint8_t codepoint_bytes(uint8_t input) {
    // we have already checked if we have indexed into a middle-byte so we shouldn't 
    // hit this, but might be good to check.
    assert(is_valid_unicode_start(input) && "Can only calculate codepoint bytes from a valid UTF-8 byte 1 format.");

    // first check if we are multi-byte at all, if not, just a single byte
    // code point. this just checks if the MSB is 1.
    if (input >> 7 == 0) return 1;

    // we only really care about the top 4 bits of the input, so
    // shift them down to work with them
    input >>= 4;

    int count = 0;

    // loop to count the number of set bits in an input
    for (; input; input >>= 1) count += input & 1;

    // the number of ones is equal to how many bytes the codepoint has
    return count;
}

// compute the 32-bit codepoint
static uint32_t compute_codepoint(char *buffer, int index, int length) {
    switch (length) {
        case 1:
            return   (buffer[index]     & 0x7F) << 0;
        case 2:
            return   (buffer[index]     & 0x1F) << 6
                   | (buffer[index + 1] & 0x3F);
        case 3:
            return   (buffer[index]     & 0x0F) << 12
                   | (buffer[index + 1] & 0x3F) << 6
                   | (buffer[index + 2] & 0x3F);
        case 4:
            return   (buffer[index]     & 0x07) << 18
                   | (buffer[index + 1] & 0x3F) << 12
                   | (buffer[index + 2] & 0x3F) << 6
                   | (buffer[index + 3] & 0x3F);
        default:
            // if we don't get one of those byte lengths, there's an issue.
            printf("ERROR IN DECODING (bytes = %d). expected at most 4.\n", length);

            exit(EXIT_FAILURE);
    }
}

// print the utf-8 codepoints of a buffer
void print_codepoints(char* buffer) {
    int index = 0;
    char byte = 0;

    while ((byte = buffer[index]) != '\0') {
        // check if we indexed into the middle of a codepoint
        if ((byte & 0xc0) == 0x80) {
            printf("ERROR: indexed the middle of a codepoint. advancing...\n");

            // TODO if we do this, we can backtrack until we found a valid codepoint
            // int retries = 0;
            // while (retries < 4) {
            //     retries++;
            //     if (is_valid_unicode_start(buffer[index - retries])) {
            //         index -= retries;
            //         continue;
            //     }
            // }

            index += 1;
            continue;
        }

        int bytes = codepoint_bytes(byte);
        uint32_t codepoint = compute_codepoint(buffer, index, bytes);

        if (bytes <= 3) {
            printf("U-%.4X: '%lc' (%d bytes)\n", codepoint, codepoint, bytes);
        } else {
            printf("U-%.6X: '%lc' (%d bytes)\n", codepoint, codepoint, bytes);
        }
        // printf("  -> bits: 0b%.32b\n", codepoint);

        index += bytes;
    }
}
