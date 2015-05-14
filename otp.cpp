#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

struct data
{
    u64 length;
    u8 bytes[1]; // NOTE: this is supposed to be longer than 1 byte!
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

size_t fileSize(const char* filename)
{
    struct stat buffer = {};
    int result = stat(filename, &buffer);

    if(result != -1)
    {
        return buffer.st_size;
    }
    else
    {
        assert(result != -1);
        return -1;
    }
}

data* readFile(const char* filename)
{
    size_t filesize = fileSize(filename);
    data* result = allocateData(filesize, false);

    FILE* handle = fopen(filename, "rb");
    size_t bytes_read = fread(result->bytes, 1, filesize, handle);
    assert(bytes_read == filesize);

    return result;
}

data* readFileEnd(const char* filename, size_t size)
{
    size_t filesize = fileSize(filename);
    printf("Filesize is %d bytes (want at least %d bytes).\n", filesize, size);

    if(filesize >= size)
    {
        data* result = allocateData(size, true);

        FILE* handle = fopen(filename, "rb");
        fseek(handle, -size, SEEK_END);

        size_t bytes_read = fread(result->bytes, 1, size, handle);
        assert(bytes_read == size);

        return result;
    }
    else
    {
        return 0;
    }
}

int main()
{
    data* message = readFile("message.txt");
    data* key = readFileEnd("secret.key", message->length);
    if(key)
    {
        printf("I have both plaintext and a key. Let the encrypting commence!\n");
    }
    else
    {
        printf("The key is too short. Aborting!\n");
    }

    return 0;
}

