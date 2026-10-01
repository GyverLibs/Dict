#include <Arduino.h>

// #define DICT_NO_INT64
// #define DICT_NO_FLOAT
// #define DICT_NO_CONVERT
#include <Dict.h>

DI_KEYS(
    kk,
    ival,
    fval);

void setup() {
    Serial.begin(115200);

    Dict d;
    d["int"] = -123;
    d["float"] = 3.14;
    d["bool"] = true;
    d["str"] = "hello";

    DI_INIT(
        d,
        (kk::ival, 1234),
        (kk::fval, 456.789),
        ("key3", 123321ul),
        ("key4", "abc"));

    d.dump(Serial);
    Serial.println(d.toJson(1));

    const char* keys[] = {"int", "float", "bool", "str"};
    Serial.println(d.toJson(keys, 4, true));

    Serial.println(d["int"].toInt());
    Serial.println(d["int"].toUint());
    Serial.println(d["bool"].toBool());
}

void loop() {}
