#include "sha256.cpp"

data* hmacPadKey(data* key, u64 blocksize)
{
    assert(key);
    data* result = allocateData(blocksize, true);

    if(key->length > blocksize)
    {
        sha256value hashValue = sha256(key);
        setDataToValue(result, &hashValue);
    }
    else if(key->length < blocksize)
    {
        memcpy(result->bytes, key->bytes, key->length);
    }
    else
    {
        memcpy(result->bytes, key->bytes, blocksize);
    }

    return result;
}

sha256value hmacSha256(data* key, data* message)
{
    int blocksize = 64; // bytes

    data* oKey = allocateData(blocksize);
    data* iKey = allocateData(blocksize);

    u64* oChunk = (u64*)oKey->bytes;
    u64* iChunk = (u64*)iKey->bytes;
    for(u8 i = 0;
        i < blocksize/sizeof(u64);
        ++i)
    {
        *oChunk++ = 0x5c5c5c5c5c5c5c5c;
        *iChunk++ = 0x3636363636363636;
    }

    data* paddedKey = hmacPadKey(key, blocksize);

    data* oKeyPad = xorData(paddedKey, oKey);
    data* iKeyPad = xorData(paddedKey, iKey);

    free(paddedKey);
    free(oKey);
    free(iKey);

    data* innerConcatenation = concatenateData(iKeyPad, message);
    data* innerHashData = allocateDataWithValue(sha256(innerConcatenation));
    data* outerConcatenation =
        concatenateData(oKeyPad, innerHashData);

    free(oKeyPad);
    free(iKeyPad);
    free(innerHashData);

    sha256value result = sha256(outerConcatenation);
    free(innerConcatenation);
    free(outerConcatenation);

    return result;
}
