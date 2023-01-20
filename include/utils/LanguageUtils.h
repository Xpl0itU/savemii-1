#pragma once

#include <romfs-wiiu.h>
#include <string>
#include <wut_types.h>

#include <coreinit/memdefaultheap.h>
#include <coreinit/time.h>
#include <coreinit/userconfig.h>

#include <cstring>

typedef enum {
    Swkbd_LanguageType__Japanese = 0,
    Swkbd_LanguageType__English = 1,
    Swkbd_LanguageType__French = 2,
    Swkbd_LanguageType__German = 3,
    Swkbd_LanguageType__Italian = 4,
    Swkbd_LanguageType__Spanish = 5,
    Swkbd_LanguageType__Chinese1 = 6,
    Swkbd_LanguageType__Korean = 7,
    Swkbd_LanguageType__Dutch = 8,
    Swkbd_LanguageType__Potuguese = 9,
    Swkbd_LanguageType__Russian = 10,
    Swkbd_LanguageType__Chinese2 = 11,
    Swkbd_LanguageType__Invalid = 12
} Swkbd_LanguageType;

struct MSG {
    uint32_t id;
    const char *msgstr;
    MSG *next;
};

class LanguageUtils {
public:
    static void loadLanguage(Swkbd_LanguageType language) __attribute__((cold));
    static std::string getLoadedLanguage();
    static Swkbd_LanguageType getSystemLanguage() __attribute__((cold));
    static void gettextCleanUp() __attribute__((__cold__));
    static const char *gettext(const char *msg) __attribute__((__hot__));

private:
    static bool gettextLoadLanguage(const char *langFile);
    static uint32_t hash_string(const char *str_param);
    static MSG *findMSG(uint32_t id);
    static void setMSG(const char *msgid, const char *msgstr);

    static MSG *baseMSG;
    static Swkbd_LanguageType sysLang;
    static Swkbd_LanguageType loadedLang;

    static constexpr uint8_t hashMultiplier = 31; // or 37
};