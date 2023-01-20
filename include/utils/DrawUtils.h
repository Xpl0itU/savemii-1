#pragma once

#include <coreinit/memory.h>

#include "schrift.h"
#include <cstdint>

// visible screen sizes
#define SCREEN_WIDTH  854
#define SCREEN_HEIGHT 480

union Color {
    explicit Color(uint32_t color) {
        this->color = color;
    }

    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;
    }

    uint32_t color{};
    struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };
};

class DrawUtils {
public:
    static void setRedraw(bool value);

    static bool getRedraw() { return redraw; }

    static void initBuffers(void *tvBuffer, uint32_t tvSize, void *drcBuffer, uint32_t drcSize);

    static void beginDraw();

    static void endDraw();

    static void clear(Color col);

    static void drawPixel(uint32_t x, uint32_t y, Color col) { drawPixel(x, y, col.r, col.g, col.b, col.a); }

    static void drawPixel(uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    static void drawRectFilled(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    static void drawRectFilled(int x1, int y1, int x2, int y2, Color col) { drawRectFilled(x1, y1, x2, y2, col.r, col.g, col.b, col.a); }

    static void drawRect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t borderSize, Color col);

    static void drawRect(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    static void drawRect(int x1, int y1, int x2, int y2, Color col) { drawRect(x1, y1, x2, y2, col.r, col.g, col.b, col.a); }

    static void drawBitmap(uint32_t x, uint32_t y, uint32_t target_width, uint32_t target_height, const uint8_t *data);

    static bool initFont();

    static bool setFont(OSSharedDataType fontType);

    static void deinitFont();

    static void setFontSize(uint32_t size);

    static void setFontColor(Color col);

    static void print(uint32_t x, uint32_t y, const char *string, bool alignRight = false);

    static void print(uint32_t x, uint32_t y, const wchar_t *string, bool alignRight = false);

    static uint32_t getTextWidth(const char *string);

    static uint32_t getTextWidth(const wchar_t *string);

    static void drawLine(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    static void drawLine(int x1, int y1, int x2, int y2, Color col) { drawLine(x1, y1, x2, y2, col.r, col.g, col.b, col.a); }

    static void drawPic(int x, int y, uint32_t w, uint32_t h, float scale, uint32_t *pixels);

    static void drawTGA(int x, int y, float scale, uint8_t *fileContent);

    static void drawRGB5A3(int x, int y, float scale, uint8_t *fileContent);

private:
    static bool redraw;
    static bool isBackBuffer;

    static uint8_t *tvBuffer;
    static uint32_t tvSize;
    static uint8_t *drcBuffer;
    static uint32_t drcSize;
};
