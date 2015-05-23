#include <stdio.h> // for debugging
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include "types.h"

u32 rotl(int n, u32 x)
{
    assert(n <= 32);
    assert(n >= -32);
    u32 result = (x << n) | (x >> (32 - n));
    return result;
}

u32 rotr(int n, u32 x)
{
    assert(n <= 32);
    assert(n >= -32);
    u32 result = (x >> n) | (x << (32 - n));
    return result;
}

u32 shr(int n, u32 x)
{
    return x >> n;
}

u32 ch(u32 x, u32 y, u32 z)
{
    u32 result = (x & y) ^ ((~x) & z);
    return result;
}

u32 maj(u32 x, u32 y, u32 z)
{
    u32 result = (x & y) ^ (x & z) ^ (y & z);
    return result;
}

u32 bigSigma0(u32 x)
{
    u32 result = rotr(2, x) ^ rotr(13, x) ^ rotr(22, x);
    return result;
}
u32 bigSigma1(u32 x)
{
    u32 result = rotr(6, x) ^ rotr(11, x) ^ rotr(25, x);
    return result;
}
u32 smallSigma0(u32 x)
{
    u32 result = rotr(7, x) ^ rotr(18, x) ^ shr(3, x);
    return result;
}
u32 smallSigma1(u32 x)
{
    u32 result = rotr(17, x) ^ rotr(19, x) ^ shr(10, x);
    return result;
}

const u32 K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, 
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967, 
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, 
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3, 
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

// Padding:
//                              ~~423~~ ~~~~~64~~~~
// 01100001 01100010 01100011 1 00...00 00...011000
//    a         b        c                    l=24
//
//             24           + 1 + 423   +    64 = 512
data* padMessage(data* message)
{
    u64 unpaddedLength = message->length;
    u64 unpaddedLengthMod64 = unpaddedLength % 64;

    u64 paddedLength = unpaddedLength - unpaddedLengthMod64 + 64;
    data* result = reallocateData(message, paddedLength, true);

    result->bytes[unpaddedLength] = 0x80;
    // Write length in _bits_
    writeBigEndian(unpaddedLength * 8, (u8*)&result->bytes + paddedLength);

    return result;
}

const u32 H0[8] =
{
    0x6a09e667, 0xbb67ae85,
    0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c,
    0x1f83d9ab, 0x5be0cd19
};

sha256value sha256(data* message)
{
    sha256value H;
    assert(memcpy(H.word, H0, sizeof(u32)*8));

    data* paddedMessage = allocateData(message->length);
    assert(memcpy(paddedMessage, message, message->length + sizeof(u64)));

    paddedMessage = padMessage(paddedMessage);

    u32 numberOfMessageBlocks = paddedMessage->length/64;
    messageSchedule W;
    u32 a, b, c, d, e, f, g, h;

    for(u32 blockNumber = 0;
        blockNumber < numberOfMessageBlocks;
        ++blockNumber)
    {
        sha256block currentBlock =
            ((sha256block*)paddedMessage->bytes) [blockNumber];

        // Prepare message schedule
        for(u8 t = 0;
            t < 16;
            ++t)
        {
            W.word[t] = readBigEndian((u32*) &currentBlock.word + t);
        }
        for(u8 t = 16;
            t < 64;
            ++t)
        {
            W.word[t] = smallSigma1(W.word[t-2]) + W.word[t-7] +
                        smallSigma0(W.word[t-15]) + W.word[t-16];
        }

        // Initialize working variables
        a = H.word[0];
        b = H.word[1];
        c = H.word[2];
        d = H.word[3];
        e = H.word[4];
        f = H.word[5];
        g = H.word[6];
        h = H.word[7];

        for(u8 t = 0;
            t < 64;
            ++t)
        {
            u32 T1 = h + bigSigma1(e) + ch(e, f, g) + K[t] + W.word[t];
            u32 T2 = bigSigma0(a) + maj(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + T1;
            d = c;
            c = b;
            b = a;
            a = T1 + T2;
        }

        H.word[0] += a;
        H.word[1] += b;
        H.word[2] += c;
        H.word[3] += d;
        H.word[4] += e;
        H.word[5] += f;
        H.word[6] += g;
        H.word[7] += h;
    }

    free(paddedMessage);

    return H;
}

void printSha256ValueAsHex(sha256value* value, FILE* stream = stdout)
{
    for(u64 i = 0;
        i < 8;
        ++i)
    {
        fprintf(stream, "%x", value->word[i]);
    }
}

inline
void printSha256ValueAsHex(data* data, FILE* stream = stdout)
{
    sha256value value = sha256(data);
    printSha256ValueAsHex(&value, stream);
}
