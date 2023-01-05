#include <draw.h>
#include <utils/DrawUtils.h>

static void drawLine(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    int x;
    int y;
    if (x1 == x2) {
        if (y1 < y2)
            for (y = y1; y <= y2; y++)
                DrawUtils::drawPixel(x1, y, r, g, b, a);
        else
            for (y = y2; y <= y1; y++)
                DrawUtils::drawPixel(x1, y, r, g, b, a);
    } else {
        if (x1 < x2)
            for (x = x1; x <= x2; x++)
                DrawUtils::drawPixel(x, y1, r, g, b, a);
        else
            for (x = x2; x <= x1; x++)
                DrawUtils::drawPixel(x, y1, r, g, b, a);
    }
}

static void drawRect(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    drawLine(x1, y1, x2, y1, r, g, b, a);
    drawLine(x2, y1, x2, y2, r, g, b, a);
    drawLine(x1, y2, x2, y2, r, g, b, a);
    drawLine(x1, y1, x1, y2, r, g, b, a);
}

static void drawPic(int x, int y, uint32_t w, uint32_t h, float scale, uint32_t *pixels) {
    uint32_t nw = w, nh = h;
    Color color(0xFFFFFFFF);
    if (scale <= 0) scale = 1;
    nw = w * scale;
    nh = h * scale;


    for (uint32_t j = y; j < (y + nh); j++) {
        for (uint32_t i = x; i < (x + nw); i++) {
            color.color = pixels[((i - x) * w / nw) + ((j - y) * h / nh) * w];
            DrawUtils::drawPixel(i, j, color);
        }
    }
}

void drawTGA(int x, int y, float scale, uint8_t *fileContent) {
    uint32_t w = tgaGetWidth(fileContent), h = tgaGetHeight(fileContent);
    uint32_t nw = w, nh = h;
    auto *out = (uint32_t *) tgaRead(fileContent, TGA_READER_RGBA);

    if (scale <= 0) scale = 1;
    nw = w * scale;
    nh = h * scale;

    drawPic(x, y, w, h, scale, out);
    if (scale > 0.5) {
        if ((x > 0) && (y > 0)) drawRect(x - 1, y - 1, x + nw, y + nh, 255, 255, 255, 128);
        if ((x > 1) && (y > 1)) drawRect(x - 2, y - 2, x + nw + 1, y + nh + 1, 255, 255, 255, 128);
        if ((x > 2) && (y > 2)) drawRect(x - 3, y - 3, x + nw + 2, y + nh + 2, 255, 255, 255, 128);
    }
    free(out);
}

void drawRGB5A3(int x, int y, float scale, uint8_t *fileContent) {
    uint32_t w = 192;
    uint32_t h = 64;
    uint32_t nw = w;
    uint32_t nh = h;

    if (scale <= 0)
        scale = 1;
    nw = w * scale;
    nh = h * scale;

    uint32_t pos = 0;
    uint32_t npos = 0;
    Color color(0xFFFFFFFF);
    auto *pixels = (uint16_t *) fileContent;

    uint8_t sum = (4 * scale);
    for (uint32_t j = y; j < (y + nh); j += sum) {
        for (uint32_t i = x; i < (x + nw); i += sum) {
            for (uint32_t sj = j; sj < (j + sum); sj++, pos++) {
                for (uint32_t si = i; si < (i + sum); si++) {
                    npos = ((si - i) / scale) + ((pos / scale) * 4);
                    if ((pixels[npos] & 0x8000) != 0) {
                        color.color = ((pixels[npos] & 0x7C00) << 17) | ((pixels[npos] & 0x3E0) << 14) |
                                  ((pixels[npos] & 0x1F) << 11) | 0xFF;
                    } else {
                        color.color = (((pixels[npos] & 0xF00) * 0x11) << 16) | (((pixels[npos] & 0xF0) * 0x11) << 12) |
                                  (((pixels[npos] & 0xF) * 0x11) << 8) | (((pixels[npos] & 0x7000) >> 12) * 0x24);
                    }
                    DrawUtils::drawPixel(si, sj, color);
                }
            }
        }
    }
    if (scale > 0.5) {
        if ((x > 0) && (y > 0)) {
            drawRect(x - 1, y - 1, x + nw, y + nh, 255, 255, 255, 128);
        }
        if ((x > 1) && (y > 1)) {
            drawRect(x - 2, y - 2, x + nw + 1, y + nh + 1, 255, 255, 255, 128);
        }
        if ((x > 2) && (y > 2)) {
            drawRect(x - 3, y - 3, x + nw + 2, y + nh + 2, 255, 255, 255, 128);
        }
    }
}