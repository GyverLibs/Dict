#pragma once
#include <stdlib.h>
#include <string.h>

#include "./type.h"

class DBlock {
   public:
    // ================ Ctr ================
    DBlock() {}
    DBlock(did_t id) {
        header.setID(id);
    }
    DBlock(did_t id, DType type) {
        header.setID(id);
        header.setType(type);
    }

    // ================ Data ================
    DHeader header;

    union {
        uint8_t* p = nullptr;
        uint32_t raw32;
        float f;
    } _data;

    // bsearch
    bool operator==(const DBlock& b) const { return header.getID() == b.header.getID(); }
    bool operator<(const DBlock& b) const { return header.getID() < b.header.getID(); }

    // ================ Func ================
    did_t id() const {
        return header.getID();
    }

    DType type() const {
        return header.getType();
    }

    bool is(DType type) const {
        return header.getType() == type;
    }

    bool same(DType other) const {
        DType t = type();
        return (t == other) ||
               (t == DType::Int && other == DType::Uint) ||
               (t == DType::Uint && other == DType::Int) ||
               (t == DType::Int64 && other == DType::Uint64) ||
               (t == DType::Uint64 && other == DType::Int64);
    }

    // инициализировать, присвоить тип и записать данные
    bool init(const void* p, uint16_t size, DType type) {
        header.setType(type);
        return update(p, size);
    }

    // изменить размер
    bool resize(uint16_t size) {
        switch (type()) {
            case DType::String:
                if (size > UINT16_MAX - 3 || !_resize(2 + size + 1)) return false;
                memcpy(_data.p, &size, 2);
                _data.p[2 + size] = 0;
                return true;

            case DType::Bin:
                if (size > UINT16_MAX - 2 || !_resize(2 + size)) return false;
                memcpy(_data.p, &size, 2);
                return true;

#ifdef DICT_NO_FLOAT
            case DType::Float:
                return false;
#endif

            case DType::Int64:
            case DType::Uint64:
#ifndef DICT_NO_INT64
                return _data.p || _resize(8);
#else
                return false;
#endif
            default:
                return true;
        }
    }

    // обновить (при совпадающем типе)
    // Int/Uint/Float всегда 32 бит
    bool update(const void* p, uint16_t size) {
        switch (type()) {
            case DType::Bool:
                _data.raw32 = *(bool*)p;
                break;

            case DType::Int:
            case DType::Uint:
                memcpy(&_data.raw32, p, 4);
                break;

            case DType::Int64:
            case DType::Uint64:
#ifndef DICT_NO_INT64
                if (!resize(8)) return false;
                memcpy(_data.p, p, 8);
                break;
#else
                return false;
#endif
#ifndef DICT_NO_FLOAT
            case DType::Float:
                memcpy(&_data.f, p, 4);
                break;
#else
                return false;
#endif

            case DType::Bin:
            case DType::String:
                if (!resize(size)) return false;
                memcpy(_data.p + 2, p, size);
                break;

            default:
                break;
        }
        return true;
    }

    // сбросить данные
    void reset() {
        switch (type()) {
            case DType::Bin:
            case DType::Int64:
            case DType::Uint64:
            case DType::String:
                free(_data.p);
                break;

            default:
                break;
        }
        _data.p = nullptr;
    }

    // вес данных
    uint16_t size() const {
        switch (type()) {
            case DType::Bool:
                return 1u;

            case DType::Int:
            case DType::Uint:
#ifndef DICT_NO_FLOAT
            case DType::Float:
#endif
                return 4u;

#ifndef DICT_NO_INT64
            case DType::Int64:
            case DType::Uint64:
                return 8u;
#endif
            case DType::Bin:
            case DType::String:
                return rawExtSize();

            default:
                return 0u;
        }
    }

    // указатель на данные размера size()
    void* data() const {
        switch (type()) {
            case DType::Int:
            case DType::Uint:
            case DType::Bool:
                return (void*)&_data.raw32;

#ifndef DICT_NO_FLOAT
            case DType::Float:
                return (void*)&_data.f;
#endif
#ifndef DICT_NO_INT64
            case DType::Int64:
            case DType::Uint64:
                return _data.p;
#endif

            case DType::Bin:
            case DType::String:
                return rawBin();

            default:
                return nullptr;
        }
    }

    void* rawBin() const {
        return _data.p ? (_data.p + 2) : nullptr;
    }

    const char* rawStr() const {
        return (const char*)rawBin();
    }

    bool rawBool() const {
        return _data.raw32;
    }

    float rawFloat() const {
        return _data.f;
    }

    uint32_t rawUint() const {
        return _data.raw32;
    }

    int32_t rawInt() const {
        return (int32_t)rawUint();
    }

    uint64_t rawUint64() const {
        return _data.p ? *(uint64_t*)_data.p : 0;
    }

    int64_t rawInt64() const {
        return (int64_t)rawUint64();
    }

    uint16_t rawExtSize() const {
        return _data.p ? *(uint16_t*)_data.p : 0u;
    }

    template <typename T>
    T toNum() const {
        switch (type()) {
            case DType::Bool: return (T)rawBool();
            case DType::Int: return (T)rawInt();
            case DType::Uint: return (T)rawUint();
#ifndef DICT_NO_INT64
            case DType::Int64: return (T)rawInt64();
            case DType::Uint64: return (T)rawUint64();
#endif
#ifndef DICT_NO_FLOAT
            case DType::Float: return (T)rawFloat();
#endif
            default: return T(0);
        }
    }

    bool hasExtSize() const {
        switch (type()) {
            case DType::Bin:
            case DType::String:
                return true;

            default:
                return false;
        }
    }

    uint16_t extSize() const {
        switch (type()) {
#ifndef DICT_NO_INT64
            case DType::Int64:
            case DType::Uint64:
                return 8;
#endif
            case DType::Bin:
                return rawExtSize() + 2;

            case DType::String:
                return rawExtSize() + 3;

            default:
                return 0;
        }
    }

    // напечатать
    template <typename P>
    void printTo(P& p) {
        switch (type()) {
            default: break;
            case DType::Bool: p.print(rawBool()); break;
            case DType::Int: p.print(rawInt()); break;
            case DType::Uint: p.print(rawUint()); break;
            case DType::String: p.print(rawStr()); break;
#ifndef DICT_NO_FLOAT
            case DType::Float: p.print(rawFloat()); break;
#endif
#ifndef DICT_NO_INT64
            case DType::Int64:
            case DType::Uint64:
#endif
            case DType::Bin: {
                uint16_t len = size();
                uint8_t* d = (uint8_t*)data();
                for (uint16_t i = 0; i < len; i++) {
                    if (i) p.print(' ');
                    if (d[i] < 0x10) p.print('0');
                    p.print(d[i], 16);
                }
            } break;
        }
    }

    // вывести в String. Без поддержки 64 бит
    template <typename S>
    void toJson(S& s) {
        switch (type()) {
            case DType::Bool: s += rawBool() ? "true" : "false"; break;
            case DType::Int: s += rawInt(); break;
            case DType::Uint: s += rawUint(); break;
#ifndef DICT_NO_INT64
            case DType::Int64: s += (long)rawInt64(); break;
            case DType::Uint64: s += (unsigned long)rawUint64(); break;
#endif
#ifndef DICT_NO_FLOAT
            case DType::Float: s += rawFloat(); break;
#endif
            case DType::String: s += '"', s += rawStr(), s += '"'; break;
            case DType::Bin: s += "\"bin\""; break;
            default: break;
        }
    }

    // вывести в тип-конвертер T(), например convert<String>(). Без поддержки 64 бит
    template <typename T>
    T convert() {
        switch (type()) {
            case DType::Bool: return (T)rawBool();
            case DType::Int: return (T)rawInt();
            case DType::Uint: return (T)rawUint();
#ifndef DICT_NO_FLOAT
            case DType::Float: return (T)rawFloat();
#endif
            case DType::String: return T(rawStr());
            case DType::Bin: return T("bin");
            default: return T(0);
        }
    }

   private:
    bool _resize(uint16_t size) {
        uint8_t* p = (uint8_t*)realloc(_data.p, size);
        if (!p) return false;

        _data.p = p;
        return true;
    }
};
