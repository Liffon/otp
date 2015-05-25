#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "types.h"
#include "sha256.cpp"

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
            result = reallocateData(result, sizeof(u64) + bytes_read + CHUNKSIZE);
        }
        else if(ferror(stream))
        {
            free(result);
            return 0;
        }
    }

    // We need this line, otherwise the message will contain gibberish
    // (or null bytes) at the end unless read from a file
    // since result->length is set to multiples of CHUNKSIZE in reallocateData
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

void describeUsage()
{
    printf("usage: otp [-t] <keyfile> [<inputfile> [<outputfile>]\n\n");
    printf("Data will be removed from the keyfile equal in length\n");
    printf("to the inputfile if the option -t is provided.\n");
}

int main(int argc, char** argv)
{
    bool truncate_key = 0;
    char c;
    
    while ((c = getopt(argc, argv, "+t")) != -1)
    {
        switch (c)
        {
            case 't':
                truncate_key = 1;
                break;
            default:
                describeUsage();
                return 0;
        }
    }
    
    u64 non_option_arg_count = argc - optind;
    
    if(non_option_arg_count < 1)
    {
        describeUsage();
        return 0;
    }
    
    FILE* keyfile = 0;
    FILE* inputfile = stdin;
    FILE* outputfile = stdout;

    if(non_option_arg_count >= 1)
    {
        keyfile = fopen(argv[optind], "r+b");
    }
    if(non_option_arg_count >= 2)
    {
        inputfile = fopen(argv[optind + 1], "rb");
    }
    if(non_option_arg_count >= 3)
    {
        outputfile = fopen(argv[optind + 2], "wb");
    }

    data* message = readUntilEof(inputfile);
    data* key = readKey(keyfile, message->length, truncate_key);
    data* result;

    int returnValue = 0;

    fprintf(stderr, "sha256: ");
    printSha256ValueAsHex(message, stderr);
    fprintf(stderr, "\n");

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

