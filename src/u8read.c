#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "u8read.h"

static uint8_t codepoint_bytes(uint8_t input) {
    // first check if we are multi-byte at all, if not, just a single byte
    // code point. this just checks if the MSB is 1.
    if (input >> 7 == 0) return 1;

    // we only really care about the top 4 bits of the input, so
    // shift them down to work with them
    input >>= 4;

    int count = 0;

    // count the number of 1's
    while (input) {
        // increment if there is a 1 in the LSB
        count += input & 1;
        input >>= 1;
    }

    // the number of ones is equal to how many bytes the codepoint has
    return count;
}

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

void print_codepoints(char* buffer) {
    int index = 0;

    while (buffer[index] != '\0') {
        // check if we indexed into the middle of a codepoint
        if ((buffer[index] & 0xc0) == 0x80) {
            // TODO if we do this, we can backtrack until we found a valid
            printf("ERROR: indexed the middle of a codepoint. advancing...\n");
            index += 1;
            continue;
        }

        int bytes = codepoint_bytes(buffer[index]);
        uint32_t codepoint = compute_codepoint(buffer, index, bytes);

        printf("U%x: '%lc' (%d bytes)\n", codepoint, codepoint, bytes);

        index += bytes;
    }
}
