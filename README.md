# Модуль 2.1: Blink на ESP32-S3

Домашнє завдання курсу Embedded Basic.
Плата ESP32-S3-WROOM-1-N16R8, PlatformIO, Arduino.

## Схема

```
GPIO18 -> 220 Ом -> LED -> GND
GPIO17 -> кнопка -> GND   (INPUT_PULLUP)
```

## Що робить

- LED блимає раз на 500 мс.
- Кнопка перемикає режими: блимання -> увімк -> вимк.
- Раз на 1000 обертів `loop()` друкує середній і максимальний час одного оберту.

## Де що в коді

| Пункт завдання | Де |
|---|---|
| 1. `enum class LedState`, клас `Led` з `init()` і `set()`, `millis()` замість `delay()` | `class Led`, `loop()` |
| 2. Налаштування в `struct Config` зі `static constexpr` | `struct Config` |
| 3. Час одного оберту `loop()` | `sumUs`, `maxUs` |
| 4. Кнопка на перериванні, `volatile bool`, антидребезг у `loop()` | `onButton()`, `loop()` |

Без `new`, `malloc` і `String`. ISR тільки ставить прапорець. У `loop()` прапорець забирається з вимкненими перериваннями, бо `volatile` не робить «прочитати і скинути» однією дією.

## Запуск

```
pio run -t upload -t monitor
```

## Результат

```
loop: avg 2 us, max 25 us
loop: avg 2 us, max 25 us
loop: avg 2 us, max 25 us
mode: on
loop: avg 2 us, max 33 us
loop: avg 2 us, max 24 us
loop: avg 2 us, max 24 us
loop: avg 2 us, max 24 us
loop: avg 2 us, max 25 us
loop: avg 2 us, max 24 us
loop: avg 2 us, max 24 us
```

Звичайний оберт `loop()` займає близько 2 мкс. Максимум 24-25 мкс припадає на оберт, де був `Serial.printf`, тобто один друк коштує приблизно 22 мкс. Там, де натиснута кнопка, друк подвійний, звідси 33 мкс. У середньому цього не видно, тому поруч друкую максимум.

## Розмір

RAM 19 468 байт (5,9 %), Flash 266 753 байт (4,1 %).
