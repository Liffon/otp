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

size_t fileSize(FILE* stream)
{
    assert(!isatty(fileno(stream)));

    struct stat buffer = {};
    int result = fstat(fileno(stream), &buffer);

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

#define CHUNKSIZE 1024

data* readUntilEof(FILE* stream)
{
    size_t toRead = 0;
    if(!isatty(fileno(stream)))
    {
        toRead = fileSize(stream);
    }
    else
    {
        toRead = CHUNKSIZE; // try with 1 KB to start with
    }

    data* result = allocateData(toRead);
    size_t bytes_read = 0;
    size_t bytes_read_in_chunk;

    while(!feof(stream))
    {
        bytes_read_in_chunk = fread(&result->bytes + bytes_read, 1, toRead, stream);
        bytes_read += bytes_read_in_chunk;

        if(bytes_read_in_chunk == CHUNKSIZE && !feof(stream)) // needs more size!
        {
            result = (data*)realloc(result, sizeof(u64) + bytes_read + CHUNKSIZE);
        }
        else if(ferror(stream))
        {
            free(result);
            return 0;
        }
    }

    result->length = bytes_read;
    return result;
}

data* readFileEnd(FILE* stream, size_t size)
{
    size_t filesize = fileSize(stream);

    if(filesize >= size)
    {
        data* result = allocateData(size, true);

        fseek(stream, -size, SEEK_END);

        size_t bytes_read = fread(&result->bytes, 1, size, stream);
        assert(bytes_read == size);

        return result;
    }
    else
    {
        return 0;
    }
}

data* readKey(FILE* stream, size_t size)
{
    data* key = readFileEnd(stream, size);

    if(key) // Make sure we get something back - means it needs to be truncated
    {
        int returnValue = ftruncate(fileno(stream), fileSize(stream) - size);
        assert(returnValue != -1);
    }

    return key;
}

data* xorData(data* key, data* message, data* destination = 0)
{
    assert(key->length == message->length);
    if(!destination)
    {
        destination = allocateData(key->length);
    }

    for(u64 i = 0;
        i < key->length;
        ++i)
    {
        destination->bytes[i] = key->bytes[i] ^ message->bytes[i];
    }

    return destination;
}

void describeUsage()
{
    printf("Usage: otp <keyfile> [inputfile [outputfile]]\n\n");
    printf("Data will be removed from the keyfile equal in length\n");
    printf("to the inputfile.\n");
}

int main(int argc, const char** argv)
{
    FILE* keyfile = 0;
    FILE* inputfile = stdin;
    FILE* outputfile = stdout;

    if(argc == 2) // assume only keyfile provided
    {
        keyfile = fopen(argv[1], "r+b");
    }
    else if(argc == 3) // assume keyfile and input provided
    {
        keyfile = fopen(argv[1], "r+b");
        inputfile = fopen(argv[2], "rb");
    }
    else if(argc == 4) // assume keyfile, input and output file provided
    {
        keyfile = fopen(argv[1], "r+b");
        inputfile = fopen(argv[2], "rb");
        outputfile = fopen(argv[3], "wb");
    }
    else
    {
        describeUsage();
        return 0;
    }

    data* message = readUntilEof(inputfile);
    data* key = readKey(keyfile, message->length);
    data* result;

    int returnValue = 0;

    if(key)
    {
        result = xorData(key, message);
        fwrite(&result->bytes, result->length, 1, outputfile);

        free(result);
        free(key);
    }
    else
    {
        fprintf(stderr, "The keyfile is too short. Aborting!\n");
        returnValue = 1;
    }

    free(message);

    fclose(keyfile);
    fclose(inputfile);
    fclose(outputfile);

    return returnValue;
}

