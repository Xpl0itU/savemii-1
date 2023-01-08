#pragma once

#define __STDC_WANT_LIB_EXT2__ 1

#include <coreinit/filesystem_fsa.h>
#include <coreinit/mcp.h>
#include <coreinit/memdefaultheap.h>
#include <coreinit/thread.h>
#include <cstdio>
#include <dirent.h>
#include <draw.h>
#include <fcntl.h>
#include <input.h>
#include <utils/DrawUtils.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

#include <mocha/mocha.h>

#define PATH_SIZE     0x400

#define VERSION_MAJOR 1
#define VERSION_MINOR 5
#define VERSION_MICRO 3
#define M_OFF         1
#define Y_OFF         1

#define COLOR_WHITE              Color(0xffffffff)
#define COLOR_BLACK              Color(0, 0, 0, 255)
#define COLOR_RED                Color(237, 28, 36, 255)
#define COLOR_BACKGROUND         Color(0x00006F00)
#define COLOR_TEXT               COLOR_WHITE
#define COLOR_TEXT2              Color(0xB3ffffff)
#define COLOR_AUTOBOOT           Color(0xaeea00ff)
#define COLOR_BORDER             Color(204, 204, 204, 255)
#define COLOR_BORDER_HIGHLIGHTED Color(0x3478e4ff)

typedef struct {
    uint32_t highID;
    uint32_t lowID;
    uint16_t listID;
    char shortName[256];
    char longName[512];
    char productCode[5];
    bool saveInit;
    bool isTitleOnUSB;
    bool isTitleDupe;
    uint16_t dupeID;
    uint8_t *iconBuf;
} Title;

typedef struct {
    uint32_t highID;
    uint32_t lowID;
    uint8_t dev;
    bool found;
} Saves;

typedef struct {
    char persistentID[9];
    uint32_t pID;
    char miiName[50];
    uint8_t slot;
} Account;

typedef enum {
    ST_YES_NO = 1,
    ST_CONFIRM_CANCEL = 2,
    ST_MULTILINE = 16,
    ST_WARNING = 32,
    ST_ERROR = 64
} Style;

extern Account *wiiuacc;
extern Account *sdacc;
extern uint8_t wiiuaccn, sdaccn;

bool initFS() __attribute__((__cold__));
void deinitFS() __attribute__((__cold__));
std::string getUSB();
void consolePrintPos(int x, int y, const char *format, ...) __attribute__((hot));
bool promptConfirm(Style st, std::string question);
void promptError(const char *message, ...);
void getAccountsWiiU();
void getAccountsSD(Title *title, uint8_t slot);
bool hasAccountSave(Title *title, bool inSD, bool iine, uint32_t user, uint8_t slot, int version);
int getLoadiineGameSaveDir(char *out, const char *productCode, const char *longName, const uint32_t highID, const uint32_t lowID);
bool getLoadiineSaveVersionList(int *out, const char *gamePath);
bool isSlotEmpty(uint32_t highID, uint32_t lowID, uint8_t slot);
bool hasCommonSave(Title *title, bool inSD, bool iine, uint8_t slot, int version);
void copySavedata(Title *title, Title *titled, int8_t allusers, int8_t allusers_d, bool common) __attribute__((hot));
void backupAllSave(Title *titles, int count, OSCalendarTime *date) __attribute__((hot));
void backupSavedata(Title *title, uint8_t slot, int8_t allusers, bool common) __attribute__((hot));
void restoreSavedata(Title *title, uint8_t slot, int8_t sdusers, int8_t allusers, bool common) __attribute__((hot));
void wipeSavedata(Title *title, int8_t allusers, bool common) __attribute__((hot));
void importFromLoadiine(Title *title, bool common, int version);
void exportToLoadiine(Title *title, bool common, int version);
int checkEntry(const char *fPath);
int32_t loadFile(const char *fPath, uint8_t **buf) __attribute__((hot));
int32_t loadTitleIcon(Title *title) __attribute__((hot));
void consolePrintPosMultiline(int x, int y, const char *format, ...) __attribute__((hot));
void consolePrintPosAligned(int y, uint16_t offset, uint8_t align, const char *format, ...) __attribute__((hot));