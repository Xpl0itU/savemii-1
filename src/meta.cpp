#include <meta.h>

#define FS_ALIGN(x) ((x + 0x3F) & ~(0x3F))

std::string Meta::getDate() {
    if (checkEntry(path.c_str()) != 0) {
        FILE *f = fopen(path.c_str(), "rb");
        fseek(f, 0, SEEK_END);
        long len = ftell(f);
        fseek(f, 0, SEEK_SET);

        char *data = (char *) aligned_alloc(0x40, FS_ALIGN(len + 1));

        fread(data, 1, len, f);
        data[len] = '\0';
        fclose(f);

        json_t *root;
        json_error_t error;

        root = json_loads(data, 0, &error);

        if (root) {
            std::string buf(json_string_value(json_object_get(root, "Date")));
            json_decref(root);

            free(data);
            return buf;
        }
        return "";
    }
    return "";
}

bool Meta::setDate(const std::string &date) {
    json_t *config = json_object();
    if (config == nullptr)
        return false;

    json_object_set_new(config, "Date", json_string(date.c_str()));

    char *configString = json_dumps(config, 0);
    if (configString == nullptr)
        return false;

    json_decref(config);

    FILE *fp = fopen(path.c_str(), "wb");
    if (fp == nullptr)
        return false;

    fwrite(configString, strlen(configString), 1, fp);

    fclose(fp);
    return true;
}

std::string Meta::getName() {
    if (checkEntry(path.c_str()) != 0) {
        FILE *f = fopen(path.c_str(), "rb");
        fseek(f, 0, SEEK_END);
        long len = ftell(f);
        fseek(f, 0, SEEK_SET);

        char *data = (char *) aligned_alloc(0x40, FS_ALIGN(len + 1));

        fread(data, 1, len, f);
        data[len] = '\0';
        fclose(f);

        json_t *root;
        json_error_t error;

        root = json_loads(data, 0, &error);

        if (root) {
            std::string buf(json_string_value(json_object_get(root, "Name")));
            json_decref(root);

            free(data);
            return buf;
        }
        return "";
    }
    return "";
}

bool Meta::setName(const std::string &name) {
    json_t *config = json_object();
    if (config == nullptr)
        return false;

    json_object_set_new(config, "Name", json_string(name.c_str()));

    char *configString = json_dumps(config, 0);
    if (configString == nullptr)
        return false;

    json_decref(config);

    FILE *fp = fopen(path.c_str(), "wb");
    if (fp == nullptr)
        return false;

    fwrite(configString, strlen(configString), 1, fp);

    fclose(fp);
    return true;
}