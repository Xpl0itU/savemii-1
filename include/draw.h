#pragma once

#include <coreinit/screen.h>
#include <tga_reader.h>

void drawTGA(int x, int y, float scale, uint8_t *fileContent);
void drawRGB5A3(int x, int y, float scale, uint8_t *pixels);