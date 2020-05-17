#include <stdio.h>
#include <sys/stat.h>

/*
    VS to WAV file converter for Roland VS. Version 0.99

    Copyright 2006 LGPL by Randy Gordon (randy@integrand.com)

    Special thanks to Danielo for solving many of the MTP patterns.
*/

long fsize(const char *const name);

void convertMTP(char *inFileName, char *outFileName, int sampleRate, int bitDepth);
void convertVSR(char *inFileName, char *outFileName, int sampleRate, int bitDepth);
void convertMT1(char *inFileName, char *outFileName, int sampleRate, int bitDepth);
void convertMT2(char *inFileName, char *outFileName, int sampleRate, int bitDepth, char *clusterSize);
void convertM16(char *inFileName, char *outFileName, int sampleRate, int bitDepth);
void convertM24(char *inFileName, char *outFileName, int sampleRate, int bitDepth, char *clusterSize);
void convertCDR(char *inFileName, char *outFileName, int sampleRate, int bitDepth);

//*****************************************************************************
int main(int argc, char **argv)
{
    int i;

    char *inFileName  = NULL;
    char *outFileName = NULL;

    char *rdacMode    = "mtp";
    char *clusterSize = "32k";
    int   sampleRate  = 44100;
    int   bitDepth    = 0;

    // Very basic argument processing
    i = 1;
    while (i<argc) {
        if (!strcmp(argv[i], "-d")) {
            // Process option argument
            if (++i >= argc) break;
            bitDepth = atoi(argv[i]);
        }
        else if (!strcmp(argv[i], "-r")) {
            if (++i >= argc) break;
            sampleRate = atoi(argv[i]);
        }
        else if (!strcmp(argv[i], "-m")) {
            if (++i >= argc) break;
            rdacMode = argv[i];
        }
        else if (!strcmp(argv[i], "-c")) {
            if (++i >= argc) break;
            clusterSize = argv[i];
        }
        else {
            // Process file name
            if   (inFileName == NULL) inFileName  = argv[i];
            else                      outFileName = argv[i];
        }
        i++;
    }

    // Set default bit-depth if not specified.
    if (bitDepth == 0) {
        if      (stricmp(rdacMode, "mtp") == 0) bitDepth = 24;
        else if (stricmp(rdacMode, "vsr") == 0) bitDepth = 24;
        else if (stricmp(rdacMode, "m24") == 0) bitDepth = 24;
        else                                    bitDepth = 16;
    }

    // Show usage if arguments are invalid
    if (inFileName  == NULL || strlen(inFileName)  == 0 ||
        outFileName == NULL || strlen(outFileName) == 0 ||
        !isSupportedBitDepth(bitDepth) ||
        !isSupportedMode(rdacMode) ||
        !isSupportedClusterSize(clusterSize)) {

        printf("\n");
        printf("**********************************************************\n");
        printf("*                                                        *\n");
        printf("*    RDAC to Wav File Converter (Version 0.99)           *\n");
        printf("*                                                        *\n");
        printf("*        Copyright Randy Gordon (randy@integrand.com)    *\n");
        printf("*                                                        *\n");
        printf("**********************************************************\n");
        printf("\n");
        printf("Usage: rdac2wav [options] <input_file> <output_file>\n\n");
        printf("\n");
        printf("where options include:\n");
        printf("\n");
        printf("    -m <rdac_mode>    : MTP,MT1,MT2,VSR,\n");
        printf("                        M16,M24,CDR             (default: MTP)\n");
        printf("    -r <sample_rate>  : select the sample rate  (default: 44100)\n");
        printf("    -d <bit_depth>    : select 24 or 16-bit WAV (default: mode dependent)\n");
        printf("    -c <cluster_size> : select 32K or 64K       (default: 32K)\n");
        printf("\n");
        printf("eg.:     rdac2wav take01df.vr6 guitar.wav\n");
        printf("\n");
        exit(0);
    }

    if      (stricmp(rdacMode, "mtp") == 0) convertMTP(inFileName, outFileName, sampleRate, bitDepth);
    else if (stricmp(rdacMode, "vsr") == 0) convertVSR(inFileName, outFileName, sampleRate, bitDepth);
    else if (stricmp(rdacMode, "mt1") == 0) convertMT1(inFileName, outFileName, sampleRate, bitDepth);
    else if (stricmp(rdacMode, "mt2") == 0) convertMT2(inFileName, outFileName, sampleRate, bitDepth, clusterSize);
    else if (stricmp(rdacMode, "m16") == 0) convertM16(inFileName, outFileName, sampleRate, bitDepth);
    else if (stricmp(rdacMode, "m24") == 0) convertM24(inFileName, outFileName, sampleRate, bitDepth, clusterSize);
    else if (stricmp(rdacMode, "cdr") == 0) convertCDR(inFileName, outFileName, sampleRate, bitDepth);
}
//*****************************************************************************
void convertMTP(char *inFileName, char *outFileName, int sampleRate, int bitDepth)
{
    int i;

    FILE *fin  = fopen(inFileName,  "rb");
    FILE *fout = fopen(outFileName, "wb");

    if (fin == NULL) {
        printf("Unable to open input file: %s.\n", inFileName);
        exit(1);
    }
    if (fout == NULL) {
        printf("Unable to open output file: %s.\n", outFileName);
        exit(1);
    }

    int fileSize  = fsize(inFileName);
    int numBlocks = fileSize/16;

    // Write the WAV header
    int  numSamples = numBlocks*16;
    writeWavHeader(fout, numSamples, sampleRate, bitDepth, 1);

    unsigned char in[16];  // Input MTP RDAC block
    int           out[16]; // Output 24-bit samples

    int d0 = 0;

    if (bitDepth == 24) {
        for (i=0; i<numBlocks; i++) {
            fread(in, 1, 16, fin);
            decodeMTP(d0, in, out);
            writeWavSamples24(fout, out);
            d0 = out[15];
        }
    }
    else if (bitDepth == 16) {
        for (i=0; i<numBlocks; i++) {
            fread(in, 1, 16, fin);
            decodeMTP(d0, in, out);
            writeWavSamples24as16(fout, out);
            d0 = out[15];
        }
    }
    fclose(fin);
    fclose(fout);
}
//*****************************************************************************
void convertVSR(char *inFileName, char *outFileName, int sampleRate, int bitDepth)
{
    int i, j;

    FILE *fin  = fopen(inFileName,  "rb");
    FILE *fout = fopen(outFileName, "wb");

    if (fin == NULL) {
        printf("Unable to open input file: %s.\n", inFileName);
        exit(1);
    }
    if (fout == NULL) {
        printf("Unable to open output file: %s.\n", outFileName);
        exit(1);
    }

    int fileSize  = fsize(inFileName);
    int numBlocks = fileSize/16;

    // Write the WAV header
    int  numSamples = numBlocks*16;
    writeWavHeader(fout, numSamples, sampleRate, bitDepth, 1);

    unsigned char in[16];  // Input VSR RDAC block
    int           out[16]; // Output 24-bit samples

    int d0 = 0;

    if (bitDepth == 24) {
        for (i=0; i<numBlocks; i++) {
            fread(in, 1, 16, fin);
            decodeMTP(d0, in, out);
            d0 = out[15];
            // VSR mode is like MTP, but seems to need this bit of post-processing.
            for (j=0;j<16; j++) {
                if      (out[j] >=  65536) out[j] -= 65536;
                else if (out[j] <= -65536) out[j] += 65536;
                else                       out[j] = 0;
            }
            writeWavSamples24(fout, out);
        }
    }
    else if (bitDepth == 16) {
        for (i=0; i<numBlocks; i++) {
            fread(in, 1, 16, fin);
            decodeMTP(d0, in, out);
            d0 = out[15];
            // VSR mode is like MTP, but seems to need this bit of post-processing.
            for (j=0;j<16; j++) {
                if      (out[j] >=  65536) out[j] -= 65536;
                else if (out[j] <= -65536) out[j] += 65536;
                else                       out[j] = 0;
            }
            writeWavSamples24as16(fout, out);
        }
    }
    fclose(fin);
    fclose(fout);
}
//*****************************************************************************
void convertMT1(char *inFileName, char *outFileName, int sampleRate, int bitDepth)
{
    int i;

    FILE *fin  = fopen(inFileName,  "rb");
    FILE *fout = fopen(outFileName, "wb");

    if (fin == NULL) {
        printf("Unable to open input file: %s.\n", inFileName);
        exit(1);
    }
    if (fout == NULL) {
        printf("Unable to open output file: %s.\n", outFileName);
        exit(1);
    }

    int fileSize  = fsize(inFileName);
    int numBlocks = fileSize/16;

    // Write the WAV header
    int  numSamples = numBlocks*16;
    writeWavHeader(fout, numSamples, sampleRate, bitDepth, 1);

    unsigned char in[16];  // Input MT1 RDAC block
    int           out[16]; // Output 16-bit samples

    int d0 = 0;

    if (bitDepth == 24) {
        for (i=0; i<numBlocks; i++) {
            fread(in, 1, 16, fin);
            decodeMT1(d0, in, out);
            writeWavSamples16as24(fout, out);
            d0 = out[15];
        }
    }
    else if (bitDepth == 16) {
        for (i=0; i<numBlocks; i++) {
            fread(in, 1, 16, fin);
            decodeMT1(d0, in, out);
            writeWavSamples16(fout, out);
            d0 = out[15];
        }
    }

    fclose(fin);
    fclose(fout);
}
//*****************************************************************************
void convertMT2(char *inFileName, char *outFileName, int sampleRate, int bitDepth, char *clusterSize)
{
    int i;

    FILE *fin  = fopen(inFileName,  "rb");
    FILE *fout = fopen(outFileName, "wb");

    if (fin == NULL) {
        printf("Unable to open input file: %s.\n", inFileName);
        exit(1);
    }
    if (fout == NULL) {
        printf("Unable to open output file: %s.\n", outFileName);
        exit(1);
    }

    int fileSize  = fsize(inFileName);

    // Determine number of blocks based on page size (for alignment)
    int pageSize      = 32768;
    int blocksPerPage = 2730;

    if (stricmp(clusterSize,"32k") == 0) {
        pageSize      = 32768;
        blocksPerPage = 2730;
    }
    else if (stricmp(clusterSize,"64k") == 0) {
        pageSize      = 65536;
        blocksPerPage = 5460;
    }

    int pagePadBytes  = pageSize - blocksPerPage*12;
    int numPages      = ((int)fileSize)/pageSize;
    int leftOverBytes = ((int)fileSize)%pageSize;
    int numBlocks     = numPages*blocksPerPage + leftOverBytes/12;

    // Write the WAV header
    int  numSamples = numBlocks*16;
    writeWavHeader(fout, numSamples, sampleRate, bitDepth, 1);


    unsigned char in[12];  // Input MT2 RDAC block
    int           out[16]; // Output 16-bit samples
    unsigned char dummy[200];

    int d0 = 0;

    if (bitDepth == 24) {
        for (i=0; i<numBlocks; i++) {
            fread(in, 1, 12, fin);
            decodeMT2(d0, in, out);
            writeWavSamples16as24(fout, out);
            d0 = out[15];
            // Check if at end of page boundary - if so, eat pad bytes.
            if ((i+1)%blocksPerPage == 0) fread(dummy, 1, pagePadBytes, fin);
        }
    }
    else if (bitDepth == 16) {
        for (i=0; i<numBlocks; i++) {
            fread(in, 1, 12, fin);
            decodeMT2(d0, in, out);
            writeWavSamples16(fout, out);
            d0 = out[15];
            // Check if at end of page boundary - if so, eat pad bytes.
            if ((i+1)%blocksPerPage == 0) fread(dummy, 1, pagePadBytes, fin);
        }
    }

    fclose(fin);
    fclose(fout);
}
//*****************************************************************************
void convertM16(char *inFileName, char *outFileName, int sampleRate, int bitDepth)
{
    int i;

    FILE *fin  = fopen(inFileName,  "rb");
    FILE *fout = fopen(outFileName, "wb");

    if (fin == NULL) {
        printf("Unable to open input file: %s.\n", inFileName);
        exit(1);
    }
    if (fout == NULL) {
        printf("Unable to open output file: %s.\n", outFileName);
        exit(1);
    }

    int fileSize  = fsize(inFileName);
    int numBlocks = fileSize/32;

    // Write the WAV header
    int  numSamples = numBlocks*16;
    writeWavHeader(fout, numSamples, sampleRate, bitDepth, 1);

    unsigned char in[32];  // Input M16 block
    int           out[16]; // Output 16-bit samples

    if (bitDepth == 24) {
        for (i=0; i<numBlocks; i++) {
            fread(in, 1, 32, fin);
            decodeM16(in, out);
            writeWavSamples16as24(fout, out);
        }
    }
    else if (bitDepth == 16) {
        for (i=0; i<numBlocks; i++) {
            fread(in, 1, 32, fin);
            decodeM16(in, out);
            writeWavSamples16(fout, out);
        }
    }

    fclose(fin);
    fclose(fout);
}
//*****************************************************************************
void convertM24(char *inFileName, char *outFileName, int sampleRate, int bitDepth, char *clusterSize)
{
    int i;

    FILE *fin  = fopen(inFileName,  "rb");
    FILE *fout = fopen(outFileName, "wb");

    if (fin == NULL) {
        printf("Unable to open input file: %s.\n", inFileName);
        exit(1);
    }
    if (fout == NULL) {
        printf("Unable to open output file: %s.\n", outFileName);
        exit(1);
    }

    int fileSize  = fsize(inFileName);

    // Determine number of blocks based on page size (for alignment)
    int pageSize      = 32768;
    int blocksPerPage = 2730;

    if (stricmp(clusterSize,"32k") == 0) {
        pageSize      = 32768;
        blocksPerPage = 682;
    }
    else if (stricmp(clusterSize,"64k") == 0) {
        pageSize      = 65536;
        blocksPerPage = 1364;
    }

    int pagePadBytes  = pageSize - blocksPerPage*48;
    int numPages      = ((int)fileSize)/pageSize;
    int leftOverBytes = ((int)fileSize)%pageSize;
    int numBlocks     = numPages*blocksPerPage + leftOverBytes/48;

    // Write the WAV header
    int  numSamples = numBlocks*16;
    writeWavHeader(fout, numSamples, sampleRate, bitDepth, 1);

    unsigned char in[48];  // Input M24 block
    int           out[16]; // Output 16-bit samples
    unsigned char dummy[200];

    if (bitDepth == 24) {
        for (i=0; i<numBlocks; i++) {
            fread(in, 1, 48, fin);
            decodeM24(in, out);
            writeWavSamples24(fout, out);
            // Check if at end of page boundary - if so, eat pad bytes.
            if ((i+1)%blocksPerPage == 0) fread(dummy, 1, pagePadBytes, fin);
        }
    }
    else if (bitDepth == 16) {
        for (i=0; i<numBlocks; i++) {
            fread(in, 1, 48, fin);
            decodeM24(in, out);
            writeWavSamples24as16(fout, out);
            // Check if at end of page boundary - if so, eat pad bytes.
            if ((i+1)%blocksPerPage == 0) fread(dummy, 1, pagePadBytes, fin);
        }
    }

    fclose(fin);
    fclose(fout);
}
//*****************************************************************************
void convertCDR(char *inFileName, char *outFileName, int sampleRate, int bitDepth)
{
    int i;

    FILE *fin  = fopen(inFileName,  "rb");
    FILE *fout = fopen(outFileName, "wb");

    if (fin == NULL) {
        printf("Unable to open input file: %s.\n", inFileName);
        exit(1);
    }
    if (fout == NULL) {
        printf("Unable to open output file: %s.\n", outFileName);
        exit(1);
    }

    int fileSize  = fsize(inFileName);
    int numBlocks = fileSize/64;

    // Write the WAV header
    int  numSamples = numBlocks*16;
    writeWavHeader(fout, numSamples, sampleRate, bitDepth, 2);

    unsigned char in[64];  // Input CDR block
    int           out[32]; // Output 16-bit samples (2 channels)

    if (bitDepth == 24) {
        for (i=0; i<numBlocks; i++) {
            fread(in, 1, 64, fin);
            decodeCDR(in, out);
            writeWavSamples16as24Stereo(fout, out);
        }
    }
    else if (bitDepth == 16) {
        for (i=0; i<numBlocks; i++) {
            fread(in, 1, 64, fin);
            decodeCDR(in, out);
            writeWavSamples16Stereo(fout, out);
        }
    }

    fclose(fin);
    fclose(fout);
}
//*****************************************************************************
int isSupportedMode(char *mode)
{
    if (stricmp(mode, "mtp") == 0) return 1;
    if (stricmp(mode, "vsr") == 0) return 1;
    if (stricmp(mode, "mt1") == 0) return 1;
    if (stricmp(mode, "mt2") == 0) return 1;
    if (stricmp(mode, "m16") == 0) return 1;
    if (stricmp(mode, "m24") == 0) return 1;
    if (stricmp(mode, "cdr") == 0) return 1;
    return 0;
}
//*****************************************************************************
int isSupportedClusterSize(char *size)
{
    if (stricmp(size, "32k") == 0) return 1;
    if (stricmp(size, "64k") == 0) return 1;
    return 0;
}
//*****************************************************************************
int isSupportedBitDepth(int bitDepth)
{
    return bitDepth == 16 || bitDepth == 24;
}
//*****************************************************************************
long fsize(const char *const name)
{
    struct stat stbuf;
    if(stat(name, &stbuf) == -1) {
        return -1; // The file could not be accessed.
    }
    return stbuf.st_size;
}
