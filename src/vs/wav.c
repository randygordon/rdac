#include <stdio.h>

/*
    VS to WAV file converter for Roland VS. Version 0.99

    Copyright 2006 LGPL by Randy Gordon (randy@integrand.com)

    Special thanks to Danielo for solving many of the MTP patterns.
*/

#define BYTE_0(xx)  (xx&0xff)
#define BYTE_1(xx)  ((xx>>8)&0xff)
#define BYTE_2(xx)  ((xx>>16)&0xff)
#define BYTE_3(xx)  ((xx>>24)&0xff)

//*****************************************************************************
void writeWavHeader(FILE *fout, int numSamples, int sampleRate, int bitDepth, int numChannels)
{
    // Write the WAV header (mono)
    int bytesPerSample = bitDepth/8;
    int numBytes       = numSamples*bytesPerSample*numChannels;

    int  numChunkBytes = numBytes + 38;

    unsigned char chunkid[] = {0x52,0x49,0x46,0x46};
    fwrite(chunkid, 1, 4, fout);

    unsigned char chunksize[] = {
        BYTE_0(numChunkBytes),
        BYTE_1(numChunkBytes),
        BYTE_2(numChunkBytes),
        BYTE_3(numChunkBytes)
    };
    fwrite(chunksize, 1, 4, fout);

    unsigned char format[] = {0x57,0x41,0x56,0x45};
    fwrite(format, 1, 4, fout);

    unsigned char subchunk1id[] = {0x66,0x6d,0x74,0x20};
    fwrite(subchunk1id, 1, 4, fout);

    unsigned char subchunk1size[] = {0x12,0x00,0x00,0x00};
    fwrite(subchunk1size, 1, 4, fout);

    unsigned char audioformat[] = {0x01,0x00};
    fwrite(audioformat, 1, 2, fout);

    unsigned char numchannels[] = {
        BYTE_0(numChannels),
        BYTE_1(numChannels)
    };
    fwrite(numchannels, 1, 2, fout);

    unsigned char samplerate[] = {
        BYTE_0(sampleRate),
        BYTE_1(sampleRate),
        BYTE_2(sampleRate),
        BYTE_3(sampleRate)
    };
    fwrite(samplerate, 1, 4, fout);

    unsigned char byterate[] = {
        BYTE_0(sampleRate*bytesPerSample*numChannels),
        BYTE_1(sampleRate*bytesPerSample*numChannels),
        BYTE_2(sampleRate*bytesPerSample*numChannels),
        BYTE_3(sampleRate*bytesPerSample*numChannels)
    };
    fwrite(byterate, 1, 4, fout);

    unsigned char blockalign[] = {
        BYTE_0(bytesPerSample*numChannels),
        BYTE_1(bytesPerSample*numChannels)
    };
    fwrite(blockalign, 1, 2, fout);

    unsigned char bitspersample[] = {
        BYTE_0(bitDepth),
        BYTE_1(bitDepth)
    };
    fwrite(bitspersample, 1, 2, fout);

    unsigned char extraparamsize[] = {0x00,0x00};
    fwrite(extraparamsize, 1, 2, fout);

    unsigned char subchunk2id[] = {0x64,0x61,0x74,0x61};
    fwrite(subchunk2id, 1, 4, fout);

    unsigned char subchunk2size[] = {
        BYTE_0(numBytes),
        BYTE_1(numBytes),
        BYTE_2(numBytes),
        BYTE_3(numBytes)
    };
    fwrite(subchunk2size, 1, 4, fout);
}
//*****************************************************************************
void writeWavSamples24(FILE *fout, int samples[])
{
    // Write it to the WAV (little-endian)
    int i;
    for (i=0; i<16; i++) {
        fputc(BYTE_0(samples[i]), fout);
        fputc(BYTE_1(samples[i]), fout);
        fputc(BYTE_2(samples[i]), fout);
    }
}
//*****************************************************************************
void writeWavSamples24as16(FILE *fout, int samples[])
{
    // Write it to the WAV (little-endian)
    // To convert the 24-bit to 16-bit, byte 0 is tossed.
    int i;
    for (i=0; i<16; i++) {
        fputc(BYTE_1(samples[i]), fout);
        fputc(BYTE_2(samples[i]), fout);
    }
}
//*****************************************************************************
void writeWavSamples16(FILE *fout, int samples[])
{
    // Write it to the WAV (little-endian)
    int i;
    for (i=0; i<16; i++) {
        fputc(BYTE_0(samples[i]), fout);
        fputc(BYTE_1(samples[i]), fout);
    }
}
//*****************************************************************************
void writeWavSamples16as24(FILE *fout, int samples[])
{
    // Write it to the WAV (little-endian)
    // To convert the 16-bit to 24-bit, set zero as low-order byte.
    int i;
    for (i=0; i<16; i++) {
        fputc(0, fout);
        fputc(BYTE_0(samples[i]), fout);
        fputc(BYTE_1(samples[i]), fout);
    }
}
//*****************************************************************************
void writeWavSamples16Stereo(FILE *fout, int samples[])
{
    // Write it to the WAV (little-endian)
    int i;
    for (i=0; i<16; i++) {
        fputc(BYTE_0(samples[i*2]),   fout);
        fputc(BYTE_1(samples[i*2]),   fout);
        fputc(BYTE_0(samples[i*2+1]), fout);
        fputc(BYTE_1(samples[i*2+1]), fout);
    }
}
//*****************************************************************************
void writeWavSamples16as24Stereo(FILE *fout, int samples[])
{
    // Write it to the WAV (little-endian)
    // To convert the 16-bit to 24-bit, set zero as low-order byte.
    int i;
    for (i=0; i<16; i++) {
        fputc(0, fout);
        fputc(BYTE_0(samples[i*2]),   fout);
        fputc(BYTE_1(samples[i*2]),   fout);
        fputc(0, fout);
        fputc(BYTE_0(samples[i*2+1]), fout);
        fputc(BYTE_1(samples[i*2+1]), fout);
    }
}
