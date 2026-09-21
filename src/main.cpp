// Модуль 2.1 — Blink на ESP32-S3 у стилі embedded C++
// GPIO18 -> 220 Ом -> LED -> GND
// GPIO17 -> кнопка -> GND (внутрішня підтяжка)
#include <Arduino.h>

struct Config {
    static constexpr uint8_t LED_PIN = 18;
    static constexpr uint8_t BTN_PIN = 17;
    static constexpr uint32_t BLINK_MS = 500;
    static constexpr uint32_t DEBOUNCE_MS = 50;
    static constexpr uint32_t STATS_EVERY = 1000;
    static constexpr uint32_t BAUD = 115200;
};

enum class LedState : uint8_t { Off, On };
enum class Mode : uint8_t { Blink, On, Off };

class Led {
public:
    constexpr Led(uint8_t pin) : pin_(pin) {}

    void init() {
        pinMode(pin_, OUTPUT);
        digitalWrite(pin_, LOW);
        state_ = LedState::Off;
    }

    void set(LedState s) {
        if (s == state_)
            return;

        state_ = s;
        digitalWrite(pin_, s == LedState::On ? HIGH : LOW);
    }

    void toggle() { set(state_ == LedState::On ? LedState::Off : LedState::On); }

private:
    const uint8_t pin_;
    LedState state_ = LedState::Off;
};

Led led(Config::LED_PIN);

// Прапорець від переривання: пишеться в ISR, читається в loop().
volatile bool buttonFlag = false;

void IRAM_ATTR onButton() { buttonFlag = true; }

Mode nextMode(Mode m) {
    switch (m) {
        case Mode::Blink: return Mode::On;
        case Mode::On: return Mode::Off;
        case Mode::Off: return Mode::Blink;
    }

    return Mode::Blink;
}

void setup() {
    Serial.begin(Config::BAUD);

    led.init();
    pinMode(Config::BTN_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(Config::BTN_PIN), onButton, FALLING);
}

void loop() {
    static Mode mode = Mode::Blink;
    static uint32_t lastToggle = 0;
    static uint32_t lastPress = 0;
    static uint32_t iterations = 0;
    static uint32_t sumUs = 0;

    const uint32_t startUs = micros();
    const uint32_t now = millis();

    // Забираємо прапорець так, щоб переривання не влізло між читанням і скиданням.
    bool pressed = false;
    noInterrupts();
    if (buttonFlag) {
        buttonFlag = false;
        pressed = true;
    }
    interrupts();

    // Антидребезг у loop(), а не в ISR.
    if (pressed && now - lastPress >= Config::DEBOUNCE_MS) {
        lastPress = now;
        mode = nextMode(mode);
    }

    switch (mode) {
        case Mode::Blink:
            if (now - lastToggle >= Config::BLINK_MS) {
                lastToggle = now;
                led.toggle();
            }
            break;

        case Mode::On:
            led.set(LedState::On);
            break;

        case Mode::Off:
            led.set(LedState::Off);
            break;
    }

    sumUs += micros() - startUs;
    iterations++;

    if (iterations >= Config::STATS_EVERY) {
        Serial.printf("loop: %lu us\n", (unsigned long)(sumUs / iterations));
        iterations = 0;
        sumUs = 0;
    }
}
