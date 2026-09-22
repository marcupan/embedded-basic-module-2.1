# Embedded C++ on ESP32 — Module 2.1

Course assignment: a blink program written in embedded C++ style, with an
interrupt-driven button and loop timing measurement.

Board: **ESP32-S3-WROOM-1-N16R8** (YD-ESP32-S3) · PlatformIO · Arduino framework.

## Wiring

```
GPIO18 ── 220 Ω ── LED ── GND
GPIO17 ── button ── GND        (internal pull-up, INPUT_PULLUP)
```

## What it does

- The LED blinks every 500 ms.
- The button cycles the mode: **Blink → On → Off → Blink**, printing the new mode.
- Every 1000 iterations the average and the worst-case duration of one `loop()`
  pass are printed over serial. The figure is the period between two `loop()`
  entries, so it includes the serial print and the framework overhead between
  calls. The average alone spreads one slow pass over a thousand fast ones and
  hides it, which is why the maximum is reported next to it — on a
  microcontroller the worst case is the number that matters.

## Requirements covered

| # | Requirement | Where |
|:-:|---|---|
| 1 | `enum class LedState`, `constexpr` pin and interval, no `delay()`, timing via `millis()`, `Led` class with `init()` and `set()` | `class Led`, `loop()` |
| 2 | Configuration struct with `static constexpr` fields, no magic numbers | `struct Config` |
| 3 | Loop iteration time, averaged every 1000 iterations | `sumUs` / `iterations` |
| 4 | Button on interrupt, `volatile bool` flag, minimal ISR, debounce in `loop()` | `onButton()`, `loop()` |

## Design notes

- **No dynamic allocation.** No `new`, `malloc`, `String` or STL containers.
  On a microcontroller the heap fragments a few kilobytes of RAM and makes
  allocation time unpredictable; all memory here is laid out at compile time.
- **No `delay()`.** Timing is `now - lastToggle >= BLINK_MS`. Unsigned
  subtraction stays correct across the `millis()` overflow.
- **`constexpr` instead of `#define`.** The value is known at compile time, has
  a type and is visible in a debugger; it ends up as an immediate operand and
  costs no RAM.
- **Explicit initialization.** All peripheral setup lives in `setup()`, in
  order: serial, output driven to a safe state, input, and `attachInterrupt()`
  last so the ISR cannot fire before the hardware is configured. No lazy
  initialization.
- **Minimal ISR.** The handler only sets `buttonFlag = true` — no serial, no
  delays, no arithmetic. `volatile` stops the compiler from caching the flag in
  a register.
- **Critical section.** `volatile` does not make read-then-clear atomic, so that
  pair is wrapped in `noInterrupts()` / `interrupts()`.
- **Debounce in `loop()`.** Contact bounce fires the ISR several times, but the
  flag simply stays set; the `now - lastPress >= DEBOUNCE_MS` check accepts only
  the first press within 50 ms.

## Build and run

```
pio run -e esp32-s3 -t upload -t monitor
```

Serial runs over the native USB port (`ARDUINO_USB_CDC_ON_BOOT=1`), at 115200
baud.

## Serial output

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

Measured on the board: a bare pass takes about 2 us, while the worst pass in
each window is 24-25 us — that is the one that ran `Serial.printf`. A single
serial print therefore costs roughly 22 us, about ten ordinary passes. The
window containing a button event peaks at 33 us, because that pass printed
twice. Over 30 seconds the program emitted 12 463 lines, i.e. about 415 000
`loop()` passes per second. None of this is visible in the average, which stays
at 2 us throughout.

## Footprint

```
RAM:   19 468 bytes (5.9 %)
Flash: 266 753 bytes (4.1 %)
```
