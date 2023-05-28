#ifndef U8READ_H_
#define U8READ_H_

#include <stdint.h>

uint8_t codepoint_bytes(uint8_t input);
wchar_t compute_codepoint(char *buffer, int index, int length);

#endif // U8READ_H_
