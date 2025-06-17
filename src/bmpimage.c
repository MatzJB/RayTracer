
#include "bmpimage.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>



void LEserial8(bit8 *buf, int index, bit8 data) {
    buf[index] = data;
}

void LEserial16(bit8 *buf, int index, bit16 data) {
    buf[index + 0] = (data & 0x00FF) >> 0 * BYTESIZE;
    buf[index + 1] = (data & 0xFF00) >> 1 * BYTESIZE;
}

void LEserial32(bit8 *buf, int index, bit32 data) {
    buf[index + 0] = (data & 0x000000FF) >> 0 * BYTESIZE;
    buf[index + 1] = (data & 0x0000FF00) >> 1 * BYTESIZE;
    buf[index + 2] = (data & 0x00FF0000) >> 2 * BYTESIZE;
    buf[index + 3] = (data & 0xFF000000) >> 3 * BYTESIZE;
}

#define A8(X,Y)    LEserial8(bmpbuffer, BMP##X, Y)
#define A16(X,Y)  LEserial16(bmpbuffer, BMP##X, Y)
#define A32(X,Y)  LEserial32(bmpbuffer, BMP##X, Y)

void SaveBMP32RGBA(bit32 *data, int xs, int ys, const char *filename) {
    bit8 *bmpbuffer, *rawbmp;
    int i, j, x, y, totalfilesize, red, green, blue, alpha, argb;
    FILE *outfile;

    totalfilesize = 54 + xs * ys * 4;

    bmpbuffer = (bit8 *) calloc(totalfilesize, sizeof (bit8));

    A8(magic1, 'B');
    A8(magic2, 'M');
    A16(unused1, 0);
    A16(unused2, 0);
    A32(dataoffset, 54); // 54 bytes header, 8 bytes palette
    A32(headersize, 40); // 40 Windows V3 - BITMAPINFOHEADER (most commonly used)
    A16(colorplanes, 1);
    A16(bitsperpixel, 32); // rgba, eg BGRA
    A32(compressiontype, 0); // no compression
    A32(horizresol, 2835); // 2835 pixels/meter = 72dpi
    A32(verticresol, 2835); // 2835 pixels/meter = 72dpi
    A32(colorsinpalette, 0); // no palette 
    A32(numofimportantcolors, 0); // all "colors" are important

    rawbmp = &bmpbuffer[BMPrawdata];
    i = 0;
    j = 0;


    for (y = 0; y < ys; y++) {
        i = xs * (ys - y - 1);
        for (x = 0; x < xs; x++) {
            argb = data[i];
            i++;
            alpha = (argb & 0xFF000000) >> 24;
            blue = (argb & 0x00FF0000) >> 16;
            green = (argb & 0x0000FF00) >> 8;
            red = (argb & 0x000000FF);
            rawbmp[j++] = blue;
            rawbmp[j++] = green;
            rawbmp[j++] = red;
            rawbmp[j++] = alpha;

        }
    }
    A32(size, totalfilesize);
    A32(rawbitmapsize, j);
    A32(width, xs);
    A32(height, ys);

    outfile = fopen(filename, "wb");
    if (outfile == NULL) {
        printf("bmpimage:Could not open file for writing.");
        return;
    }

    fwrite(bmpbuffer, sizeof (bit8), totalfilesize, outfile);
    fclose(outfile);
    free(bmpbuffer);

    return;
}
