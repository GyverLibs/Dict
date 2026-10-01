#pragma once
#include <stdint.h>
#include <string.h>

class DictIO {
   public:
    template <typename T>
    bool write(T& buf) { return write(&buf, sizeof(T)); }
    bool write(void* buf, uint16_t len) { return _process(buf, len, true); }

    template <typename T>
    bool read(T& buf) { return read(&buf, sizeof(T)); }
    bool read(void* buf, uint16_t len) { return _process(buf, len, false); }

   protected:
    virtual bool _process(void* buf, uint16_t len, bool write) = 0;
};

class DiBufIO : public DictIO {
   public:
    DiBufIO(uint8_t* buf, uint16_t len) : _buf(buf), _len(len) {}

    bool _process(void* buf, uint16_t len, bool write) override {
        if (!len) return true;
        if (len > _len) return false;
        write ? memcpy(_buf, buf, len) : memcpy(buf, _buf, len);
        _buf += len;
        _len -= len;
        return true;
    }

   private:
    uint8_t* _buf;
    uint16_t _len;
};

template <typename T>
class DiStreamIO : public DictIO {
   public:
    DiStreamIO(T& str) : _str(str) {}

    bool _process(void* buf, uint16_t len, bool write) override {
        if (write) return _str.write((const uint8_t*)buf, len) == len;
        else return _str.readBytes((uint8_t*)buf, len) == len;
    }

   private:
    T& _str;
};

template <typename T>
class DiArrayIO : public DictIO {
   public:
    DiArrayIO(T& arr, uint16_t pos = 0) : _arr(arr), _pos(pos) {}

    bool _process(void* buf, uint16_t len, bool write) override {
        uint8_t* p = (uint8_t*)buf;

        while (len--) {
            if (write) _arr[_pos++] = *p++;
            else *p++ = _arr[_pos++];
        }
        return true;
    }

   private:
    T& _arr;
    uint16_t _pos;
};

template <typename EE>
class DiEEIO : public DictIO {
   public:
    DiEEIO(EE& ee, uint16_t addr = 0) : _ee(ee), _pos(addr) {}

   protected:
    bool _process(void* buf, uint16_t len, bool write) override {
        uint8_t* p = (uint8_t*)buf;

        while (len--) {
            if (write) {
                if (_ee.read(_pos) != *p) _ee.write(_pos, *p);
                ++_pos;
                ++p;
            } else {
                *p++ = _ee.read(_pos++);
            }
        }
        return true;
    }

   private:
    EE& _ee;
    uint16_t _pos;
};