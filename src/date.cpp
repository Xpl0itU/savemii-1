#include <date.h>

#define FS_ALIGN(x) ((x + 0x3F) & ~(0x3F))

std::string Date::get() {
    if (checkEntry(path.c_str()) != 0) {
        FILE *f = nullptr;
        long len = 0;

        /* open in read binary mode */
        f = fopen(path.c_str(), "rb");
        /* get the length */
        fseek(f, 0, SEEK_END);
        len = ftell(f);
        fseek(f, 0, SEEK_SET);

        char *data = (char *) aligned_alloc(0x40, FS_ALIGN(len + 1));

        fread(data, 1, len, f);
        data[len] = '\0';
        fclose(f);

        char *out = nullptr;
        json_t *root;
        json_error_t error;

        root = json_loads(data, 0, &error);

        if (root) {
            out = (char *) json_string_value(json_object_get(root, "Date"));
            std::string buf;
            buf.assign(out);
            json_decref(root);

            free(data);
            return buf;
        }
        return "";
    }
    return "";
}

bool Date::set(const std::string &date) {
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
