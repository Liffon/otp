#include "hmac.cpp"

int main()
{
    data* key = allocateDataWithValue("key");
    data* message = allocateDataWithValue("The quick brown fox jumps over the lazy dog");
    
    sha256value hashValue = hmacSha256(key, message);
    printSha256ValueAsHex(&hashValue);

    free(key);
    free(message);

    return 0;
}
