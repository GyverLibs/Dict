#include <Arduino.h>
#include <LittleFS.h>
#include <DictFile.h>

DictFile db(&LittleFS, "/settings.dat", 10000);

void setup() {
    LittleFS.begin();
    db.begin();
    db.init(DKEY("period"), 1000);  // если такой ячейки нет - создат и инициализирует 1000
}

void loop() {
    db.tick();  // автоматически запишет в файл при изменении
}