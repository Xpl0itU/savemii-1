/**
 * tga_reader.h
 *
 * Copyright (c) 2014 Kenji Sasaki
 * Released under the MIT license.
 * https://github.com/npedotnet/TGAReader/blob/master/LICENSE
 *
 * English document
 * https://github.com/npedotnet/TGAReader/blob/master/README.md
 *
 * Japanese document
 * http://3dtech.jp/wiki/index.php?TGAReader
 *
 */

#pragma once

#include <cstdio>
#include <cstdlib>

#ifdef __cplusplus
extern "C" {
#endif

using TGA_ORDER = struct _TGA_ORDER {
    int redShift;
    int greenShift;
    int blueShift;
    int alphaShift;
};

extern const TGA_ORDER *TGA_READER_ARGB;
extern const TGA_ORDER *TGA_READER_ABGR;
extern const TGA_ORDER *TGA_READER_RGBA;

void *tgaMalloc(size_t size);
void tgaFree(void *memory);
int tgaGetWidth(const unsigned char *buffer);
int tgaGetHeight(const unsigned char *buffer);
int *tgaRead(const unsigned char *buffer, const TGA_ORDER *order);

#ifdef __cplusplus
}
#endif