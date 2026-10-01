#include <Arduino.h>
#include <DictEE.h>
#include <EEPROM.h>

DictEE db(0, 10000);

void setup() {
    // EEPROM.begin();  // ESP
    db.begin();
    db.init(DKEY("period"), 1000);  // если такой ячейки нет - создат и инициализирует 1000
}

void loop() {
    db.tick();  // автоматически запишет в файл при изменении
}