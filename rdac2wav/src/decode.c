#include <stdio.h>

/*
    RDAC to WAV file converter for Roland VS. Version 0.99

    Copyright 2006 LGPL by Randy Gordon (randy@integrand.com)

    Special thanks to Danielo Aue (aka Danielo) for solving
    many of the MTP patterns and other suggested enhancements.

    Changes:
        0.99 - Fixed CDR mode.
        0.98 - Added support for CDR.
        0.97 - Sign extend for M16 amd M24.
        0.96 - Fixed bug with MT2 pattern 11111010.
        0.95 - Refactored for some indexing speed improvement.
               Added support for MT1, MT2, M16 and M24.
               Corrected interpolation calculation.
               Optimized overflow checks.
        0.94 - Prevent 24-bit overflow.
        0.93 - Incorporate Danielo's decrement operation.
        0.92 - Daniel Aue suggests decrementing linear samples
               to match results of VS-2480 WAV conversion.
        0.91 - Added support for sample rate and bit depth.
        0.90 - Initial release.
*/

// Lookup table for relevant pattern

int patterns[] = {
    0, /* 00..00.. */
    0, /* 00..00.. */
    0, /* 00..00.. */
    0, /* 00..00.. */
    1, /* 00..01.. */
    1, /* 00..01.. */
    1, /* 00..01.. */
    1, /* 00..01.. */
    2, /* 00..10.. */
    2, /* 00..10.. */
    2, /* 00..10.. */
    2, /* 00..10.. */
    3, /* 00..11.. */
    3, /* 00..11.. */
    3, /* 00..11.. */
    3, /* 00..11.. */
    0, /* 00..00.. */
    0, /* 00..00.. */
    0, /* 00..00.. */
    0, /* 00..00.. */
    1, /* 00..01.. */
    1, /* 00..01.. */
    1, /* 00..01.. */
    1, /* 00..01.. */
    2, /* 00..10.. */
    2, /* 00..10.. */
    2, /* 00..10.. */
    2, /* 00..10.. */
    3, /* 00..11.. */
    3, /* 00..11.. */
    3, /* 00..11.. */
    3, /* 00..11.. */
    0, /* 00..00.. */
    0, /* 00..00.. */
    0, /* 00..00.. */
    0, /* 00..00.. */
    1, /* 00..01.. */
    1, /* 00..01.. */
    1, /* 00..01.. */
    1, /* 00..01.. */
    2, /* 00..10.. */
    2, /* 00..10.. */
    2, /* 00..10.. */
    2, /* 00..10.. */
    3, /* 00..11.. */
    3, /* 00..11.. */
    3, /* 00..11.. */
    3, /* 00..11.. */
    0, /* 00..00.. */
    0, /* 00..00.. */
    0, /* 00..00.. */
    0, /* 00..00.. */
    1, /* 00..01.. */
    1, /* 00..01.. */
    1, /* 00..01.. */
    1, /* 00..01.. */
    2, /* 00..10.. */
    2, /* 00..10.. */
    2, /* 00..10.. */
    2, /* 00..10.. */
    3, /* 00..11.. */
    3, /* 00..11.. */
    3, /* 00..11.. */
    3, /* 00..11.. */
    4, /* 01..00.. */
    4, /* 01..00.. */
    4, /* 01..00.. */
    4, /* 01..00.. */
    5, /* 01..01.. */
    5, /* 01..01.. */
    5, /* 01..01.. */
    5, /* 01..01.. */
    6, /* 01..10.. */
    6, /* 01..10.. */
    6, /* 01..10.. */
    6, /* 01..10.. */
    7, /* 01..11.. */
    7, /* 01..11.. */
    7, /* 01..11.. */
    7, /* 01..11.. */
    4, /* 01..00.. */
    4, /* 01..00.. */
    4, /* 01..00.. */
    4, /* 01..00.. */
    5, /* 01..01.. */
    5, /* 01..01.. */
    5, /* 01..01.. */
    5, /* 01..01.. */
    6, /* 01..10.. */
    6, /* 01..10.. */
    6, /* 01..10.. */
    6, /* 01..10.. */
    7, /* 01..11.. */
    7, /* 01..11.. */
    7, /* 01..11.. */
    7, /* 01..11.. */
    4, /* 01..00.. */
    4, /* 01..00.. */
    4, /* 01..00.. */
    4, /* 01..00.. */
    5, /* 01..01.. */
    5, /* 01..01.. */
    5, /* 01..01.. */
    5, /* 01..01.. */
    6, /* 01..10.. */
    6, /* 01..10.. */
    6, /* 01..10.. */
    6, /* 01..10.. */
    7, /* 01..11.. */
    7, /* 01..11.. */
    7, /* 01..11.. */
    7, /* 01..11.. */
    4, /* 01..00.. */
    4, /* 01..00.. */
    4, /* 01..00.. */
    4, /* 01..00.. */
    5, /* 01..01.. */
    5, /* 01..01.. */
    5, /* 01..01.. */
    5, /* 01..01.. */
    6, /* 01..10.. */
    6, /* 01..10.. */
    6, /* 01..10.. */
    6, /* 01..10.. */
    7, /* 01..11.. */
    7, /* 01..11.. */
    7, /* 01..11.. */
    7, /* 01..11.. */
    8, /* 10..00.. */
    8, /* 10..00.. */
    8, /* 10..00.. */
    8, /* 10..00.. */
    9, /* 10..01.. */
    9, /* 10..01.. */
    9, /* 10..01.. */
    9, /* 10..01.. */
    10, /* 10..10.. */
    10, /* 10..10.. */
    10, /* 10..10.. */
    10, /* 10..10.. */
    11, /* 10..11.. */
    11, /* 10..11.. */
    11, /* 10..11.. */
    11, /* 10..11.. */
    8, /* 10..00.. */
    8, /* 10..00.. */
    8, /* 10..00.. */
    8, /* 10..00.. */
    9, /* 10..01.. */
    9, /* 10..01.. */
    9, /* 10..01.. */
    9, /* 10..01.. */
    10, /* 10..10.. */
    10, /* 10..10.. */
    10, /* 10..10.. */
    10, /* 10..10.. */
    11, /* 10..11.. */
    11, /* 10..11.. */
    11, /* 10..11.. */
    11, /* 10..11.. */
    8, /* 10..00.. */
    8, /* 10..00.. */
    8, /* 10..00.. */
    8, /* 10..00.. */
    9, /* 10..01.. */
    9, /* 10..01.. */
    9, /* 10..01.. */
    9, /* 10..01.. */
    10, /* 10..10.. */
    10, /* 10..10.. */
    10, /* 10..10.. */
    10, /* 10..10.. */
    11, /* 10..11.. */
    11, /* 10..11.. */
    11, /* 10..11.. */
    11, /* 10..11.. */
    8, /* 10..00.. */
    8, /* 10..00.. */
    8, /* 10..00.. */
    8, /* 10..00.. */
    9, /* 10..01.. */
    9, /* 10..01.. */
    9, /* 10..01.. */
    9, /* 10..01.. */
    10, /* 10..10.. */
    10, /* 10..10.. */
    10, /* 10..10.. */
    10, /* 10..10.. */
    11, /* 10..11.. */
    11, /* 10..11.. */
    11, /* 10..11.. */
    11, /* 10..11.. */
    12, /* 110.000. */
    12, /* 110.000. */
    13, /* 110.001. */
    13, /* 110.001. */
    14, /* 110.010. */
    14, /* 110.010. */
    15, /* 110.011. */
    15, /* 110.011. */
    16, /* 110.100. */
    16, /* 110.100. */
    17, /* 110.101. */
    17, /* 110.101. */
    18, /* 110.110. */
    18, /* 110.110. */
    19, /* 110.111. */
    19, /* 110.111. */
    12, /* 110.000. */
    12, /* 110.000. */
    13, /* 110.001. */
    13, /* 110.001. */
    14, /* 110.010. */
    14, /* 110.010. */
    15, /* 110.011. */
    15, /* 110.011. */
    16, /* 110.100. */
    16, /* 110.100. */
    17, /* 110.101. */
    17, /* 110.101. */
    18, /* 110.110. */
    18, /* 110.110. */
    19, /* 110.111. */
    19, /* 110.111. */
    20, /* 111.000. */
    20, /* 111.000. */
    21, /* 111.001. */
    21, /* 111.001. */
    22, /* 111.010. */
    22, /* 111.010. */
    23, /* 111.011. */
    23, /* 111.011. */
    24, /* 111.100. */
    24, /* 111.100. */
    25, /* 11101010 */
    26, /* 11101011 */
    27, /* 11101100 */
    28, /* 11101101 */
    29, /* 11101110 */
    30, /* 11101111 */
    20, /* 111.000. */
    20, /* 111.000. */
    21, /* 111.001. */
    21, /* 111.001. */
    22, /* 111.010. */
    22, /* 111.010. */
    23, /* 111.011. */
    23, /* 111.011. */
    24, /* 111.100. */
    24, /* 111.100. */
    31, /* 11111010 */
    32, /* 11111011 */
    33, /* 11111100 */
    34, /* 11111101 */
    35, /* 11111110 */
    36, /* 11111111 */
};

// Macros for fast operations

#define SIGN_EXTEND_4(xx)  xx = -(xx & 0x00000008) | xx
#define SIGN_EXTEND_5(xx)  xx = -(xx & 0x00000010) | xx
#define SIGN_EXTEND_6(xx)  xx = -(xx & 0x00000020) | xx
#define SIGN_EXTEND_7(xx)  xx = -(xx & 0x00000040) | xx
#define SIGN_EXTEND_8(xx)  xx = -(xx & 0x00000080) | xx
#define SIGN_EXTEND_9(xx)  xx = -(xx & 0x00000100) | xx
#define SIGN_EXTEND_10(xx) xx = -(xx & 0x00000200) | xx
#define SIGN_EXTEND_11(xx) xx = -(xx & 0x00000400) | xx
#define SIGN_EXTEND_12(xx) xx = -(xx & 0x00000800) | xx
#define SIGN_EXTEND_13(xx) xx = -(xx & 0x00001000) | xx
#define SIGN_EXTEND_14(xx) xx = -(xx & 0x00002000) | xx
#define SIGN_EXTEND_15(xx) xx = -(xx & 0x00004000) | xx
#define SIGN_EXTEND_16(xx) xx = -(xx & 0x00008000) | xx
#define SIGN_EXTEND_24(xx) xx = -(xx & 0x00800000) | xx

#define SHIFT_ROUND_0(xx)
#define SHIFT_ROUND_1(xx)  xx = (xx<<1)  | 0x00000001
#define SHIFT_ROUND_2(xx)  xx = (xx<<2)  | 0x00000002
#define SHIFT_ROUND_3(xx)  xx = (xx<<3)  | 0x00000004
#define SHIFT_ROUND_4(xx)  xx = (xx<<4)  | 0x00000008
#define SHIFT_ROUND_5(xx)  xx = (xx<<5)  | 0x00000010
#define SHIFT_ROUND_6(xx)  xx = (xx<<6)  | 0x00000020
#define SHIFT_ROUND_7(xx)  xx = (xx<<7)  | 0x00000040
#define SHIFT_ROUND_8(xx)  xx = (xx<<8)  | 0x00000080
#define SHIFT_ROUND_9(xx)  xx = (xx<<9)  | 0x00000100
#define SHIFT_ROUND_10(xx) xx = (xx<<10) | 0x00000200
#define SHIFT_ROUND_11(xx) xx = (xx<<11) | 0x00000400
#define SHIFT_ROUND_12(xx) xx = (xx<<12) | 0x00000800
#define SHIFT_ROUND_13(xx) xx = (xx<<13) | 0x00001000
#define SHIFT_ROUND_14(xx) xx = (xx<<14) | 0x00002000
#define SHIFT_ROUND_15(xx) xx = (xx<<15) | 0x00004000
#define SHIFT_ROUND_16(xx) xx = (xx<<16) | 0x00008000
#define SHIFT_ROUND_17(xx) xx = (xx<<17) | 0x00010000

// Danielo suggests that the following operations may be performed.
// I think with the new interpolation in the version, that these are not necessary.
// My tests with MT1 and MT2 now give identical results with the BR8 converter.
//#define DO_TWEAK
//#define DANIELO_DOUBLE

#ifdef DO_TWEAK
    #define TWEAK_2(out) \
                out[7]--; out[15]--;
    #define TWEAK_4(out) \
                out[3]--; out[7]--; out[11]--; out[15]--;
    #define TWEAK_8(out) \
                out[1]--; out[3]--;  out[5]--;  out[7]--; out[9]--; out[11]--; out[13]--; out[15]--;
    #define TWEAK_16(out) \
                out[0]--;  out[1]--;  out[2]--;  out[3]--;  out[4]--;  out[5]--;  out[6]--;  out[7]--; \
                out[8]--;  out[9]--;  out[10]--; out[11]--; out[12]--; out[13]--; out[14]--; out[15]--;
#else
    #define TWEAK_2(out)
    #define TWEAK_4(out)
    #define TWEAK_8(out)
    #define TWEAK_16(out)
#endif

#define INTERPOLATE(xx, aa, bb) (aa + bb) < 0 ? (xx = xx + (aa + bb - 1)/2) : (xx = xx + (aa + bb)/2);

#define INTERPOLATE_2(d0, out) \
            INTERPOLATE(out[3],  d0,      out[7]); \
            INTERPOLATE(out[1],  d0,      out[3]); \
            INTERPOLATE(out[5],  out[3],  out[7]); \
            INTERPOLATE(out[11], out[7],  out[15]); \
            INTERPOLATE(out[9],  out[7],  out[11]); \
            INTERPOLATE(out[13], out[11], out[15]); \
            INTERPOLATE(out[0],  d0,      out[1]); \
            INTERPOLATE(out[2],  out[1],  out[3]); \
            INTERPOLATE(out[4],  out[3],  out[5]); \
            INTERPOLATE(out[6],  out[5],  out[7]); \
            INTERPOLATE(out[8],  out[7],  out[9]); \
            INTERPOLATE(out[10], out[9],  out[11]); \
            INTERPOLATE(out[12], out[11], out[13]); \
            INTERPOLATE(out[14], out[13], out[15]);

#define INTERPOLATE_4(d0, out) \
            INTERPOLATE(out[1],  d0,      out[3]); \
            INTERPOLATE(out[5],  out[3],  out[7]); \
            INTERPOLATE(out[9],  out[7],  out[11]); \
            INTERPOLATE(out[13], out[11], out[15]); \
            INTERPOLATE(out[0],  d0,      out[1]); \
            INTERPOLATE(out[2],  out[1],  out[3]); \
            INTERPOLATE(out[4],  out[3],  out[5]); \
            INTERPOLATE(out[6],  out[5],  out[7]); \
            INTERPOLATE(out[8],  out[7],  out[9]); \
            INTERPOLATE(out[10], out[9],  out[11]); \
            INTERPOLATE(out[12], out[11], out[13]); \
            INTERPOLATE(out[14], out[13], out[15]);

#define INTERPOLATE_8(d0, out) \
            INTERPOLATE(out[0],  d0,      out[1]); \
            INTERPOLATE(out[2],  out[1],  out[3]); \
            INTERPOLATE(out[4],  out[3],  out[5]); \
            INTERPOLATE(out[6],  out[5],  out[7]); \
            INTERPOLATE(out[8],  out[7],  out[9]); \
            INTERPOLATE(out[10], out[9],  out[11]); \
            INTERPOLATE(out[12], out[11], out[13]); \
            INTERPOLATE(out[14], out[13], out[15]);

#define LIMIT_16(xx) xx = xx<-32768?-32768:(xx>32767?32767:xx)
#define LIMIT_24(xx) xx = xx<-8388608?-8388608:(xx>8388607?8388607:xx)

#define PREVENT_OVERFLOW_16(out) \
            LIMIT_16(out[0]);  LIMIT_16(out[1]);  LIMIT_16(out[2]);  LIMIT_16(out[3]); \
            LIMIT_16(out[4]);  LIMIT_16(out[5]);  LIMIT_16(out[6]);  LIMIT_16(out[7]); \
            LIMIT_16(out[8]);  LIMIT_16(out[9]);  LIMIT_16(out[10]); LIMIT_16(out[11]); \
            LIMIT_16(out[12]); LIMIT_16(out[13]); LIMIT_16(out[14]); LIMIT_16(out[15]);
#define PREVENT_OVERFLOW_24(out) \
            LIMIT_24(out[0]);  LIMIT_24(out[1]);  LIMIT_24(out[2]);  LIMIT_24(out[3]); \
            LIMIT_24(out[4]);  LIMIT_24(out[5]);  LIMIT_24(out[6]);  LIMIT_24(out[7]); \
            LIMIT_24(out[8]);  LIMIT_24(out[9]);  LIMIT_24(out[10]); LIMIT_24(out[11]); \
            LIMIT_24(out[12]); LIMIT_24(out[13]); LIMIT_24(out[14]); LIMIT_24(out[15]);

#define ZERO(out) \
            out[0]  = 0; out[1]  = 0; out[2]  = 0; out[3]  = 0; \
            out[4]  = 0; out[5]  = 0; out[6]  = 0; out[7]  = 0; \
            out[8]  = 0; out[9]  = 0; out[10] = 0; out[11] = 0; \
            out[12] = 0; out[13] = 0; out[14] = 0; out[15] = 0;

#define DOUBLE_ODDS(out) \
            out[0] <<= 1; out[2]  <<= 1; out[4]  <<= 1; out[6]  <<= 1; \
            out[8] <<= 1; out[10] <<= 1; out[12] <<= 1; out[14] <<= 1;

#define MASK_L(xx, mask, shift) (xx&mask)<<shift
#define MASK_R(xx, mask, shift) (xx&mask)>>shift


//*****************************************************************************
void decodeMTP(int d0, unsigned char *in, int *out)
{
    // Decodes a 16-byte MTP RDAC block into 24-bit samples.

    int patternIndex = MASK_L(in[0], 0xf0, 0) | MASK_R(in[2], 0xf0, 4);

    int pattern = patterns[patternIndex];

    int *p = out;

    switch (pattern) {

        /*=====================================================================
            PATTERN B
            pp888888 88888887 ppgggggg gggggggf
            77777666 66666555 fffffeee eeeeeddd
            55544444 44443333 dddccccc ccccbbbb
            33222222 22111111 bbaaaaaa aa999999
        */

        case 0:  /* 00..00.. */

            *p     = MASK_L(in[13], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[12], 0x3f, 2) | MASK_R(in[13], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[9],  0x0f, 2) | MASK_R(in[12], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[8],  0x1f, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_9(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[5],  0x07, 3) | MASK_R(in[8],  0xe0, 5); SIGN_EXTEND_6(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[4],  0x07, 5) | MASK_R(in[5],  0xf8, 3); SIGN_EXTEND_8(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[1],  0x01, 5) | MASK_R(in[4],  0xf8, 3); SIGN_EXTEND_6(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[0],  0x3f, 7) | MASK_R(in[1],  0xfe, 1); SIGN_EXTEND_13(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[14], 0x3f, 2) | MASK_R(in[15], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[11], 0x0f, 2) | MASK_R(in[14], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[10], 0x1f, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_9(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[7],  0x07, 3) | MASK_R(in[10], 0xe0, 5); SIGN_EXTEND_6(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[6],  0x07, 5) | MASK_R(in[7],  0xf8, 3); SIGN_EXTEND_8(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[3],  0x01, 5) | MASK_R(in[6],  0xf8, 3); SIGN_EXTEND_6(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[2],  0x3f, 7) | MASK_R(in[3],  0xfe, 1); SIGN_EXTEND_13(*p); SHIFT_ROUND_6(*p);

            // 2 linears

            TWEAK_2(out);

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_24(out);

            return;

        case 1:  /* 00..01.. */

            *p     = MASK_L(in[13], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[12], 0x3f, 2) | MASK_R(in[13], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[9],  0x0f, 2) | MASK_R(in[12], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[8],  0x1f, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_9(*p);  SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[5],  0x07, 3) | MASK_R(in[8],  0xe0, 5); SIGN_EXTEND_6(*p);  SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[4],  0x07, 5) | MASK_R(in[5],  0xf8, 3); SIGN_EXTEND_8(*p);  SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[1],  0x01, 5) | MASK_R(in[4],  0xf8, 3); SIGN_EXTEND_6(*p);  SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[0],  0x3f, 7) | MASK_R(in[1],  0xfe, 1); SIGN_EXTEND_13(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[14], 0x3f, 2) | MASK_R(in[15], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[11], 0x0f, 2) | MASK_R(in[14], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[10], 0x1f, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_9(*p);  SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[7],  0x07, 3) | MASK_R(in[10], 0xe0, 5); SIGN_EXTEND_6(*p);  SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[6],  0x07, 5) | MASK_R(in[7],  0xf8, 3); SIGN_EXTEND_8(*p);  SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[3],  0x01, 5) | MASK_R(in[6],  0xf8, 3); SIGN_EXTEND_6(*p);  SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[2],  0x3f, 7) | MASK_R(in[3],  0xfe, 1); SIGN_EXTEND_13(*p); SHIFT_ROUND_7(*p);

            // 2 linears

            TWEAK_2(out);

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_24(out);

            return;

        case 2:  /* 00..10.. */

            *p     = MASK_L(in[13], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[12], 0x3f, 2) | MASK_R(in[13], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[9],  0x0f, 2) | MASK_R(in[12], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[8],  0x1f, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_9(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[5],  0x07, 3) | MASK_R(in[8],  0xe0, 5); SIGN_EXTEND_6(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[4],  0x07, 5) | MASK_R(in[5],  0xf8, 3); SIGN_EXTEND_8(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[1],  0x01, 5) | MASK_R(in[4],  0xf8, 3); SIGN_EXTEND_6(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[0],  0x3f, 7) | MASK_R(in[1],  0xfe, 1); SIGN_EXTEND_13(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[14], 0x3f, 2) | MASK_R(in[15], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[11], 0x0f, 2) | MASK_R(in[14], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[10], 0x1f, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_9(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[7],  0x07, 3) | MASK_R(in[10], 0xe0, 5); SIGN_EXTEND_6(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[6],  0x07, 5) | MASK_R(in[7],  0xf8, 3); SIGN_EXTEND_8(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[3],  0x01, 5) | MASK_R(in[6],  0xf8, 3); SIGN_EXTEND_6(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[2],  0x3f, 7) | MASK_R(in[3],  0xfe, 1); SIGN_EXTEND_13(*p); SHIFT_ROUND_8(*p);

            // 2 linears

            TWEAK_2(out);

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_24(out);

            return;

        case 3:  /* 00..11.. */

            *p     = MASK_L(in[13], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[12], 0x3f, 2) | MASK_R(in[13], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[9],  0x0f, 2) | MASK_R(in[12], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[8],  0x1f, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_9(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[5],  0x07, 3) | MASK_R(in[8],  0xe0, 5); SIGN_EXTEND_6(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[4],  0x07, 5) | MASK_R(in[5],  0xf8, 3); SIGN_EXTEND_8(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[1],  0x01, 5) | MASK_R(in[4],  0xf8, 3); SIGN_EXTEND_6(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[0],  0x3f, 7) | MASK_R(in[1],  0xfe, 1); SIGN_EXTEND_13(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[14], 0x3f, 2) | MASK_R(in[15], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[11], 0x0f, 2) | MASK_R(in[14], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[10], 0x1f, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_9(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[7],  0x07, 3) | MASK_R(in[10], 0xe0, 5); SIGN_EXTEND_6(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[6],  0x07, 5) | MASK_R(in[7],  0xf8, 3); SIGN_EXTEND_8(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[3],  0x01, 5) | MASK_R(in[6],  0xf8, 3); SIGN_EXTEND_6(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[2],  0x3f, 7) | MASK_R(in[3],  0xfe, 1); SIGN_EXTEND_13(*p); SHIFT_ROUND_9(*p);

            // 2 linears

            TWEAK_2(out);

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_24(out);

            return;

        case 4:  /* 01..00.. */

            *p     = MASK_L(in[13], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[12], 0x3f, 2) | MASK_R(in[13], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[9],  0x0f, 2) | MASK_R(in[12], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[8],  0x1f, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_9(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[5],  0x07, 3) | MASK_R(in[8],  0xe0, 5); SIGN_EXTEND_6(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[4],  0x07, 5) | MASK_R(in[5],  0xf8, 3); SIGN_EXTEND_8(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[1],  0x01, 5) | MASK_R(in[4],  0xf8, 3); SIGN_EXTEND_6(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[0],  0x3f, 7) | MASK_R(in[1],  0xfe, 1); SIGN_EXTEND_13(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[14], 0x3f, 2) | MASK_R(in[15], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[11], 0x0f, 2) | MASK_R(in[14], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[10], 0x1f, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_9(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[7],  0x07, 3) | MASK_R(in[10], 0xe0, 5); SIGN_EXTEND_6(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[6],  0x07, 5) | MASK_R(in[7],  0xf8, 3); SIGN_EXTEND_8(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[3],  0x01, 5) | MASK_R(in[6],  0xf8, 3); SIGN_EXTEND_6(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[2],  0x3f, 7) | MASK_R(in[3],  0xfe, 1); SIGN_EXTEND_13(*p); SHIFT_ROUND_10(*p);

            // 2 linears

            TWEAK_2(out);

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_24(out);

            return;

        case 5:  /* 01..01.. */

            *p     = MASK_L(in[13], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[12], 0x3f, 2) | MASK_R(in[13], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[9],  0x0f, 2) | MASK_R(in[12], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[8],  0x1f, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_9(*p);  SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[5],  0x07, 3) | MASK_R(in[8],  0xe0, 5); SIGN_EXTEND_6(*p);  SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[4],  0x07, 5) | MASK_R(in[5],  0xf8, 3); SIGN_EXTEND_8(*p);  SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[1],  0x01, 5) | MASK_R(in[4],  0xf8, 3); SIGN_EXTEND_6(*p);  SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[0],  0x3f, 7) | MASK_R(in[1],  0xfe, 1); SIGN_EXTEND_13(*p); SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[14], 0x3f, 2) | MASK_R(in[15], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[11], 0x0f, 2) | MASK_R(in[14], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[10], 0x1f, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_9(*p);  SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[7],  0x07, 3) | MASK_R(in[10], 0xe0, 5); SIGN_EXTEND_6(*p);  SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[6],  0x07, 5) | MASK_R(in[7],  0xf8, 3); SIGN_EXTEND_8(*p);  SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[3],  0x01, 5) | MASK_R(in[6],  0xf8, 3); SIGN_EXTEND_6(*p);  SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[2],  0x3f, 7) | MASK_R(in[3],  0xfe, 1); SIGN_EXTEND_13(*p); SHIFT_ROUND_11(*p);

            // 2 linears

            TWEAK_2(out);

            INTERPOLATE_2(d0, out);

            PREVENT_OVERFLOW_24(out);

            return;

        /*=====================================================================
            PATTERN D
            pp888888 88877777 ppgggggg gggfffff
            77666666 66555555 ffeeeeee eedddddd
            54444444 44333333 dccccccc ccbbbbbb
            32222222 21111111 baaaaaaa a9999999
        */

        case 6:  /* 01..10.. */

            *p     = MASK_L(in[13], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[12], 0x7f, 1) | MASK_R(in[13], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[9],  0x3f, 1) | MASK_R(in[12], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[8],  0x7f, 2) | MASK_R(in[9],  0xc0, 6); SIGN_EXTEND_9(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[5],  0x3f, 1) | MASK_R(in[8],  0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[4],  0x3f, 2) | MASK_R(in[5],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[1],  0x1f, 2) | MASK_R(in[4],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[0],  0x3f, 3) | MASK_R(in[1],  0xe0, 5); SIGN_EXTEND_9(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[15], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[14], 0x7f, 1) | MASK_R(in[15], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[11], 0x3f, 1) | MASK_R(in[14], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[10], 0x7f, 2) | MASK_R(in[11], 0xc0, 6); SIGN_EXTEND_9(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[7],  0x3f, 1) | MASK_R(in[10], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[6],  0x3f, 2) | MASK_R(in[7],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[3],  0x1f, 2) | MASK_R(in[6],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[2],  0x3f, 3) | MASK_R(in[3],  0xe0, 5); SIGN_EXTEND_9(*p); SHIFT_ROUND_10(*p);

            // 4 linears

            TWEAK_4(out);

            INTERPOLATE_4(d0, out);

            //PREVENT_OVERFLOW_24(out);

            return;

        case 7:  /* 01..11.. */

            *p     = MASK_L(in[13], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[12], 0x7f, 1) | MASK_R(in[13], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[9],  0x3f, 1) | MASK_R(in[12], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[8],  0x7f, 2) | MASK_R(in[9],  0xc0, 6); SIGN_EXTEND_9(*p); SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[5],  0x3f, 1) | MASK_R(in[8],  0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[4],  0x3f, 2) | MASK_R(in[5],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[1],  0x1f, 2) | MASK_R(in[4],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[0],  0x3f, 3) | MASK_R(in[1],  0xe0, 5); SIGN_EXTEND_9(*p); SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[15], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[14], 0x7f, 1) | MASK_R(in[15], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[11], 0x3f, 1) | MASK_R(in[14], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[10], 0x7f, 2) | MASK_R(in[11], 0xc0, 6); SIGN_EXTEND_9(*p); SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[7],  0x3f, 1) | MASK_R(in[10], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[6],  0x3f, 2) | MASK_R(in[7],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[3],  0x1f, 2) | MASK_R(in[6],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[2],  0x3f, 3) | MASK_R(in[3],  0xe0, 5); SIGN_EXTEND_9(*p); SHIFT_ROUND_11(*p);

            // 4 linears

            TWEAK_4(out);

            INTERPOLATE_4(d0, out);

            //PREVENT_OVERFLOW_24(out);

            return;

        case 8:  /* 10..00.. */

            *p     = MASK_L(in[13], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[12], 0x7f, 1) | MASK_R(in[13], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[9],  0x3f, 1) | MASK_R(in[12], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[8],  0x7f, 2) | MASK_R(in[9],  0xc0, 6); SIGN_EXTEND_9(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[5],  0x3f, 1) | MASK_R(in[8],  0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[4],  0x3f, 2) | MASK_R(in[5],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[1],  0x1f, 2) | MASK_R(in[4],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[0],  0x3f, 3) | MASK_R(in[1],  0xe0, 5); SIGN_EXTEND_9(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[15], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[14], 0x7f, 1) | MASK_R(in[15], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[11], 0x3f, 1) | MASK_R(in[14], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[10], 0x7f, 2) | MASK_R(in[11], 0xc0, 6); SIGN_EXTEND_9(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[7],  0x3f, 1) | MASK_R(in[10], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[6],  0x3f, 2) | MASK_R(in[7],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[3],  0x1f, 2) | MASK_R(in[6],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[2],  0x3f, 3) | MASK_R(in[3],  0xe0, 5); SIGN_EXTEND_9(*p); SHIFT_ROUND_12(*p);

            // 4 linears

            TWEAK_4(out);

            INTERPOLATE_4(d0, out);

            //PREVENT_OVERFLOW_24(out);

            return;

        case 9:  /* 10..01.. */

            *p     = MASK_L(in[13], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[12], 0x7f, 1) | MASK_R(in[13], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[9],  0x3f, 1) | MASK_R(in[12], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[8],  0x7f, 2) | MASK_R(in[9],  0xc0, 6); SIGN_EXTEND_9(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[5],  0x3f, 1) | MASK_R(in[8],  0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[4],  0x3f, 2) | MASK_R(in[5],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[1],  0x1f, 2) | MASK_R(in[4],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[0],  0x3f, 3) | MASK_R(in[1],  0xe0, 5); SIGN_EXTEND_9(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[15], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[14], 0x7f, 1) | MASK_R(in[15], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[11], 0x3f, 1) | MASK_R(in[14], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[10], 0x7f, 2) | MASK_R(in[11], 0xc0, 6); SIGN_EXTEND_9(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[7],  0x3f, 1) | MASK_R(in[10], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[6],  0x3f, 2) | MASK_R(in[7],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[3],  0x1f, 2) | MASK_R(in[6],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[2],  0x3f, 3) | MASK_R(in[3],  0xe0, 5); SIGN_EXTEND_9(*p); SHIFT_ROUND_13(*p);

            // 4 linears

            TWEAK_4(out);

            INTERPOLATE_4(d0, out);

            //PREVENT_OVERFLOW_24(out);

            return;

        case 10: /* 10..10.. */

            *p     = MASK_L(in[13], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[12], 0x7f, 1) | MASK_R(in[13], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[9],  0x3f, 1) | MASK_R(in[12], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[8],  0x7f, 2) | MASK_R(in[9],  0xc0, 6); SIGN_EXTEND_9(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[5],  0x3f, 1) | MASK_R(in[8],  0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[4],  0x3f, 2) | MASK_R(in[5],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[1],  0x1f, 2) | MASK_R(in[4],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[0],  0x3f, 3) | MASK_R(in[1],  0xe0, 5); SIGN_EXTEND_9(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[15], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[14], 0x7f, 1) | MASK_R(in[15], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[11], 0x3f, 1) | MASK_R(in[14], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[10], 0x7f, 2) | MASK_R(in[11], 0xc0, 6); SIGN_EXTEND_9(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[7],  0x3f, 1) | MASK_R(in[10], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[6],  0x3f, 2) | MASK_R(in[7],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[3],  0x1f, 2) | MASK_R(in[6],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[2],  0x3f, 3) | MASK_R(in[3],  0xe0, 5); SIGN_EXTEND_9(*p); SHIFT_ROUND_14(*p);

            // 4 linears

            TWEAK_4(out);

            INTERPOLATE_4(d0, out);

            //PREVENT_OVERFLOW_24(out);

            return;

        case 11: /* 10..11.. */

            *p     = MASK_L(in[13], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_15(*p);
            *(++p) = MASK_L(in[12], 0x7f, 1) | MASK_R(in[13], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_15(*p);
            *(++p) = MASK_L(in[9],  0x3f, 1) | MASK_R(in[12], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_15(*p);
            *(++p) = MASK_L(in[8],  0x7f, 2) | MASK_R(in[9],  0xc0, 6); SIGN_EXTEND_9(*p); SHIFT_ROUND_15(*p);
            *(++p) = MASK_L(in[5],  0x3f, 1) | MASK_R(in[8],  0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_15(*p);
            *(++p) = MASK_L(in[4],  0x3f, 2) | MASK_R(in[5],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_15(*p);
            *(++p) = MASK_L(in[1],  0x1f, 2) | MASK_R(in[4],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_15(*p);
            *(++p) = MASK_L(in[0],  0x3f, 3) | MASK_R(in[1],  0xe0, 5); SIGN_EXTEND_9(*p); SHIFT_ROUND_15(*p);
            *(++p) = MASK_L(in[15], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_15(*p);
            *(++p) = MASK_L(in[14], 0x7f, 1) | MASK_R(in[15], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_15(*p);
            *(++p) = MASK_L(in[11], 0x3f, 1) | MASK_R(in[14], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_15(*p);
            *(++p) = MASK_L(in[10], 0x7f, 2) | MASK_R(in[11], 0xc0, 6); SIGN_EXTEND_9(*p); SHIFT_ROUND_15(*p);
            *(++p) = MASK_L(in[7],  0x3f, 1) | MASK_R(in[10], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_15(*p);
            *(++p) = MASK_L(in[6],  0x3f, 2) | MASK_R(in[7],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_15(*p);
            *(++p) = MASK_L(in[3],  0x1f, 2) | MASK_R(in[6],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_15(*p);
            *(++p) = MASK_L(in[2],  0x3f, 3) | MASK_R(in[3],  0xe0, 5); SIGN_EXTEND_9(*p); SHIFT_ROUND_15(*p);

            // 4 linears

            TWEAK_4(out);

            INTERPOLATE_4(d0, out);

            PREVENT_OVERFLOW_24(out);

            return;

        /*=====================================================================
            PATTERN A
            ppp88888 88888888 pppggggg gggggggg
            87777776 66666655 gffffffe eeeeeedd
            55554444 44444333 ddddcccc cccccbbb
            33322222 22111111 bbbaaaaa aa999999
        */

        case 12: /* 110.000. */

            *p     = MASK_L(in[13], 0x3f, 0);                                                    SIGN_EXTEND_6(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[12], 0x1f, 2) | MASK_R(in[13], 0xc0, 6);                          SIGN_EXTEND_7(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[9],  0x07, 3) | MASK_R(in[12], 0xe0, 5);                          SIGN_EXTEND_6(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[8],  0x0f, 5) | MASK_R(in[9],  0xf8, 3);                          SIGN_EXTEND_9(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[5],  0x03, 4) | MASK_R(in[8],  0xf0, 4);                          SIGN_EXTEND_6(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[4],  0x01, 6) | MASK_R(in[5],  0xfc, 2);                          SIGN_EXTEND_7(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_R(in[4],  0x7e, 1);                                                    SIGN_EXTEND_6(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[0],  0x1f, 9) | MASK_L(in[1],  0xff, 1) | MASK_R(in[4], 0x80, 7); SIGN_EXTEND_14(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                                                    SIGN_EXTEND_6(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[14], 0x1f, 2) | MASK_R(in[15], 0xc0, 6);                          SIGN_EXTEND_7(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[11], 0x07, 3) | MASK_R(in[14], 0xe0, 5);                          SIGN_EXTEND_6(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[10], 0x0f, 5) | MASK_R(in[11], 0xf8, 3);                          SIGN_EXTEND_9(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[7],  0x03, 4) | MASK_R(in[10], 0xf0, 4);                          SIGN_EXTEND_6(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[6],  0x01, 6) | MASK_R(in[7],  0xfc, 2);                          SIGN_EXTEND_7(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_R(in[6],  0x7e, 1);                                                    SIGN_EXTEND_6(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[2],  0x1f, 9) | MASK_L(in[3],  0xff, 1) | MASK_R(in[6], 0x80, 7); SIGN_EXTEND_14(*p); SHIFT_ROUND_5(*p);

            // 2 linears

            TWEAK_2(out);

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_24(out);

            return;

        case 13: /* 110.001. */

            *p     = MASK_L(in[13], 0x3f, 0);                                                    SIGN_EXTEND_6(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[12], 0x1f, 2) | MASK_R(in[13], 0xc0, 6);                          SIGN_EXTEND_7(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[9],  0x07, 3) | MASK_R(in[12], 0xe0, 5);                          SIGN_EXTEND_6(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[8],  0x0f, 5) | MASK_R(in[9],  0xf8, 3);                          SIGN_EXTEND_9(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[5],  0x03, 4) | MASK_R(in[8],  0xf0, 4);                          SIGN_EXTEND_6(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[4],  0x01, 6) | MASK_R(in[5],  0xfc, 2);                          SIGN_EXTEND_7(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_R(in[4],  0x7e, 1);                                                    SIGN_EXTEND_6(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[0],  0x1f, 9) | MASK_L(in[1],  0xff, 1) | MASK_R(in[4], 0x80, 7); SIGN_EXTEND_14(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                                                    SIGN_EXTEND_6(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[14], 0x1f, 2) | MASK_R(in[15], 0xc0, 6);                          SIGN_EXTEND_7(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[11], 0x07, 3) | MASK_R(in[14], 0xe0, 5);                          SIGN_EXTEND_6(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[10], 0x0f, 5) | MASK_R(in[11], 0xf8, 3);                          SIGN_EXTEND_9(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[7],  0x03, 4) | MASK_R(in[10], 0xf0, 4);                          SIGN_EXTEND_6(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[6],  0x01, 6) | MASK_R(in[7],  0xfc, 2);                          SIGN_EXTEND_7(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_R(in[6],  0x7e, 1);                                                    SIGN_EXTEND_6(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[2],  0x1f, 9) | MASK_L(in[3],  0xff, 1) | MASK_R(in[6], 0x80, 7); SIGN_EXTEND_14(*p); SHIFT_ROUND_6(*p);

            // 2 linears

            TWEAK_2(out);

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_24(out);

            return;

        case 14: /* 110.010. */

            *p     = MASK_L(in[13], 0x3f, 0);                                                    SIGN_EXTEND_6(*p);  SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[12], 0x1f, 2) | MASK_R(in[13], 0xc0, 6);                          SIGN_EXTEND_7(*p);  SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[9],  0x07, 3) | MASK_R(in[12], 0xe0, 5);                          SIGN_EXTEND_6(*p);  SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[8],  0x0f, 5) | MASK_R(in[9],  0xf8, 3);                          SIGN_EXTEND_9(*p);  SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[5],  0x03, 4) | MASK_R(in[8],  0xf0, 4);                          SIGN_EXTEND_6(*p);  SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[4],  0x01, 6) | MASK_R(in[5],  0xfc, 2);                          SIGN_EXTEND_7(*p);  SHIFT_ROUND_7(*p);
            *(++p) = MASK_R(in[4],  0x7e, 1);                                                    SIGN_EXTEND_6(*p);  SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[0],  0x1f, 9) | MASK_L(in[1],  0xff, 1) | MASK_R(in[4], 0x80, 7); SIGN_EXTEND_14(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                                                    SIGN_EXTEND_6(*p);  SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[14], 0x1f, 2) | MASK_R(in[15], 0xc0, 6);                          SIGN_EXTEND_7(*p);  SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[11], 0x07, 3) | MASK_R(in[14], 0xe0, 5);                          SIGN_EXTEND_6(*p);  SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[10], 0x0f, 5) | MASK_R(in[11], 0xf8, 3);                          SIGN_EXTEND_9(*p);  SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[7],  0x03, 4) | MASK_R(in[10], 0xf0, 4);                          SIGN_EXTEND_6(*p);  SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[6],  0x01, 6) | MASK_R(in[7],  0xfc, 2);                          SIGN_EXTEND_7(*p);  SHIFT_ROUND_7(*p);
            *(++p) = MASK_R(in[6],  0x7e, 1);                                                    SIGN_EXTEND_6(*p);  SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[2],  0x1f, 9) | MASK_L(in[3],  0xff, 1) | MASK_R(in[6], 0x80, 7); SIGN_EXTEND_14(*p); SHIFT_ROUND_7(*p);

            // 2 linears

            TWEAK_2(out);

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_24(out);

            return;

        case 15: /* 110.011. */

            *p     = MASK_L(in[13], 0x3f, 0);                                                    SIGN_EXTEND_6(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[12], 0x1f, 2) | MASK_R(in[13], 0xc0, 6);                          SIGN_EXTEND_7(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[9],  0x07, 3) | MASK_R(in[12], 0xe0, 5);                          SIGN_EXTEND_6(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[8],  0x0f, 5) | MASK_R(in[9],  0xf8, 3);                          SIGN_EXTEND_9(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[5],  0x03, 4) | MASK_R(in[8],  0xf0, 4);                          SIGN_EXTEND_6(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[4],  0x01, 6) | MASK_R(in[5],  0xfc, 2);                          SIGN_EXTEND_7(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_R(in[4],  0x7e, 1);                                                    SIGN_EXTEND_6(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[0],  0x1f, 9) | MASK_L(in[1],  0xff, 1) | MASK_R(in[4], 0x80, 7); SIGN_EXTEND_14(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                                                    SIGN_EXTEND_6(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[14], 0x1f, 2) | MASK_R(in[15], 0xc0, 6);                          SIGN_EXTEND_7(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[11], 0x07, 3) | MASK_R(in[14], 0xe0, 5);                          SIGN_EXTEND_6(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[10], 0x0f, 5) | MASK_R(in[11], 0xf8, 3);                          SIGN_EXTEND_9(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[7],  0x03, 4) | MASK_R(in[10], 0xf0, 4);                          SIGN_EXTEND_6(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[6],  0x01, 6) | MASK_R(in[7],  0xfc, 2);                          SIGN_EXTEND_7(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_R(in[6],  0x7e, 1);                                                    SIGN_EXTEND_6(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[2],  0x1f, 9) | MASK_L(in[3],  0xff, 1) | MASK_R(in[6], 0x80, 7); SIGN_EXTEND_14(*p); SHIFT_ROUND_8(*p);

            // 2 linears

            TWEAK_2(out);

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_24(out);

            return;

        case 16: /* 110.100. */

            *p     = MASK_L(in[13], 0x3f, 0);                                                    SIGN_EXTEND_6(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[12], 0x1f, 2) | MASK_R(in[13], 0xc0, 6);                          SIGN_EXTEND_7(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[9],  0x07, 3) | MASK_R(in[12], 0xe0, 5);                          SIGN_EXTEND_6(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[8],  0x0f, 5) | MASK_R(in[9],  0xf8, 3);                          SIGN_EXTEND_9(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[5],  0x03, 4) | MASK_R(in[8],  0xf0, 4);                          SIGN_EXTEND_6(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[4],  0x01, 6) | MASK_R(in[5],  0xfc, 2);                          SIGN_EXTEND_7(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_R(in[4],  0x7e, 1);                                                    SIGN_EXTEND_6(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[0],  0x1f, 9) | MASK_L(in[1],  0xff, 1) | MASK_R(in[4], 0x80, 7); SIGN_EXTEND_14(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                                                    SIGN_EXTEND_6(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[14], 0x1f, 2) | MASK_R(in[15], 0xc0, 6);                          SIGN_EXTEND_7(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[11], 0x07, 3) | MASK_R(in[14], 0xe0, 5);                          SIGN_EXTEND_6(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[10], 0x0f, 5) | MASK_R(in[11], 0xf8, 3);                          SIGN_EXTEND_9(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[7],  0x03, 4) | MASK_R(in[10], 0xf0, 4);                          SIGN_EXTEND_6(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[6],  0x01, 6) | MASK_R(in[7],  0xfc, 2);                          SIGN_EXTEND_7(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_R(in[6],  0x7e, 1);                                                    SIGN_EXTEND_6(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[2],  0x1f, 9) | MASK_L(in[3],  0xff, 1) | MASK_R(in[6], 0x80, 7); SIGN_EXTEND_14(*p); SHIFT_ROUND_9(*p);

            // 2 linears

            TWEAK_2(out);

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_24(out);

            return;

        case 17: /* 110.101. */

            *p     = MASK_L(in[13], 0x3f, 0);                                                    SIGN_EXTEND_6(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[12], 0x1f, 2) | MASK_R(in[13], 0xc0, 6);                          SIGN_EXTEND_7(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[9],  0x07, 3) | MASK_R(in[12], 0xe0, 5);                          SIGN_EXTEND_6(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[8],  0x0f, 5) | MASK_R(in[9],  0xf8, 3);                          SIGN_EXTEND_9(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[5],  0x03, 4) | MASK_R(in[8],  0xf0, 4);                          SIGN_EXTEND_6(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[4],  0x01, 6) | MASK_R(in[5],  0xfc, 2);                          SIGN_EXTEND_7(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_R(in[4],  0x7e, 1);                                                    SIGN_EXTEND_6(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[0],  0x1f, 9) | MASK_L(in[1],  0xff, 1) | MASK_R(in[4], 0x80, 7); SIGN_EXTEND_14(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                                                    SIGN_EXTEND_6(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[14], 0x1f, 2) | MASK_R(in[15], 0xc0, 6);                          SIGN_EXTEND_7(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[11], 0x07, 3) | MASK_R(in[14], 0xe0, 5);                          SIGN_EXTEND_6(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[10], 0x0f, 5) | MASK_R(in[11], 0xf8, 3);                          SIGN_EXTEND_9(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[7],  0x03, 4) | MASK_R(in[10], 0xf0, 4);                          SIGN_EXTEND_6(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[6],  0x01, 6) | MASK_R(in[7],  0xfc, 2);                          SIGN_EXTEND_7(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_R(in[6],  0x7e, 1);                                                    SIGN_EXTEND_6(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[2],  0x1f, 9) | MASK_L(in[3],  0xff, 1) | MASK_R(in[6], 0x80, 7); SIGN_EXTEND_14(*p); SHIFT_ROUND_10(*p);

            // 2 linears

            TWEAK_2(out);

            INTERPOLATE_2(d0, out);

            PREVENT_OVERFLOW_24(out);

            return;

        /*=====================================================================
            PATTERN B3
            ppp88888 88888887 pppggggg gggggggf
            77777666 66666555 fffffeee eeeeeddd
            55544444 44443333 dddccccc ccccbbbb
            33222222 22111111 bbaaaaaa aa999999
        */

        case 18: /* 110.110. */

            *p     = MASK_L(in[13], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[12], 0x3f, 2) | MASK_R(in[13], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[9],  0x0f, 2) | MASK_R(in[12], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[8],  0x1f, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_9(*p);  SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[5],  0x07, 3) | MASK_R(in[8],  0xe0, 5); SIGN_EXTEND_6(*p);  SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[4],  0x07, 5) | MASK_R(in[5],  0xf8, 3); SIGN_EXTEND_8(*p);  SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[1],  0x01, 5) | MASK_R(in[4],  0xf8, 3); SIGN_EXTEND_6(*p);  SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[0],  0x1f, 7) | MASK_R(in[1],  0xfe, 1); SIGN_EXTEND_12(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[14], 0x3f, 2) | MASK_R(in[15], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[11], 0x0f, 2) | MASK_R(in[14], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[10], 0x1f, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_9(*p);  SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[7],  0x07, 3) | MASK_R(in[10], 0xe0, 5); SIGN_EXTEND_6(*p);  SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[6],  0x07, 5) | MASK_R(in[7],  0xf8, 3); SIGN_EXTEND_8(*p);  SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[3],  0x01, 5) | MASK_R(in[6],  0xf8, 3); SIGN_EXTEND_6(*p);  SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[2],  0x1f, 7) | MASK_R(in[3],  0xfe, 1); SIGN_EXTEND_12(*p); SHIFT_ROUND_12(*p);

            // 2 linears

            TWEAK_2(out);

            INTERPOLATE_2(d0, out);

            PREVENT_OVERFLOW_24(out);

            return;

        /*=====================================================================
            PATTERN C
            ppp88888 88888877 pppggggg ggggggff
            77776666 66665555 ffffeeee eeeedddd
            55444444 44443333 ddcccccc ccccbbbb
            33222222 22111111 bbaaaaaa aa999999
        */
        case 19: /* 110.111. */

            *p     = MASK_L(in[13], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[12], 0x3f, 2) | MASK_R(in[13], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[9],  0x0f, 2) | MASK_R(in[12], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[8],  0x3f, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_10(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[5],  0x0f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[4],  0x0f, 4) | MASK_R(in[5],  0xf0, 4); SIGN_EXTEND_8(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[1],  0x03, 4) | MASK_R(in[4],  0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[0],  0x1f, 6) | MASK_R(in[1],  0xfc, 2); SIGN_EXTEND_11(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[14], 0x3f, 2) | MASK_R(in[15], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[11], 0x0f, 2) | MASK_R(in[14], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[10], 0x3f, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_10(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[7],  0x0f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[6],  0x0f, 4) | MASK_R(in[7],  0xf0, 4); SIGN_EXTEND_8(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[3],  0x03, 4) | MASK_R(in[6],  0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[2],  0x1f, 6) | MASK_R(in[3],  0xfc, 2); SIGN_EXTEND_11(*p); SHIFT_ROUND_8(*p);

            // 2 linears

            TWEAK_2(out);

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_24(out);

            return;

        case 20: /* 111.000. */

            *p     = MASK_L(in[13], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[12], 0x3f, 2) | MASK_R(in[13], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[9],  0x0f, 2) | MASK_R(in[12], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[8],  0x3f, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_10(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[5],  0x0f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[4],  0x0f, 4) | MASK_R(in[5],  0xf0, 4); SIGN_EXTEND_8(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[1],  0x03, 4) | MASK_R(in[4],  0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[0],  0x1f, 6) | MASK_R(in[1],  0xfc, 2); SIGN_EXTEND_11(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[14], 0x3f, 2) | MASK_R(in[15], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[11], 0x0f, 2) | MASK_R(in[14], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[10], 0x3f, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_10(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[7],  0x0f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[6],  0x0f, 4) | MASK_R(in[7],  0xf0, 4); SIGN_EXTEND_8(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[3],  0x03, 4) | MASK_R(in[6],  0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[2],  0x1f, 6) | MASK_R(in[3],  0xfc, 2); SIGN_EXTEND_11(*p); SHIFT_ROUND_9(*p);

            // 2 linears

            TWEAK_2(out);

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_24(out);

            return;

        case 21: /* 111.001. */

            *p     = MASK_L(in[13], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[12], 0x3f, 2) | MASK_R(in[13], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[9],  0x0f, 2) | MASK_R(in[12], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[8],  0x3f, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_10(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[5],  0x0f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[4],  0x0f, 4) | MASK_R(in[5],  0xf0, 4); SIGN_EXTEND_8(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[1],  0x03, 4) | MASK_R(in[4],  0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[0],  0x1f, 6) | MASK_R(in[1],  0xfc, 2); SIGN_EXTEND_11(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[14], 0x3f, 2) | MASK_R(in[15], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[11], 0x0f, 2) | MASK_R(in[14], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[10], 0x3f, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_10(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[7],  0x0f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[6],  0x0f, 4) | MASK_R(in[7],  0xf0, 4); SIGN_EXTEND_8(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[3],  0x03, 4) | MASK_R(in[6],  0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[2],  0x1f, 6) | MASK_R(in[3],  0xfc, 2); SIGN_EXTEND_11(*p); SHIFT_ROUND_10(*p);

            // 2 linears

            TWEAK_2(out);

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_24(out);

            return;

        case 22: /* 111.010. */

            *p     = MASK_L(in[13], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[12], 0x3f, 2) | MASK_R(in[13], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[9],  0x0f, 2) | MASK_R(in[12], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[8],  0x3f, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_10(*p); SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[5],  0x0f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[4],  0x0f, 4) | MASK_R(in[5],  0xf0, 4); SIGN_EXTEND_8(*p);  SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[1],  0x03, 4) | MASK_R(in[4],  0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[0],  0x1f, 6) | MASK_R(in[1],  0xfc, 2); SIGN_EXTEND_11(*p); SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[14], 0x3f, 2) | MASK_R(in[15], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[11], 0x0f, 2) | MASK_R(in[14], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[10], 0x3f, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_10(*p); SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[7],  0x0f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[6],  0x0f, 4) | MASK_R(in[7],  0xf0, 4); SIGN_EXTEND_8(*p);  SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[3],  0x03, 4) | MASK_R(in[6],  0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[2],  0x1f, 6) | MASK_R(in[3],  0xfc, 2); SIGN_EXTEND_11(*p); SHIFT_ROUND_11(*p);

            // 2 linears

            TWEAK_2(out);

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_24(out);

            return;

        case 23: /* 111.011. */

            *p     = MASK_L(in[13], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[12], 0x3f, 2) | MASK_R(in[13], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[9],  0x0f, 2) | MASK_R(in[12], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[8],  0x3f, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_10(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[5],  0x0f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[4],  0x0f, 4) | MASK_R(in[5],  0xf0, 4); SIGN_EXTEND_8(*p);  SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[1],  0x03, 4) | MASK_R(in[4],  0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[0],  0x1f, 6) | MASK_R(in[1],  0xfc, 2); SIGN_EXTEND_11(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[14], 0x3f, 2) | MASK_R(in[15], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[11], 0x0f, 2) | MASK_R(in[14], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[10], 0x3f, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_10(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[7],  0x0f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[6],  0x0f, 4) | MASK_R(in[7],  0xf0, 4); SIGN_EXTEND_8(*p);  SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[3],  0x03, 4) | MASK_R(in[6],  0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[2],  0x1f, 6) | MASK_R(in[3],  0xfc, 2); SIGN_EXTEND_11(*p); SHIFT_ROUND_12(*p);

            // 2 linears

            TWEAK_2(out);

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_24(out);

            return;

        case 24: /* 111.100. */

            *p     = MASK_L(in[13], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[12], 0x3f, 2) | MASK_R(in[13], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[9],  0x0f, 2) | MASK_R(in[12], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[8],  0x3f, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_10(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[5],  0x0f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[4],  0x0f, 4) | MASK_R(in[5],  0xf0, 4); SIGN_EXTEND_8(*p);  SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[1],  0x03, 4) | MASK_R(in[4],  0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[0],  0x1f, 6) | MASK_R(in[1],  0xfc, 2); SIGN_EXTEND_11(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[14], 0x3f, 2) | MASK_R(in[15], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[11], 0x0f, 2) | MASK_R(in[14], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[10], 0x3f, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_10(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[7],  0x0f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[6],  0x0f, 4) | MASK_R(in[7],  0xf0, 4); SIGN_EXTEND_8(*p);  SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[3],  0x03, 4) | MASK_R(in[6],  0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[2],  0x1f, 6) | MASK_R(in[3],  0xfc, 2); SIGN_EXTEND_11(*p); SHIFT_ROUND_13(*p);

            // 2 linears

            TWEAK_2(out);

            INTERPOLATE_2(d0, out);

            PREVENT_OVERFLOW_24(out);

            return;

        /*=====================================================================
            PATTERN F
            pppp8888 88887777 ppppgggg ggggffff
            77766666 66655555 fffeeeee eeeddddd
            55444444 44333333 ddcccccc ccbbbbbb
            32222222 21111111 baaaaaaa a9999999
        */

        case 25: /* 11101010 */

            *p     = MASK_L(in[13], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[12], 0x7f, 1) | MASK_R(in[13], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[9],  0x3f, 1) | MASK_R(in[12], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[8],  0x3f, 2) | MASK_R(in[9],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[5],  0x1f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[4],  0x1f, 3) | MASK_R(in[5],  0xe0, 5); SIGN_EXTEND_8(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[1],  0x0f, 3) | MASK_R(in[4],  0xe0, 5); SIGN_EXTEND_7(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[0],  0x0f, 4) | MASK_R(in[1],  0xf0, 4); SIGN_EXTEND_8(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[15], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[14], 0x7f, 1) | MASK_R(in[15], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[11], 0x3f, 1) | MASK_R(in[14], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[10], 0x3f, 2) | MASK_R(in[11], 0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[7],  0x1f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[6],  0x1f, 3) | MASK_R(in[7],  0xe0, 5); SIGN_EXTEND_8(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[3],  0x0f, 3) | MASK_R(in[6],  0xe0, 5); SIGN_EXTEND_7(*p); SHIFT_ROUND_12(*p);
            *(++p) = MASK_L(in[2],  0x0f, 4) | MASK_R(in[3],  0xf0, 4); SIGN_EXTEND_8(*p); SHIFT_ROUND_12(*p);

            // 8 linears

            TWEAK_8(out);

            INTERPOLATE_8(d0, out);

            //PREVENT_OVERFLOW_24(out);

            return;

        case 26: /* 11101011 */

            *p     = MASK_L(in[13], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[12], 0x7f, 1) | MASK_R(in[13], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[9],  0x3f, 1) | MASK_R(in[12], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[8],  0x3f, 2) | MASK_R(in[9],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[5],  0x1f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[4],  0x1f, 3) | MASK_R(in[5],  0xe0, 5); SIGN_EXTEND_8(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[1],  0x0f, 3) | MASK_R(in[4],  0xe0, 5); SIGN_EXTEND_7(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[0],  0x0f, 4) | MASK_R(in[1],  0xf0, 4); SIGN_EXTEND_8(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[15], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[14], 0x7f, 1) | MASK_R(in[15], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[11], 0x3f, 1) | MASK_R(in[14], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[10], 0x3f, 2) | MASK_R(in[11], 0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[7],  0x1f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[6],  0x1f, 3) | MASK_R(in[7],  0xe0, 5); SIGN_EXTEND_8(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[3],  0x0f, 3) | MASK_R(in[6],  0xe0, 5); SIGN_EXTEND_7(*p); SHIFT_ROUND_13(*p);
            *(++p) = MASK_L(in[2],  0x0f, 4) | MASK_R(in[3],  0xf0, 4); SIGN_EXTEND_8(*p); SHIFT_ROUND_13(*p);

            // 8 linears

            TWEAK_8(out);

            INTERPOLATE_8(d0, out);

            //PREVENT_OVERFLOW_24(out);

            return;

        case 27: /* 11101100 */

            *p     = MASK_L(in[13], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[12], 0x7f, 1) | MASK_R(in[13], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[9],  0x3f, 1) | MASK_R(in[12], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[8],  0x3f, 2) | MASK_R(in[9],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[5],  0x1f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[4],  0x1f, 3) | MASK_R(in[5],  0xe0, 5); SIGN_EXTEND_8(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[1],  0x0f, 3) | MASK_R(in[4],  0xe0, 5); SIGN_EXTEND_7(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[0],  0x0f, 4) | MASK_R(in[1],  0xf0, 4); SIGN_EXTEND_8(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[15], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[14], 0x7f, 1) | MASK_R(in[15], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[11], 0x3f, 1) | MASK_R(in[14], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[10], 0x3f, 2) | MASK_R(in[11], 0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[7],  0x1f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[6],  0x1f, 3) | MASK_R(in[7],  0xe0, 5); SIGN_EXTEND_8(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[3],  0x0f, 3) | MASK_R(in[6],  0xe0, 5); SIGN_EXTEND_7(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[2],  0x0f, 4) | MASK_R(in[3],  0xf0, 4); SIGN_EXTEND_8(*p); SHIFT_ROUND_14(*p);

            // 8 linears

            TWEAK_8(out);

            INTERPOLATE_8(d0, out);

            //PREVENT_OVERFLOW_24(out);

            return;

        case 28: /* 11101101 */

            *p     = MASK_L(in[13], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_15(*p);
            *(++p) = MASK_L(in[12], 0x7f, 1) | MASK_R(in[13], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_15(*p);
            *(++p) = MASK_L(in[9],  0x3f, 1) | MASK_R(in[12], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_15(*p);
            *(++p) = MASK_L(in[8],  0x3f, 2) | MASK_R(in[9],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_15(*p);
            *(++p) = MASK_L(in[5],  0x1f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_15(*p);
            *(++p) = MASK_L(in[4],  0x1f, 3) | MASK_R(in[5],  0xe0, 5); SIGN_EXTEND_8(*p); SHIFT_ROUND_15(*p);
            *(++p) = MASK_L(in[1],  0x0f, 3) | MASK_R(in[4],  0xe0, 5); SIGN_EXTEND_7(*p); SHIFT_ROUND_15(*p);
            *(++p) = MASK_L(in[0],  0x0f, 4) | MASK_R(in[1],  0xf0, 4); SIGN_EXTEND_8(*p); SHIFT_ROUND_15(*p);
            *(++p) = MASK_L(in[15], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_15(*p);
            *(++p) = MASK_L(in[14], 0x7f, 1) | MASK_R(in[15], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_15(*p);
            *(++p) = MASK_L(in[11], 0x3f, 1) | MASK_R(in[14], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_15(*p);
            *(++p) = MASK_L(in[10], 0x3f, 2) | MASK_R(in[11], 0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_15(*p);
            *(++p) = MASK_L(in[7],  0x1f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_15(*p);
            *(++p) = MASK_L(in[6],  0x1f, 3) | MASK_R(in[7],  0xe0, 5); SIGN_EXTEND_8(*p); SHIFT_ROUND_15(*p);
            *(++p) = MASK_L(in[3],  0x0f, 3) | MASK_R(in[6],  0xe0, 5); SIGN_EXTEND_7(*p); SHIFT_ROUND_15(*p);
            *(++p) = MASK_L(in[2],  0x0f, 4) | MASK_R(in[3],  0xf0, 4); SIGN_EXTEND_8(*p); SHIFT_ROUND_15(*p);

            // 8 linears

            TWEAK_8(out);

            INTERPOLATE_8(d0, out);

            //PREVENT_OVERFLOW_24(out);

            return;

        case 29: /* 11101110 */

            *p     = MASK_L(in[13], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[12], 0x7f, 1) | MASK_R(in[13], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[9],  0x3f, 1) | MASK_R(in[12], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[8],  0x3f, 2) | MASK_R(in[9],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[5],  0x1f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[4],  0x1f, 3) | MASK_R(in[5],  0xe0, 5); SIGN_EXTEND_8(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[1],  0x0f, 3) | MASK_R(in[4],  0xe0, 5); SIGN_EXTEND_7(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[0],  0x0f, 4) | MASK_R(in[1],  0xf0, 4); SIGN_EXTEND_8(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[15], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[14], 0x7f, 1) | MASK_R(in[15], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[11], 0x3f, 1) | MASK_R(in[14], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[10], 0x3f, 2) | MASK_R(in[11], 0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[7],  0x1f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[6],  0x1f, 3) | MASK_R(in[7],  0xe0, 5); SIGN_EXTEND_8(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[3],  0x0f, 3) | MASK_R(in[6],  0xe0, 5); SIGN_EXTEND_7(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[2],  0x0f, 4) | MASK_R(in[3],  0xf0, 4); SIGN_EXTEND_8(*p); SHIFT_ROUND_16(*p);

            // 8 linears

            TWEAK_8(out);

            INTERPOLATE_8(d0, out);

            PREVENT_OVERFLOW_24(out);

            return;

        case 30: /* 11101111 */

#ifdef DANIELO_DOUBLE
            *p     = MASK_L(in[13], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_17(*p);
            *(++p) = MASK_L(in[12], 0x7f, 1) | MASK_R(in[13], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[9],  0x3f, 1) | MASK_R(in[12], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_17(*p);
            *(++p) = MASK_L(in[8],  0x3f, 2) | MASK_R(in[9],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[5],  0x1f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_17(*p);
            *(++p) = MASK_L(in[4],  0x1f, 3) | MASK_R(in[5],  0xe0, 5); SIGN_EXTEND_8(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[1],  0x0f, 3) | MASK_R(in[4],  0xe0, 5); SIGN_EXTEND_7(*p); SHIFT_ROUND_17(*p);
            *(++p) = MASK_L(in[0],  0x0f, 4) | MASK_R(in[1],  0xf0, 4); SIGN_EXTEND_8(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[15], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_17(*p);
            *(++p) = MASK_L(in[14], 0x7f, 1) | MASK_R(in[15], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[11], 0x3f, 1) | MASK_R(in[14], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_17(*p);
            *(++p) = MASK_L(in[10], 0x3f, 2) | MASK_R(in[11], 0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[7],  0x1f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_17(*p);
            *(++p) = MASK_L(in[6],  0x1f, 3) | MASK_R(in[7],  0xe0, 5); SIGN_EXTEND_8(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[3],  0x0f, 3) | MASK_R(in[6],  0xe0, 5); SIGN_EXTEND_7(*p); SHIFT_ROUND_17(*p);
            *(++p) = MASK_L(in[2],  0x0f, 4) | MASK_R(in[3],  0xf0, 4); SIGN_EXTEND_8(*p); SHIFT_ROUND_16(*p);

            // 16 linears - but odd samples are

            TWEAK_16(out);
#else
            *p     = MASK_L(in[13], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[12], 0x7f, 1) | MASK_R(in[13], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[9],  0x3f, 1) | MASK_R(in[12], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[8],  0x3f, 2) | MASK_R(in[9],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[5],  0x1f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[4],  0x1f, 3) | MASK_R(in[5],  0xe0, 5); SIGN_EXTEND_8(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[1],  0x0f, 3) | MASK_R(in[4],  0xe0, 5); SIGN_EXTEND_7(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[0],  0x0f, 4) | MASK_R(in[1],  0xf0, 4); SIGN_EXTEND_8(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[15], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[14], 0x7f, 1) | MASK_R(in[15], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[11], 0x3f, 1) | MASK_R(in[14], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[10], 0x3f, 2) | MASK_R(in[11], 0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[7],  0x1f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[6],  0x1f, 3) | MASK_R(in[7],  0xe0, 5); SIGN_EXTEND_8(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[3],  0x0f, 3) | MASK_R(in[6],  0xe0, 5); SIGN_EXTEND_7(*p); SHIFT_ROUND_16(*p);
            *(++p) = MASK_L(in[2],  0x0f, 4) | MASK_R(in[3],  0xf0, 4); SIGN_EXTEND_8(*p); SHIFT_ROUND_16(*p);

            // 16 linears - but odd samples are doubled

            DOUBLE_ODDS(out);

            TWEAK_16(out);
#endif

            PREVENT_OVERFLOW_24(out);

            return;

        /*=====================================================================
            PATTERN E
            pppp8888 88888877 ppppgggg ggggggff
            77776666 66665555 ffffeeee eeeedddd
            55444444 44443333 ddcccccc ccccbbbb
            33222222 22111111 bbaaaaaa aa999999
        */

        case 31: /* 11111010 */

            *p     = MASK_L(in[13], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[12], 0x3f, 2) | MASK_R(in[13], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[9],  0x0f, 2) | MASK_R(in[12], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[8],  0x3f, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_10(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[5],  0x0f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[4],  0x0f, 4) | MASK_R(in[5],  0xf0, 4); SIGN_EXTEND_8(*p);  SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[1],  0x03, 4) | MASK_R(in[4],  0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[0],  0x0f, 6) | MASK_R(in[1],  0xfc, 2); SIGN_EXTEND_10(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[14], 0x3f, 2) | MASK_R(in[15], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[11], 0x0f, 2) | MASK_R(in[14], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[10], 0x3f, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_10(*p); SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[7],  0x0f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[6],  0x0f, 4) | MASK_R(in[7],  0xf0, 4); SIGN_EXTEND_8(*p);  SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[3],  0x03, 4) | MASK_R(in[6],  0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_14(*p);
            *(++p) = MASK_L(in[2],  0x0f, 6) | MASK_R(in[3],  0xfc, 2); SIGN_EXTEND_10(*p); SHIFT_ROUND_14(*p);

            // 4 Linears

            TWEAK_4(out);

            INTERPOLATE_4(d0, out);

            PREVENT_OVERFLOW_24(out);

            return;

        /*=====================================================================
            PATTERN B4
            pppp8888 88888887 ppppgggg gggggggf
            77777766 66666555 ffffffee eeeeeddd
            55554444 44433333 ddddcccc cccbbbbb
            33222222 21111111 bbaaaaaa a9999999
        */

        case 32: /* 11111011 */

            *p     = MASK_L(in[13], 0x7f, 0);                           SIGN_EXTEND_7(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[12], 0x3f, 1) | MASK_R(in[13], 0x80, 7); SIGN_EXTEND_7(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[9],  0x1f, 2) | MASK_R(in[12], 0xc0, 6); SIGN_EXTEND_7(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[8],  0x0f, 3) | MASK_R(in[9],  0xe0, 5); SIGN_EXTEND_7(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[5],  0x07, 4) | MASK_R(in[8],  0xf0, 4); SIGN_EXTEND_7(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[4],  0x03, 5) | MASK_R(in[5],  0xf8, 3); SIGN_EXTEND_7(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[1],  0x01, 6) | MASK_R(in[4],  0xfc, 2); SIGN_EXTEND_7(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[0],  0x0f, 7) | MASK_R(in[1],  0xfe, 1); SIGN_EXTEND_11(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[15], 0x7f, 0);                           SIGN_EXTEND_7(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[14], 0x3f, 1) | MASK_R(in[15], 0x80, 7); SIGN_EXTEND_7(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[11], 0x1f, 2) | MASK_R(in[14], 0xc0, 6); SIGN_EXTEND_7(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[10], 0x0f, 3) | MASK_R(in[11], 0xe0, 5); SIGN_EXTEND_7(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[7],  0x07, 4) | MASK_R(in[10], 0xf0, 4); SIGN_EXTEND_7(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[6],  0x03, 5) | MASK_R(in[7],  0xf8, 3); SIGN_EXTEND_7(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[3],  0x01, 6) | MASK_R(in[6],  0xfc, 2); SIGN_EXTEND_7(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[2],  0x0f, 7) | MASK_R(in[3],  0xfe, 1); SIGN_EXTEND_11(*p); SHIFT_ROUND_4(*p);

            // 2 linears

            TWEAK_2(out);

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_24(out);

            return;

        case 33: /* 11111100 */

            *p     = MASK_L(in[13], 0x7f, 0);                           SIGN_EXTEND_7(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[12], 0x3f, 1) | MASK_R(in[13], 0x80, 7); SIGN_EXTEND_7(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[9],  0x1f, 2) | MASK_R(in[12], 0xc0, 6); SIGN_EXTEND_7(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[8],  0x0f, 3) | MASK_R(in[9],  0xe0, 5); SIGN_EXTEND_7(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[5],  0x07, 4) | MASK_R(in[8],  0xf0, 4); SIGN_EXTEND_7(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[4],  0x03, 5) | MASK_R(in[5],  0xf8, 3); SIGN_EXTEND_7(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[1],  0x01, 6) | MASK_R(in[4],  0xfc, 2); SIGN_EXTEND_7(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[0],  0x0f, 7) | MASK_R(in[1],  0xfe, 1); SIGN_EXTEND_11(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[15], 0x7f, 0);                           SIGN_EXTEND_7(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[14], 0x3f, 1) | MASK_R(in[15], 0x80, 7); SIGN_EXTEND_7(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[11], 0x1f, 2) | MASK_R(in[14], 0xc0, 6); SIGN_EXTEND_7(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[10], 0x0f, 3) | MASK_R(in[11], 0xe0, 5); SIGN_EXTEND_7(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[7],  0x07, 4) | MASK_R(in[10], 0xf0, 4); SIGN_EXTEND_7(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[6],  0x03, 5) | MASK_R(in[7],  0xf8, 3); SIGN_EXTEND_7(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[3],  0x01, 6) | MASK_R(in[6],  0xfc, 2); SIGN_EXTEND_7(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[2],  0x0f, 7) | MASK_R(in[3],  0xfe, 1); SIGN_EXTEND_11(*p); SHIFT_ROUND_5(*p);

            // 2 linears

            TWEAK_2(out);

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_24(out);

            return;

        case 34: /* 11111101 */

            *p     = MASK_L(in[13], 0x7f, 0);                           SIGN_EXTEND_7(*p);
            *(++p) = MASK_L(in[12], 0x3f, 1) | MASK_R(in[13], 0x80, 7); SIGN_EXTEND_7(*p);
            *(++p) = MASK_L(in[9],  0x1f, 2) | MASK_R(in[12], 0xc0, 6); SIGN_EXTEND_7(*p);
            *(++p) = MASK_L(in[8],  0x0f, 3) | MASK_R(in[9],  0xe0, 5); SIGN_EXTEND_7(*p);
            *(++p) = MASK_L(in[5],  0x07, 4) | MASK_R(in[8],  0xf0, 4); SIGN_EXTEND_7(*p);
            *(++p) = MASK_L(in[4],  0x03, 5) | MASK_R(in[5],  0xf8, 3); SIGN_EXTEND_7(*p);
            *(++p) = MASK_L(in[1],  0x01, 6) | MASK_R(in[4],  0xfc, 2); SIGN_EXTEND_7(*p);
            *(++p) = MASK_L(in[0],  0x0f, 7) | MASK_R(in[1],  0xfe, 1); SIGN_EXTEND_11(*p);
            *(++p) = MASK_L(in[15], 0x7f, 0);                           SIGN_EXTEND_7(*p);
            *(++p) = MASK_L(in[14], 0x3f, 1) | MASK_R(in[15], 0x80, 7); SIGN_EXTEND_7(*p);
            *(++p) = MASK_L(in[11], 0x1f, 2) | MASK_R(in[14], 0xc0, 6); SIGN_EXTEND_7(*p);
            *(++p) = MASK_L(in[10], 0x0f, 3) | MASK_R(in[11], 0xe0, 5); SIGN_EXTEND_7(*p);
            *(++p) = MASK_L(in[7],  0x07, 4) | MASK_R(in[10], 0xf0, 4); SIGN_EXTEND_7(*p);
            *(++p) = MASK_L(in[6],  0x03, 5) | MASK_R(in[7],  0xf8, 3); SIGN_EXTEND_7(*p);
            *(++p) = MASK_L(in[3],  0x01, 6) | MASK_R(in[6],  0xfc, 2); SIGN_EXTEND_7(*p);
            *(++p) = MASK_L(in[2],  0x0f, 7) | MASK_R(in[3],  0xfe, 1); SIGN_EXTEND_11(*p);

            // 2 linears

            TWEAK_2(out);

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_24(out);

            return;

        case 35: /* 11111110 */

            *p     = MASK_L(in[13], 0x7f, 0);                           SIGN_EXTEND_7(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[12], 0x3f, 1) | MASK_R(in[13], 0x80, 7); SIGN_EXTEND_7(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[9],  0x1f, 2) | MASK_R(in[12], 0xc0, 6); SIGN_EXTEND_7(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[8],  0x0f, 3) | MASK_R(in[9],  0xe0, 5); SIGN_EXTEND_7(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[5],  0x07, 4) | MASK_R(in[8],  0xf0, 4); SIGN_EXTEND_7(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[4],  0x03, 5) | MASK_R(in[5],  0xf8, 3); SIGN_EXTEND_7(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[1],  0x01, 6) | MASK_R(in[4],  0xfc, 2); SIGN_EXTEND_7(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[0],  0x0f, 7) | MASK_R(in[1],  0xfe, 1); SIGN_EXTEND_11(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[15], 0x7f, 0);                           SIGN_EXTEND_7(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[14], 0x3f, 1) | MASK_R(in[15], 0x80, 7); SIGN_EXTEND_7(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[11], 0x1f, 2) | MASK_R(in[14], 0xc0, 6); SIGN_EXTEND_7(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[10], 0x0f, 3) | MASK_R(in[11], 0xe0, 5); SIGN_EXTEND_7(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[7],  0x07, 4) | MASK_R(in[10], 0xf0, 4); SIGN_EXTEND_7(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[6],  0x03, 5) | MASK_R(in[7],  0xf8, 3); SIGN_EXTEND_7(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[3],  0x01, 6) | MASK_R(in[6],  0xfc, 2); SIGN_EXTEND_7(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[2],  0x0f, 7) | MASK_R(in[3],  0xfe, 1); SIGN_EXTEND_11(*p); SHIFT_ROUND_2(*p);

            // 2 linears

            TWEAK_2(out);

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_24(out);

            return;

        case 36: /* 11111111 */

            *p     = MASK_L(in[13], 0x7f, 0);                           SIGN_EXTEND_7(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[12], 0x3f, 1) | MASK_R(in[13], 0x80, 7); SIGN_EXTEND_7(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[9],  0x1f, 2) | MASK_R(in[12], 0xc0, 6); SIGN_EXTEND_7(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[8],  0x0f, 3) | MASK_R(in[9],  0xe0, 5); SIGN_EXTEND_7(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[5],  0x07, 4) | MASK_R(in[8],  0xf0, 4); SIGN_EXTEND_7(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[4],  0x03, 5) | MASK_R(in[5],  0xf8, 3); SIGN_EXTEND_7(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[1],  0x01, 6) | MASK_R(in[4],  0xfc, 2); SIGN_EXTEND_7(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[0],  0x0f, 7) | MASK_R(in[1],  0xfe, 1); SIGN_EXTEND_11(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[15], 0x7f, 0);                           SIGN_EXTEND_7(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[14], 0x3f, 1) | MASK_R(in[15], 0x80, 7); SIGN_EXTEND_7(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[11], 0x1f, 2) | MASK_R(in[14], 0xc0, 6); SIGN_EXTEND_7(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[10], 0x0f, 3) | MASK_R(in[11], 0xe0, 5); SIGN_EXTEND_7(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[7],  0x07, 4) | MASK_R(in[10], 0xf0, 4); SIGN_EXTEND_7(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[6],  0x03, 5) | MASK_R(in[7],  0xf8, 3); SIGN_EXTEND_7(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[3],  0x01, 6) | MASK_R(in[6],  0xfc, 2); SIGN_EXTEND_7(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[2],  0x0f, 7) | MASK_R(in[3],  0xfe, 1); SIGN_EXTEND_11(*p); SHIFT_ROUND_3(*p);

            // 2 linears

            TWEAK_2(out);

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_24(out);

            return;

        default: break;
    }
}


//*****************************************************************************
void decodeMT1(int d0, unsigned char *in, int *out)
{
    // Decodes a 16-byte MT1 RDAC block into 16-bit samples

    int patternIndex = MASK_L(in[0], 0xf0, 0) | MASK_R(in[2], 0xf0, 4);

    int pattern = patterns[patternIndex];

    int *p = out;

    switch (pattern) {

        /*=====================================================================
            PATTERN B
            pp888888 88888887 ppgggggg gggggggf
            77777666 66666555 fffffeee eeeeeddd
            55544444 44443333 dddccccc ccccbbbb
            33222222 22111111 bbaaaaaa aa999999
        */

        case 0:  /* 00..00.. */

            /* Unknown - never occurs? */
            ZERO(out);

            //printf("Unknown pattern: %d\n", pattern);

            return;

        case 1:  /* 00..01.. */

            /* Unknown - never occurs? */
            ZERO(out);

            //printf("Unknown pattern: %d\n", pattern);

            return;

        case 2:  /* 00..10.. */

            *p     = MASK_L(in[13], 0x3f, 0);                           SIGN_EXTEND_6(*p);
            *(++p) = MASK_L(in[12], 0x3f, 2) | MASK_R(in[13], 0xc0, 6); SIGN_EXTEND_8(*p);
            *(++p) = MASK_L(in[9],  0x0f, 2) | MASK_R(in[12], 0xc0, 6); SIGN_EXTEND_6(*p);
            *(++p) = MASK_L(in[8],  0x1f, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_9(*p);
            *(++p) = MASK_L(in[5],  0x07, 3) | MASK_R(in[8],  0xe0, 5); SIGN_EXTEND_6(*p);
            *(++p) = MASK_L(in[4],  0x07, 5) | MASK_R(in[5],  0xf8, 3); SIGN_EXTEND_8(*p);
            *(++p) = MASK_L(in[1],  0x01, 5) | MASK_R(in[4],  0xf8, 3); SIGN_EXTEND_6(*p);
            *(++p) = MASK_L(in[0],  0x3f, 7) | MASK_R(in[1],  0xfe, 1); SIGN_EXTEND_13(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                           SIGN_EXTEND_6(*p);
            *(++p) = MASK_L(in[14], 0x3f, 2) | MASK_R(in[15], 0xc0, 6); SIGN_EXTEND_8(*p);
            *(++p) = MASK_L(in[11], 0x0f, 2) | MASK_R(in[14], 0xc0, 6); SIGN_EXTEND_6(*p);
            *(++p) = MASK_L(in[10], 0x1f, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_9(*p);
            *(++p) = MASK_L(in[7],  0x07, 3) | MASK_R(in[10], 0xe0, 5); SIGN_EXTEND_6(*p);
            *(++p) = MASK_L(in[6],  0x07, 5) | MASK_R(in[7],  0xf8, 3); SIGN_EXTEND_8(*p);
            *(++p) = MASK_L(in[3],  0x01, 5) | MASK_R(in[6],  0xf8, 3); SIGN_EXTEND_6(*p);
            *(++p) = MASK_L(in[2],  0x3f, 7) | MASK_R(in[3],  0xfe, 1); SIGN_EXTEND_13(*p);

            // 2 linears

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 3:  /* 00..11.. */

            *p     = MASK_L(in[13], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[12], 0x3f, 2) | MASK_R(in[13], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[9],  0x0f, 2) | MASK_R(in[12], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[8],  0x1f, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_9(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[5],  0x07, 3) | MASK_R(in[8],  0xe0, 5); SIGN_EXTEND_6(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[4],  0x07, 5) | MASK_R(in[5],  0xf8, 3); SIGN_EXTEND_8(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[1],  0x01, 5) | MASK_R(in[4],  0xf8, 3); SIGN_EXTEND_6(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[0],  0x3f, 7) | MASK_R(in[1],  0xfe, 1); SIGN_EXTEND_13(*p); SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[14], 0x3f, 2) | MASK_R(in[15], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[11], 0x0f, 2) | MASK_R(in[14], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[10], 0x1f, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_9(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[7],  0x07, 3) | MASK_R(in[10], 0xe0, 5); SIGN_EXTEND_6(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[6],  0x07, 5) | MASK_R(in[7],  0xf8, 3); SIGN_EXTEND_8(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[3],  0x01, 5) | MASK_R(in[6],  0xf8, 3); SIGN_EXTEND_6(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[2],  0x3f, 7) | MASK_R(in[3],  0xfe, 1); SIGN_EXTEND_13(*p); SHIFT_ROUND_1(*p);

            // 2 linears

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 4:  /* 01..00.. */

            *p     = MASK_L(in[13], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[12], 0x3f, 2) | MASK_R(in[13], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[9],  0x0f, 2) | MASK_R(in[12], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[8],  0x1f, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_9(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[5],  0x07, 3) | MASK_R(in[8],  0xe0, 5); SIGN_EXTEND_6(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[4],  0x07, 5) | MASK_R(in[5],  0xf8, 3); SIGN_EXTEND_8(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[1],  0x01, 5) | MASK_R(in[4],  0xf8, 3); SIGN_EXTEND_6(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[0],  0x3f, 7) | MASK_R(in[1],  0xfe, 1); SIGN_EXTEND_13(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[14], 0x3f, 2) | MASK_R(in[15], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[11], 0x0f, 2) | MASK_R(in[14], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[10], 0x1f, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_9(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[7],  0x07, 3) | MASK_R(in[10], 0xe0, 5); SIGN_EXTEND_6(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[6],  0x07, 5) | MASK_R(in[7],  0xf8, 3); SIGN_EXTEND_8(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[3],  0x01, 5) | MASK_R(in[6],  0xf8, 3); SIGN_EXTEND_6(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[2],  0x3f, 7) | MASK_R(in[3],  0xfe, 1); SIGN_EXTEND_13(*p); SHIFT_ROUND_2(*p);

            // 2 linears

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 5:  /* 01..01.. */

            *p     = MASK_L(in[13], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[12], 0x3f, 2) | MASK_R(in[13], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[9],  0x0f, 2) | MASK_R(in[12], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[8],  0x1f, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_9(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[5],  0x07, 3) | MASK_R(in[8],  0xe0, 5); SIGN_EXTEND_6(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[4],  0x07, 5) | MASK_R(in[5],  0xf8, 3); SIGN_EXTEND_8(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[1],  0x01, 5) | MASK_R(in[4],  0xf8, 3); SIGN_EXTEND_6(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[0],  0x3f, 7) | MASK_R(in[1],  0xfe, 1); SIGN_EXTEND_13(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[14], 0x3f, 2) | MASK_R(in[15], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[11], 0x0f, 2) | MASK_R(in[14], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[10], 0x1f, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_9(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[7],  0x07, 3) | MASK_R(in[10], 0xe0, 5); SIGN_EXTEND_6(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[6],  0x07, 5) | MASK_R(in[7],  0xf8, 3); SIGN_EXTEND_8(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[3],  0x01, 5) | MASK_R(in[6],  0xf8, 3); SIGN_EXTEND_6(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[2],  0x3f, 7) | MASK_R(in[3],  0xfe, 1); SIGN_EXTEND_13(*p); SHIFT_ROUND_3(*p);

            // 2 linears

            INTERPOLATE_2(d0, out);

            PREVENT_OVERFLOW_16(out);

            return;

        /*=====================================================================
            PATTERN D
            pp888888 88877777 ppgggggg gggfffff
            77666666 66555555 ffeeeeee eedddddd
            54444444 44333333 dccccccc ccbbbbbb
            32222222 21111111 baaaaaaa a9999999
        */

        case 6:  /* 01..10.. */

            *p     = MASK_L(in[13], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[12], 0x7f, 1) | MASK_R(in[13], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[9],  0x3f, 1) | MASK_R(in[12], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[8],  0x7f, 2) | MASK_R(in[9],  0xc0, 6); SIGN_EXTEND_9(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[5],  0x3f, 1) | MASK_R(in[8],  0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[4],  0x3f, 2) | MASK_R(in[5],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[1],  0x1f, 2) | MASK_R(in[4],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[0],  0x3f, 3) | MASK_R(in[1],  0xe0, 5); SIGN_EXTEND_9(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[15], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[14], 0x7f, 1) | MASK_R(in[15], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[11], 0x3f, 1) | MASK_R(in[14], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[10], 0x7f, 2) | MASK_R(in[11], 0xc0, 6); SIGN_EXTEND_9(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[7],  0x3f, 1) | MASK_R(in[10], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[6],  0x3f, 2) | MASK_R(in[7],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[3],  0x1f, 2) | MASK_R(in[6],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[2],  0x3f, 3) | MASK_R(in[3],  0xe0, 5); SIGN_EXTEND_9(*p); SHIFT_ROUND_2(*p);

            // 4 linears

            INTERPOLATE_4(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 7:  /* 01..11.. */

            *p     = MASK_L(in[13], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[12], 0x7f, 1) | MASK_R(in[13], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[9],  0x3f, 1) | MASK_R(in[12], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[8],  0x7f, 2) | MASK_R(in[9],  0xc0, 6); SIGN_EXTEND_9(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[5],  0x3f, 1) | MASK_R(in[8],  0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[4],  0x3f, 2) | MASK_R(in[5],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[1],  0x1f, 2) | MASK_R(in[4],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[0],  0x3f, 3) | MASK_R(in[1],  0xe0, 5); SIGN_EXTEND_9(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[15], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[14], 0x7f, 1) | MASK_R(in[15], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[11], 0x3f, 1) | MASK_R(in[14], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[10], 0x7f, 2) | MASK_R(in[11], 0xc0, 6); SIGN_EXTEND_9(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[7],  0x3f, 1) | MASK_R(in[10], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[6],  0x3f, 2) | MASK_R(in[7],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[3],  0x1f, 2) | MASK_R(in[6],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[2],  0x3f, 3) | MASK_R(in[3],  0xe0, 5); SIGN_EXTEND_9(*p); SHIFT_ROUND_3(*p);

            // 4 linears

            INTERPOLATE_4(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 8:  /* 10..00.. */

            *p     = MASK_L(in[13], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[12], 0x7f, 1) | MASK_R(in[13], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[9],  0x3f, 1) | MASK_R(in[12], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[8],  0x7f, 2) | MASK_R(in[9],  0xc0, 6); SIGN_EXTEND_9(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[5],  0x3f, 1) | MASK_R(in[8],  0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[4],  0x3f, 2) | MASK_R(in[5],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[1],  0x1f, 2) | MASK_R(in[4],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[0],  0x3f, 3) | MASK_R(in[1],  0xe0, 5); SIGN_EXTEND_9(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[15], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[14], 0x7f, 1) | MASK_R(in[15], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[11], 0x3f, 1) | MASK_R(in[14], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[10], 0x7f, 2) | MASK_R(in[11], 0xc0, 6); SIGN_EXTEND_9(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[7],  0x3f, 1) | MASK_R(in[10], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[6],  0x3f, 2) | MASK_R(in[7],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[3],  0x1f, 2) | MASK_R(in[6],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[2],  0x3f, 3) | MASK_R(in[3],  0xe0, 5); SIGN_EXTEND_9(*p); SHIFT_ROUND_4(*p);

            // 4 linears

            INTERPOLATE_4(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 9:  /* 10..01.. */

            *p     = MASK_L(in[13], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[12], 0x7f, 1) | MASK_R(in[13], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[9],  0x3f, 1) | MASK_R(in[12], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[8],  0x7f, 2) | MASK_R(in[9],  0xc0, 6); SIGN_EXTEND_9(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[5],  0x3f, 1) | MASK_R(in[8],  0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[4],  0x3f, 2) | MASK_R(in[5],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[1],  0x1f, 2) | MASK_R(in[4],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[0],  0x3f, 3) | MASK_R(in[1],  0xe0, 5); SIGN_EXTEND_9(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[15], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[14], 0x7f, 1) | MASK_R(in[15], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[11], 0x3f, 1) | MASK_R(in[14], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[10], 0x7f, 2) | MASK_R(in[11], 0xc0, 6); SIGN_EXTEND_9(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[7],  0x3f, 1) | MASK_R(in[10], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[6],  0x3f, 2) | MASK_R(in[7],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[3],  0x1f, 2) | MASK_R(in[6],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[2],  0x3f, 3) | MASK_R(in[3],  0xe0, 5); SIGN_EXTEND_9(*p); SHIFT_ROUND_5(*p);

            // 4 linears

            INTERPOLATE_4(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 10: /* 10..10.. */

            *p     = MASK_L(in[13], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[12], 0x7f, 1) | MASK_R(in[13], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[9],  0x3f, 1) | MASK_R(in[12], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[8],  0x7f, 2) | MASK_R(in[9],  0xc0, 6); SIGN_EXTEND_9(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[5],  0x3f, 1) | MASK_R(in[8],  0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[4],  0x3f, 2) | MASK_R(in[5],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[1],  0x1f, 2) | MASK_R(in[4],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[0],  0x3f, 3) | MASK_R(in[1],  0xe0, 5); SIGN_EXTEND_9(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[15], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[14], 0x7f, 1) | MASK_R(in[15], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[11], 0x3f, 1) | MASK_R(in[14], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[10], 0x7f, 2) | MASK_R(in[11], 0xc0, 6); SIGN_EXTEND_9(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[7],  0x3f, 1) | MASK_R(in[10], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[6],  0x3f, 2) | MASK_R(in[7],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[3],  0x1f, 2) | MASK_R(in[6],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[2],  0x3f, 3) | MASK_R(in[3],  0xe0, 5); SIGN_EXTEND_9(*p); SHIFT_ROUND_6(*p);

            // 4 linears

            INTERPOLATE_4(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 11: /* 10..11.. */

            *p     = MASK_L(in[13], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[12], 0x7f, 1) | MASK_R(in[13], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[9],  0x3f, 1) | MASK_R(in[12], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[8],  0x7f, 2) | MASK_R(in[9],  0xc0, 6); SIGN_EXTEND_9(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[5],  0x3f, 1) | MASK_R(in[8],  0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[4],  0x3f, 2) | MASK_R(in[5],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[1],  0x1f, 2) | MASK_R(in[4],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[0],  0x3f, 3) | MASK_R(in[1],  0xe0, 5); SIGN_EXTEND_9(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[15], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[14], 0x7f, 1) | MASK_R(in[15], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[11], 0x3f, 1) | MASK_R(in[14], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[10], 0x7f, 2) | MASK_R(in[11], 0xc0, 6); SIGN_EXTEND_9(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[7],  0x3f, 1) | MASK_R(in[10], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[6],  0x3f, 2) | MASK_R(in[7],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[3],  0x1f, 2) | MASK_R(in[6],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[2],  0x3f, 3) | MASK_R(in[3],  0xe0, 5); SIGN_EXTEND_9(*p); SHIFT_ROUND_7(*p);

            // 4 linears

            INTERPOLATE_4(d0, out);

            PREVENT_OVERFLOW_16(out);

            return;

        /*=====================================================================
            PATTERN A
            ppp88888 88888888 pppggggg gggggggg
            87777776 66666655 gffffffe eeeeeedd
            55554444 44444333 ddddcccc cccccbbb
            33322222 22111111 bbbaaaaa aa999999
        */

        case 12: /* 110.000. */

            /* Unknown - never occurs? */
            ZERO(out);

            //printf("Unknown pattern: %d\n", pattern);

            return;

        case 13: /* 110.001. */

            /* Unknown - never occurs? */
            ZERO(out);

            //printf("Unknown pattern: %d\n", pattern);

            return;

        case 14: /* 110.010. */

            /* Unknown - never occurs? */
            ZERO(out);

            //printf("Unknown pattern: %d\n", pattern);

            return;

        case 15: /* 110.011. */

            *p     = MASK_L(in[13], 0x3f, 0);                                                    SIGN_EXTEND_6(*p);
            *(++p) = MASK_L(in[12], 0x1f, 2) | MASK_R(in[13], 0xc0, 6);                          SIGN_EXTEND_7(*p);
            *(++p) = MASK_L(in[9],  0x07, 3) | MASK_R(in[12], 0xe0, 5);                          SIGN_EXTEND_6(*p);
            *(++p) = MASK_L(in[8],  0x0f, 5) | MASK_R(in[9],  0xf8, 3);                          SIGN_EXTEND_9(*p);
            *(++p) = MASK_L(in[5],  0x03, 4) | MASK_R(in[8],  0xf0, 4);                          SIGN_EXTEND_6(*p);
            *(++p) = MASK_L(in[4],  0x01, 6) | MASK_R(in[5],  0xfc, 2);                          SIGN_EXTEND_7(*p);
            *(++p) = MASK_R(in[4],  0x7e, 1);                                                    SIGN_EXTEND_6(*p);
            *(++p) = MASK_L(in[0],  0x1f, 9) | MASK_L(in[1],  0xff, 1) | MASK_R(in[4], 0x80, 7); SIGN_EXTEND_14(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                                                    SIGN_EXTEND_6(*p);
            *(++p) = MASK_L(in[14], 0x1f, 2) | MASK_R(in[15], 0xc0, 6);                          SIGN_EXTEND_7(*p);
            *(++p) = MASK_L(in[11], 0x07, 3) | MASK_R(in[14], 0xe0, 5);                          SIGN_EXTEND_6(*p);
            *(++p) = MASK_L(in[10], 0x0f, 5) | MASK_R(in[11], 0xf8, 3);                          SIGN_EXTEND_9(*p);
            *(++p) = MASK_L(in[7],  0x03, 4) | MASK_R(in[10], 0xf0, 4);                          SIGN_EXTEND_6(*p);
            *(++p) = MASK_L(in[6],  0x01, 6) | MASK_R(in[7],  0xfc, 2);                          SIGN_EXTEND_7(*p);
            *(++p) = MASK_R(in[6],  0x7e, 1);                                                    SIGN_EXTEND_6(*p);
            *(++p) = MASK_L(in[2],  0x1f, 9) | MASK_L(in[3],  0xff, 1) | MASK_R(in[6], 0x80, 7); SIGN_EXTEND_14(*p);

            // 2 linears

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 16: /* 110.100. */

            *p     = MASK_L(in[13], 0x3f, 0);                                                    SIGN_EXTEND_6(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[12], 0x1f, 2) | MASK_R(in[13], 0xc0, 6);                          SIGN_EXTEND_7(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[9],  0x07, 3) | MASK_R(in[12], 0xe0, 5);                          SIGN_EXTEND_6(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[8],  0x0f, 5) | MASK_R(in[9],  0xf8, 3);                          SIGN_EXTEND_9(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[5],  0x03, 4) | MASK_R(in[8],  0xf0, 4);                          SIGN_EXTEND_6(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[4],  0x01, 6) | MASK_R(in[5],  0xfc, 2);                          SIGN_EXTEND_7(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_R(in[4],  0x7e, 1);                                                    SIGN_EXTEND_6(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[0],  0x1f, 9) | MASK_L(in[1],  0xff, 1) | MASK_R(in[4], 0x80, 7); SIGN_EXTEND_14(*p); SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                                                    SIGN_EXTEND_6(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[14], 0x1f, 2) | MASK_R(in[15], 0xc0, 6);                          SIGN_EXTEND_7(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[11], 0x07, 3) | MASK_R(in[14], 0xe0, 5);                          SIGN_EXTEND_6(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[10], 0x0f, 5) | MASK_R(in[11], 0xf8, 3);                          SIGN_EXTEND_9(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[7],  0x03, 4) | MASK_R(in[10], 0xf0, 4);                          SIGN_EXTEND_6(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[6],  0x01, 6) | MASK_R(in[7],  0xfc, 2);                          SIGN_EXTEND_7(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_R(in[6],  0x7e, 1);                                                    SIGN_EXTEND_6(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[2],  0x1f, 9) | MASK_L(in[3],  0xff, 1) | MASK_R(in[6], 0x80, 7); SIGN_EXTEND_14(*p); SHIFT_ROUND_1(*p);

            // 2 linears

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 17: /* 110.101. */

            *p     = MASK_L(in[13], 0x3f, 0);                                                    SIGN_EXTEND_6(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[12], 0x1f, 2) | MASK_R(in[13], 0xc0, 6);                          SIGN_EXTEND_7(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[9],  0x07, 3) | MASK_R(in[12], 0xe0, 5);                          SIGN_EXTEND_6(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[8],  0x0f, 5) | MASK_R(in[9],  0xf8, 3);                          SIGN_EXTEND_9(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[5],  0x03, 4) | MASK_R(in[8],  0xf0, 4);                          SIGN_EXTEND_6(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[4],  0x01, 6) | MASK_R(in[5],  0xfc, 2);                          SIGN_EXTEND_7(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_R(in[4],  0x7e, 1);                                                    SIGN_EXTEND_6(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[0],  0x1f, 9) | MASK_L(in[1],  0xff, 1) | MASK_R(in[4], 0x80, 7); SIGN_EXTEND_14(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                                                    SIGN_EXTEND_6(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[14], 0x1f, 2) | MASK_R(in[15], 0xc0, 6);                          SIGN_EXTEND_7(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[11], 0x07, 3) | MASK_R(in[14], 0xe0, 5);                          SIGN_EXTEND_6(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[10], 0x0f, 5) | MASK_R(in[11], 0xf8, 3);                          SIGN_EXTEND_9(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[7],  0x03, 4) | MASK_R(in[10], 0xf0, 4);                          SIGN_EXTEND_6(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[6],  0x01, 6) | MASK_R(in[7],  0xfc, 2);                          SIGN_EXTEND_7(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_R(in[6],  0x7e, 1);                                                    SIGN_EXTEND_6(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[2],  0x1f, 9) | MASK_L(in[3],  0xff, 1) | MASK_R(in[6], 0x80, 7); SIGN_EXTEND_14(*p); SHIFT_ROUND_2(*p);

            // 2 linears

            INTERPOLATE_2(d0, out);

            PREVENT_OVERFLOW_16(out);

            return;

        /*=====================================================================
            PATTERN B3
            ppp88888 88888887 pppggggg gggggggf
            77777666 66666555 fffffeee eeeeeddd
            55544444 44443333 dddccccc ccccbbbb
            33222222 22111111 bbaaaaaa aa999999
        */

        case 18: /* 110.110. */

            *p     = MASK_L(in[13], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[12], 0x3f, 2) | MASK_R(in[13], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[9],  0x0f, 2) | MASK_R(in[12], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[8],  0x1f, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_9(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[5],  0x07, 3) | MASK_R(in[8],  0xe0, 5); SIGN_EXTEND_6(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[4],  0x07, 5) | MASK_R(in[5],  0xf8, 3); SIGN_EXTEND_8(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[1],  0x01, 5) | MASK_R(in[4],  0xf8, 3); SIGN_EXTEND_6(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[0],  0x1f, 7) | MASK_R(in[1],  0xfe, 1); SIGN_EXTEND_12(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[14], 0x3f, 2) | MASK_R(in[15], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[11], 0x0f, 2) | MASK_R(in[14], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[10], 0x1f, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_9(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[7],  0x07, 3) | MASK_R(in[10], 0xe0, 5); SIGN_EXTEND_6(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[6],  0x07, 5) | MASK_R(in[7],  0xf8, 3); SIGN_EXTEND_8(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[3],  0x01, 5) | MASK_R(in[6],  0xf8, 3); SIGN_EXTEND_6(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[2],  0x1f, 7) | MASK_R(in[3],  0xfe, 1); SIGN_EXTEND_12(*p); SHIFT_ROUND_4(*p);

            // 2 linears

            INTERPOLATE_2(d0, out);

            PREVENT_OVERFLOW_16(out);

            return;

        /*=====================================================================
            PATTERN C
            ppp88888 88888877 pppggggg ggggggff
            77776666 66665555 ffffeeee eeeedddd
            55444444 44443333 ddcccccc ccccbbbb
            33222222 22111111 bbaaaaaa aa999999
        */
        case 19: /* 110.111. */

            *p     = MASK_L(in[13], 0x3f, 0);                           SIGN_EXTEND_6(*p);
            *(++p) = MASK_L(in[12], 0x3f, 2) | MASK_R(in[13], 0xc0, 6); SIGN_EXTEND_8(*p);
            *(++p) = MASK_L(in[9],  0x0f, 2) | MASK_R(in[12], 0xc0, 6); SIGN_EXTEND_6(*p);
            *(++p) = MASK_L(in[8],  0x3f, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_10(*p);
            *(++p) = MASK_L(in[5],  0x0f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_6(*p);
            *(++p) = MASK_L(in[4],  0x0f, 4) | MASK_R(in[5],  0xf0, 4); SIGN_EXTEND_8(*p);
            *(++p) = MASK_L(in[1],  0x03, 4) | MASK_R(in[4],  0xf0, 4); SIGN_EXTEND_6(*p);
            *(++p) = MASK_L(in[0],  0x1f, 6) | MASK_R(in[1],  0xfc, 2); SIGN_EXTEND_11(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                           SIGN_EXTEND_6(*p);
            *(++p) = MASK_L(in[14], 0x3f, 2) | MASK_R(in[15], 0xc0, 6); SIGN_EXTEND_8(*p);
            *(++p) = MASK_L(in[11], 0x0f, 2) | MASK_R(in[14], 0xc0, 6); SIGN_EXTEND_6(*p);
            *(++p) = MASK_L(in[10], 0x3f, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_10(*p);
            *(++p) = MASK_L(in[7],  0x0f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_6(*p);
            *(++p) = MASK_L(in[6],  0x0f, 4) | MASK_R(in[7],  0xf0, 4); SIGN_EXTEND_8(*p);
            *(++p) = MASK_L(in[3],  0x03, 4) | MASK_R(in[6],  0xf0, 4); SIGN_EXTEND_6(*p);
            *(++p) = MASK_L(in[2],  0x1f, 6) | MASK_R(in[3],  0xfc, 2); SIGN_EXTEND_11(*p);

            // 2 linears

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 20: /* 111.000. */

            *p     = MASK_L(in[13], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[12], 0x3f, 2) | MASK_R(in[13], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[9],  0x0f, 2) | MASK_R(in[12], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[8],  0x3f, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_10(*p); SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[5],  0x0f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[4],  0x0f, 4) | MASK_R(in[5],  0xf0, 4); SIGN_EXTEND_8(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[1],  0x03, 4) | MASK_R(in[4],  0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[0],  0x1f, 6) | MASK_R(in[1],  0xfc, 2); SIGN_EXTEND_11(*p); SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[14], 0x3f, 2) | MASK_R(in[15], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[11], 0x0f, 2) | MASK_R(in[14], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[10], 0x3f, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_10(*p); SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[7],  0x0f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[6],  0x0f, 4) | MASK_R(in[7],  0xf0, 4); SIGN_EXTEND_8(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[3],  0x03, 4) | MASK_R(in[6],  0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[2],  0x1f, 6) | MASK_R(in[3],  0xfc, 2); SIGN_EXTEND_11(*p); SHIFT_ROUND_1(*p);

            // 2 linears

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 21: /* 111.001. */

            *p     = MASK_L(in[13], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[12], 0x3f, 2) | MASK_R(in[13], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[9],  0x0f, 2) | MASK_R(in[12], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[8],  0x3f, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_10(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[5],  0x0f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[4],  0x0f, 4) | MASK_R(in[5],  0xf0, 4); SIGN_EXTEND_8(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[1],  0x03, 4) | MASK_R(in[4],  0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[0],  0x1f, 6) | MASK_R(in[1],  0xfc, 2); SIGN_EXTEND_11(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[14], 0x3f, 2) | MASK_R(in[15], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[11], 0x0f, 2) | MASK_R(in[14], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[10], 0x3f, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_10(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[7],  0x0f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[6],  0x0f, 4) | MASK_R(in[7],  0xf0, 4); SIGN_EXTEND_8(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[3],  0x03, 4) | MASK_R(in[6],  0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[2],  0x1f, 6) | MASK_R(in[3],  0xfc, 2); SIGN_EXTEND_11(*p); SHIFT_ROUND_2(*p);

            // 2 linears

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 22: /* 111.010. */

            *p     = MASK_L(in[13], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[12], 0x3f, 2) | MASK_R(in[13], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[9],  0x0f, 2) | MASK_R(in[12], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[8],  0x3f, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_10(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[5],  0x0f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[4],  0x0f, 4) | MASK_R(in[5],  0xf0, 4); SIGN_EXTEND_8(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[1],  0x03, 4) | MASK_R(in[4],  0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[0],  0x1f, 6) | MASK_R(in[1],  0xfc, 2); SIGN_EXTEND_11(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[14], 0x3f, 2) | MASK_R(in[15], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[11], 0x0f, 2) | MASK_R(in[14], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[10], 0x3f, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_10(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[7],  0x0f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[6],  0x0f, 4) | MASK_R(in[7],  0xf0, 4); SIGN_EXTEND_8(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[3],  0x03, 4) | MASK_R(in[6],  0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[2],  0x1f, 6) | MASK_R(in[3],  0xfc, 2); SIGN_EXTEND_11(*p); SHIFT_ROUND_3(*p);

            // 2 linears

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 23: /* 111.011. */

            *p     = MASK_L(in[13], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[12], 0x3f, 2) | MASK_R(in[13], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[9],  0x0f, 2) | MASK_R(in[12], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[8],  0x3f, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_10(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[5],  0x0f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[4],  0x0f, 4) | MASK_R(in[5],  0xf0, 4); SIGN_EXTEND_8(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[1],  0x03, 4) | MASK_R(in[4],  0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[0],  0x1f, 6) | MASK_R(in[1],  0xfc, 2); SIGN_EXTEND_11(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[14], 0x3f, 2) | MASK_R(in[15], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[11], 0x0f, 2) | MASK_R(in[14], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[10], 0x3f, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_10(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[7],  0x0f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[6],  0x0f, 4) | MASK_R(in[7],  0xf0, 4); SIGN_EXTEND_8(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[3],  0x03, 4) | MASK_R(in[6],  0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[2],  0x1f, 6) | MASK_R(in[3],  0xfc, 2); SIGN_EXTEND_11(*p); SHIFT_ROUND_4(*p);

            // 2 linears

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 24: /* 111.100. */

            *p     = MASK_L(in[13], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[12], 0x3f, 2) | MASK_R(in[13], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[9],  0x0f, 2) | MASK_R(in[12], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[8],  0x3f, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_10(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[5],  0x0f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[4],  0x0f, 4) | MASK_R(in[5],  0xf0, 4); SIGN_EXTEND_8(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[1],  0x03, 4) | MASK_R(in[4],  0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[0],  0x1f, 6) | MASK_R(in[1],  0xfc, 2); SIGN_EXTEND_11(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[14], 0x3f, 2) | MASK_R(in[15], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[11], 0x0f, 2) | MASK_R(in[14], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[10], 0x3f, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_10(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[7],  0x0f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[6],  0x0f, 4) | MASK_R(in[7],  0xf0, 4); SIGN_EXTEND_8(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[3],  0x03, 4) | MASK_R(in[6],  0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[2],  0x1f, 6) | MASK_R(in[3],  0xfc, 2); SIGN_EXTEND_11(*p); SHIFT_ROUND_5(*p);

            // 2 linears

            INTERPOLATE_2(d0, out);

            PREVENT_OVERFLOW_16(out);

            return;

        /*=====================================================================
            PATTERN F
            pppp8888 88887777 ppppgggg ggggffff
            77766666 66655555 fffeeeee eeeddddd
            55444444 44333333 ddcccccc ccbbbbbb
            32222222 21111111 baaaaaaa a9999999
        */

        case 25: /* 11101010 */

            *p     = MASK_L(in[13], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[12], 0x7f, 1) | MASK_R(in[13], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[9],  0x3f, 1) | MASK_R(in[12], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[8],  0x3f, 2) | MASK_R(in[9],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[5],  0x1f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[4],  0x1f, 3) | MASK_R(in[5],  0xe0, 5); SIGN_EXTEND_8(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[1],  0x0f, 3) | MASK_R(in[4],  0xe0, 5); SIGN_EXTEND_7(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[0],  0x0f, 4) | MASK_R(in[1],  0xf0, 4); SIGN_EXTEND_8(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[15], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[14], 0x7f, 1) | MASK_R(in[15], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[11], 0x3f, 1) | MASK_R(in[14], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[10], 0x3f, 2) | MASK_R(in[11], 0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[7],  0x1f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[6],  0x1f, 3) | MASK_R(in[7],  0xe0, 5); SIGN_EXTEND_8(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[3],  0x0f, 3) | MASK_R(in[6],  0xe0, 5); SIGN_EXTEND_7(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[2],  0x0f, 4) | MASK_R(in[3],  0xf0, 4); SIGN_EXTEND_8(*p); SHIFT_ROUND_4(*p);

            // 8 linears

            INTERPOLATE_8(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 26: /* 11101011 */

            *p     = MASK_L(in[13], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[12], 0x7f, 1) | MASK_R(in[13], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[9],  0x3f, 1) | MASK_R(in[12], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[8],  0x3f, 2) | MASK_R(in[9],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[5],  0x1f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[4],  0x1f, 3) | MASK_R(in[5],  0xe0, 5); SIGN_EXTEND_8(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[1],  0x0f, 3) | MASK_R(in[4],  0xe0, 5); SIGN_EXTEND_7(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[0],  0x0f, 4) | MASK_R(in[1],  0xf0, 4); SIGN_EXTEND_8(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[15], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[14], 0x7f, 1) | MASK_R(in[15], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[11], 0x3f, 1) | MASK_R(in[14], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[10], 0x3f, 2) | MASK_R(in[11], 0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[7],  0x1f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[6],  0x1f, 3) | MASK_R(in[7],  0xe0, 5); SIGN_EXTEND_8(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[3],  0x0f, 3) | MASK_R(in[6],  0xe0, 5); SIGN_EXTEND_7(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[2],  0x0f, 4) | MASK_R(in[3],  0xf0, 4); SIGN_EXTEND_8(*p); SHIFT_ROUND_5(*p);

            // 8 linears

            INTERPOLATE_8(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 27: /* 11101100 */

            *p     = MASK_L(in[13], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[12], 0x7f, 1) | MASK_R(in[13], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[9],  0x3f, 1) | MASK_R(in[12], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[8],  0x3f, 2) | MASK_R(in[9],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[5],  0x1f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[4],  0x1f, 3) | MASK_R(in[5],  0xe0, 5); SIGN_EXTEND_8(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[1],  0x0f, 3) | MASK_R(in[4],  0xe0, 5); SIGN_EXTEND_7(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[0],  0x0f, 4) | MASK_R(in[1],  0xf0, 4); SIGN_EXTEND_8(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[15], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[14], 0x7f, 1) | MASK_R(in[15], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[11], 0x3f, 1) | MASK_R(in[14], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[10], 0x3f, 2) | MASK_R(in[11], 0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[7],  0x1f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[6],  0x1f, 3) | MASK_R(in[7],  0xe0, 5); SIGN_EXTEND_8(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[3],  0x0f, 3) | MASK_R(in[6],  0xe0, 5); SIGN_EXTEND_7(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[2],  0x0f, 4) | MASK_R(in[3],  0xf0, 4); SIGN_EXTEND_8(*p); SHIFT_ROUND_6(*p);

            // 8 linears

            INTERPOLATE_8(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 28: /* 11101101 */

            *p     = MASK_L(in[13], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[12], 0x7f, 1) | MASK_R(in[13], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[9],  0x3f, 1) | MASK_R(in[12], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[8],  0x3f, 2) | MASK_R(in[9],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[5],  0x1f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[4],  0x1f, 3) | MASK_R(in[5],  0xe0, 5); SIGN_EXTEND_8(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[1],  0x0f, 3) | MASK_R(in[4],  0xe0, 5); SIGN_EXTEND_7(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[0],  0x0f, 4) | MASK_R(in[1],  0xf0, 4); SIGN_EXTEND_8(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[15], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[14], 0x7f, 1) | MASK_R(in[15], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[11], 0x3f, 1) | MASK_R(in[14], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[10], 0x3f, 2) | MASK_R(in[11], 0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[7],  0x1f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[6],  0x1f, 3) | MASK_R(in[7],  0xe0, 5); SIGN_EXTEND_8(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[3],  0x0f, 3) | MASK_R(in[6],  0xe0, 5); SIGN_EXTEND_7(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[2],  0x0f, 4) | MASK_R(in[3],  0xf0, 4); SIGN_EXTEND_8(*p); SHIFT_ROUND_7(*p);

            // 8 linears

            INTERPOLATE_8(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 29: /* 11101110 */

            *p     = MASK_L(in[13], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[12], 0x7f, 1) | MASK_R(in[13], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[9],  0x3f, 1) | MASK_R(in[12], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[8],  0x3f, 2) | MASK_R(in[9],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[5],  0x1f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[4],  0x1f, 3) | MASK_R(in[5],  0xe0, 5); SIGN_EXTEND_8(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[1],  0x0f, 3) | MASK_R(in[4],  0xe0, 5); SIGN_EXTEND_7(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[0],  0x0f, 4) | MASK_R(in[1],  0xf0, 4); SIGN_EXTEND_8(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[15], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[14], 0x7f, 1) | MASK_R(in[15], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[11], 0x3f, 1) | MASK_R(in[14], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[10], 0x3f, 2) | MASK_R(in[11], 0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[7],  0x1f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[6],  0x1f, 3) | MASK_R(in[7],  0xe0, 5); SIGN_EXTEND_8(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[3],  0x0f, 3) | MASK_R(in[6],  0xe0, 5); SIGN_EXTEND_7(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[2],  0x0f, 4) | MASK_R(in[3],  0xf0, 4); SIGN_EXTEND_8(*p); SHIFT_ROUND_8(*p);

            // 8 linears

            INTERPOLATE_8(d0, out);

            PREVENT_OVERFLOW_16(out);

            return;

        case 30: /* 11101111 */

#ifdef DANIELO_DOUBLE
            *p     = MASK_L(in[13], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[12], 0x7f, 1) | MASK_R(in[13], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[9],  0x3f, 1) | MASK_R(in[12], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[8],  0x3f, 2) | MASK_R(in[9],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[5],  0x1f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[4],  0x1f, 3) | MASK_R(in[5],  0xe0, 5); SIGN_EXTEND_8(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[1],  0x0f, 3) | MASK_R(in[4],  0xe0, 5); SIGN_EXTEND_7(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[0],  0x0f, 4) | MASK_R(in[1],  0xf0, 4); SIGN_EXTEND_8(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[15], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[14], 0x7f, 1) | MASK_R(in[15], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[11], 0x3f, 1) | MASK_R(in[14], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[10], 0x3f, 2) | MASK_R(in[11], 0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[7],  0x1f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[6],  0x1f, 3) | MASK_R(in[7],  0xe0, 5); SIGN_EXTEND_8(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[3],  0x0f, 3) | MASK_R(in[6],  0xe0, 5); SIGN_EXTEND_7(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[2],  0x0f, 4) | MASK_R(in[3],  0xf0, 4); SIGN_EXTEND_8(*p); SHIFT_ROUND_8(*p);

            // 16 linears - but odd samples are doubled
#else
            *p     = MASK_L(in[13], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[12], 0x7f, 1) | MASK_R(in[13], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[9],  0x3f, 1) | MASK_R(in[12], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[8],  0x3f, 2) | MASK_R(in[9],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[5],  0x1f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[4],  0x1f, 3) | MASK_R(in[5],  0xe0, 5); SIGN_EXTEND_8(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[1],  0x0f, 3) | MASK_R(in[4],  0xe0, 5); SIGN_EXTEND_7(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[0],  0x0f, 4) | MASK_R(in[1],  0xf0, 4); SIGN_EXTEND_8(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[15], 0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[14], 0x7f, 1) | MASK_R(in[15], 0x80, 7); SIGN_EXTEND_8(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[11], 0x3f, 1) | MASK_R(in[14], 0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[10], 0x3f, 2) | MASK_R(in[11], 0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[7],  0x1f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_7(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[6],  0x1f, 3) | MASK_R(in[7],  0xe0, 5); SIGN_EXTEND_8(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[3],  0x0f, 3) | MASK_R(in[6],  0xe0, 5); SIGN_EXTEND_7(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[2],  0x0f, 4) | MASK_R(in[3],  0xf0, 4); SIGN_EXTEND_8(*p); SHIFT_ROUND_8(*p);

            // 16 linears - but odd samples are doubled

            DOUBLE_ODDS(out);
#endif

            PREVENT_OVERFLOW_16(out);

            return;

        /*=====================================================================
            PATTERN E
            pppp8888 88888877 ppppgggg ggggggff
            77776666 66665555 ffffeeee eeeedddd
            55444444 44443333 ddcccccc ccccbbbb
            33222222 22111111 bbaaaaaa aa999999
        */

        case 31: /* 11111010 */

            *p     = MASK_L(in[13], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[12], 0x3f, 2) | MASK_R(in[13], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[9],  0x0f, 2) | MASK_R(in[12], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[8],  0x3f, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_10(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[5],  0x0f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[4],  0x0f, 4) | MASK_R(in[5],  0xf0, 4); SIGN_EXTEND_8(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[1],  0x03, 4) | MASK_R(in[4],  0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[0],  0x0f, 6) | MASK_R(in[1],  0xfc, 2); SIGN_EXTEND_10(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[15], 0x3f, 0);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[14], 0x3f, 2) | MASK_R(in[15], 0xc0, 6); SIGN_EXTEND_8(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[11], 0x0f, 2) | MASK_R(in[14], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[10], 0x3f, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_10(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[7],  0x0f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_6(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[6],  0x0f, 4) | MASK_R(in[7],  0xf0, 4); SIGN_EXTEND_8(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[3],  0x03, 4) | MASK_R(in[6],  0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[2],  0x0f, 6) | MASK_R(in[3],  0xfc, 2); SIGN_EXTEND_10(*p); SHIFT_ROUND_6(*p);

            // 4 Linears

            INTERPOLATE_4(d0, out);

            PREVENT_OVERFLOW_16(out);

            return;

        /*=====================================================================
            PATTERN B4
            pppp8888 88888887 ppppgggg gggggggf
            77777766 66666555 ffffffee eeeeeddd
            55554444 44433333 ddddcccc cccbbbbb
            33222222 21111111 bbaaaaaa a9999999
        */

        case 32: /* 11111011 */

            /* Unknown - never occurs? */
            ZERO(out);

            //printf("Unknown pattern: %d\n", pattern);

            return;

        case 33: /* 11111100 */

            /* Unknown - never occurs? */
            ZERO(out);

            //printf("Unknown pattern: %d\n", pattern);

            return;

        case 34: /* 11111101 */

            /* Unknown - never occurs? */
            ZERO(out);

            //printf("Unknown pattern: %d\n", pattern);

            return;

        case 35: /* 11111110 */

            /* Unknown - never occurs? */
            ZERO(out);

            //printf("Unknown pattern: %d\n", pattern);

            return;

        case 36: /* 11111111 */

            /* Unknown - never occurs? */
            ZERO(out);

            //printf("Unknown pattern: %d\n", pattern);

            return;

        default: break;
    }
}

//*****************************************************************************
void decodeMT2(int d0, unsigned char *in, int *out)
{
    // Decodes a 12-byte MT2 RDAC block into 16-bit samples.

    int patternIndex = MASK_L(in[0], 0xf0, 0) | MASK_R(in[2], 0xf0, 4);

    int pattern = patterns[patternIndex];

    int *p = out;

    switch (pattern) {

        /*=====================================================================
            PATTERN A
            pp888888 88888777 ppgggggg gggggfff
            76666665 55544444 feeeeeed dddccccc
            44333322 22221111 ccbbbbaa aaaa9999
        */

        case 0:  /* 00..00.. */

            *p     = MASK_L(in[9],  0x0f, 0);                           SIGN_EXTEND_4(*p);
            *(++p) = MASK_L(in[8],  0x03, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_6(*p);
            *(++p) = MASK_R(in[8],  0x3c, 2);                           SIGN_EXTEND_4(*p);
            *(++p) = MASK_L(in[5],  0x1f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_7(*p);
            *(++p) = MASK_L(in[4],  0x01, 3) | MASK_R(in[5],  0xe0, 5); SIGN_EXTEND_4(*p);
            *(++p) = MASK_R(in[4],  0x7e, 1);                           SIGN_EXTEND_6(*p);
            *(++p) = MASK_L(in[1],  0x07, 1) | MASK_R(in[4],  0x80, 7); SIGN_EXTEND_4(*p);
            *(++p) = MASK_L(in[0],  0x3f, 5) | MASK_R(in[1],  0xf8, 3); SIGN_EXTEND_11(*p);
            *(++p) = MASK_L(in[11], 0x0f, 0);                           SIGN_EXTEND_4(*p);
            *(++p) = MASK_L(in[10], 0x03, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_6(*p);
            *(++p) = MASK_R(in[10], 0x3c, 2);                           SIGN_EXTEND_4(*p);
            *(++p) = MASK_L(in[7],  0x1f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_7(*p);
            *(++p) = MASK_L(in[6],  0x01, 3) | MASK_R(in[7],  0xe0, 5); SIGN_EXTEND_4(*p);
            *(++p) = MASK_R(in[6],  0x7e, 1);                           SIGN_EXTEND_6(*p);
            *(++p) = MASK_L(in[3],  0x07, 1) | MASK_R(in[6],  0x80, 7); SIGN_EXTEND_4(*p);
            *(++p) = MASK_L(in[2],  0x3f, 5) | MASK_R(in[3],  0xf8, 3); SIGN_EXTEND_11(*p);

            // 2 linears

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 1:  /* 00..01.. */

            *p     = MASK_L(in[9],  0x0f, 0);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[8],  0x03, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_R(in[8],  0x3c, 2);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[5],  0x1f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_7(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[4],  0x01, 3) | MASK_R(in[5],  0xe0, 5); SIGN_EXTEND_4(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_R(in[4],  0x7e, 1);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[1],  0x07, 1) | MASK_R(in[4],  0x80, 7); SIGN_EXTEND_4(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[0],  0x3f, 5) | MASK_R(in[1],  0xf8, 3); SIGN_EXTEND_11(*p); SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[11], 0x0f, 0);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[10], 0x03, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_R(in[10], 0x3c, 2);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[7],  0x1f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_7(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[6],  0x01, 3) | MASK_R(in[7],  0xe0, 5); SIGN_EXTEND_4(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_R(in[6],  0x7e, 1);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[3],  0x07, 1) | MASK_R(in[6],  0x80, 7); SIGN_EXTEND_4(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[2],  0x3f, 5) | MASK_R(in[3],  0xf8, 3); SIGN_EXTEND_11(*p); SHIFT_ROUND_1(*p);

            // 2 linears

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 2:  /* 00..10.. */

            *p     = MASK_L(in[9],  0x0f, 0);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[8],  0x03, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_R(in[8],  0x3c, 2);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[5],  0x1f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_7(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[4],  0x01, 3) | MASK_R(in[5],  0xe0, 5); SIGN_EXTEND_4(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_R(in[4],  0x7e, 1);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[1],  0x07, 1) | MASK_R(in[4],  0x80, 7); SIGN_EXTEND_4(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[0],  0x3f, 5) | MASK_R(in[1],  0xf8, 3); SIGN_EXTEND_11(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[11], 0x0f, 0);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[10], 0x03, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_R(in[10], 0x3c, 2);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[7],  0x1f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_7(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[6],  0x01, 3) | MASK_R(in[7],  0xe0, 5); SIGN_EXTEND_4(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_R(in[6],  0x7e, 1);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[3],  0x07, 1) | MASK_R(in[6],  0x80, 7); SIGN_EXTEND_4(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[2],  0x3f, 5) | MASK_R(in[3],  0xf8, 3); SIGN_EXTEND_11(*p); SHIFT_ROUND_2(*p);

            // 2 linears

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 3:  /* 00..11.. */

            *p     = MASK_L(in[9],  0x0f, 0);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[8],  0x03, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_R(in[8],  0x3c, 2);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[5],  0x1f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_7(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[4],  0x01, 3) | MASK_R(in[5],  0xe0, 5); SIGN_EXTEND_4(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_R(in[4],  0x7e, 1);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[1],  0x07, 1) | MASK_R(in[4],  0x80, 7); SIGN_EXTEND_4(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[0],  0x3f, 5) | MASK_R(in[1],  0xf8, 3); SIGN_EXTEND_11(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[11], 0x0f, 0);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[10], 0x03, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_R(in[10], 0x3c, 2);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[7],  0x1f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_7(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[6],  0x01, 3) | MASK_R(in[7],  0xe0, 5); SIGN_EXTEND_4(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_R(in[6],  0x7e, 1);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[3],  0x07, 1) | MASK_R(in[6],  0x80, 7); SIGN_EXTEND_4(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[2],  0x3f, 5) | MASK_R(in[3],  0xf8, 3); SIGN_EXTEND_11(*p); SHIFT_ROUND_3(*p);

            // 2 linears

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 4:  /* 01..00.. */

            *p     = MASK_L(in[9],  0x0f, 0);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[8],  0x03, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_R(in[8],  0x3c, 2);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[5],  0x1f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_7(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[4],  0x01, 3) | MASK_R(in[5],  0xe0, 5); SIGN_EXTEND_4(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_R(in[4],  0x7e, 1);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[1],  0x07, 1) | MASK_R(in[4],  0x80, 7); SIGN_EXTEND_4(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[0],  0x3f, 5) | MASK_R(in[1],  0xf8, 3); SIGN_EXTEND_11(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[11], 0x0f, 0);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[10], 0x03, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_R(in[10], 0x3c, 2);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[7],  0x1f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_7(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[6],  0x01, 3) | MASK_R(in[7],  0xe0, 5); SIGN_EXTEND_4(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_R(in[6],  0x7e, 1);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[3],  0x07, 1) | MASK_R(in[6],  0x80, 7); SIGN_EXTEND_4(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[2],  0x3f, 5) | MASK_R(in[3],  0xf8, 3); SIGN_EXTEND_11(*p); SHIFT_ROUND_4(*p);

            // 2 linears

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 5:  /* 01..01.. */

            *p     = MASK_L(in[9],  0x0f, 0);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[8],  0x03, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_R(in[8],  0x3c, 2);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[5],  0x1f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_7(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[4],  0x01, 3) | MASK_R(in[5],  0xe0, 5); SIGN_EXTEND_4(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_R(in[4],  0x7e, 1);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[1],  0x07, 1) | MASK_R(in[4],  0x80, 7); SIGN_EXTEND_4(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[0],  0x3f, 5) | MASK_R(in[1],  0xf8, 3); SIGN_EXTEND_11(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[11], 0x0f, 0);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[10], 0x03, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_R(in[10], 0x3c, 2);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[7],  0x1f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_7(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[6],  0x01, 3) | MASK_R(in[7],  0xe0, 5); SIGN_EXTEND_4(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_R(in[6],  0x7e, 1);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[3],  0x07, 1) | MASK_R(in[6],  0x80, 7); SIGN_EXTEND_4(*p);  SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[2],  0x3f, 5) | MASK_R(in[3],  0xf8, 3); SIGN_EXTEND_11(*p); SHIFT_ROUND_5(*p);

            // 2 linears

            INTERPOLATE_2(d0, out);

            PREVENT_OVERFLOW_16(out);

            return;

        /*=====================================================================
            PATTERN B
            pp888888 87777766 ppgggggg gfffffee
            66665555 54444444 eeeedddd dccccccc
            33333222 22211111 bbbbbaaa aaa99999
        */

        case 6:  /* 01..10.. */

            *p     = MASK_L(in[9],  0x1f, 0);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[8],  0x07, 3) | MASK_R(in[9],  0xe0, 5); SIGN_EXTEND_6(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_R(in[8],  0xf8, 3);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_R(in[5],  0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[4],  0x0f, 1) | MASK_R(in[5],  0x80, 7); SIGN_EXTEND_5(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[1],  0x03, 4) | MASK_R(in[4],  0xf0, 4); SIGN_EXTEND_6(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_R(in[1],  0x7c, 2);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[0],  0x3f, 1) | MASK_R(in[1],  0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[11], 0x1f, 0);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[10], 0x07, 3) | MASK_R(in[11], 0xe0, 5); SIGN_EXTEND_6(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_R(in[10], 0xf8, 3);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_R(in[7],  0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[6],  0x0f, 1) | MASK_R(in[7],  0x80, 7); SIGN_EXTEND_5(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[3],  0x03, 4) | MASK_R(in[6],  0xf0, 4); SIGN_EXTEND_6(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_R(in[3],  0x7c, 2);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[2],  0x3f, 1) | MASK_R(in[3],  0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_4(*p);

            // 4 linears

            INTERPOLATE_4(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 7:  /* 01..11.. */

            *p     = MASK_L(in[9],  0x1f, 0);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[8],  0x07, 3) | MASK_R(in[9],  0xe0, 5); SIGN_EXTEND_6(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_R(in[8],  0xf8, 3);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_R(in[5],  0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[4],  0x0f, 1) | MASK_R(in[5],  0x80, 7); SIGN_EXTEND_5(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[1],  0x03, 4) | MASK_R(in[4],  0xf0, 4); SIGN_EXTEND_6(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_R(in[1],  0x7c, 2);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[0],  0x3f, 1) | MASK_R(in[1],  0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[11], 0x1f, 0);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[10], 0x07, 3) | MASK_R(in[11], 0xe0, 5); SIGN_EXTEND_6(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_R(in[10], 0xf8, 3);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_R(in[7],  0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[6],  0x0f, 1) | MASK_R(in[7],  0x80, 7); SIGN_EXTEND_5(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[3],  0x03, 4) | MASK_R(in[6],  0xf0, 4); SIGN_EXTEND_6(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_R(in[3],  0x7c, 2);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[2],  0x3f, 1) | MASK_R(in[3],  0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_5(*p);

            // 4 linears

            INTERPOLATE_4(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 8:  /* 10..00.. */

            *p     = MASK_L(in[9],  0x1f, 0);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[8],  0x07, 3) | MASK_R(in[9],  0xe0, 5); SIGN_EXTEND_6(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_R(in[8],  0xf8, 3);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_R(in[5],  0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[4],  0x0f, 1) | MASK_R(in[5],  0x80, 7); SIGN_EXTEND_5(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[1],  0x03, 4) | MASK_R(in[4],  0xf0, 4); SIGN_EXTEND_6(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_R(in[1],  0x7c, 2);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[0],  0x3f, 1) | MASK_R(in[1],  0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[11], 0x1f, 0);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[10], 0x07, 3) | MASK_R(in[11], 0xe0, 5); SIGN_EXTEND_6(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_R(in[10], 0xf8, 3);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_R(in[7],  0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[6],  0x0f, 1) | MASK_R(in[7],  0x80, 7); SIGN_EXTEND_5(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[3],  0x03, 4) | MASK_R(in[6],  0xf0, 4); SIGN_EXTEND_6(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_R(in[3],  0x7c, 2);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[2],  0x3f, 1) | MASK_R(in[3],  0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_6(*p);

            // 4 linears

            INTERPOLATE_4(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 9:  /* 10..01.. */

            *p     = MASK_L(in[9],  0x1f, 0);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[8],  0x07, 3) | MASK_R(in[9],  0xe0, 5); SIGN_EXTEND_6(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_R(in[8],  0xf8, 3);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_R(in[5],  0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[4],  0x0f, 1) | MASK_R(in[5],  0x80, 7); SIGN_EXTEND_5(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[1],  0x03, 4) | MASK_R(in[4],  0xf0, 4); SIGN_EXTEND_6(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_R(in[1],  0x7c, 2);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[0],  0x3f, 1) | MASK_R(in[1],  0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[11], 0x1f, 0);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[10], 0x07, 3) | MASK_R(in[11], 0xe0, 5); SIGN_EXTEND_6(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_R(in[10], 0xf8, 3);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_R(in[7],  0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[6],  0x0f, 1) | MASK_R(in[7],  0x80, 7); SIGN_EXTEND_5(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[3],  0x03, 4) | MASK_R(in[6],  0xf0, 4); SIGN_EXTEND_6(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_R(in[3],  0x7c, 2);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[2],  0x3f, 1) | MASK_R(in[3],  0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_7(*p);

            // 4 linears

            INTERPOLATE_4(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 10: /* 10..10.. */

            *p     = MASK_L(in[9],  0x1f, 0);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[8],  0x07, 3) | MASK_R(in[9],  0xe0, 5); SIGN_EXTEND_6(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_R(in[8],  0xf8, 3);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_R(in[5],  0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[4],  0x0f, 1) | MASK_R(in[5],  0x80, 7); SIGN_EXTEND_5(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[1],  0x03, 4) | MASK_R(in[4],  0xf0, 4); SIGN_EXTEND_6(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_R(in[1],  0x7c, 2);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[0],  0x3f, 1) | MASK_R(in[1],  0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[11], 0x1f, 0);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[10], 0x07, 3) | MASK_R(in[11], 0xe0, 5); SIGN_EXTEND_6(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_R(in[10], 0xf8, 3);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_R(in[7],  0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[6],  0x0f, 1) | MASK_R(in[7],  0x80, 7); SIGN_EXTEND_5(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[3],  0x03, 4) | MASK_R(in[6],  0xf0, 4); SIGN_EXTEND_6(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_R(in[3],  0x7c, 2);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[2],  0x3f, 1) | MASK_R(in[3],  0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_8(*p);

            // 4 linears

            INTERPOLATE_4(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 11: /* 10..11.. */

            *p     = MASK_L(in[9],  0x1f, 0);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[8],  0x07, 3) | MASK_R(in[9],  0xe0, 5); SIGN_EXTEND_6(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_R(in[8],  0xf8, 3);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_R(in[5],  0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[4],  0x0f, 1) | MASK_R(in[5],  0x80, 7); SIGN_EXTEND_5(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[1],  0x03, 4) | MASK_R(in[4],  0xf0, 4); SIGN_EXTEND_6(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_R(in[1],  0x7c, 2);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[0],  0x3f, 1) | MASK_R(in[1],  0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[11], 0x1f, 0);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[10], 0x07, 3) | MASK_R(in[11], 0xe0, 5); SIGN_EXTEND_6(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_R(in[10], 0xf8, 3);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_R(in[7],  0x7f, 0);                           SIGN_EXTEND_7(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[6],  0x0f, 1) | MASK_R(in[7],  0x80, 7); SIGN_EXTEND_5(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[3],  0x03, 4) | MASK_R(in[6],  0xf0, 4); SIGN_EXTEND_6(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_R(in[3],  0x7c, 2);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[2],  0x3f, 1) | MASK_R(in[3],  0x80, 7); SIGN_EXTEND_7(*p); SHIFT_ROUND_9(*p);

            // 4 linears

            INTERPOLATE_4(d0, out);

            PREVENT_OVERFLOW_16(out);

            return;

        /*=====================================================================
            PATTERN F
            ppp88888 88888887 pppggggg gggggggf
            77766666 55554444 fffeeeee ddddcccc
            44433332 22221111 cccbbbba aaaa9999
        */

        case 12: /* 110.000. */

            /* Unknown
            */
            ZERO(out);

            return;

        case 13: /* 110.001. */

            *p     = MASK_L(in[9],  0x0f, 0);                           SIGN_EXTEND_4(*p);
            *(++p) = MASK_L(in[8],  0x01, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_5(*p);
            *(++p) = MASK_R(in[8],  0x1e, 1);                           SIGN_EXTEND_4(*p);
            *(++p) = MASK_L(in[5],  0x0f, 3) | MASK_R(in[8],  0xe0, 5); SIGN_EXTEND_7(*p);
            *(++p) = MASK_R(in[5],  0xf0, 4);                           SIGN_EXTEND_4(*p);
            *(++p) = MASK_R(in[4],  0x1f, 0);                           SIGN_EXTEND_5(*p);
            *(++p) = MASK_L(in[1],  0x01, 3) | MASK_R(in[4],  0xe0, 5); SIGN_EXTEND_4(*p);
            *(++p) = MASK_L(in[0],  0x1f, 7) | MASK_R(in[1],  0xfe, 1); SIGN_EXTEND_12(*p);
            *(++p) = MASK_L(in[11], 0x0f, 0);                           SIGN_EXTEND_4(*p);
            *(++p) = MASK_L(in[10], 0x01, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_5(*p);
            *(++p) = MASK_R(in[10], 0x1e, 1);                           SIGN_EXTEND_4(*p);
            *(++p) = MASK_L(in[7],  0x0f, 3) | MASK_R(in[10], 0xe0, 5); SIGN_EXTEND_7(*p);
            *(++p) = MASK_R(in[7],  0xf0, 4);                           SIGN_EXTEND_4(*p);
            *(++p) = MASK_R(in[6],  0x1f, 0);                           SIGN_EXTEND_5(*p);
            *(++p) = MASK_L(in[3],  0x01, 3) | MASK_R(in[6],  0xe0, 5); SIGN_EXTEND_4(*p);
            *(++p) = MASK_L(in[2],  0x1f, 7) | MASK_R(in[3],  0xfe, 1); SIGN_EXTEND_12(*p);

            // 2 linears

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 14: /* 110.010. */

            *p     = MASK_L(in[9],  0x0f, 0);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[8],  0x01, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_5(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_R(in[8],  0x1e, 1);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[5],  0x0f, 3) | MASK_R(in[8],  0xe0, 5); SIGN_EXTEND_7(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_R(in[5],  0xf0, 4);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_R(in[4],  0x1f, 0);                           SIGN_EXTEND_5(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[1],  0x01, 3) | MASK_R(in[4],  0xe0, 5); SIGN_EXTEND_4(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[0],  0x1f, 7) | MASK_R(in[1],  0xfe, 1); SIGN_EXTEND_12(*p); SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[11], 0x0f, 0);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[10], 0x01, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_5(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_R(in[10], 0x1e, 1);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[7],  0x0f, 3) | MASK_R(in[10], 0xe0, 5); SIGN_EXTEND_7(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_R(in[7],  0xf0, 4);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_R(in[6],  0x1f, 0);                           SIGN_EXTEND_5(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[3],  0x01, 3) | MASK_R(in[6],  0xe0, 5); SIGN_EXTEND_4(*p);  SHIFT_ROUND_1(*p);
            *(++p) = MASK_L(in[2],  0x1f, 7) | MASK_R(in[3],  0xfe, 1); SIGN_EXTEND_12(*p); SHIFT_ROUND_1(*p);

            // 2 linears

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 15: /* 110.011. */

            *p     = MASK_L(in[9],  0x0f, 0);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[8],  0x01, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_5(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_R(in[8],  0x1e, 1);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[5],  0x0f, 3) | MASK_R(in[8],  0xe0, 5); SIGN_EXTEND_7(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_R(in[5],  0xf0, 4);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_R(in[4],  0x1f, 0);                           SIGN_EXTEND_5(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[1],  0x01, 3) | MASK_R(in[4],  0xe0, 5); SIGN_EXTEND_4(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[0],  0x1f, 7) | MASK_R(in[1],  0xfe, 1); SIGN_EXTEND_12(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[11], 0x0f, 0);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[10], 0x01, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_5(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_R(in[10], 0x1e, 1);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[7],  0x0f, 3) | MASK_R(in[10], 0xe0, 5); SIGN_EXTEND_7(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_R(in[7],  0xf0, 4);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_R(in[6],  0x1f, 0);                           SIGN_EXTEND_5(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[3],  0x01, 3) | MASK_R(in[6],  0xe0, 5); SIGN_EXTEND_4(*p);  SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[2],  0x1f, 7) | MASK_R(in[3],  0xfe, 1); SIGN_EXTEND_12(*p); SHIFT_ROUND_2(*p);

            // 2 linears

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 16: /* 110.100. */

            *p     = MASK_L(in[9],  0x0f, 0);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[8],  0x01, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_5(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_R(in[8],  0x1e, 1);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[5],  0x0f, 3) | MASK_R(in[8],  0xe0, 5); SIGN_EXTEND_7(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_R(in[5],  0xf0, 4);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_R(in[4],  0x1f, 0);                           SIGN_EXTEND_5(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[1],  0x01, 3) | MASK_R(in[4],  0xe0, 5); SIGN_EXTEND_4(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[0],  0x1f, 7) | MASK_R(in[1],  0xfe, 1); SIGN_EXTEND_12(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[11], 0x0f, 0);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[10], 0x01, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_5(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_R(in[10], 0x1e, 1);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[7],  0x0f, 3) | MASK_R(in[10], 0xe0, 5); SIGN_EXTEND_7(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_R(in[7],  0xf0, 4);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_R(in[6],  0x1f, 0);                           SIGN_EXTEND_5(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[3],  0x01, 3) | MASK_R(in[6],  0xe0, 5); SIGN_EXTEND_4(*p);  SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[2],  0x1f, 7) | MASK_R(in[3],  0xfe, 1); SIGN_EXTEND_12(*p); SHIFT_ROUND_3(*p);

            // 2 linears

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 17: /* 110.101. */

            *p     = MASK_L(in[9],  0x0f, 0);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[8],  0x01, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_5(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_R(in[8],  0x1e, 1);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[5],  0x0f, 3) | MASK_R(in[8],  0xe0, 5); SIGN_EXTEND_7(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_R(in[5],  0xf0, 4);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_R(in[4],  0x1f, 0);                           SIGN_EXTEND_5(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[1],  0x01, 3) | MASK_R(in[4],  0xe0, 5); SIGN_EXTEND_4(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[0],  0x1f, 7) | MASK_R(in[1],  0xfe, 1); SIGN_EXTEND_12(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[11], 0x0f, 0);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[10], 0x01, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_5(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_R(in[10], 0x1e, 1);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[7],  0x0f, 3) | MASK_R(in[10], 0xe0, 5); SIGN_EXTEND_7(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_R(in[7],  0xf0, 4);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_R(in[6],  0x1f, 0);                           SIGN_EXTEND_5(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[3],  0x01, 3) | MASK_R(in[6],  0xe0, 5); SIGN_EXTEND_4(*p);  SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[2],  0x1f, 7) | MASK_R(in[3],  0xfe, 1); SIGN_EXTEND_12(*p); SHIFT_ROUND_4(*p);

            // 2 linears

            INTERPOLATE_2(d0, out);

            PREVENT_OVERFLOW_16(out);

            return;

        /*=====================================================================
            PATTERN_G
            ppp88888 88888777 pppggggg gggggfff
            76666665 55544444 feeeeeed dddccccc
            44333322 22221111 ccbbbbaa aaaa9999
            */

        case 18: /* 110.110. */

            *p     = MASK_L(in[9],  0x0f, 0);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[8],  0x03, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_R(in[8],  0x3c, 2);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[5],  0x1f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_7(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[4],  0x01, 3) | MASK_R(in[5],  0xe0, 5); SIGN_EXTEND_4(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_R(in[4],  0x7e, 1);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[1],  0x07, 1) | MASK_R(in[4],  0x80, 7); SIGN_EXTEND_4(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[0],  0x1f, 5) | MASK_R(in[1],  0xf8, 3); SIGN_EXTEND_10(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[11], 0x0f, 0);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[10], 0x03, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_6(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_R(in[10], 0x3c, 2);                           SIGN_EXTEND_4(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[7],  0x1f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_7(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[6],  0x01, 3) | MASK_R(in[7],  0xe0, 5); SIGN_EXTEND_4(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_R(in[6],  0x7e, 1);                           SIGN_EXTEND_6(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[3],  0x07, 1) | MASK_R(in[6],  0x80, 7); SIGN_EXTEND_4(*p);  SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[2],  0x1f, 5) | MASK_R(in[3],  0xf8, 3); SIGN_EXTEND_10(*p); SHIFT_ROUND_6(*p);

            // 2 linears

            INTERPOLATE_2(d0, out);

            PREVENT_OVERFLOW_16(out);

            return;

        /*=====================================================================
            PATTERN_E
            ppp88888 88887777 pppggggg ggggffff
            66666655 55444444 eeeeeedd ddcccccc
            44333322 22221111 ccbbbbaa aaaa9999
        */
        case 19: /* 110.111. */

            *p     = MASK_L(in[9],  0x0f, 0);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[8],  0x03, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_6(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_R(in[8],  0x3c, 2);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[5],  0x3f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[4],  0x03, 2) | MASK_R(in[5],  0xc0, 6); SIGN_EXTEND_4(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_R(in[4],  0xfc, 2);                           SIGN_EXTEND_6(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[1],  0x0f, 0);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[0],  0x1f, 4) | MASK_R(in[1],  0xf0, 4); SIGN_EXTEND_9(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[11], 0x0f, 0);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[10], 0x03, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_6(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_R(in[10], 0x3c, 2);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[7],  0x3f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[6],  0x03, 2) | MASK_R(in[7],  0xc0, 6); SIGN_EXTEND_4(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_R(in[6],  0xfc, 2);                           SIGN_EXTEND_6(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[3],  0x0f, 0);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_2(*p);
            *(++p) = MASK_L(in[2],  0x1f, 4) | MASK_R(in[3],  0xf0, 4); SIGN_EXTEND_9(*p); SHIFT_ROUND_2(*p);

            // 2 linears

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 20: /* 111.000. */

            *p     = MASK_L(in[9],  0x0f, 0);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[8],  0x03, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_6(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_R(in[8],  0x3c, 2);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[5],  0x3f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[4],  0x03, 2) | MASK_R(in[5],  0xc0, 6); SIGN_EXTEND_4(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_R(in[4],  0xfc, 2);                           SIGN_EXTEND_6(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[1],  0x0f, 0);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[0],  0x1f, 4) | MASK_R(in[1],  0xf0, 4); SIGN_EXTEND_9(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[11], 0x0f, 0);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[10], 0x03, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_6(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_R(in[10], 0x3c, 2);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[7],  0x3f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[6],  0x03, 2) | MASK_R(in[7],  0xc0, 6); SIGN_EXTEND_4(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_R(in[6],  0xfc, 2);                           SIGN_EXTEND_6(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[3],  0x0f, 0);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_3(*p);
            *(++p) = MASK_L(in[2],  0x1f, 4) | MASK_R(in[3],  0xf0, 4); SIGN_EXTEND_9(*p); SHIFT_ROUND_3(*p);

            // 2 linears

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 21: /* 111.001. */

            *p     = MASK_L(in[9],  0x0f, 0);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[8],  0x03, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_6(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_R(in[8],  0x3c, 2);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[5],  0x3f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[4],  0x03, 2) | MASK_R(in[5],  0xc0, 6); SIGN_EXTEND_4(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_R(in[4],  0xfc, 2);                           SIGN_EXTEND_6(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[1],  0x0f, 0);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[0],  0x1f, 4) | MASK_R(in[1],  0xf0, 4); SIGN_EXTEND_9(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[11], 0x0f, 0);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[10], 0x03, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_6(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_R(in[10], 0x3c, 2);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[7],  0x3f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[6],  0x03, 2) | MASK_R(in[7],  0xc0, 6); SIGN_EXTEND_4(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_R(in[6],  0xfc, 2);                           SIGN_EXTEND_6(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[3],  0x0f, 0);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_4(*p);
            *(++p) = MASK_L(in[2],  0x1f, 4) | MASK_R(in[3],  0xf0, 4); SIGN_EXTEND_9(*p); SHIFT_ROUND_4(*p);

            // 2 linears

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 22: /* 111.010. */

            *p     = MASK_L(in[9],  0x0f, 0);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[8],  0x03, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_6(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_R(in[8],  0x3c, 2);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[5],  0x3f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[4],  0x03, 2) | MASK_R(in[5],  0xc0, 6); SIGN_EXTEND_4(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_R(in[4],  0xfc, 2);                           SIGN_EXTEND_6(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[1],  0x0f, 0);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[0],  0x1f, 4) | MASK_R(in[1],  0xf0, 4); SIGN_EXTEND_9(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[11], 0x0f, 0);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[10], 0x03, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_6(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_R(in[10], 0x3c, 2);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[7],  0x3f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[6],  0x03, 2) | MASK_R(in[7],  0xc0, 6); SIGN_EXTEND_4(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_R(in[6],  0xfc, 2);                           SIGN_EXTEND_6(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[3],  0x0f, 0);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_5(*p);
            *(++p) = MASK_L(in[2],  0x1f, 4) | MASK_R(in[3],  0xf0, 4); SIGN_EXTEND_9(*p); SHIFT_ROUND_5(*p);

            // 2 linears

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 23: /* 111.011. */

            *p     = MASK_L(in[9],  0x0f, 0);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[8],  0x03, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_6(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_R(in[8],  0x3c, 2);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[5],  0x3f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[4],  0x03, 2) | MASK_R(in[5],  0xc0, 6); SIGN_EXTEND_4(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_R(in[4],  0xfc, 2);                           SIGN_EXTEND_6(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[1],  0x0f, 0);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[0],  0x1f, 4) | MASK_R(in[1],  0xf0, 4); SIGN_EXTEND_9(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[11], 0x0f, 0);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[10], 0x03, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_6(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_R(in[10], 0x3c, 2);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[7],  0x3f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[6],  0x03, 2) | MASK_R(in[7],  0xc0, 6); SIGN_EXTEND_4(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_R(in[6],  0xfc, 2);                           SIGN_EXTEND_6(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[3],  0x0f, 0);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[2],  0x1f, 4) | MASK_R(in[3],  0xf0, 4); SIGN_EXTEND_9(*p); SHIFT_ROUND_6(*p);

            // 2 linears

            INTERPOLATE_2(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 24: /* 111.100. */

            *p     = MASK_L(in[9],  0x0f, 0);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[8],  0x03, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_6(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_R(in[8],  0x3c, 2);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[5],  0x3f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[4],  0x03, 2) | MASK_R(in[5],  0xc0, 6); SIGN_EXTEND_4(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_R(in[4],  0xfc, 2);                           SIGN_EXTEND_6(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[1],  0x0f, 0);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[0],  0x1f, 4) | MASK_R(in[1],  0xf0, 4); SIGN_EXTEND_9(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[11], 0x0f, 0);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[10], 0x03, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_6(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_R(in[10], 0x3c, 2);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[7],  0x3f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[6],  0x03, 2) | MASK_R(in[7],  0xc0, 6); SIGN_EXTEND_4(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_R(in[6],  0xfc, 2);                           SIGN_EXTEND_6(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[3],  0x0f, 0);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[2],  0x1f, 4) | MASK_R(in[3],  0xf0, 4); SIGN_EXTEND_9(*p); SHIFT_ROUND_7(*p);

            // 2 linears

            INTERPOLATE_2(d0, out);

            PREVENT_OVERFLOW_16(out);

            return;

        /*=====================================================================
            PATTERN_C
            pppp8888 88777776 ppppgggg ggfffffe
            66666555 55444444 eeeeeddd ddcccccc
            33333222 22211111 bbbbbaaa aaa99999
        */

        case 25: /* 11101010 */

            *p     = MASK_L(in[9],  0x1f, 0);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[8],  0x07, 3) | MASK_R(in[9],  0xe0, 5); SIGN_EXTEND_6(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_R(in[8],  0xf8, 3);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_R(in[5],  0x3f, 0);                           SIGN_EXTEND_6(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[4],  0x07, 2) | MASK_R(in[5],  0xc0, 6); SIGN_EXTEND_5(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[1],  0x01, 5) | MASK_R(in[4],  0xf8, 3); SIGN_EXTEND_6(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_R(in[1],  0x3e, 1);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[0],  0x0f, 2) | MASK_R(in[1],  0xc0, 6); SIGN_EXTEND_6(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[11], 0x1f, 0);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[10], 0x07, 3) | MASK_R(in[11], 0xe0, 5); SIGN_EXTEND_6(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_R(in[10], 0xf8, 3);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_R(in[7],  0x3f, 0);                           SIGN_EXTEND_6(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[6],  0x07, 2) | MASK_R(in[7],  0xc0, 6); SIGN_EXTEND_5(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[3],  0x01, 5) | MASK_R(in[6],  0xf8, 3); SIGN_EXTEND_6(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_R(in[3],  0x3e, 1);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_6(*p);
            *(++p) = MASK_L(in[2],  0x0f, 2) | MASK_R(in[3],  0xc0, 6); SIGN_EXTEND_6(*p); SHIFT_ROUND_6(*p);

            // 8 linears

            INTERPOLATE_8(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 26: /* 11101011 */

            *p     = MASK_L(in[9],  0x1f, 0);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[8],  0x07, 3) | MASK_R(in[9],  0xe0, 5); SIGN_EXTEND_6(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_R(in[8],  0xf8, 3);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_R(in[5],  0x3f, 0);                           SIGN_EXTEND_6(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[4],  0x07, 2) | MASK_R(in[5],  0xc0, 6); SIGN_EXTEND_5(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[1],  0x01, 5) | MASK_R(in[4],  0xf8, 3); SIGN_EXTEND_6(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_R(in[1],  0x3e, 1);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[0],  0x0f, 2) | MASK_R(in[1],  0xc0, 6); SIGN_EXTEND_6(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[11], 0x1f, 0);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[10], 0x07, 3) | MASK_R(in[11], 0xe0, 5); SIGN_EXTEND_6(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_R(in[10], 0xf8, 3);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_R(in[7],  0x3f, 0);                           SIGN_EXTEND_6(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[6],  0x07, 2) | MASK_R(in[7],  0xc0, 6); SIGN_EXTEND_5(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[3],  0x01, 5) | MASK_R(in[6],  0xf8, 3); SIGN_EXTEND_6(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_R(in[3],  0x3e, 1);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_7(*p);
            *(++p) = MASK_L(in[2],  0x0f, 2) | MASK_R(in[3],  0xc0, 6); SIGN_EXTEND_6(*p); SHIFT_ROUND_7(*p);

            // 8 linears

            INTERPOLATE_8(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 27: /* 11101100 */

            *p     = MASK_L(in[9],  0x1f, 0);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[8],  0x07, 3) | MASK_R(in[9],  0xe0, 5); SIGN_EXTEND_6(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_R(in[8],  0xf8, 3);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_R(in[5],  0x3f, 0);                           SIGN_EXTEND_6(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[4],  0x07, 2) | MASK_R(in[5],  0xc0, 6); SIGN_EXTEND_5(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[1],  0x01, 5) | MASK_R(in[4],  0xf8, 3); SIGN_EXTEND_6(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_R(in[1],  0x3e, 1);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[0],  0x0f, 2) | MASK_R(in[1],  0xc0, 6); SIGN_EXTEND_6(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[11], 0x1f, 0);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[10], 0x07, 3) | MASK_R(in[11], 0xe0, 5); SIGN_EXTEND_6(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_R(in[10], 0xf8, 3);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_R(in[7],  0x3f, 0);                           SIGN_EXTEND_6(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[6],  0x07, 2) | MASK_R(in[7],  0xc0, 6); SIGN_EXTEND_5(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[3],  0x01, 5) | MASK_R(in[6],  0xf8, 3); SIGN_EXTEND_6(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_R(in[3],  0x3e, 1);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[2],  0x0f, 2) | MASK_R(in[3],  0xc0, 6); SIGN_EXTEND_6(*p); SHIFT_ROUND_8(*p);

            // 8 linears

            INTERPOLATE_8(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 28: /* 11101101 */

            *p     = MASK_L(in[9],  0x1f, 0);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[8],  0x07, 3) | MASK_R(in[9],  0xe0, 5); SIGN_EXTEND_6(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_R(in[8],  0xf8, 3);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_R(in[5],  0x3f, 0);                           SIGN_EXTEND_6(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[4],  0x07, 2) | MASK_R(in[5],  0xc0, 6); SIGN_EXTEND_5(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[1],  0x01, 5) | MASK_R(in[4],  0xf8, 3); SIGN_EXTEND_6(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_R(in[1],  0x3e, 1);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[0],  0x0f, 2) | MASK_R(in[1],  0xc0, 6); SIGN_EXTEND_6(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[11], 0x1f, 0);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[10], 0x07, 3) | MASK_R(in[11], 0xe0, 5); SIGN_EXTEND_6(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_R(in[10], 0xf8, 3);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_R(in[7],  0x3f, 0);                           SIGN_EXTEND_6(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[6],  0x07, 2) | MASK_R(in[7],  0xc0, 6); SIGN_EXTEND_5(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[3],  0x01, 5) | MASK_R(in[6],  0xf8, 3); SIGN_EXTEND_6(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_R(in[3],  0x3e, 1);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_9(*p);
            *(++p) = MASK_L(in[2],  0x0f, 2) | MASK_R(in[3],  0xc0, 6); SIGN_EXTEND_6(*p); SHIFT_ROUND_9(*p);

            // 8 linears

            INTERPOLATE_8(d0, out);

            //PREVENT_OVERFLOW_16(out);

            return;

        case 29: /* 11101110 */

            *p     = MASK_L(in[9],  0x1f, 0);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[8],  0x07, 3) | MASK_R(in[9],  0xe0, 5); SIGN_EXTEND_6(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_R(in[8],  0xf8, 3);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_R(in[5],  0x3f, 0);                           SIGN_EXTEND_6(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[4],  0x07, 2) | MASK_R(in[5],  0xc0, 6); SIGN_EXTEND_5(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[1],  0x01, 5) | MASK_R(in[4],  0xf8, 3); SIGN_EXTEND_6(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_R(in[1],  0x3e, 1);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[0],  0x0f, 2) | MASK_R(in[1],  0xc0, 6); SIGN_EXTEND_6(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[11], 0x1f, 0);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[10], 0x07, 3) | MASK_R(in[11], 0xe0, 5); SIGN_EXTEND_6(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_R(in[10], 0xf8, 3);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_R(in[7],  0x3f, 0);                           SIGN_EXTEND_6(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[6],  0x07, 2) | MASK_R(in[7],  0xc0, 6); SIGN_EXTEND_5(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[3],  0x01, 5) | MASK_R(in[6],  0xf8, 3); SIGN_EXTEND_6(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_R(in[3],  0x3e, 1);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[2],  0x0f, 2) | MASK_R(in[3],  0xc0, 6); SIGN_EXTEND_6(*p); SHIFT_ROUND_10(*p);

            // 8 linears

            INTERPOLATE_8(d0, out);

            PREVENT_OVERFLOW_16(out);

            return;

        case 30: /* 11101111 */

#ifdef DANIELO_DOUBLE
            *p     = MASK_L(in[9],  0x1f, 0);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[8],  0x07, 3) | MASK_R(in[9],  0xe0, 5); SIGN_EXTEND_6(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_R(in[8],  0xf8, 3);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_11(*p);
            *(++p) = MASK_R(in[5],  0x3f, 0);                           SIGN_EXTEND_6(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[4],  0x07, 2) | MASK_R(in[5],  0xc0, 6); SIGN_EXTEND_5(*p); SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[1],  0x01, 5) | MASK_R(in[4],  0xf8, 3); SIGN_EXTEND_6(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_R(in[1],  0x3e, 1);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[0],  0x0f, 2) | MASK_R(in[1],  0xc0, 6); SIGN_EXTEND_6(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[11], 0x1f, 0);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[10], 0x07, 3) | MASK_R(in[11], 0xe0, 5); SIGN_EXTEND_6(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_R(in[10], 0xf8, 3);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_11(*p);
            *(++p) = MASK_R(in[7],  0x3f, 0);                           SIGN_EXTEND_6(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[6],  0x07, 2) | MASK_R(in[7],  0xc0, 6); SIGN_EXTEND_5(*p); SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[3],  0x01, 5) | MASK_R(in[6],  0xf8, 3); SIGN_EXTEND_6(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_R(in[3],  0x3e, 1);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_11(*p);
            *(++p) = MASK_L(in[2],  0x0f, 2) | MASK_R(in[3],  0xc0, 6); SIGN_EXTEND_6(*p); SHIFT_ROUND_10(*p);

            // 16 linears - but odd samples are doubled
#else
            *p     = MASK_L(in[9],  0x1f, 0);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[8],  0x07, 3) | MASK_R(in[9],  0xe0, 5); SIGN_EXTEND_6(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_R(in[8],  0xf8, 3);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_R(in[5],  0x3f, 0);                           SIGN_EXTEND_6(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[4],  0x07, 2) | MASK_R(in[5],  0xc0, 6); SIGN_EXTEND_5(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[1],  0x01, 5) | MASK_R(in[4],  0xf8, 3); SIGN_EXTEND_6(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_R(in[1],  0x3e, 1);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[0],  0x0f, 2) | MASK_R(in[1],  0xc0, 6); SIGN_EXTEND_6(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[11], 0x1f, 0);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[10], 0x07, 3) | MASK_R(in[11], 0xe0, 5); SIGN_EXTEND_6(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_R(in[10], 0xf8, 3);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_R(in[7],  0x3f, 0);                           SIGN_EXTEND_6(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[6],  0x07, 2) | MASK_R(in[7],  0xc0, 6); SIGN_EXTEND_5(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[3],  0x01, 5) | MASK_R(in[6],  0xf8, 3); SIGN_EXTEND_6(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_R(in[3],  0x3e, 1);                           SIGN_EXTEND_5(*p); SHIFT_ROUND_10(*p);
            *(++p) = MASK_L(in[2],  0x0f, 2) | MASK_R(in[3],  0xc0, 6); SIGN_EXTEND_6(*p); SHIFT_ROUND_10(*p);

            // 16 linears - but odd samples are doubled

            DOUBLE_ODDS(out);
#endif

            PREVENT_OVERFLOW_16(out);

            return;

        /*=====================================================================
            PATTERN_D
            pppp8888 88887777 ppppggggggggffff
            66666655 55444444 eeeeeeddddcccccc
            44333322 22221111 ccbbbbaaaaaa9999
        */

        case 31: /* 11111010 */

            *p     = MASK_L(in[9],  0x0f, 0);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[8],  0x03, 4) | MASK_R(in[9],  0xf0, 4); SIGN_EXTEND_6(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_R(in[8],  0x3c, 2);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[5],  0x3f, 2) | MASK_R(in[8],  0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[4],  0x03, 2) | MASK_R(in[5],  0xc0, 6); SIGN_EXTEND_4(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_R(in[4],  0xfc, 2);                           SIGN_EXTEND_6(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_R(in[1],  0x0f, 0);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[0],  0x0f, 4) | MASK_R(in[1],  0xf0, 4); SIGN_EXTEND_8(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[11], 0x0f, 0);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[10], 0x03, 4) | MASK_R(in[11], 0xf0, 4); SIGN_EXTEND_6(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_R(in[10], 0x3c, 2);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[7],  0x3f, 2) | MASK_R(in[10], 0xc0, 6); SIGN_EXTEND_8(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[6],  0x03, 2) | MASK_R(in[7],  0xc0, 6); SIGN_EXTEND_4(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_R(in[6],  0xfc, 2);                           SIGN_EXTEND_6(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_R(in[3],  0x0f, 0);                           SIGN_EXTEND_4(*p); SHIFT_ROUND_8(*p);
            *(++p) = MASK_L(in[2],  0x0f, 4) | MASK_R(in[3],  0xf0, 4); SIGN_EXTEND_8(*p); SHIFT_ROUND_8(*p);

            // 4 Linears

            INTERPOLATE_4(d0, out);

            PREVENT_OVERFLOW_16(out);

            return;

        /*=====================================================================
            PATTERN ?
        */

        case 32: /* 11111011 */

            /* Unknown - never occurs? */
            ZERO(out);

            return;

        case 33: /* 11111100 */

            /* Unknown - never occurs? */
            ZERO(out);

            return;

        case 34: /* 11111101 */

            /* Unknown - never occurs? */
            ZERO(out);

            return;

        case 35: /* 11111110 */

            /* Unknown - never occurs? */
            ZERO(out);

            return;

        case 36: /* 11111111 */

            /* Unknown - never occurs? */
            ZERO(out);

            return;

        default: break;
    }
}

//*****************************************************************************
void decodeM16(unsigned char *in, int *out)
{
    // Decodes a 32-byte M16 block into 16 16-bit samples
    int i;
    for (i=0; i<16; i++) {
        out[i] = MASK_L(in[i*2], 0xff, 8) | MASK_R(in[i*2+1], 0xff, 0);
        SIGN_EXTEND_16(out[i]);
    }
}

//*****************************************************************************
void decodeM24(unsigned char *in, int *out)
{
    // Decodes a 48-byte M24 block into 16 24-bit samples
    int i;
    for (i=0; i<16; i++) {
        out[i] = MASK_L(in[i*3], 0xff, 16) | MASK_L(in[i*3+1], 0xff, 8) | MASK_R(in[i*3+2], 0xff, 0);
        SIGN_EXTEND_24(out[i]);
    }
}

//*****************************************************************************
void decodeCDR(unsigned char *in, int *out)
{
    // Decodes a 64-byte CDR block into 32 16-bit samples (2 interleaved channels, little-endian)
    int i;
    for (i=0; i<16; i++) {
        out[i*2]   = MASK_L(in[i*4+1], 0xff, 8) | MASK_R(in[i*4],   0xff, 0);
        out[i*2+1] = MASK_L(in[i*4+3], 0xff, 8) | MASK_R(in[i*4+2], 0xff, 0);
        SIGN_EXTEND_16(out[i*2]);
        SIGN_EXTEND_16(out[i*2+1]);
    }
}
