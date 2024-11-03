#ifndef _BITMAP_H
#define _BITMAP_H

#define byte unsigned char
#define ushort unsigned short

typedef struct Bitmap {
  byte bitCount;
  ushort* bits;
} Bitmap;

ushort* initBitmap(Bitmap* bitmap, byte size);


#endif