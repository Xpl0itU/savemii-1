#include <configMenu.h>
#include <icon.h>
#include <input.h>
#include <json.h>
#include <language.h>
#include <coreinit/debug.h>
#include <utils/DrawUtils.h>
#include <main.h>
#include <savemng.h>
#include <sndcore2/core.h>
#include <state.h>
#include <string.hpp>

static uint8_t slot = 0;
static int8_t allusers = -1, allusers_d = -1, sdusers = -1;
static bool common = true;
static Menu menu = mainMenu;
static Mode mode;
static Task task;
static bool sortAscending = true;
static int targ = 0, tsort = 1;
static int cursor = 0, scroll = 0;
static int cursorb = 0, cursort = 0, scrollb = 0;
static int wiiuTitlesCount = 0, vWiiTitlesCount = 0;
static std::array<const char *, 4> sortn;

template<class It>
static void sortTitle(It titles, It last, int tsort = 1, bool sortAscending = true) {
    switch (tsort) {
        case 0:
            std::ranges::sort(titles, last, std::ranges::less{}, &Title::listID);
            break;
        case 1: {
            const auto proj = [](const Title &title) {
                return std::string_view(title.shortName);
            };
            if (sortAscending == true) {
                std::ranges::sort(titles, last, std::ranges::less{}, proj);
            } else {
                std::ranges::sort(titles, last, std::ranges::greater{}, proj);
            }
            break;
        }
        case 2:
            if (sortAscending == true) {
                std::ranges::sort(titles, last, std::ranges::less{}, &Title::isTitleOnUSB);
            } else {
                std::ranges::sort(titles, last, std::ranges::greater{}, &Title::isTitleOnUSB);
            }
            break;
        case 3: {
            const auto proj = [](const Title &title) {
                return std::make_tuple(title.isTitleOnUSB,
                                       std::string_view(title.shortName));
            };
            if (sortAscending == true) {
                std::ranges::sort(titles, last, std::ranges::less{}, proj);
            } else {
                std::ranges::sort(titles, last, std::ranges::greater{}, proj);
            }
            break;
        }
    }
}

static void disclaimer() {
    consolePrintPosAligned(14, 0, 1, gettext("Disclaimer:"));
    consolePrintPosAligned(15, 0, 1, gettext("There is always the potential for a brick."));
    consolePrintPosAligned(16, 0, 1, gettext("Everything you do with this software is your own responsibility"));
}

static Title *loadWiiUTitles(int run) {
    static char *tList;
    static uint32_t receivedCount;
    const std::array<const uint32_t, 2> highIDs = {0x00050000, 0x00050002};
    // Source: haxchi installer
    if (run == 0) {
        int mcp_handle = MCP_Open();
        int count = MCP_TitleCount(mcp_handle);
        int listSize = count * 0x61;
        tList = (char *) memalign(32, listSize);
        memset(tList, 0, listSize);
        receivedCount = count;
        MCP_TitleList(mcp_handle, &receivedCount, (MCPTitleListType *) tList, listSize);
        MCP_Close(mcp_handle);
        return nullptr;
    }

    int usable = receivedCount;
    int j = 0;
    auto *savesl = (Saves *) malloc(receivedCount * sizeof(Saves));
    if (savesl == nullptr) {
        promptError(gettext("Out of memory."));
        return nullptr;
    }
    for (uint32_t i = 0; i < receivedCount; i++) {
        char *element = tList + (i * 0x61);
        savesl[j].highID = *(uint32_t *) (element);
        if (savesl[j].highID != (0x00050000 | 0x00050002)) {
            usable--;
            continue;
        }
        savesl[j].lowID = *(uint32_t *) (element + 4);
        savesl[j].dev = static_cast<uint8_t>(!(memcmp(element + 0x56, "usb", 4) == 0));
        savesl[j].found = false;
        j++;
    }
    savesl = (Saves *) realloc(savesl, usable * sizeof(Saves));

    int foundCount = 0;
    int pos = 0;
    int tNoSave = usable;
    for (int i = 0; i <= 1; i++) {
        for (uint8_t a = 0; a < 2; a++) {
            std::string path = stringFormat("%s/usr/save/%08x", (i == 0) ? getUSB().c_str() : "storage_mlc01:", highIDs[a]);
            DIR *dir = opendir(path.c_str());
            if (dir != nullptr) {
                struct dirent *data;
                while ((data = readdir(dir)) != nullptr) {
                    if (data->d_name[0] == '.')
                        continue;

                    path = stringFormat("%s/usr/save/%08x/%s/user", (i == 0) ? getUSB().c_str() : "storage_mlc01:", highIDs[a], data->d_name);
                    if (checkEntry(path.c_str()) == 2) {
                        path = stringFormat("%s/usr/save/%08x/%s/meta/meta.xml", (i == 0) ? getUSB().c_str() : "storage_mlc01:", highIDs[a],
                                            data->d_name);
                        if (checkEntry(path.c_str()) == 1) {
                            for (int i = 0; i < usable; i++) {
                                if ((savesl[i].highID == (0x00050000 | 0x00050002)) &&
                                    (strtoul(data->d_name, nullptr, 16) == savesl[i].lowID)) {
                                    savesl[i].found = true;
                                    tNoSave--;
                                    break;
                                }
                            }
                            foundCount++;
                        }
                    }
                }
                closedir(dir);
            }
        }
    }

    foundCount += tNoSave;
    auto *saves = (Saves *) malloc((foundCount + tNoSave) * sizeof(Saves));
    if (saves == nullptr) {
        promptError(gettext("Out of memory."));
        return nullptr;
    }

    for (uint8_t a = 0; a < 2; a++) {
        for (int i = 0; i <= 1; i++) {
            std::string path = stringFormat("%s/usr/save/%08x", (i == 0) ? getUSB().c_str() : "storage_mlc01:", highIDs[a]);
            DIR *dir = opendir(path.c_str());
            if (dir != nullptr) {
                struct dirent *data;
                while ((data = readdir(dir)) != nullptr) {
                    if (data->d_name[0] == '.')
                        continue;

                    path = stringFormat("%s/usr/save/%08x/%s/meta/meta.xml", (i == 0) ? getUSB().c_str() : "storage_mlc01:", highIDs[a],
                                        data->d_name);
                    if (checkEntry(path.c_str()) == 1) {
                        saves[pos].highID = highIDs[a];
                        saves[pos].lowID = strtoul(data->d_name, nullptr, 16);
                        saves[pos].dev = i;
                        saves[pos].found = false;
                        pos++;
                    }
                }
                closedir(dir);
            }
        }
    }

    for (int i = 0; i < usable; i++) {
        if (!savesl[i].found) {
            saves[pos].highID = savesl[i].highID;
            saves[pos].lowID = savesl[i].lowID;
            saves[pos].dev = savesl[i].dev;
            saves[pos].found = true;
            pos++;
        }
    }

    auto *titles = (Title *) malloc(foundCount * sizeof(Title));
    if (titles == nullptr) {
        promptError(gettext("Out of memory."));
        return nullptr;
    }

    for (int i = 0; i < foundCount; i++) {
        uint32_t highID = saves[i].highID;
        uint32_t lowID = saves[i].lowID;
        bool isTitleOnUSB = saves[i].dev == 0u;

        const std::string path = stringFormat("%s/usr/%s/%08x/%08x/meta/meta.xml", isTitleOnUSB ? getUSB().c_str() : "storage_mlc01:",
                                              saves[i].found ? "title" : "save", highID, lowID);
        titles[wiiuTitlesCount].saveInit = !saves[i].found;

        char *xmlBuf = nullptr;
        if (loadFile(path.c_str(), (uint8_t **) &xmlBuf) > 0) {
            char *cptr = strchr(strstr(xmlBuf, "product_code"), '>') + 7;
            memset(titles[wiiuTitlesCount].productCode, 0, sizeof(titles[wiiuTitlesCount].productCode));
            strlcpy(titles[wiiuTitlesCount].productCode, cptr, strcspn(cptr, "<") + 1);

            cptr = strchr(strstr(xmlBuf, "shortname_en"), '>') + 1;
            memset(titles[wiiuTitlesCount].shortName, 0, sizeof(titles[wiiuTitlesCount].shortName));
            if (strcspn(cptr, "<") == 0)
                cptr = strchr(strstr(xmlBuf, "shortname_ja"), '>') + 1;

            decodeXMLEscapeLine(std::string(cptr));
            strlcpy(titles[wiiuTitlesCount].shortName, decodeXMLEscapeLine(std::string(cptr)).c_str(), strcspn(decodeXMLEscapeLine(std::string(cptr)).c_str(), "<") + 1);

            cptr = strchr(strstr(xmlBuf, "longname_en"), '>') + 1;
            memset(titles[i].longName, 0, sizeof(titles[i].longName));
            if (strcspn(cptr, "<") == 0)
                cptr = strchr(strstr(xmlBuf, "longname_ja"), '>') + 1;

            strlcpy(titles[wiiuTitlesCount].longName, decodeXMLEscapeLine(std::string(cptr)).c_str(), strcspn(decodeXMLEscapeLine(std::string(cptr)).c_str(), "<") + 1);

            free(xmlBuf);
        }

        titles[wiiuTitlesCount].isTitleDupe = false;
        for (int i = 0; i < wiiuTitlesCount; i++) {
            if ((titles[i].highID == highID) && (titles[i].lowID == lowID)) {
                titles[wiiuTitlesCount].isTitleDupe = true;
                titles[wiiuTitlesCount].dupeID = i;
                titles[i].isTitleDupe = true;
                titles[i].dupeID = wiiuTitlesCount;
            }
        }

        titles[wiiuTitlesCount].highID = highID;
        titles[wiiuTitlesCount].lowID = lowID;
        titles[wiiuTitlesCount].isTitleOnUSB = isTitleOnUSB;
        titles[wiiuTitlesCount].listID = wiiuTitlesCount;
        if (loadTitleIcon(&titles[wiiuTitlesCount]) < 0)
            titles[wiiuTitlesCount].iconBuf = nullptr;

        wiiuTitlesCount++;

        DrawUtils::clear(COLOR_BLACK);
        DrawUtils::beginDraw();
        disclaimer();
        drawTGA(298, 144, 1, icon_tga);
        consolePrintPosAligned(10, 0, 1, gettext("Loaded %i Wii U titles."), wiiuTitlesCount);
        DrawUtils::endDraw();
    }

    free(savesl);
    free(saves);
    free(tList);
    return titles;
}

static Title *loadWiiTitles() {
    std::array<const char *, 3> highIDs = {"00010000", "00010001", "00010004"};
    bool found = false;
    static const std::array<std::array<const uint32_t, 2>, 7> blacklist = {{{0x00010000, 0x00555044},
                                                                            {0x00010000, 0x00555045},
                                                                            {0x00010000, 0x0055504A},
                                                                            {0x00010000, 0x524F4E45},
                                                                            {0x00010000, 0x52543445},
                                                                            {0x00010001, 0x48424344},
                                                                            {0x00010001, 0x554E454F}}};

    std::string pathW;
    for (int k = 0; k < 3; k++) {
        pathW = stringFormat("storage_slccmpt01:/title/%s", highIDs[k]);
        DIR *dir = opendir(pathW.c_str());
        if (dir != nullptr) {
            struct dirent *data;
            while ((data = readdir(dir)) != nullptr) {
                for (int ii = 0; ii < 7; ii++) {
                    if (blacklist[ii][0] == strtoul(highIDs[k], nullptr, 16)) {
                        if (blacklist[ii][1] == strtoul(data->d_name, nullptr, 16)) {
                            found = true;
                            break;
                        }
                    }
                }
                if (found) {
                    found = false;
                    continue;
                }

                vWiiTitlesCount++;
            }
            closedir(dir);
        }
    }
    if (vWiiTitlesCount == 0)
        return nullptr;

    auto *titles = (Title *) malloc(vWiiTitlesCount * sizeof(Title));
    if (titles == nullptr) {
        promptError(gettext("Out of memory."));
        return nullptr;
    }

    int i = 0;
    for (int k = 0; k < 3; k++) {
        pathW = stringFormat("storage_slccmpt01:/title/%s", highIDs[k]);
        DIR *dir = opendir(pathW.c_str());
        if (dir != nullptr) {
            struct dirent *data;
            while ((data = readdir(dir)) != nullptr) {
                for (int ii = 0; ii < 7; ii++) {
                    if (blacklist[ii][0] == strtoul(highIDs[k], nullptr, 16)) {
                        if (blacklist[ii][1] == strtoul(data->d_name, nullptr, 16)) {
                            found = true;
                            break;
                        }
                    }
                }
                if (found) {
                    found = false;
                    continue;
                }

                const std::string path = stringFormat("storage_slccmpt01:/title/%s/%s/data/banner.bin", highIDs[k], data->d_name);
                FILE *file = fopen(path.c_str(), "rb");
                if (file != nullptr) {
                    fseek(file, 0x20, SEEK_SET);
                    auto *bnrBuf = (uint16_t *) malloc(0x80);
                    if (bnrBuf != nullptr) {
                        fread(bnrBuf, 0x02, 0x20, file);
                        memset(titles[i].shortName, 0, sizeof(titles[i].shortName));
                        for (int j = 0, k = 0; j < 0x20; j++) {
                            if (bnrBuf[j] < 0x80) {
                                titles[i].shortName[k++] = (char) bnrBuf[j];
                            } else if ((bnrBuf[j] & 0xF000) > 0) {
                                titles[i].shortName[k++] = 0xE0 | ((bnrBuf[j] & 0xF000) >> 12);
                                titles[i].shortName[k++] = 0x80 | ((bnrBuf[j] & 0xFC0) >> 6);
                                titles[i].shortName[k++] = 0x80 | (bnrBuf[j] & 0x3F);
                            } else if (bnrBuf[j] < 0x400) {
                                titles[i].shortName[k++] = 0xC0 | ((bnrBuf[j] & 0x3C0) >> 6);
                                titles[i].shortName[k++] = 0x80 | (bnrBuf[j] & 0x3F);
                            } else {
                                titles[i].shortName[k++] = 0xD0 | ((bnrBuf[j] & 0x3C0) >> 6);
                                titles[i].shortName[k++] = 0x80 | (bnrBuf[j] & 0x3F);
                            }
                        }

                        memset(titles[i].longName, 0, sizeof(titles[i].longName));
                        for (int j = 0x20, k = 0; j < 0x40; j++) {
                            if (bnrBuf[j] < 0x80) {
                                titles[i].longName[k++] = (char) bnrBuf[j];
                            } else if ((bnrBuf[j] & 0xF000) > 0) {
                                titles[i].longName[k++] = 0xE0 | ((bnrBuf[j] & 0xF000) >> 12);
                                titles[i].longName[k++] = 0x80 | ((bnrBuf[j] & 0xFC0) >> 6);
                                titles[i].longName[k++] = 0x80 | (bnrBuf[j] & 0x3F);
                            } else if (bnrBuf[j] < 0x400) {
                                titles[i].longName[k++] = 0xC0 | ((bnrBuf[j] & 0x3C0) >> 6);
                                titles[i].longName[k++] = 0x80 | (bnrBuf[j] & 0x3F);
                            } else {
                                titles[i].longName[k++] = 0xD0 | ((bnrBuf[j] & 0x3C0) >> 6);
                                titles[i].longName[k++] = 0x80 | (bnrBuf[j] & 0x3F);
                            }
                        }
                        titles[i].saveInit = true;

                        free(bnrBuf);
                    }
                    fclose(file);
                } else {
                    sprintf(titles[i].shortName, gettext("%s%s (No banner.bin)"), highIDs[k], data->d_name);
                    memset(titles[i].longName, 0, sizeof(titles[i].longName));
                    titles[i].saveInit = false;
                }

                titles[i].highID = strtoul(highIDs[k], nullptr, 16);
                titles[i].lowID = strtoul(data->d_name, nullptr, 16);

                titles[i].listID = i;
                memcpy(titles[i].productCode, &titles[i].lowID, 4);
                for (int ii = 0; ii < 4; ii++)
                    if (titles[i].productCode[ii] == 0)
                        titles[i].productCode[ii] = '.';
                titles[i].productCode[4] = 0;
                titles[i].isTitleOnUSB = false;
                titles[i].isTitleDupe = false;
                titles[i].dupeID = 0;
                if (!titles[i].saveInit || (loadTitleIcon(&titles[i]) < 0))
                    titles[i].iconBuf = nullptr;
                i++;

                DrawUtils::clear(COLOR_BLACK);
                DrawUtils::beginDraw();
                disclaimer();
                drawTGA(298, 144, 1, icon_tga);
                consolePrintPosAligned(10, 0, 1, gettext("Loaded %i Wii U titles."), wiiuTitlesCount);
                consolePrintPosAligned(11, 0, 1, gettext("Loaded %i Wii titles."), i);
                DrawUtils::endDraw();
            }
            closedir(dir);
        }
    }

    return titles;
}

static void unloadTitles(Title *titles, int count) {
    for (int i = 0; i < count; i++)
        if (titles[i].iconBuf != nullptr)
            free(titles[i].iconBuf);
    if (titles != nullptr)
        free(titles);
}

int main() {
    AXInit();
    AXQuit();
    WHBProcInit();
    OSScreenInit();

    uint32_t tvBufferSize  = OSScreenGetBufferSizeEx(SCREEN_TV);
    uint32_t drcBufferSize = OSScreenGetBufferSizeEx(SCREEN_DRC);

    auto *screenBuffer = (uint8_t *) memalign(0x100, tvBufferSize + drcBufferSize);
    if (!screenBuffer) {
        OSFatal("Fail to allocate screenBuffer");
    }
    memset(screenBuffer, 0, tvBufferSize + drcBufferSize);

    OSScreenSetBufferEx(SCREEN_TV, screenBuffer);
    OSScreenSetBufferEx(SCREEN_DRC, screenBuffer + tvBufferSize);

    OSScreenEnableEx(SCREEN_TV, TRUE);
    OSScreenEnableEx(SCREEN_DRC, TRUE);

    DrawUtils::initBuffers(screenBuffer, tvBufferSize, screenBuffer + tvBufferSize, drcBufferSize);

    if (!DrawUtils::initFont()) {
        OSFatal("Failed to init font");
    }
    
    WPADInit();
    KPADInit();
    WPADEnableURCC(1);
    loadWiiUTitles(0);
    initState();

    int res = romfsInit();
    if (res) {
        promptError("Failed to init romfs: %d", res);
        DrawUtils::endDraw();
        WHBProcShutdown();
        return 0;
    }

    Swkbd_LanguageType systemLanguage = getSystemLanguage();
    loadLanguage(systemLanguage);

    sortn = {gettext("None"), gettext("Name"), gettext("Storage"), gettext("Storage+Name")};

    if (!initFS()) {
        promptError(gettext("initFS failed. Please make sure your MochaPayload is up-to-date"));
        DrawUtils::endDraw();
        WHBProcShutdown();
        return 0;
    }

    DrawUtils::beginDraw();
    DrawUtils::clear(COLOR_BLACK);
    DrawUtils::endDraw();
    Title *wiiutitles = loadWiiUTitles(1);
    Title *wiititles = loadWiiTitles();
    int *versionList = (int *) malloc(0x100 * sizeof(int));
    getAccountsWiiU();

    sortTitle(wiiutitles, wiiutitles + wiiuTitlesCount, tsort, sortAscending);
    sortTitle(wiititles, wiititles + vWiiTitlesCount, tsort, sortAscending);

    bool redraw = true;
    int entrycount = 0;
    Input input;
    while (AppRunning()) {
        Title *titles = mode != WiiU ? wiititles : wiiutitles;
        int count = mode != WiiU ? vWiiTitlesCount : wiiuTitlesCount;

        if (redraw) {
            DrawUtils::beginDraw();
            DrawUtils::clear(COLOR_BACKGROUND);

            consolePrintPos(0, 0, "SaveMii v%u.%u.%u", VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO);
            consolePrintPos(0, 1, "----------------------------------------------------------------------------");

            switch (menu) {
                case mainMenu: {
                    entrycount = 3;
                    consolePrintPos(M_OFF, 2, gettext("   Wii U Save Management (%u Title%s)"), wiiuTitlesCount,
                                    (wiiuTitlesCount > 1) ? "s" : "");
                    consolePrintPos(M_OFF, 3, gettext("   vWii Save Management (%u Title%s)"), vWiiTitlesCount,
                                    (vWiiTitlesCount > 1) ? "s" : "");
                    consolePrintPos(M_OFF, 4, gettext("   Batch Backup"));
                    consolePrintPos(M_OFF, 2 + cursor, "\u2192");
                    consolePrintPosAligned(17, 4, 2, gettext("\uE002: Options \ue000: Select Mode"));
                } break;
                case selectTitle: {
                    if (mode == batchBackup) {
                        entrycount = 3;
                        consolePrintPos(M_OFF, 2, gettext("   Backup All (%u Title%s)"), wiiuTitlesCount + vWiiTitlesCount,
                                        ((wiiuTitlesCount + vWiiTitlesCount) > 1) ? "s" : "");
                        consolePrintPos(M_OFF, 3, gettext("   Backup Wii U (%u Title%s)"), wiiuTitlesCount,
                                        (wiiuTitlesCount > 1) ? "s" : "");
                        consolePrintPos(M_OFF, 4, gettext("   Backup vWii (%u Title%s)"), vWiiTitlesCount,
                                        (vWiiTitlesCount > 1) ? "s" : "");
                        consolePrintPos(M_OFF, 2 + cursor, "\u2192");
                        consolePrintPosAligned(17, 4, 2, gettext("\ue000: Backup  \ue001: Back"));
                    } else {
                        consolePrintPos(39, 0, gettext("%s Sort: %s \ue084"),
                                        (tsort > 0) ? ((sortAscending == true) ? "\ue083 \u2193" : "\ue083 \u2191") : "", sortn[tsort]);
                        entrycount = count;
                        for (int i = 0; i < 14; i++) {
                            if (i + scroll < 0 || i + scroll >= count)
                                break;
                            DrawUtils::setFontColor(static_cast<Color>(0x00FF00FF));
                            if (!titles[i + scroll].saveInit)
                                DrawUtils::setFontColor(static_cast<Color>(0xFFFF00FF));
                            if (strcmp(titles[i + scroll].shortName, "DONT TOUCH ME") == 0)
                                DrawUtils::setFontColor(static_cast<Color>(0xFF0000FF));
                            if (strlen(titles[i + scroll].shortName) != 0u)
                                consolePrintPos(M_OFF, i + 2, "   %s %s%s%s", titles[i + scroll].shortName,
                                                titles[i + scroll].isTitleOnUSB ? "(USB)" : ((mode == WiiU) ? "(NAND)" : ""),
                                                titles[i + scroll].isTitleDupe ? " [D]" : "",
                                                titles[i + scroll].saveInit ? "" : gettext(" [Not Init]"));
                            else
                                consolePrintPos(M_OFF, i + 2, "   %08lx%08lx", titles[i + scroll].highID,
                                                titles[i + scroll].lowID);
                            DrawUtils::setFontColor(COLOR_TEXT);
                            if (mode == WiiU) {
                                if (titles[i + scroll].iconBuf != nullptr) {
                                    drawTGA((M_OFF + 4) * 12 - 2, (i + 3) * 24, 0.18, titles[i + scroll].iconBuf);
                                }
                            } else if (mode == vWii) {
                                if (titles[i + scroll].iconBuf != nullptr) {
                                    drawRGB5A3((M_OFF + 2) * 12 - 2, (i + 3) * 24 + 3, 0.25,
                                               titles[i + scroll].iconBuf);
                                }
                            }
                        }
                        if (mode == WiiU)
                            consolePrintPos(-1, 2 + cursor, "\u2192");
                        else if (mode == vWii)
                            consolePrintPos(-3, 2 + cursor, "\u2192");
                        consolePrintPosAligned(17, 4, 2, gettext("\ue000: Select Game  \ue001: Back"));
                    }
                } break;
                case selectTask: {
                    entrycount = 3 + 2 * static_cast<int>(mode == WiiU) + 1 * static_cast<int>((mode == WiiU) && (titles[targ].isTitleDupe));
                    consolePrintPos(M_OFF, 2, "   [%08X-%08X] [%s]", titles[targ].highID, titles[targ].lowID,
                                    titles[targ].productCode);
                    consolePrintPos(M_OFF, 3, "   %s", titles[targ].shortName);
                    consolePrintPos(M_OFF, 5, gettext("   Backup savedata"));
                    consolePrintPos(M_OFF, 6, gettext("   Restore savedata"));
                    consolePrintPos(M_OFF, 7, gettext("   Wipe savedata"));
                    if (mode == WiiU) {
                        consolePrintPos(M_OFF, 8, gettext("   Import from loadiine"));
                        consolePrintPos(M_OFF, 9, gettext("   Export to loadiine"));
                        if (titles[targ].isTitleDupe)
                            consolePrintPos(M_OFF, 10, gettext("   Copy Savedata to Title in %s"),
                                            titles[targ].isTitleOnUSB ? "NAND" : "USB");
                        if (titles[targ].iconBuf != nullptr)
                            drawTGA(660, 80, 1, titles[targ].iconBuf);
                    } else if (mode == vWii)
                        if (titles[targ].iconBuf != nullptr)
                            drawRGB5A3(645, 80, 1, titles[targ].iconBuf);
                    consolePrintPos(M_OFF, 2 + 3 + cursor, "\u2192");
                    consolePrintPosAligned(17, 4, 2, gettext("\ue000: Select Task  \ue001: Back"));
                } break;
                case selectOptions: {
                    entrycount = 3;
                    consolePrintPos(M_OFF, 2, "[%08X-%08X] %s", titles[targ].highID, titles[targ].lowID,
                                    titles[targ].shortName);

                    if (task == copytoOtherDevice) {
                        consolePrintPos(M_OFF, 4, gettext("Destination:"));
                        consolePrintPos(M_OFF, 5, "    (%s)", titles[targ].isTitleOnUSB ? "NAND" : "USB");
                    } else if (task > 2) {
                        entrycount = 2;
                        consolePrintPos(M_OFF, 4, gettext("Select %s:"), gettext("version"));
                        consolePrintPos(M_OFF, 5, "   < v%u >", versionList != nullptr ? versionList[slot] : 0);
                    } else if (task == wipe) {
                        consolePrintPos(M_OFF, 4, gettext("Delete from:"));
                        consolePrintPos(M_OFF, 5, "    (%s)", titles[targ].isTitleOnUSB ? "USB" : "NAND");
                    } else {
                        consolePrintPos(M_OFF, 4, gettext("Select %s:"), gettext("slot"));

                        if (((titles[targ].highID & 0xFFFFFFF0) == 0x00010000) && (slot == 255))
                            consolePrintPos(M_OFF, 5, "   < SaveGame Manager GX > (%s)",
                                            isSlotEmpty(titles[targ].highID, titles[targ].lowID, slot) ? gettext("Empty")
                                                                                                       : gettext("Used"));
                        else
                            consolePrintPos(M_OFF, 5, "   < %03u > (%s)", slot,
                                            isSlotEmpty(titles[targ].highID, titles[targ].lowID, slot) ? gettext("Empty")
                                                                                                       : gettext("Used"));
                    }

                    if (mode == WiiU) {
                        if (task == restore) {
                            if (!isSlotEmpty(titles[targ].highID, titles[targ].lowID, slot)) {
                                entrycount++;
                                consolePrintPos(M_OFF, 7, gettext("Select SD user to copy from:"));
                                if (sdusers == -1)
                                    consolePrintPos(M_OFF, 8, "   < %s >", gettext("all users"));
                                else
                                    consolePrintPos(M_OFF, 8, "   < %s > (%s)", sdacc[sdusers].persistentID,
                                                    hasAccountSave(&titles[targ], true, false, sdacc[sdusers].pID,
                                                                   slot, 0)
                                                            ? gettext("Has Save")
                                                            : gettext("Empty"));
                            }
                        }

                        if (task == wipe) {
                            consolePrintPos(M_OFF, 7, gettext("Select Wii U user to delete from:"));
                            if (allusers == -1)
                                consolePrintPos(M_OFF, 8, "   < %s >", gettext("all users"));
                            else
                                consolePrintPos(M_OFF, 8, "   < %s (%s) > (%s)", wiiuacc[allusers].miiName,
                                                wiiuacc[allusers].persistentID,
                                                hasAccountSave(&titles[targ], false, false, wiiuacc[allusers].pID,
                                                               slot, 0)
                                                        ? gettext("Has Save")
                                                        : gettext("Empty"));
                        }

                        if ((task == backup) || (task == restore) || (task == copytoOtherDevice)) {
                            if ((task == restore) && isSlotEmpty(titles[targ].highID, titles[targ].lowID, slot))
                                entrycount--;
                            else {
                                consolePrintPos(M_OFF, (task == restore) ? 10 : 7, gettext("Select Wii U user%s:"),
                                                (task == copytoOtherDevice) ? gettext(" to copy from") : ((task == restore) ? gettext(" to copy to") : ""));
                                if (allusers == -1)
                                    consolePrintPos(M_OFF, (task == restore) ? 11 : 8, "   < %s >", gettext("all users"));
                                else
                                    consolePrintPos(M_OFF, (task == restore) ? 11 : 8, "   < %s (%s) > (%s)",
                                                    wiiuacc[allusers].miiName, wiiuacc[allusers].persistentID,
                                                    hasAccountSave(&titles[targ],
                                                                   (!((task == backup) || (task == restore) || (task == copytoOtherDevice))),
                                                                   (!((task < 3) || (task == copytoOtherDevice))),
                                                                   wiiuacc[allusers].pID, slot,
                                                                   versionList != nullptr ? versionList[slot] : 0)
                                                            ? gettext("Has Save")
                                                            : gettext("Empty"));
                            }
                        }
                        if ((task == backup) || (task == restore))
                            if (!isSlotEmpty(titles[targ].highID, titles[targ].lowID, slot)) {
                                Date *dateObj = new Date(titles[targ].highID, titles[targ].lowID, slot);
                                consolePrintPos(M_OFF, 15, gettext("Date: %s"),
                                                dateObj->get().c_str());
                                delete dateObj;
                            }

                        if (task == copytoOtherDevice) {
                            entrycount++;
                            consolePrintPos(M_OFF, 10, gettext("Select Wii U user%s:"), (task == copytoOtherDevice) ? gettext(" to copy to") : "");
                            if (allusers_d == -1)
                                consolePrintPos(M_OFF, 11, "   < %s >", gettext("all users"));
                            else
                                consolePrintPos(M_OFF, 11, "   < %s (%s) > (%s)", wiiuacc[allusers_d].miiName,
                                                wiiuacc[allusers_d].persistentID,
                                                hasAccountSave(&titles[titles[targ].dupeID], false, false,
                                                               wiiuacc[allusers_d].pID, 0, 0)
                                                        ? gettext("Has Save")
                                                        : gettext("Empty"));
                        }

                        if ((task != importLoadiine) && (task != exportLoadiine)) {
                            if (allusers > -1) {
                                if (hasCommonSave(&titles[targ],
                                                  (!((task == backup) || (task == wipe) || (task == copytoOtherDevice))),
                                                  (!((task < 3) || (task == copytoOtherDevice))), slot,
                                                  versionList != nullptr ? versionList[slot] : 0)) {
                                    consolePrintPos(M_OFF, (task == restore) || (task == copytoOtherDevice) ? 13 : 10,
                                                    gettext("Include 'common' save?"));
                                    consolePrintPos(M_OFF, (task == restore) || (task == copytoOtherDevice) ? 14 : 11, "   < %s >",
                                                    common ? gettext("yes") : gettext("no "));
                                } else {
                                    common = false;
                                    consolePrintPos(M_OFF, (task == restore) || (task == copytoOtherDevice) ? 13 : 10,
                                                    gettext("No 'common' save found."));
                                    entrycount--;
                                }
                            } else {
                                common = false;
                                entrycount--;
                            }
                        } else {
                            if (hasCommonSave(&titles[targ], true, true, slot, versionList != nullptr ? versionList[slot] : 0)) {
                                consolePrintPos(M_OFF, 7, gettext("Include 'common' save?"));
                                consolePrintPos(M_OFF, 8, "   < %s >", common ? gettext("yes") : gettext("no "));
                            } else {
                                common = false;
                                consolePrintPos(M_OFF, 7, gettext("No 'common' save found."));
                                entrycount--;
                            }
                        }

                        consolePrintPos(M_OFF, 5 + cursor * 3, "\u2192");
                        if (titles[targ].iconBuf != nullptr)
                            drawTGA(660, 100, 1, titles[targ].iconBuf);
                    } else if (mode == vWii) {
                        entrycount = 1;
                        if (titles[targ].iconBuf != nullptr)
                            drawRGB5A3(650, 100, 1, titles[targ].iconBuf);
                        if (!isSlotEmpty(titles[targ].highID, titles[targ].lowID, slot)) {
                            Date *dateObj = new Date(titles[targ].highID, titles[targ].lowID, slot);
                            consolePrintPos(M_OFF, 15, gettext("Date: %s"),
                                            dateObj->get().c_str());
                            delete dateObj;
                        }
                    }

                    switch (task) {
                        case backup:
                            consolePrintPosAligned(17, 4, 2, gettext("\ue000: Backup  \ue001: Back"));
                            break;
                        case restore:
                            consolePrintPosAligned(17, 4, 2, gettext("\ue000: Restore  \ue001: Back"));
                            break;
                        case wipe:
                            consolePrintPosAligned(17, 4, 2, gettext("\ue000: Wipe  \ue001: Back"));
                            break;
                        case importLoadiine:
                            consolePrintPosAligned(17, 4, 2, gettext("\ue000: Import  \ue001: Back"));
                            break;
                        case exportLoadiine:
                            consolePrintPosAligned(17, 4, 2, gettext("\ue000: Export  \ue001: Back"));
                            break;
                        case copytoOtherDevice:
                            consolePrintPosAligned(17, 4, 2, gettext("\ue000: Copy  \ue001: Back"));
                            break;
                    }
                } break;
            }
            consolePrintPos(0, 16, "----------------------------------------------------------------------------");
            consolePrintPos(0, 17, gettext("Press \ue044 to exit."));

            DrawUtils::endDraw();
            redraw = false;
        }

        input.read();

        if (input.get(TRIGGER, PAD_BUTTON_ANY))
            redraw = true;

        if (input.get(TRIGGER, PAD_BUTTON_DOWN)) {
            if (entrycount <= 14)
                cursor = (cursor + 1) % entrycount;
            else if (cursor < 6)
                cursor++;
            else if (((cursor + scroll + 1) % entrycount) != 0)
                scroll++;
            else
                cursor = scroll = 0;
        } else if (input.get(TRIGGER, PAD_BUTTON_UP)) {
            if (scroll > 0)
                cursor -= (cursor > 6) ? 1 : 0 * (scroll--);
            else if (cursor > 0)
                cursor--;
            else if (entrycount > 14)
                scroll = entrycount - (cursor = 6) - 1;
            else
                cursor = entrycount - 1;
        }

        if (input.get(TRIGGER, PAD_BUTTON_LEFT)) {
            if (menu == selectOptions) {
                if (task == copytoOtherDevice) {
                    switch (cursor) {
                        case 0:
                            break;
                        case 1:
                            allusers = ((allusers == -1) ? -1 : (allusers - 1));
                            allusers_d = allusers;
                            break;
                        case 2:
                            allusers_d = (((allusers == -1) || (allusers_d == -1)) ? -1 : (allusers_d - 1));
                            allusers_d = ((allusers > -1) && (allusers_d == -1)) ? 0 : allusers_d;
                            break;
                        case 3:
                            common ^= 1;
                            break;
                    }
                } else if (task == restore) {
                    switch (cursor) {
                        case 0:
                            slot--;
                            getAccountsSD(&titles[targ], slot);
                            break;
                        case 1:
                            sdusers = ((sdusers == -1) ? -1 : (sdusers - 1));
                            allusers = ((sdusers == -1) ? -1 : allusers);
                            break;
                        case 2:
                            allusers = (((allusers == -1) || (sdusers == -1)) ? -1 : (allusers - 1));
                            allusers = ((sdusers > -1) && (allusers == -1)) ? 0 : allusers;
                            break;
                        case 3:
                            common ^= 1;
                            break;
                    }
                } else if (task == wipe) {
                    switch (cursor) {
                        case 0:
                            break;
                        case 1:
                            allusers = ((allusers == -1) ? -1 : (allusers - 1));
                            break;
                        case 2:
                            common ^= 1;
                            break;
                    }
                } else if ((task == importLoadiine) || (task == exportLoadiine)) {
                    switch (cursor) {
                        case 0:
                            slot--;
                            break;
                        case 1:
                            common ^= 1;
                            break;
                    }
                } else {
                    switch (cursor) {
                        case 0:
                            slot--;
                            break;
                        case 1:
                            allusers = ((allusers == -1) ? -1 : (allusers - 1));
                            break;
                        case 2:
                            common ^= 1;
                            break;
                    }
                }
            }
        } else if (input.get(TRIGGER, PAD_BUTTON_RIGHT)) {
            if (menu == selectOptions) {
                if (task == copytoOtherDevice) {
                    switch (cursor) {
                        case 0:
                            break;
                        case 1:
                            allusers = ((allusers == (wiiuaccn - 1)) ? (wiiuaccn - 1) : (allusers + 1));
                            allusers_d = allusers;
                            break;
                        case 2:
                            allusers_d = ((allusers_d == (wiiuaccn - 1)) ? (wiiuaccn - 1) : (allusers_d + 1));
                            allusers_d = (allusers == -1) ? -1 : allusers_d;
                            break;
                        case 3:
                            common ^= 1;
                            break;
                    }
                } else if (task == restore) {
                    switch (cursor) {
                        case 0:
                            slot++;
                            getAccountsSD(&titles[targ], slot);
                            break;
                        case 1:
                            sdusers = ((sdusers == (sdaccn - 1)) ? (sdaccn - 1) : (sdusers + 1));
                            allusers = ((sdusers > -1) && (allusers == -1)) ? 0 : allusers;
                            break;
                        case 2:
                            allusers = ((allusers == (wiiuaccn - 1)) ? (wiiuaccn - 1) : (allusers + 1));
                            allusers = (sdusers == -1) ? -1 : allusers;
                            break;
                        case 3:
                            common ^= 1;
                            break;
                    }
                } else if (task == wipe) {
                    switch (cursor) {
                        case 0:
                            break;
                        case 1:
                            allusers = ((allusers == (wiiuaccn - 1)) ? (wiiuaccn - 1) : (allusers + 1));
                            break;
                        case 2:
                            common ^= 1;
                            break;
                    }
                } else if ((task == importLoadiine) || (task == exportLoadiine)) {
                    switch (cursor) {
                        case 0:
                            slot++;
                            break;
                        case 1:
                            common ^= 1;
                            break;
                    }
                } else {
                    switch (cursor) {
                        case 0:
                            slot++;
                            break;
                        case 1:
                            allusers = ((allusers == (wiiuaccn - 1)) ? (wiiuaccn - 1) : (allusers + 1));
                            break;
                        case 2:
                            common ^= 1;
                            break;
                    }
                }
            }
        }

        if (input.get(TRIGGER, PAD_BUTTON_R)) {
            if (menu == selectTitle) {
                tsort = (tsort + 1) % 4;
                sortTitle(titles, titles + count, tsort, sortAscending);
            } else if (menu == selectTask)
                targ = (targ + 1) % count;
        }

        if (input.get(TRIGGER, PAD_BUTTON_L)) {
            if ((menu == selectTitle) && (tsort > 0)) {
                sortAscending = !sortAscending;
                sortTitle(titles, titles + count, tsort, sortAscending);
            } else if (menu == selectTask) {
                targ--;
                if (targ < 0)
                    targ = count - 1;
            }
        }

        if (input.get(TRIGGER, PAD_BUTTON_A)) {
            DrawUtils::clear(COLOR_BLACK);
            if (menu < 3) {
                if (menu == mainMenu) {
                    mode = (Mode) cursor;
                    if (mode == WiiU && ((wiiutitles == nullptr) || (wiiuTitlesCount == 0))) {
                        promptError(gettext("No Wii U titles found."));
                        continue;
                    }
                    if (mode == vWii && ((wiititles == nullptr) || (vWiiTitlesCount == 0))) {
                        promptError(gettext("No vWii saves found."));
                        continue;
                    }
                }

                if (menu == selectTitle) {
                    targ = cursor + scroll;
                    cursorb = cursor;
                    scrollb = scroll;
                    if (mode == batchBackup) {
                        OSCalendarTime dateTime;
                        switch (cursor) {
                            case 0:
                                dateTime.tm_year = 0;
                                backupAllSave(wiiutitles, wiiuTitlesCount, &dateTime);
                                backupAllSave(wiititles, vWiiTitlesCount, &dateTime);
                                break;
                            case 1:
                                backupAllSave(wiiutitles, wiiuTitlesCount, nullptr);
                                break;
                            case 2:
                                backupAllSave(wiititles, vWiiTitlesCount, nullptr);
                                break;
                        }
                        continue;
                    }
                    if (titles[targ].highID == 0 || titles[targ].lowID == 0)
                        continue;
                    if ((mode == WiiU) && (strcmp(titles[targ].shortName, "DONT TOUCH ME") == 0)) {
                        if (!promptConfirm(ST_ERROR, gettext("CBHC save. Could be dangerous to modify. Continue?")) ||
                            !promptConfirm(ST_WARNING, gettext("Are you REALLY sure?"))) {
                            continue;
                        }
                    }
                    std::string path = stringFormat("%s/usr/title/000%x/%x/code/fw.img",
                                                    (titles[targ].isTitleOnUSB) ? getUSB().c_str() : "storage_mlc01:", titles[targ].highID,
                                                    titles[targ].lowID);
                    if ((mode == WiiU) && (checkEntry(path.c_str()) != 0))
                        if (!promptConfirm(ST_ERROR, gettext("vWii saves are in the vWii section. Continue?")))
                            continue;
                    if ((mode == WiiU) && (!titles[targ].saveInit)) {
                        if (!promptConfirm(ST_WARNING, gettext("Recommended to run Game at least one time. Continue?")) ||
                            !promptConfirm(ST_WARNING, gettext("Are you REALLY sure?"))) {
                            continue;
                        }
                    }
                }

                if (menu == selectTask) {
                    task = (Task) cursor;
                    cursort = cursor;

                    if (task == backup) {
                        if (!titles[targ].saveInit) {
                            promptError(gettext("No save to Backup."));
                            continue;
                        }
                    }

                    if (task == restore) {
                        getAccountsSD(&titles[targ], slot);
                        allusers = ((sdusers == -1) ? -1 : allusers);
                        sdusers = ((allusers == -1) ? -1 : sdusers);
                    }

                    if (task == wipe) {
                        if (!titles[targ].saveInit) {
                            promptError(gettext("No save to Wipe."));
                            continue;
                        }
                    }

                    if ((task == importLoadiine) || (task == exportLoadiine)) {
                        char gamePath[PATH_SIZE];
                        memset(versionList, 0, 0x100 * sizeof(int));
                        if (getLoadiineGameSaveDir(gamePath, titles[targ].productCode, titles[targ].longName, titles[targ].highID, titles[targ].lowID) != 0)
                            continue;
                        getLoadiineSaveVersionList(versionList, gamePath);
                        if (task == importLoadiine) {
                            importFromLoadiine(&titles[targ], common, versionList != nullptr ? versionList[slot] : 0);
                            continue;
                        }
                        if (task == exportLoadiine) {
                            if (!titles[targ].saveInit) {
                                promptError(gettext("No save to Export."));
                                continue;
                            }
                            exportToLoadiine(&titles[targ], common, versionList != nullptr ? versionList[slot] : 0);
                            continue;
                        }
                    }

                    if (task == copytoOtherDevice) {
                        if (!titles[targ].saveInit) {
                            promptError(gettext("No save to Copy."));
                            continue;
                        }
                    }
                }
                menu++;
                cursor = scroll = 0;
            } else {
                switch (task) {
                    case backup:
                        backupSavedata(&titles[targ], slot, allusers, common);
                        break;
                    case restore:
                        restoreSavedata(&titles[targ], slot, sdusers, allusers, common);
                        break;
                    case wipe:
                        wipeSavedata(&titles[targ], allusers, common);
                        break;
                    case copytoOtherDevice:
                        for (int i = 0; i < count; i++) {
                            if (titles[i].listID == titles[targ].dupeID) {
                                copySavedata(&titles[targ], &titles[i], allusers, allusers_d, common);
                                break;
                            }
                        }
                        break;
                }
            }
        } else if (input.get(TRIGGER, PAD_BUTTON_B) && menu > 0) {
            DrawUtils::clear(COLOR_BLACK);
            menu--;
            cursor = scroll = 0;
            if (menu == selectTitle) {
                cursor = cursorb;
                scroll = scrollb;
            }
            if (menu == selectTask)
                cursor = cursort;
        } else if (input.get(TRIGGER, PAD_BUTTON_X) && menu == mainMenu) {
            configMenu();
            sortn = {gettext("None"), gettext("Name"), gettext("Storage"), gettext("Storage+Name")};
            unloadTitles(wiititles, vWiiTitlesCount);
            vWiiTitlesCount = 0;
            wiititles = loadWiiTitles();
            sortTitle(wiititles, wiititles + vWiiTitlesCount, tsort, sortAscending);
        }
    }

    unloadTitles(wiiutitles, wiiuTitlesCount);
    unloadTitles(wiititles, vWiiTitlesCount);
    free(versionList);

    deinitFS();
    gettextCleanUp();
    romfsExit();

    DrawUtils::deinitFont();
    shutdownState();
    WHBProcShutdown();
    ProcUIShutdown();
    return 0;
}
