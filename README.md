[![latest](https://img.shields.io/github/v/release/GyverLibs/Dict.svg?color=brightgreen)](https://github.com/GyverLibs/Dict/releases/latest/download/Dict.zip)
[![PIO](https://badges.registry.platformio.org/packages/gyverlibs/library/Dict.svg)](https://registry.platformio.org/libraries/gyverlibs/Dict)
[![Foo](https://img.shields.io/badge/Website-AlexGyver.ru-blue.svg?style=flat-square)](https://alexgyver.ru/)
[![Foo](https://img.shields.io/badge/%E2%82%BD%24%E2%82%AC%20%D0%9F%D0%BE%D0%B4%D0%B4%D0%B5%D1%80%D0%B6%D0%B0%D1%82%D1%8C-%D0%B0%D0%B2%D1%82%D0%BE%D1%80%D0%B0-orange.svg?style=flat-square)](https://alexgyver.ru/support_alex/)
[![Foo](https://img.shields.io/badge/README-ENGLISH-blueviolet.svg?style=flat-square)](https://github-com.translate.goog/GyverLibs/Dict?_x_tr_sl=ru&_x_tr_tl=en)  

[![Foo](https://img.shields.io/badge/ПОДПИСАТЬСЯ-НА%20ОБНОВЛЕНИЯ-brightgreen.svg?style=social&logo=telegram&color=blue)](https://t.me/GyverLibs)

# Dict
Компактный "словарь" для Arduino

- Это более лёгкий и быстрый аналог GyverDB
- Поддерживает хранение чисел, строк и бинарных данных по хэш-ключам
- Механизм автоматического сохранения в файл и EEPROM

### Совместимость
Arduino-совместимые платформы

### Зависимости
- [GTL](https://github.com/GyverLibs/GTL)
- [FOR_MACRO](https://github.com/GyverLibs/FOR_MACRO)

## Содержание
- [API](#api)
- [Особенности и сценарии работы](#usage)
- [Установка](#install)
- [Баги и обратная связь](#feedback)

<a id="api"></a>

## API
### Настройки
Определяются до подключения `Dict.h`

```cpp
// отключить поддержку float чисел
#define DICT_NO_FLOAT

// отключить поддержку 64-бит чисел
#define DICT_NO_INT64

// отключить конвертацию численных типов и bool
#define DICT_NO_CONVERT
```

### Dict
```cpp
// access by hash
Access get(T id);
Access operator[](T id);

// access by str
Access get(char* key);
Access operator[](char* key);

// очистить
void clear();

// полностью освободить память
void reset();

// вывести все ключи в массив длиной length()
void getKeys(did_t* ids);

// есть запись с таким ключом
bool has(did_t id);
bool has(const char* id);

// количество ячеек
uint16_t length();

// полный размер в памяти
size_t size();

// вывести всё содержимое
void dump(Print& p);

// вывести в JSON объект
String toJson(bool beauty = false);

// вывести в JSON объект
String toJson(char** keys, size_t len, bool beauty = false);

// найти блок
DBlock* findBlock(did_t id);

// экспортный размер (для writeTo)
size_t writeSize();

// экспортировать
bool writeTo(DictIO& writer);

// импортировать
bool readFrom(DictIO& reader);

// удалить
bool remove(did_t id);
bool remove(const char* id);

// str
bool set(did_t id, char* s, uint16_t len);
bool set(did_t id, char* s);
bool init(did_t id, char* s, uint16_t len);
bool init(did_t id, char* s);
bool update(did_t id, char* s, uint16_t len);
bool update(did_t id, char* s);

// Arduino String
bool set(did_t id, const String& s);
bool init(did_t id, const String& s);
bool update(did_t id, const String& s);

// bin
bool set(did_t id, T& v);
bool set(did_t id, void* v, uint16_t len);
bool init(did_t id, T& v);
bool init(did_t id, void* v, uint16_t len);
bool update(did_t id, T& v);
bool update(did_t id, void* v, uint16_t len);

// обработчик изменения значения
void onChange(ChangeCallback cb);

// вызвать обработчик onChange
void callChange(did_t id);

// вызвать обработчик onChange и обновление
void change(did_t id);
```

Здесь `T` обозначает отдельные перегрузки для `bool`, `char`, `signed char`, `short`, `int`, `long`, `long long`, их беззнаковых вариантов, `float` и `double`.

```cpp
// numeric
bool set(did_t id, T v);
bool init(did_t id, T v);
bool update(did_t id, T v);
```

### Dict::Access
Прокси для `dict[id]` и `dict["key"]`. Числовые преобразования и сравнения ниже обозначены через `T` для тех же отдельных числовых перегрузок, что и в `Dict`.

```cpp
// получить id блока
did_t id();

// получить блок
DBlock* block();
operator DBlock*();

// валидный блок
bool valid();

// напечатать
void printTo(P& p);
void printlnTo(P& p);

// вывести в тип-конвертер T(), например convert<String>()
T convert();

// преобразовать указатель на данные напрямую к указанному типу
T* as();

// записать
void operator=(T& v);

// удалить
bool remove();

// размер данных
uint16_t size();

// указатель на данные размера size()
void* data();

// convert
bool toBool();
int32_t toInt();
uint32_t toUint();
int64_t toInt64();
uint64_t toUint64();
float toFloat();
bool toStr(char (&s)[N]);
bool toStr(char* s, uint16_t cap);
T* toBin();
bool toBin(T& v);
bool toBin(void* v, uint16_t size);
char* c_str();
operator char*();

// operators
bool operator==(char* v);
bool operator!=(char* v);

long operator++();
long operator++(int);
long operator--();
long operator--(int);

bool operator==(T v);
bool operator!=(T v);
bool operator>(T v);
bool operator<(T v);
bool operator>=(T v);
bool operator<=(T v);

T operator+=(T val);
T operator-=(T val);
T operator*=(T val);
T operator/=(T val);
T operator%=(T val);
T operator|=(T val);
T operator&=(T val);
```

### DictEE
Наследует `DictAuto` и API словаря. Требует `EEPROM.h`.

```cpp
DictEE(uint16_t addr = 0, uint16_t timeoutMs = 10000);

// вызывать в loop(). Запишет данные по тайм-ауту
bool tick();

// обновить сейчас
bool update();

// прочитать бд из EEPROM
bool begin();
```

### DictFile
Наследует `DictAuto` и API словаря. Доступен только на ESP8266 и ESP32.

```cpp
DictFile(fs::FS* fs, char* path, uint16_t timeoutMs = 10000);

// вызывать в loop(). Запишет данные по тайм-ауту
bool tick();

// обновить сейчас
bool update();

// прочитать бд из файла
bool begin();
```

<a id="usage"></a>

## Особенности и сценарии работы
Dict хранит данные в формате ключ:значение. По ключу можно записать данные в ячейку и прочитать их из неё:

- Ключ - 29 бит число, по сути БД это массив на 2^29 ячеек
- Значение - данные любого типа: числа, строки, любые бинарные данные

```cpp
d[0] = 123;
d[2] = 3.14;
d[100] = "hello";
```

Пока в ячейку ничего не записано - она не существует и не занимает память. К ключу следует относиться как к **уникальному идентификатору ячейки**, а не как к порядковому номеру в массиве или индексу.

### Ключи
Для повышения читаемости кода вместо номеров ячеек удобнее использовать константы, например `enum`. Это очень удобно, потому что IDE подскажет список имеющихся ключей при вводе `keys::`, а значения подставит компилятор:

```cpp
enum keys : size_t {
    key1,
    key2,
    mykey,
    lolkek,
};

d[keys::key1] = 123;
d[keys::key2] = 3.14;
d[keys::mykey] = "hello";
```

При активной разработке и хранении БД в энергонезависимой памяти (в файле, чтение при загрузке МК) данный подход неудобен, т.к. удаление или добавление ключа в середине списка приведёт к смещению нумерации и под старыми ключами окажутся новые данные. Для сохранения читаемости и уникальности каждого ключа можно использовать хэш-строки - строка при помощи специальной функции преобразуется в число, которое соответствует только этой строке. Данная возможность встроена в Dict - можно обращаться к ячейкам по строковому ключу, библиотека сама посчитает хэш и найдёт ячейку:

```cpp
d["key1"] = 123;
```

Но строка займёт место в памяти, а рантайм хэш замедлит работу. Для ускорения и облегчения можно использовать внешнюю хэш-функцию вида `hash = hash + (hash << 5) + *str++`. С библиотекой идёт функция `DKEY`, которая выполняется на этапе компиляции и сразу превращается в число:

```cpp
d[DKEY("key1")] = 123;
```

В этом случае enum тоже можно использовать для подсказок IDE, но чуть в другом виде:

```cpp
// enum с хэшами
enum keys : size_t {
    key1 = DKEY("key1"),
    key2 = DKEY("key2"),
    mykey = DKEY("mykey"),
};

d[keys::key1] = 123;
d[keys::key2] = 3.14;
d[keys::mykey] = "hello";
```

Теперь enum хранит хэши и не боится удаления или добавления ключей - они уникальны. Для более короткой записи в библиотеке есть удобный макрос:

```cpp
DI_KEYS(keys,
    key1,
    key2,
    mykey   // последняя запятая не ставится
);
```

Он развернётся в такой же хэш-enum как в примере выше. Рекомендуется использовать этот вариант как самый удобный и оптимальный, а также он автоматически проверяет коллизии хэшей (будет ошибка компиляции) - они крайне маловероятны, но с длинными ключами риск повышается.

> Есть ещё `DI_KEYS_CLASS` - он создаёт `enum class` с более защищёнными именами констант

### Значения
Есть 3 варианта записи в ячейку:

- `init(ключ, значение)` - создать ячейку с указанным значением, если ячейки с таким ключом нет или она имеет другой тип данных. Удобно для задания начальных значений в DictFile
- `update(ключ, значение)` - обновить данные, если ячейка с таким ключом существует
- `set(ключ, значение)` - записать данные, создав ячейку если она не существует
  - Аналог `d[ключ] = значение`

Для инициализации можно использовать более короткий макрос:

```cpp
DI_INIT(
    d,
    (kk::ival, 1234),
    (kk::fval, 456.789),
    ("key3", 123321ul),
    ("key4", "abc")
);
```

### Запись и чтение
```cpp
// запись черз set
// тип ячейки выбирается по переданным данным
db.set(TypedKeys::value, 77);   // int
db.set(TypedKeys::value, 77u);  // uint

// запись через знак равно
db[Keys::counter] = 123L;           // long
db[Keys::ratio] = 3.14f;            // float
db[Keys::enabled] = true;           // bool
db[Keys::name] = "Dict!";           // string
db["runtime_key"] = uint32_t(456);  // uint
```

> Конвертации на запись между строковыми и численными типами нет, но есть между численными (int->float и наоборот)

```cpp
// прокси поддерживает сравнения и арифметические операции с числовыми ячейками
if (db[Keys::counter] >= 300) Serial.println(F("counter >= 300"));
db[Keys::counter]++;
db[Keys::counter] += 10;
db[Keys::counter] &= 0x1ff;
```

```cpp
// числовые значения конвертируются при чтении
int32_t counter = db[Keys::counter];        // автоматически по левому типу
float ratio = db[Keys::ratio].toFloat();    // явно
bool enabled = (bool)db[Keys::enabled];     // явно

// строку можно получить указателем или скопировать в свой буфер
const char* name = db[Keys::name].c_str();

char nameCopy[16];
bool nameCopied = db[Keys::name].toStr(nameCopy);
```

```cpp
// можно сравнивать, в том числе строки
db[Keys::name] == "Dict!";
db[Keys::ratio] == 12.34;
db[Keys::counter] >= 500;
```

> Конвертации на чтение между строковыми и численными типами нет, но есть между численными (int->float и наоборот)

```cpp
// массивы и структуры сохраняются как бинарные данные
const uint8_t sourceBytes[] = {1, 2, 3, 4, 5};
db[Keys::bytes] = sourceBytes;

Config sourceConfig{1000, 2};
db[Keys::config] = sourceConfig;

// вывод в свой экземпляр
// безопасный, проверяет размер типа
uint8_t copiedBytes[sizeof(sourceBytes)]{};
bool bytesCopied = db[Keys::bytes].toBin(copiedBytes);

Config copiedConfig{};
bool configCopied = db[Keys::config].toBin(copiedConfig);

// доступ к данным в памяти Dict
// здесь указатель не выровнен! на некоторых платформах может ломаться
Config* config = db[Keys::config].toBin<Config>();

// более сырой доступ к указателю
// data() и as() не проверяют тип и размер
void* rawData = db[Keys::config].data();

Config* rawConfig = db[Keys::config].as<Config>();
if (rawConfig) Serial.println(rawConfig->period);
```

```cpp
// содержимое можно вывести целиком
db.dump(Serial);

// или собрать в JSON
Serial.println(db.toJson(true));

// собрать в JSON только с переданными текстовыми ключами
const char* jsonKeys[] = {"counter", "ratio", "enabled", "name"};
Serial.println(db.toJson(jsonKeys, 4, true));
```

### Уведомления
```cpp
db.onChange([](did_t id) {
    Serial.print(id, HEX);
    Serial.println();
});

db["name"] = "updated";
```

Обработчик принимает числовой ID. Прямая правка через указатель на бинарный блок не вызывает уведомление: после неё вызовите `db.change(id)`. `callChange(id)` вызывает только обработчик. `remove()`, `clear()` и `reset()` отмечают изменение для автосохранения, но не вызывают обработчик.

### Автоматическое сохранение
```cpp
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
```

Пример предназначен для ESP8266/ESP32. `begin()` читает файл, а при его отсутствии создаёт пустой. `tick()` переносит запись на заданный интервал после последнего замеченного изменения, а `update()` сохраняет немедленно. При ошибке сохранения `tick()` повторит попытку после таймаута. Деструкторы `DictFile` и `DictEE` также вызывают сохранение, поэтому хранилище должно оставаться доступным до уничтожения объекта.

Для EEPROM подключите `DictEE.h` и создайте `DictEE db(0, 10000)`. На ESP перед `db.begin()` самостоятельно вызовите `EEPROM.begin(capacity)` с достаточным размером - при сохранении библиотека вызывает `EEPROM.commit()`. Начальный адрес и доступную вместимость EEPROM выбирает приложение.

<a id="install"></a>

## Установка
- Библиотеку можно найти по названию **Dict** и установить через менеджер библиотек в:
    - Arduino IDE
    - Arduino IDE v2
    - PlatformIO
- [Скачать библиотеку](https://github.com/GyverLibs/Dict/archive/refs/heads/main.zip) .zip архивом для ручной установки:
    - Распаковать и положить в *C:\Program Files (x86)\Arduino\libraries* (Windows x64)
    - Распаковать и положить в *C:\Program Files\Arduino\libraries* (Windows x32)
    - Распаковать и положить в *Документы/Arduino/libraries/*
    - (Arduino IDE) автоматическая установка из .zip: *Скетч/Подключить библиотеку/Добавить .ZIP библиотеку…* и указать скачанный архив
- Читай более подробную инструкцию по установке библиотек [здесь](https://alexgyver.ru/arduino-first/#%D0%A3%D1%81%D1%82%D0%B0%D0%BD%D0%BE%D0%B2%D0%BA%D0%B0_%D0%B1%D0%B8%D0%B1%D0%BB%D0%B8%D0%BE%D1%82%D0%B5%D0%BA)
### Обновление
- Рекомендую всегда обновлять библиотеку: в новых версиях исправляются ошибки и баги, а также проводится оптимизация и добавляются новые фичи
- Через менеджер библиотек IDE: найти библиотеку как при установке и нажать "Обновить"
- Вручную: **удалить папку со старой версией**, а затем положить на её место новую. "Замену" делать нельзя: иногда в новых версиях удаляются файлы, которые останутся при замене и могут привести к ошибкам!

<a id="feedback"></a>

## Баги и обратная связь
При нахождении багов создавайте **Issue**, а лучше сразу пишите на почту [alex@alexgyver.ru](mailto:alex@alexgyver.ru)  
Библиотека открыта для доработки и ваших **Pull Request**'ов!

При сообщении о багах или некорректной работе библиотеки нужно обязательно указывать:
- Версия библиотеки
- Какой используется МК
- Версия SDK (для ESP)
- Версия Arduino IDE
- Корректно ли работают ли встроенные примеры, в которых используются функции и конструкции, приводящие к багу в вашем коде
- Какой код загружался, какая работа от него ожидалась и как он работает в реальности
- В идеале приложить минимальный код, в котором наблюдается баг. Не полотно из тысячи строк, а минимальный код
