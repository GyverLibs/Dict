#pragma once
#include <GTL.h>

#include "./core/block.h"
#include "./core/io.h"
#include "./core/macro.h"
#include "./core/type.h"

class Dict {
    typedef void (*ChangeCallback)(did_t id);

    enum class CMD : uint8_t {
        Init,    // создать и записать, если нету или отличается тип
        Set,     // создать если нету, записать
        Update,  // обновить, не создавать если нету
    };

   public:
    // =============== ACCESS ===============
    class Access {
       public:
        Access(Dict* di, did_t id) : _di(di), _id(id) {}

        // получить id блока
        did_t id() const {
            return _id;
        }

        // получить блок
        DBlock* block() const {
            return _di->findBlock(_id);
        }

        operator DBlock*() const {
            return block();
        }

        // валидный блок
        bool valid() const {
            return block();
        }

        // напечатать
        template <typename P>
        void printTo(P& p) {
            DBlock* b = block();
            if (b) b->printTo(p);
        }

        template <typename P>
        void printlnTo(P& p) {
            printTo(p);
            p.println();
        }

        // вывести в тип-конвертер T(), например convert<String>()
        template <typename T>
        T convert() {
            DBlock* b = block();
            return b ? b->convert<T>() : T();
        }

        // записать
        template <typename T>
        void operator=(const T& v) {
            _di->set(_id, v);
        }

        // удалить
        bool remove() {
            return _di->remove(_id);
        }

        // размер данных
        uint16_t size() const {
            DBlock* b = block();
            return b ? b->size() : 0;
        }

        // указатель на данные размера size()
        void* data() {
            DBlock* b = block();
            return b ? b->data() : nullptr;
        }

        // ================ Export ================

        bool toBool() const {
            DBlock* b = block();
#ifndef DICT_NO_CONVERT
            return b ? b->toNum<bool>() : 0;
#else
            return (b && b->is(DType::Bool)) ? b->rawBool() : 0;
#endif
        }

        int32_t toInt() const {
            DBlock* b = block();
#ifndef DICT_NO_CONVERT
            return b ? b->toNum<int32_t>() : 0;
#else
            return (b && b->same(DType::Int)) ? b->rawInt() : 0;
#endif
        }

        uint32_t toUint() const {
            DBlock* b = block();
#ifndef DICT_NO_CONVERT
            return b ? b->toNum<uint32_t>() : 0;
#else
            return (b && b->same(DType::Uint)) ? b->rawUint() : 0;
#endif
        }

        int64_t toInt64() const {
            DBlock* b = block();
#ifndef DICT_NO_CONVERT
            return b ? b->toNum<int64_t>() : 0;
#else
            return (b && b->same(DType::Int64)) ? b->rawInt64() : 0;
#endif
        }

        uint64_t toUint64() const {
            DBlock* b = block();
#ifndef DICT_NO_CONVERT
            return b ? b->toNum<uint64_t>() : 0;
#else
            return (b && b->same(DType::Uint64)) ? b->rawUint64() : 0;
#endif
        }

        float toFloat() const {
            DBlock* b = block();
#ifndef DICT_NO_CONVERT
            return b ? b->toNum<float>() : 0.0f;
#else
            return (b && b->is(DType::Float)) ? b->rawFloat() : 0.0f;
#endif
        }

        template <uint16_t N>
        bool toStr(char (&s)[N]) const {
            return toStr(s, N);
        }
        bool toStr(char* s, uint16_t cap) const {
            DBlock* b = block();
            if (!cap || !b || b->type() != DType::String) return false;

            uint16_t wrote = b->size();
            if (wrote > cap) wrote = cap;
            memcpy(s, b->rawStr(), wrote);
            s[wrote < cap ? wrote : cap - 1] = 0;
            return true;
        }

        template <typename T>
        T* toBin() {
            DBlock* b = block();
            return (b && b->type() == DType::Bin && b->size() == sizeof(T)) ? (T*)b->rawBin() : nullptr;
        }
        template <typename T>
        bool toBin(T& v) const {
            return toBin(&v, sizeof(T));
        }
        bool toBin(void* v, uint16_t size) const {
            DBlock* b = block();
            return (b && b->type() == DType::Bin && b->size() == size) ? (memcpy(v, b->rawBin(), size), true) : false;
        }

        // преобразовать указатель на данные напрямую к указанному типу
        template <typename T>
        T* as() {
            DBlock* b = block();
            return b ? (T*)b->data() : nullptr;
        }

        // ================ Str ================

        const char* c_str() const {
            DBlock* b = block();
            return (b && b->type() == DType::String) ? b->rawStr() : "";
        }
        operator const char*() const { return c_str(); }

        bool operator==(const char* v) const { return !strcmp(c_str(), v); }
        bool operator!=(const char* v) const { return strcmp(c_str(), v); }

        // ================ Compare ================

#define ACC_MAKE_OPERATOR(T, func)                              \
    operator T() const { return (T)func(); }                    \
    bool operator==(const T v) const { return (T)func() == v; } \
    bool operator!=(const T v) const { return (T)func() != v; } \
    bool operator>(const T v) const { return (T)func() > v; }   \
    bool operator<(const T v) const { return (T)func() < v; }   \
    bool operator>=(const T v) const { return (T)func() >= v; } \
    bool operator<=(const T v) const { return (T)func() <= v; }

        ACC_MAKE_OPERATOR(bool, toBool)
        ACC_MAKE_OPERATOR(signed char, toInt)
        ACC_MAKE_OPERATOR(char, toInt)
        ACC_MAKE_OPERATOR(short, toInt)
        ACC_MAKE_OPERATOR(int, toInt)
        ACC_MAKE_OPERATOR(long, toInt)
        ACC_MAKE_OPERATOR(long long, toInt64)
        ACC_MAKE_OPERATOR(unsigned char, toUint)
        ACC_MAKE_OPERATOR(unsigned short, toUint)
        ACC_MAKE_OPERATOR(unsigned int, toUint)
        ACC_MAKE_OPERATOR(unsigned long, toUint)
        ACC_MAKE_OPERATOR(unsigned long long, toUint64)
        ACC_MAKE_OPERATOR(float, toFloat)
        ACC_MAKE_OPERATOR(double, toFloat)

        // ================ Change ================

        // incr decr
        long operator++() {
            long val = long(*this) + 1;
            *this = val;
            return val;
        }

        long operator++(int) {  // post
            long val = long(*this);
            *this = long(val + 1);
            return val;
        }

        long operator--() {
            long val = long(*this) - 1;
            *this = val;
            return val;
        }

        long operator--(int) {  // post
            long val = long(*this);
            *this = long(val - 1);
            return val;
        }

        // compl
        template <typename T>
        T operator+=(T val) {
            val = T(*this) + val;
            *this = val;
            return val;
        }

        template <typename T>
        T operator-=(T val) {
            val = T(*this) - val;
            *this = val;
            return val;
        }

        template <typename T>
        T operator*=(T val) {
            val = T(*this) * val;
            *this = val;
            return val;
        }

        template <typename T>
        T operator/=(T val) {
            val = T(*this) / val;
            *this = val;
            return val;
        }

        template <typename T>
        T operator%=(T val) {
            val = T(*this) % val;
            *this = val;
            return val;
        }

        // bin
        template <typename T>
        T operator|=(T val) {
            val = T(*this) | val;
            *this = val;
            return val;
        }

        template <typename T>
        T operator&=(T val) {
            val = T(*this) & val;
            *this = val;
            return val;
        }

       private:
        Dict* _di;
        did_t _id;
    };

    // ================ Dict ================

    Dict() {
        _blocks.setOversize(1);
    }

    ~Dict() {
        _clear();
    }

    Dict(Dict&&) = delete;
    Dict(const Dict&) = delete;
    Dict& operator=(Dict&&) = delete;
    Dict& operator=(const Dict&) = delete;

    // ================ Access ================
    // by hash
    template <typename T>
    Access get(T id) { return Access(this, DI_MASK_ID(id)); }

    template <typename T>
    Access operator[](T id) { return get(id); }

    // by str
    Access get(const char* key) { return Access(this, DKEY_R(key)); }
    Access operator[](const char* key) { return get(key); }

    // ================ Utils ================

    // очистить
    void clear() {
        if (_blocks.length()) {
            _clear();
            _changed = true;
        }
    }

    // полностью освободить память
    void reset() {
        bool dirty = _blocks.length();
        _clear();
        _blocks.reset();
        if (dirty) _changed = true;
    }

    // вывести все ключи в массив длиной length()
    void getKeys(did_t* ids) const {
        for (size_t i = 0; i < _blocks.length(); i++) {
            ids[i] = _blocks[i].header.getID();
        }
    }

    // есть запись с таким ключом
    bool has(did_t id) {
        return _findPos(id);
    }
    bool has(const char* id) {
        return has(DKEY_R(id));
    }

    // количество ячеек
    uint16_t length() const {
        return _blocks.length();
    }

    // полный размер в памяти
    size_t size() {
        size_t sz = _blocks.size();
        for (size_t i = 0; i < length(); i++) {
            sz += _blocks[i].extSize();
        }
        return sz;
    }

#ifdef ARDUINO

    // вывести всё содержимое
    template <typename P>
    void dump(P& p) {
        p.print(F("Dict dump: "));
        p.print(_blocks.length());
        p.print(F(" entries ("));
        p.print(size());
        p.println(F(" bytes)"));
        for (size_t i = 0; i < _blocks.length(); i++) {
            if (i <= 9) p.write('0');
            p.print(i);
            p.print(". 0x");
            uint32_t id = _blocks[i].header.getID();
            for (int i = 7; i >= 0; i--) {
                p.print((id >> (i * 4)) & 0xF, HEX);
            }
            p.write(' '), p.write(' ');
            switch (_blocks[i].type()) {
                case DType::Bool: p.print(F("[Bool]  ")); break;
                case DType::Int: p.print(F("[Int32] ")); break;
                case DType::Uint: p.print(F("[Uint32]")); break;
                case DType::Int64: p.print(F("[Int64] ")); break;
                case DType::Uint64: p.print(F("[Uint64]")); break;
                case DType::Float: p.print(F("[Float] ")); break;
                case DType::String: p.print(F("[String]")); break;
                case DType::Bin: p.print(F("[Binary]")); break;
            }
            p.write(' '), p.write(' ');
            _blocks[i].printTo(p);
            p.println();
        }
    }

    // вывести в JSON объект
    String toJson(bool beauty = false) {
        uint16_t len = _blocks.length();
        String s('{');
        char ids[9];

        for (size_t i = 0; i < len; i++) {
            DBlock& b = _blocks[i];
            if (beauty) s += "\r\n  ";
            ultoa(b.header.getID(), ids, 16);
            s += '"';
            s += ids;
            s += '"';
            s += ':';
            if (beauty) s += ' ';
            b.toJson(s);
            if (i != len - 1u) s += ',';
        }

        if (beauty) s += "\r\n";
        s += '}';
        return s;
    }

    // вывести в JSON объект
    String toJson(const char** keys, size_t len, bool beauty = false) {
        String s('{');
        bool comma = false;

        for (size_t i = 0; i < len; i++) {
            DBlock* b = findBlock(DKEY_R(keys[i]));
            if (!b) continue;

            if (comma) s += ',';
            comma = true;
            if (beauty) s += "\r\n  ";
            s += '"';
            s += keys[i];
            s += '"';
            s += ':';
            if (beauty) s += ' ';
            b->toJson(s);
        }

        if (beauty) s += "\r\n";
        s += '}';
        return s;
    }

#endif

    // найти блок
    DBlock* findBlock(did_t id) {
        return _findPos(id).ptr;
    }

    // ================ Serialize ================

    // экспортный размер (для writeTo)
    size_t writeSize() const {
        size_t sz = 2;  // len
        for (size_t i = 0; i < length(); i++) {
            sz += sizeof(_blocks[i].header);
            if (_blocks[i].hasExtSize()) sz += 2;
            sz += _blocks[i].size();
        }
        return sz;
    }

    // экспортировать
    bool writeTo(DictIO& writer) {
        uint16_t len = _blocks.length();
        if (!writer.write(len)) return false;

        for (uint16_t i = 0; i < len; i++) {
            DBlock& b = _blocks[i];
            uint16_t size = b.size();

            if (!writer.write(b.header) ||
                (b.hasExtSize() && !writer.write(size)) ||
                !writer.write(b.data(), size)) return false;
        }

        return true;
    }

    // импортировать
    bool readFrom(DictIO& reader) {
        _clear();
        uint16_t len;
        if (!reader.read(len) ||
            !_blocks.reserve(len)) return false;

        for (uint16_t i = 0; i < len; i++) {
            DBlock b;
            uint16_t size = 0;
            if (!reader.read(b.header) ||
                (i && b.id() <= _blocks[i - 1].id()) ||
                (b.hasExtSize() && !reader.read(size)) ||
                !b.resize(size) ||
                !reader.read(b.data(), b.size())) {
                b.reset();
                _clear();
                return false;
            }
            _blocks.push(b);
        }
        for (size_t i = 0; i < _blocks.length(); i++) {
            callChange(_blocks[i].header.getID());
        }
        return true;
    }

    // удалить
    bool remove(const char* id) {
        return remove(DKEY_R(id));
    }
    bool remove(did_t id) {
        Bpos b = _findPos(id);
        if (b) {
            b.ptr->reset();
            _blocks.remove(b);
            _cache.reset();
            _changed = true;
        }
        return b;
    }

    // ================ Write ================
// numeric
#define DICT_MAKE_FN(T, fn)                                                 \
    bool set(did_t id, T v) { return fn(id, CMD::Set, v); }                 \
    bool init(did_t id, T v) { return fn(id, CMD::Init, v); }               \
    bool update(did_t id, T v) { return fn(id, CMD::Update, v); }           \
    bool set(const char* id, T v) { return fn(DKEY_R(id), CMD::Set, v); }   \
    bool init(const char* id, T v) { return fn(DKEY_R(id), CMD::Init, v); } \
    bool update(const char* id, T v) { return fn(DKEY_R(id), CMD::Update, v); }

    DICT_MAKE_FN(signed char, _i32)
    DICT_MAKE_FN(char, _i32)
    DICT_MAKE_FN(short, _i32)
    DICT_MAKE_FN(int, _i32)
    DICT_MAKE_FN(long, _i32)
    DICT_MAKE_FN(long long, _i64)

    DICT_MAKE_FN(unsigned char, _u32)
    DICT_MAKE_FN(unsigned short, _u32)
    DICT_MAKE_FN(unsigned int, _u32)
    DICT_MAKE_FN(unsigned long, _u32)
    DICT_MAKE_FN(unsigned long long, _u64)
    DICT_MAKE_FN(bool, _bool)
    DICT_MAKE_FN(float, _float)
    DICT_MAKE_FN(double, _float)

#undef DICT_MAKE_FN

    // str
    bool set(did_t id, const char* s, uint16_t len) { return _process(id, CMD::Set, (void*)s, len, DType::String); }
    bool set(did_t id, const char* s) { return s && set(id, s, strlen(s)); }
    bool init(did_t id, const char* s, uint16_t len) { return _process(id, CMD::Init, (void*)s, len, DType::String); }
    bool init(did_t id, const char* s) { return s && init(id, s, strlen(s)); }
    bool update(did_t id, const char* s, uint16_t len) { return _process(id, CMD::Update, (void*)s, len, DType::String); }
    bool update(did_t id, const char* s) { return s && update(id, s, strlen(s)); }

#ifdef ARDUINO
#define DICT_STRING_FN(fn)                                                       \
    bool fn(did_t id, const String& s) { return fn(id, s.c_str(), s.length()); } \
    bool fn(const char* id, const String& s) { return fn(DKEY_R(id), s); }

    DICT_STRING_FN(set)
    DICT_STRING_FN(init)
    DICT_STRING_FN(update)

#undef DICT_STRING_FN
#endif

    // bin
    template <typename T>
    bool set(did_t id, const T& v) { return set(id, &v, sizeof(T)); }
    bool set(did_t id, const void* v, uint16_t len) { return _process(id, CMD::Set, (void*)v, len, DType::Bin); }

    template <typename T>
    bool init(did_t id, const T& v) { return init(id, &v, sizeof(T)); }
    bool init(did_t id, const void* v, uint16_t len) { return _process(id, CMD::Init, (void*)v, len, DType::Bin); }

    template <typename T>
    bool update(did_t id, const T& v) { return update(id, &v, sizeof(T)); }
    bool update(did_t id, const void* v, uint16_t len) { return _process(id, CMD::Update, (void*)v, len, DType::Bin); }

// string id
#define DICT_STR_ID_FN(fn)                                                                  \
    bool fn(const char* id, const char* s) { return fn(DKEY_R(id), s); }                    \
    bool fn(const char* id, const char* s, uint16_t len) { return fn(DKEY_R(id), s, len); } \
    bool fn(const char* id, const void* data, uint16_t len) { return fn(DKEY_R(id), data, len); }

    DICT_STR_ID_FN(set)
    DICT_STR_ID_FN(init)
    DICT_STR_ID_FN(update)

#undef DICT_STR_ID_FN

    // типизированный enum id
    template <typename ID, typename T>
    typename DEnableIf<DIsEnum<ID>::value, bool>::type set(ID id, const T& v) { return set(did_t(id), v); }
    template <typename ID, typename T>
    typename DEnableIf<DIsEnum<ID>::value, bool>::type init(ID id, const T& v) { return init(did_t(id), v); }
    template <typename ID, typename T>
    typename DEnableIf<DIsEnum<ID>::value, bool>::type update(ID id, const T& v) { return update(did_t(id), v); }

    template <typename ID, typename T>
    typename DEnableIf<DIsEnum<ID>::value, bool>::type set(ID id, const T* v, uint16_t len) { return set(did_t(id), v, len); }
    template <typename ID, typename T>
    typename DEnableIf<DIsEnum<ID>::value, bool>::type init(ID id, const T* v, uint16_t len) { return init(did_t(id), v, len); }
    template <typename ID, typename T>
    typename DEnableIf<DIsEnum<ID>::value, bool>::type update(ID id, const T* v, uint16_t len) { return update(did_t(id), v, len); }

    // обработчик изменения значения
    void onChange(ChangeCallback cb) {
        _chng_cb = cb;
    }

    // вызвать обработчик onChange
    void callChange(did_t id) {
        if (_chng_cb) _chng_cb(DI_MASK_ID(id));
    }

    // вызвать обработчик onChange и обновление
    void change(did_t id) {
        _changed = true;
        callChange(id);
    }

    // ================ Private ================
   protected:
    bool changed() {
        return _changed ? (_changed = false, true) : false;
    }

   private:
    using Bpos = gtl::bsearch_t<DBlock>;

    gtl::stack<DBlock> _blocks;
    ChangeCallback _chng_cb = nullptr;
    bool _changed = false;

    struct Cache {
        did_t id = 0;
        int16_t idx = -1;

        inline void reset() {
            idx = -1;
        }
        inline bool is(did_t id) {
            return idx >= 0 && this->id == id;
        }
        inline bool valid() {
            return idx >= 0;
        }
    };
    Cache _cache;

    void _clear() {
        while (_blocks.length()) _blocks.pop().reset();
        _cache.reset();
    }

    bool _create(did_t id, void* data, uint16_t size, DType type) {
        DBlock block(id);
        _cache.reset();
        if (block.init(data, size, type) && _blocks.addSort(block)) return true;

        block.reset();
        return false;
    }

    Bpos _findPos(did_t id) {
        id = DI_MASK_ID(id);
        if (_cache.is(id)) return Bpos((_cache.idx), &_blocks[_cache.idx]);

        Bpos pos = _blocks.searchSort(DBlock(id));
        if (pos) _cache.id = id, _cache.idx = pos.idx;
        return pos;
    }

#ifndef DICT_NO_CONVERT
    template <typename T>
    bool _convert(did_t id, CMD cmd, T v, DType type) {
        DBlock* b = findBlock(id);
        if (b && cmd != CMD::Init && b->type() != type) switch (b->type()) {
                case DType::Bool: return _bool(id, cmd, v);
                case DType::Int: return _i32(id, cmd, v);
                case DType::Uint: return _u32(id, cmd, v);
#ifndef DICT_NO_INT64
                case DType::Int64: return _i64(id, cmd, v);
                case DType::Uint64: return _u64(id, cmd, v);
#endif
#ifndef DICT_NO_FLOAT
                case DType::Float: return _float(id, cmd, v);
#endif
                default: break;
            }
        return _processBlock(id, b, cmd, &v, sizeof(T), type);
    }
#endif

    bool _i32(did_t id, CMD cmd, long v) {
#ifndef DICT_NO_CONVERT
        return _convert(id, cmd, v, DType::Int);
#else
        return _process(id, cmd, &v, sizeof(v), DType::Int);
#endif
    }
    bool _u32(did_t id, CMD cmd, unsigned long v) {
#ifndef DICT_NO_CONVERT
        return _convert(id, cmd, v, DType::Uint);
#else
        return _process(id, cmd, &v, sizeof(v), DType::Uint);
#endif
    }
    bool _i64(did_t id, CMD cmd, long long v) {
#ifndef DICT_NO_CONVERT
        return _convert(id, cmd, v, DType::Int64);
#else
        return _process(id, cmd, &v, sizeof(v), DType::Int64);
#endif
    }
    bool _u64(did_t id, CMD cmd, unsigned long long v) {
#ifndef DICT_NO_CONVERT
        return _convert(id, cmd, v, DType::Uint64);
#else
        return _process(id, cmd, &v, sizeof(v), DType::Uint64);
#endif
    }
    bool _bool(did_t id, CMD cmd, bool v) {
#ifndef DICT_NO_CONVERT
        return _convert(id, cmd, v, DType::Bool);
#else
        return _process(id, cmd, &v, sizeof(v), DType::Bool);
#endif
    }
    bool _float(did_t id, CMD cmd, float v) {
#ifndef DICT_NO_CONVERT
        return _convert(id, cmd, v, DType::Float);
#else
        return _process(id, cmd, &v, sizeof(v), DType::Float);
#endif
    }

    bool _process(did_t id, CMD cmd, void* data, uint16_t size, DType type) {
        return _processBlock(id, findBlock(id), cmd, data, size, type);
    }
    bool _processBlock(did_t id, DBlock* b, CMD cmd, void* data, uint16_t size, DType type) {
        if (size && !data) return false;
#ifdef DICT_NO_INT64
        if (type == DType::Int64 || type == DType::Uint64) return false;
#endif
#ifdef DICT_NO_FLOAT
        if (type == DType::Float) return false;
#endif

        switch (cmd) {
            case CMD::Init:
                if (b) {
                    if (b->is(type)) {
                        return false;
                    } else {
                        // b->reset();
                        // if (!b->init(data, size, type)) return false;
                        DBlock next(id);
                        if (!next.init(data, size, type)) {
                            next.reset();
                            return false;
                        }
                        b->reset();
                        *b = next;
                    }
                } else {
                    if (!_create(id, data, size, type)) return false;
                }
                break;

            case CMD::Set:
                if (b) {
                    if (!b->same(type) || !b->update(data, size)) return false;
                } else {
                    if (!_create(id, data, size, type)) return false;
                }
                break;

            case CMD::Update:
                if (!b || !b->same(type) || !b->update(data, size)) return false;
                break;
        }

        change(id);
        return true;
    }
};
