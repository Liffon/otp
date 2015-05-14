#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <iostream>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

struct data
{
    u64 length;
    u8 bytes[];
};

data* allocateData(size_t size, bool clear = false)
{
    data* result = (data*)malloc(sizeof(u64) + size);
    result->length = size;

    u8* bytes = (u8*)&result->bytes;

    if(clear)
    {
        for(u64 i = 0;
            i < size;
            ++i)
        {
            bytes[i] = 0;
        }
    }

    return result;
}

data* readKey()
{
    data* result = allocateData(8, true);

    ((u64*)result->bytes)[0] = 0x1234567890abcdef;
    
    return result;
}

int main()
{
    data* key = readKey();
    data* message = allocateData(8, true);

    return 0;
}

