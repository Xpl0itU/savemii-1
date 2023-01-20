#pragma once

#include <jansson.h>
#include <utils/StringUtils.h>

class Date {
public:
    Date(uint32_t high, uint32_t low, uint8_t s) : highID(high),
                                                   lowID(low),
                                                   slot(s),
                                                   path(StringUtils::stringFormat("sd:/wiiu/backups/%08x%08x/%u/savemiiMeta.json", highID, lowID, slot)) {
    }
    std::string get();
    bool set(const std::string &date);

private:
    uint32_t highID;
    uint32_t lowID;
    uint8_t slot;
    std::string path;
};