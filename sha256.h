/*********************************************************************
* Filename:   sha256.h
* Author:     Anunay Chandra (Cypherock)
* Copyright:
* Disclaimer: This code is presented "as is" without any guarantees.
* Details:    Defines the API for the corresponding SHA256 implementation.
*********************************************************************/

#ifndef SHA256_H
#define SHA256_H

/*************************** HEADER FILES ***************************/
#include <stdint.h>

/****************************** DATA TYPES *******************************/
typedef unsigned char uchar;  // SHA256 outputs a 32 byte digest
typedef unsigned int uint; // 32-bit word, change to "long" for 16-bit machines

/****************************** MACROS ******************************/
#define SHA256_BLOCK_SIZE 32

typedef struct{
    uchar data[64];
    uint datalen;
    uint bitlen[2];
    uint state[8];
} SHA256_CTX;

/*********************** HELPER FUNCTIONS **********************/
uint8_t len(uint16_t a[]);
// {
//     return *(&a + 1) - a;
// }

uint8_t len(uint8_t a[]);
// {
//     return *(&a + 1) - a;
// }

uint8_t count_bits(uint16_t x);
/*{
    uint8_t count = 0;
    while (x > 0)
    {
        x = x >> 1;
        count++;
    }

    return count;
}*/

/*********************** FUNCTION DECLARATIONS **********************/
void sha256_init(SHA256_CTX *ctx);
void sha256_update(SHA256_CTX *ctx, uchar data[], uint len);
void sha256_final(SHA256_CTX *ctx, uchar hash[]);

#endif // SHA256_H
