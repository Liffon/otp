#ifndef TYPES_H

#include <stdlib.h>
#include <stdint.h>

#define arrayLength(array) (sizeof(array)/sizeof((array)[0]))

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

struct sha256value
{
    u32 word[8];
};
struct sha256block
{
    u32 word[16];
};
struct messageSchedule
{
    u32 word[64];
};

inline
i64 ceilingDivide(i64 numerator, i64 denominator)
{
    return ((numerator - 1)/denominator + 1);
}

u32 readBigEndian(u32* wordAddress)
{
    u8* byteAddress = (u8*)wordAddress;
    u32 result = (byteAddress[0] << 24) | (byteAddress[1] << 16)
               | (byteAddress[2] << 8)  | (byteAddress[3]);

    return result;
}

void writeBigEndian(u64 value, u8* destinationEndPlusOne)
{
    auto nthLastByte = [](u64 value, int n)
    {
        assert(n < (signed int)sizeof(u64));
        assert(n >= 0);
        return (u8)((value >> (8 * n)) & 0xff);
    };

    int minusOffset = 0;
    for(int offset = -1;
        offset > -(int)sizeof(u64) - 1;
        --offset)
    {
        destinationEndPlusOne[offset] = nthLastByte(value, minusOffset);
        ++minusOffset;
    }
}

void writeBigEndian(u32 value, u8* destination)
{
    *destination++ = (u8)(value >> 24);
    *destination++ = (u8)((value >> 16) & 0xff);
    *destination++ = (u8)((value >> 8) & 0xff);
    *destination = (u8)((value) & 0xff);
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

inline
void setDataToValue(data* destination, sha256value* value)
{
    for(u8 wordIndex = 0;
        wordIndex < arrayLength(value->word);
        ++wordIndex)
    {
        writeBigEndian(value->word[wordIndex], &destination->bytes[wordIndex * 4]);
    }
}

data* allocateDataWithValue(const char* string, size_t sizeLimit = 256)
{
    u8* byte = (u8*)string;
    u32 stringLength = 0;
    while(*byte++ && stringLength < sizeLimit)
    {
        ++stringLength;
    }

    data* result = allocateData(stringLength);
    memcpy(result->bytes, string, stringLength);

    return result;
}

// This could maybe have been a simple cast,
// but needs additional logic to account for endianness
data* allocateDataWithValue(sha256value* hashValue)
{
    assert(hashValue);
    data* result = allocateData(sizeof(sha256value));

    setDataToValue(result, hashValue);
    return result;
}

inline
data* allocateDataWithValue(sha256value hashValue)
{
    sha256value tempValue = hashValue;
    return allocateDataWithValue(&tempValue);
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

data* xorData(data* key, data* message, data* destination = 0)
{
    assert(key->length == message->length);
    if(!destination)
    {
        destination = allocateData(key->length);
    }
    
    // Pontus: the following assumes the data is allocated in chunks divisible by chunk_size
    
    u64* destination_chunks = (u64*) &destination->bytes;
    u64* key_chunks = (u64*) &key->bytes;
    u64* message_chunks = (u64*) &message->bytes;
    
    i64 chunk_size = 8; // bytes
    i64 chunk_count = ceilingDivide(key->length, chunk_size);
    
    for(i64 chunk_index = 0;
        chunk_index < chunk_count;
        ++chunk_index)
    {
        *destination_chunks++ = *key_chunks++ ^ *message_chunks++;
    }

    return destination;
}

data* concatenateData(data* leftData, data* rightData, data* destination = 0)
{
    u64 totalLength = leftData->length + rightData->length;
    if(!destination)
    {
        destination = allocateData(totalLength);
    }
    assert(destination->length == totalLength);

    memcpy(destination->bytes, leftData->bytes, leftData->length);
    memcpy(destination->bytes + leftData->length, rightData->bytes,
            rightData->length);

    return destination;
}

#define TYPES_H
#endif
