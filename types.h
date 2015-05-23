#ifndef TYPES_H

#include <stdlib.h>
#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

struct data
{
    u64 length;
    u8 bytes[1]; // NOTE: this is supposed to be longer than 1 byte!
};

inline
i64 ceilingDivide(i64 numerator, i64 denominator)
{
    return ((numerator - 1)/denominator + 1);
}

data* allocateData(u64 length, bool clear = false)
{
    u64 longAlignedLength = ceilingDivide(length, 8) * 8;
    data* result;
    if(clear)
    {
        result = (data*)calloc(1, sizeof(u64) + longAlignedLength);
    }
    else
    {
        result = (data*)malloc(sizeof(u64) + longAlignedLength);
    }
    result->length = length;
    return result;
}

data* reallocateData(data* oldData, u64 newLength, bool clear = false)
{
    u64 chunkCount = ceilingDivide(newLength, 8);
    u64 newLongAlignedLength = chunkCount * 8;
    u64 oldLength = oldData->length;
    data* result = (data*)realloc(oldData, sizeof(u64) + newLongAlignedLength);
    result->length = newLength;

    if(clear && (newLength > oldLength))
    {
        u64* byteChunks = (u64*) &result->bytes;
        for(u64 chunkIndex = ceilingDivide(oldLength, 8);
            chunkIndex < chunkCount;
            ++chunkIndex)
        {
            *(byteChunks + chunkIndex) = 0;
        }
    }

    return result;
}

#define TYPES_H
#endif
