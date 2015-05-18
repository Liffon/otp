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

data* readKey(FILE* stream, size_t size, bool truncate_key = true)
{
    data* key = readFileEnd(stream, size);

    if(key && truncate_key)
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

int main(int argc, char** argv)
{
    u64 truncate_key = 0;
    char c;
    
    while ((c = getopt(argc, argv, "+t")) != -1)
    {
        switch (c)
        {
            case 't':
                truncate_key = 1;
                break;
            default:
                assert(false);
                break;
        }
    }
    
    u64 non_option_arg_count = argc - optind + 1;
    
    printf("%llu\n", non_option_arg_count);
    
    if(non_option_arg_count < 2)
    {
        describeUsage();
        return 0;
    }
    
    FILE* keyfile = 0;
    FILE* inputfile = stdin;
    FILE* outputfile = stdout;

    if(non_option_arg_count >= 2)
    {
        keyfile = fopen(argv[optind], "r+b");
    }
    if(non_option_arg_count >= 3)
    {
        inputfile = fopen(argv[optind + 1], "rb");
    }
    if(non_option_arg_count >= 4)
    {
        outputfile = fopen(argv[optind + 2], "wb");
    }

    data* message = readUntilEof(inputfile);
    data* key = readKey(keyfile, message->length, truncate_key);
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

