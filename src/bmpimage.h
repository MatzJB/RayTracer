//header file for bmpimage.c


typedef unsigned char bit8;
typedef unsigned short bit16;
typedef unsigned int bit32;

enum { //http://en.wikipedia.org/wiki/BMP_file_format
    BMPmagic1 = 0,
    BMPmagic2 = 1,
    BMPsize = 2,
    BMPunused1 = 6,
    BMPunused2 = 8,
    BMPdataoffset = 10,
    BMPheadersize = 14,
    BMPwidth = 18,
    BMPheight = 22,
    BMPcolorplanes = 26,
    BMPbitsperpixel = 28,
    BMPcompressiontype = 30,
    BMPrawbitmapsize = 34,
    BMPhorizresol = 38,
    BMPverticresol = 42,
    BMPcolorsinpalette = 46,
    BMPnumofimportantcolors = 50,

    BMPrawdata = 54
};

// LEserialX, put data in a serialized buffer at a specified location,
//  and make sure it's stored in a little-endian format as required by the BMP file format

#define BYTESIZE 8

void LEserial8(bit8 *buf, int index, bit8 data);
void LEserial16(bit8 *buf, int index, bit16 data);


void LEserial32(bit8 *buf, int index, bit32 data);


#define A8(X,Y)    LEserial8(bmpbuffer, BMP##X, Y)
#define A16(X,Y)  LEserial16(bmpbuffer, BMP##X, Y)
#define A32(X,Y)  LEserial32(bmpbuffer, BMP##X, Y)

void SaveBMP32RGBA(bit32 *data, int xs, int ys, const char *filename);

