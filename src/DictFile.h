#pragma once
#if defined(ESP8266) || defined(ESP32)

#include <FS.h>

#include "DictAuto.h"

class DictFile : public DictAuto {
   public:
    DictFile(fs::FS* fs, const char* path, uint16_t timeoutMs = 10000) : DictAuto(timeoutMs), _fs(fs), _path(path) {}

    ~DictFile() {
        update();
    }

    // прочитать бд из файла
    bool begin() {
        if (!_fs || !_path) return false;

        if (_fs->exists(_path)) {
            File file = _fs->open(_path, "r");
            if (!file) return false;

            DiStreamIO<Stream> io(file);
            return readFrom(io);
        } else {
            File file = _fs->open(_path, "w");
            return file;
        }
    }

   private:
    fs::FS* _fs;
    const char* _path;

    bool _update() override {
        if (!_fs || !_path) return false;

        File file = _fs->open(_path, "w");
        if (!file) return false;

        DiStreamIO<Stream> io(file);
        return writeTo(io);
    }
};

#endif
