#pragma once
#include <FOR_MACRO.h>

#include "./type.h"

// маскировать по ID_MASK маске
#define DI_MASK_ID(id) (did_t(id) & DHeader::ID_MASK)

// чистый хэш compile
static constexpr did_t _dhash(char const* str, uint32_t hash = 0) {
    return *str ? _dhash(str + 1, hash + (hash << 5) + *str) : hash;
}

// чистый хэш runtime
static did_t _dhash_r(char const* str) {
    uint32_t hash = 0;
    while (*str) hash = hash + (hash << 5) + *str++;
    return hash;
}

// хэш compile с маской id
static constexpr did_t DKEY(char const* str) {
    return DI_MASK_ID(_dhash(str));
}

// хэш runtime с маской id
static did_t DKEY_R(char const* str) {
    return DI_MASK_ID(_dhash_r(str));
}

// 000xxxxx xxxxxxxx xxxxxxxx xxxxxxxx
#define _DI_HASH(name) (DKEY(#name))
#define _DI_INIT(N, i, p, val) p.init val;
#define _DI_KEY(N, i, p, val) val = _DI_HASH(val),
#define _DI_CASE(N, i, p, val) case _DI_HASH(val):
#define _DI_CASE_CLASS(N, i, p, val) case p::val:

// DI_INIT
#define DI_INIT(name, ...) FOR_MACRO(_DI_INIT, name, __VA_ARGS__)

// _DI_CHECK_COLLS
#define _DI_CHECK_COLLS(name, case_macro, ...)      \
    inline void _FM_CONCAT(__ditest_, __LINE__)() { \
        switch (name()) {                           \
            FOR_MACRO(case_macro, name, __VA_ARGS__) \
            break;                                  \
        }                                           \
    }

// DI_KEYS
#define DI_KEYS(name, ...)                 \
    enum name : did_t {                    \
        FOR_MACRO(_DI_KEY, 0, __VA_ARGS__) \
    };                                     \
    _DI_CHECK_COLLS(name, _DI_CASE, __VA_ARGS__)

// DI_KEYS_CLASS
#define DI_KEYS_CLASS(name, ...)           \
    enum class name : did_t {              \
        FOR_MACRO(_DI_KEY, 0, __VA_ARGS__) \
    };                                     \
    _DI_CHECK_COLLS(name, _DI_CASE_CLASS, __VA_ARGS__)
