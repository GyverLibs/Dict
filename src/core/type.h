#pragma once
#include <stdint.h>

typedef uint32_t did_t;

template <bool enabled, typename T = void>
struct DEnableIf {};

template <typename T>
struct DEnableIf<true, T> {
    typedef T type;
};

template <typename T>
struct DIsEnum {
    static const bool value = __is_enum(T);
};

enum class DType : uint8_t {
    Bool,
    Int,
    Uint,
    Int64,
    Uint64,
    Float,
    String,
    Bin,
};

#if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
struct DHeader {
    uint32_t id : 29;
    uint32_t type : 3;

    static constexpr uint32_t ID_MASK = 0x1FFFFFFFUL;
    static constexpr uint32_t TYPE_MASK = 0xE0000000UL;

    inline did_t getID() const {
        return id;
    }

    inline DType getType() const {
        return DType(type);
    }

    inline void setID(did_t nid) {
        id = nid;
    }

    inline void setType(DType ntype) {
        type = uint32_t(ntype);
    }
};
#else
struct DHeader {
    uint32_t raw = 0;

    static constexpr uint32_t ID_MASK = 0x1FFFFFFFUL;
    static constexpr uint32_t TYPE_MASK = 0xE0000000UL;

    inline did_t getID() const {
        return raw & ID_MASK;
    }

    inline DType getType() const {
        return DType(raw >> 29);
    }

    inline void setID(did_t id) {
        raw = (raw & ~ID_MASK) | (id & ID_MASK);
    }

    inline void setType(DType type) {
        raw = (raw & ~TYPE_MASK) | ((uint32_t(type) & 0x7) << 29);
    }
};
#endif
