#ifndef TYPES_H

#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

struct data
{
    u64 length;
    u8 bytes[1]; // NOTE: this is supposed to be longer than 1 byte!
};

#define TYPES_H
#endif
