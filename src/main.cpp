#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <malloc.h>

#include <algorithm>
#include <array>

#include <coreinit/ios.h>
#include <coreinit/mcp.h>
#include <coreinit/screen.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <padscore/kpad.h>
#include <sysapp/launch.h>
#include <sysapp/title.h>
#include <vpad/input.h>
#include <whb/proc.h>

#include <ApplicationState.h>
#include <icon.h>
#include <date.h>
#include <savemng.h>
#include <menu/MainMenuState.h>
#include <utils/DrawUtils.h>
#include <utils/LanguageUtils.h>
#include <utils/StateUtils.h>
#include <utils/StringUtils.h>
#include <utils/InputUtils.h>

#include <coreinit/debug.h>
#include <proc_ui/procui.h>
#include <sndcore2/core.h>

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
    consolePrintPosAligned(14, 0, 1, LanguageUtils::gettext("Disclaimer:"));
    consolePrintPosAligned(15, 0, 1, LanguageUtils::gettext("There is always the potential for a brick."));
    consolePrintPosAligned(16, 0, 1, LanguageUtils::gettext("Everything you do with this software is your own responsibility"));
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
        promptError(LanguageUtils::gettext("Out of memory."));
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
            std::string path = StringUtils::stringFormat("%s/usr/save/%08x", (i == 0) ? getUSB().c_str() : "storage_mlc01:", highIDs[a]);
            DIR *dir = opendir(path.c_str());
            if (dir != nullptr) {
                struct dirent *data;
                while ((data = readdir(dir)) != nullptr) {
                    if (data->d_name[0] == '.')
                        continue;

                    path = StringUtils::stringFormat("%s/usr/save/%08x/%s/user", (i == 0) ? getUSB().c_str() : "storage_mlc01:", highIDs[a], data->d_name);
                    if (checkEntry(path.c_str()) == 2) {
                        path = StringUtils::stringFormat("%s/usr/save/%08x/%s/meta/meta.xml", (i == 0) ? getUSB().c_str() : "storage_mlc01:", highIDs[a],
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
        promptError(LanguageUtils::gettext("Out of memory."));
        return nullptr;
    }

    for (uint8_t a = 0; a < 2; a++) {
        for (int i = 0; i <= 1; i++) {
            std::string path = StringUtils::stringFormat("%s/usr/save/%08x", (i == 0) ? getUSB().c_str() : "storage_mlc01:", highIDs[a]);
            DIR *dir = opendir(path.c_str());
            if (dir != nullptr) {
                struct dirent *data;
                while ((data = readdir(dir)) != nullptr) {
                    if (data->d_name[0] == '.')
                        continue;

                    path = StringUtils::stringFormat("%s/usr/save/%08x/%s/meta/meta.xml", (i == 0) ? getUSB().c_str() : "storage_mlc01:", highIDs[a],
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
        promptError(LanguageUtils::gettext("Out of memory."));
        return nullptr;
    }

    for (int i = 0; i < foundCount; i++) {
        uint32_t highID = saves[i].highID;
        uint32_t lowID = saves[i].lowID;
        bool isTitleOnUSB = saves[i].dev == 0u;

        const std::string path = StringUtils::stringFormat("%s/usr/%s/%08x/%08x/meta/meta.xml", isTitleOnUSB ? getUSB().c_str() : "storage_mlc01:",
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

            StringUtils::decodeXMLEscapeLine(std::string(cptr));
            strlcpy(titles[wiiuTitlesCount].shortName, StringUtils::decodeXMLEscapeLine(std::string(cptr)).c_str(), strcspn(StringUtils::decodeXMLEscapeLine(std::string(cptr)).c_str(), "<") + 1);

            cptr = strchr(strstr(xmlBuf, "longname_en"), '>') + 1;
            memset(titles[i].longName, 0, sizeof(titles[i].longName));
            if (strcspn(cptr, "<") == 0)
                cptr = strchr(strstr(xmlBuf, "longname_ja"), '>') + 1;

            strlcpy(titles[wiiuTitlesCount].longName, StringUtils::decodeXMLEscapeLine(std::string(cptr)).c_str(), strcspn(StringUtils::decodeXMLEscapeLine(std::string(cptr)).c_str(), "<") + 1);

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

        DrawUtils::beginDraw();
        DrawUtils::clear(COLOR_BLACK);
        disclaimer();
        DrawUtils::drawTGA(298, 144, 1, icon_tga);
        consolePrintPosAligned(10, 0, 1, LanguageUtils::gettext("Loaded %i Wii U titles."), wiiuTitlesCount);
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
        pathW = StringUtils::stringFormat("storage_slccmpt01:/title/%s", highIDs[k]);
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
        promptError(LanguageUtils::gettext("Out of memory."));
        return nullptr;
    }

    int i = 0;
    for (int k = 0; k < 3; k++) {
        pathW = StringUtils::stringFormat("storage_slccmpt01:/title/%s", highIDs[k]);
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

                const std::string path = StringUtils::stringFormat("storage_slccmpt01:/title/%s/%s/data/banner.bin", highIDs[k], data->d_name);
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
                    sprintf(titles[i].shortName, LanguageUtils::gettext("%s%s (No banner.bin)"), highIDs[k], data->d_name);
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

                DrawUtils::beginDraw();
                DrawUtils::clear(COLOR_BLACK);
                disclaimer();
                DrawUtils::drawTGA(298, 144, 1, icon_tga);
                consolePrintPosAligned(10, 0, 1, LanguageUtils::gettext("Loaded %i Wii U titles."), wiiuTitlesCount);
                consolePrintPosAligned(11, 0, 1, LanguageUtils::gettext("Loaded %i Wii titles."), i);
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
    State::init();

    int res = romfsInit();
    if (res) {
        promptError("Failed to init romfs: %d", res);
        DrawUtils::endDraw();
        State::shutdown();
        return 0;
    }

    Swkbd_LanguageType systemLanguage = LanguageUtils::getSystemLanguage();
    LanguageUtils::loadLanguage(systemLanguage);

    if (!initFS()) {
        promptError(LanguageUtils::gettext("initFS failed. Please make sure your MochaPayload is up-to-date"));
        DrawUtils::endDraw();
        State::shutdown();
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
    std::unique_ptr<MainMenuState> state = std::make_unique<MainMenuState>(wiiutitles, wiititles, wiiuTitlesCount, vWiiTitlesCount);
    while (State::AppRunning()) {
        Title *titles = mode != WiiU ? wiititles : wiiutitles;
        int count = mode != WiiU ? vWiiTitlesCount : wiiuTitlesCount;
        
        input.read();

        if (input.get(TRIGGER, PAD_BUTTON_ANY))
            redraw = true;

        if (redraw) {
            DrawUtils::beginDraw();
            DrawUtils::clear(COLOR_BACKGROUND);

            consolePrintPos(0, 0, "SaveMii v%u.%u.%u", VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO);
            DrawUtils::drawRectFilled(48, 49, 526, 51, COLOR_WHITE);

            state->update(&input);
            state->render();

            DrawUtils::drawRectFilled(48, 406, 526, 408, COLOR_WHITE);
            consolePrintPos(0, 17, LanguageUtils::gettext("Press \ue044 to exit."));

            DrawUtils::endDraw();
            redraw = false;
        }
    }

    unloadTitles(wiiutitles, wiiuTitlesCount);
    unloadTitles(wiititles, vWiiTitlesCount);
    free(versionList);

    deinitFS();
    LanguageUtils::gettextCleanUp();
    romfsExit();

    DrawUtils::deinitFont();
    State::shutdown();
    return 0;
}
