#pragma once
#include <Arduino.h>

#include "Dict.h"

class DictAuto : public Dict {
   public:
    DictAuto(uint16_t timeoutMs) : _prd(timeoutMs) {}

    // вызывать в loop(). Запишет данные по тайм-ауту
    bool tick() {
        if (Dict::changed()) {
            _tmr = millis();
            if (!_tmr) --_tmr;
        }
        if (_tmr && uint16_t(uint16_t(millis()) - _tmr) >= _prd) {
            return update();
        }
        return false;
    }

    // обновить сейчас
    bool update() {
        _tmr = 0;
        changed();
        if (_update()) return true;

        _tmr = millis();
        if (!_tmr) --_tmr;
        return false;
    }

    using Dict::update;

   private:
    uint16_t _tmr = 0;
    uint16_t _prd;

    virtual bool _update() = 0;
};
