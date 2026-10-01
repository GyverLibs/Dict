#pragma once
#include <EEPROM.h>

#include "DictAuto.h"

class DictEE : public DictAuto {
   public:
    DictEE(uint16_t addr = 0, uint16_t timeoutMs = 10000) : DictAuto(timeoutMs), _addr(addr) {}

    ~DictEE() {
        update();
    }

    // прочитать бд из EEPROM
    bool begin() {
        DiEEIO<EEPROMClass> io(EEPROM, _addr);
        return readFrom(io);
    }

   private:
    uint16_t _addr;

    bool _update() override {
        DiEEIO<EEPROMClass> io(EEPROM, _addr);
        if (!writeTo(io)) return false;

#if defined(ESP8266) || defined(ESP32)
        return EEPROM.commit();
#else
        return true;
#endif
    }
};
