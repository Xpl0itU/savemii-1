#include <coreinit/memdefaultheap.h>
#include <jansson.h>
#include <savemng.h>
#include <utils/DrawUtils.h>
#include <utils/LanguageUtils.h>

MSG *LanguageUtils::baseMSG = nullptr;

Swkbd_LanguageType LanguageUtils::sysLang;
Swkbd_LanguageType LanguageUtils::loadedLang;

void LanguageUtils::loadLanguage(Swkbd_LanguageType language) {
    loadedLang = language;
    switch (language) {
        case Swkbd_LanguageType__Japanese:
            DrawUtils::setFont(OS_SHAREDDATATYPE_FONT_STANDARD);
            gettextLoadLanguage("romfs:/japanese.json");
            break;
        case Swkbd_LanguageType__English:
            DrawUtils::setFont(OS_SHAREDDATATYPE_FONT_STANDARD);
            gettextLoadLanguage("romfs:/english.json");
            break;
        /*case Swkbd_LanguageType__French:
			gettextLoadLanguage("romfs:/french.json");
            break;
		*/
        case Swkbd_LanguageType__German:
            gettextLoadLanguage("romfs:/german.json");
            break;
        case Swkbd_LanguageType__Italian:
            DrawUtils::setFont(OS_SHAREDDATATYPE_FONT_STANDARD);
            gettextLoadLanguage("romfs:/italian.json");
            break;
        case Swkbd_LanguageType__Spanish:
            DrawUtils::setFont(OS_SHAREDDATATYPE_FONT_STANDARD);
            gettextLoadLanguage("romfs:/spanish.json");
            break;
        case Swkbd_LanguageType__Chinese1:
            DrawUtils::setFont(OS_SHAREDDATATYPE_FONT_CHINESE);
            gettextLoadLanguage("romfs:/TChinese.json");
            break;
        case Swkbd_LanguageType__Korean:
            DrawUtils::setFont(OS_SHAREDDATATYPE_FONT_KOREAN);
            gettextLoadLanguage("romfs:/korean.json");
            break;
        /*
        case Swkbd_LanguageType__Dutch:
            gettextLoadLanguage("romfs:/dutch.json");
            break;
        */
        case Swkbd_LanguageType__Portuguese:
			gettextLoadLanguage("romfs:/portuguese.json");
            break;
        case Swkbd_LanguageType__Russian:
            DrawUtils::setFont(OS_SHAREDDATATYPE_FONT_STANDARD);
            gettextLoadLanguage("romfs:/russian.json");
            break;
        case Swkbd_LanguageType__Chinese2:
            DrawUtils::setFont(OS_SHAREDDATATYPE_FONT_CHINESE);
            gettextLoadLanguage("romfs:/SChinese.json");
            break;
        default:
            DrawUtils::setFont(OS_SHAREDDATATYPE_FONT_STANDARD);
            gettextLoadLanguage("romfs:/english.json");
            break;
    }
}

std::string LanguageUtils::getLoadedLanguage() {
    switch (loadedLang) {
        case Swkbd_LanguageType__Japanese:
            return gettext("Japanese");
        case Swkbd_LanguageType__English:
            return gettext("English");
        /*case Swkbd_LanguageType__French:
			return gettext("French");
		*/
        case Swkbd_LanguageType__German:
            return gettext("German");
        case Swkbd_LanguageType__Italian:
            return gettext("Italian");
        case Swkbd_LanguageType__Spanish:
            return gettext("Spanish");
        case Swkbd_LanguageType__Chinese1:
            return gettext("Traditional Chinese");
        case Swkbd_LanguageType__Korean:
            return gettext("Korean");
        /*
        case Swkbd_LanguageType__Dutch:
            return gettext("Dutch");
        */
        case Swkbd_LanguageType__Portuguese:
            return gettext("Portuguese");
        case Swkbd_LanguageType__Russian:
            return gettext("Russian");
        case Swkbd_LanguageType__Chinese2:
            return gettext("Simplified Chinese");
        default:
            return gettext("English");
    }
}

Swkbd_LanguageType LanguageUtils::getSystemLanguage() {
    UCHandle handle = UCOpen();
    if (handle < 1)
        sysLang = Swkbd_LanguageType__English;

    auto *settings = (UCSysConfig *) MEMAllocFromDefaultHeapEx(sizeof(UCSysConfig), 0x40);
    if (settings == nullptr) {
        UCClose(handle);
        sysLang = Swkbd_LanguageType__English;
    }

    strcpy(settings->name, "cafe.language");
    settings->access = 0;
    settings->dataType = UC_DATATYPE_UNSIGNED_INT;
    settings->error = UC_ERROR_OK;
    settings->dataSize = sizeof(Swkbd_LanguageType);
    settings->data = &sysLang;

    UCError err = UCReadSysConfig(handle, 1, settings);
    UCClose(handle);
    MEMFreeToDefaultHeap(settings);
    if (err != UC_ERROR_OK)
        sysLang = Swkbd_LanguageType__English;
    return sysLang;
}

// Hashing function from https://stackoverflow.com/a/2351171
uint32_t LanguageUtils::hash_string(const char *str_param) {
    uint32_t hash = 0;

    while (*str_param != '\0')
        hash = hashMultiplier * hash + *str_param++;

    return hash;
}

MSG *LanguageUtils::findMSG(uint32_t id) {
    for (MSG *msg = baseMSG; msg; msg = msg->next)
        if (msg->id == id)
            return msg;

    return nullptr;
}

void LanguageUtils::setMSG(const char *msgid, const char *msgstr) {
    if (!msgstr)
        return;

    uint32_t id = hash_string(msgid);
    MSG *msg = (MSG *) MEMAllocFromDefaultHeap(sizeof(MSG));
    msg->id = id;
    msg->msgstr = strdup(msgstr);
    msg->next = baseMSG;
    baseMSG = msg;
}

void LanguageUtils::gettextCleanUp() {
    while (baseMSG) {
        MSG *nextMsg = baseMSG->next;
        MEMFreeToDefaultHeap((void *) (baseMSG->msgstr));
        MEMFreeToDefaultHeap(baseMSG);
        baseMSG = nextMsg;
    }
}

bool LanguageUtils::gettextLoadLanguage(const char *langFile) {
    uint8_t *buffer;
    int32_t size = loadFile(langFile, &buffer);
    if (buffer == nullptr)
        return false;

    bool ret = true;
    json_t *json = json_loadb((const char *) buffer, size, 0, nullptr);
    if (json) {
        size = json_object_size(json);
        if (size != 0) {
            const char *key;
            json_t *value;
            json_object_foreach(json, key, value) if (json_is_string(value))
                    setMSG(key, json_string_value(value));
        } else {
            ret = false;
        }

        json_decref(json);
    } else {
        ret = false;
    }

    MEMFreeToDefaultHeap(buffer);
    return ret;
}

const char *LanguageUtils::gettext(const char *msgid) {
    MSG *msg = findMSG(hash_string(msgid));
    return msg ? msg->msgstr : msgid;
}
