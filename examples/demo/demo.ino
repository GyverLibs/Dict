#include <Arduino.h>

// облегчение-отключение фич
// #define DICT_NO_INT64
// #define DICT_NO_FLOAT
// #define DICT_NO_CONVERT
#include <Dict.h>

// база данных ключей
DI_KEYS(
    Keys,
    counter,
    ratio,
    enabled,
    name,
    bytes,
    config);

// база данных ключей enum class
DI_KEYS_CLASS(
    TypedKeys,
    value);

// структура для примера
struct Config {
    uint16_t period;
    uint8_t mode;
};

// обработчик изменения ячейки
void onDictChange(did_t id) {
    Serial.print(F("changed 0x"));
    Serial.println(id, HEX);
}

void setup() {
    Serial.begin(115200);

    // обычная БД в оперативной памяти
    Dict db;

    // обработчик изменения ячейки
    db.onChange(onDictChange);

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

    // init создаёт значение только при отсутствии ключа или отличии типа
    // если ячейка уже есть - данные не будут обновлены
    bool timeoutCreated = db.init("timeout", uint32_t(1000));
    bool timeoutKept = !db.init("timeout", uint32_t(5000));

    // update обновляет только существующую ячейку совместимого типа
    bool timeoutUpdated = db.update("timeout", uint32_t(1500));
    bool missingRejected = !db.update(DKEY("missing"), 1);

    // set сохраняет исходный числовой тип ячейки и конвертирует новое число в него
    db.set(Keys::counter, 321.9f);

    // строка не парсится в число и несовместимая запись будет отклонена
    bool wrongTypeRejected = !db.set(Keys::counter, "123456");

    // init позволяет явно заменить тип существующей ячейки
    db.init("flexible", 123);
    db.init("flexible", "now string");

    // числовые значения конвертируются при чтении
    int32_t counter = db[Keys::counter];
    float ratio = db[Keys::ratio].toFloat();
    bool enabled = db[Keys::enabled].toBool();

    // строку можно получить указателем или скопировать в свой буфер
    const char* name = db[Keys::name].c_str();
    char nameCopy[16];
    bool nameCopied = db[Keys::name].toStr(nameCopy);
    String nameString = db[Keys::name].convert<String>();

    Serial.print(F("counter: "));
    Serial.println(counter);
    Serial.print(F("ratio: "));
    Serial.println(ratio);
    Serial.print(F("enabled: "));
    Serial.println(enabled);
    Serial.print(F("name: "));
    Serial.println(name);
    Serial.print(F("name copy: "));
    Serial.println(nameCopied ? nameCopy : "error");
    Serial.print(F("name String: "));
    Serial.println(nameString);

    Serial.print(F("init/update checks: "));
    Serial.println(timeoutCreated && timeoutKept && timeoutUpdated && missingRejected && wrongTypeRejected ? F("ok") : F("error"));

    // прокси поддерживает сравнения и арифметические операции
    if (db[Keys::counter] >= 300) Serial.println(F("counter >= 300"));
    db[Keys::counter]++;
    db[Keys::counter] += 10;
    db[Keys::counter] &= 0x1ff;

    // массивы и структуры сохраняются как бинарные данные
    const uint8_t sourceBytes[] = {1, 2, 3, 4, 5};
    db[Keys::bytes] = sourceBytes;

    // экспорт через toBin
    uint8_t copiedBytes[sizeof(sourceBytes)]{};
    bool bytesCopied = db[Keys::bytes].toBin(copiedBytes);

    // хранение структуры
    Config sourceConfig{1000, 2};
    db[Keys::config] = sourceConfig;

    // безопасное копирование - проверяет тип и точный размер
    Config copiedConfig{};
    bool configCopied = db[Keys::config].toBin(copiedConfig);

    // указатель позволяет менять данные без копирования
    // но здесь указатель не выровнен! на некоторых платформах может ломаться
    Config* config = db[Keys::config].toBin<Config>();
    if (config) {
        config->mode = 3;
        db.change(Keys::config);
    }

    Serial.print(F("binary checks: "));
    Serial.println(bytesCopied && configCopied ? F("ok") : F("error"));

    // более сырой доступ к указателю
    // data и as не проверяют тип и размер
    void* rawData = db[Keys::config].data();
    Config* rawConfig = db[Keys::config].as<Config>();
    if (rawData && rawConfig) Serial.println(rawConfig->period);

    // содержимое можно вывести целиком
    db.dump(Serial);

    // или собрать в JSON
    Serial.println(db.toJson(true));

    // собрать в JSON только с переданными текстовыми ключами
    const char* jsonKeys[] = {"counter", "ratio", "enabled", "name"};
    Serial.println(db.toJson(jsonKeys, 4, true));

    // словарь можно сериализовать в память и восстановить
    uint8_t storage[256];
    if (db.writeSize() <= sizeof(storage)) {
        DiBufIO writer(storage, sizeof(storage));
        if (db.writeTo(writer)) {
            Dict restored;
            DiBufIO reader(storage, sizeof(storage));
            if (restored.readFrom(reader)) {
                Serial.print(F("restored entries: "));
                Serial.println(restored.length());
            }
        }
    }

    // удалить можно через словарь или прокси
    db.remove("timeoutID");
    db["runtime_key"].remove();

    Serial.print(F("entries: "));
    Serial.println(db.length());
    Serial.print(F("memory: "));
    Serial.println(db.size());
}

void loop() {}
