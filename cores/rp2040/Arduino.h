/*
    Arduino header for the Raspberry Pi Pico RP2040

    Copyright (c) 2021 Earle F. Philhower, III <earlephilhower@yahoo.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "stdlib_noniso.h" // Wacky deprecated AVR compatibility functions
#include "RP2040Version.h"
#include "api/ArduinoAPI.h"
#include "api/itoa.h" // ARM toolchain doesn't provide itoa etc, provide them
#include <pico.h>
#undef PICO_RP2350A // Set in the RP2350 SDK boards file, overridden in the variant pins_arduino.h
#include <pins_arduino.h>
#include <hardware/gpio.h> // Required for the port*Register macros
#include "debug_internal.h"

// Chip sanity checking.  SDK uses interesting way of separating 2350A from 2350B, see https://github.com/raspberrypi/pico-sdk/issues/2364
#if (!defined(PICO_RP2040) && !defined(PICO_RP2350)) || defined(PICO_RP2040) && defined(PICO_RP2350)
#error Invalid core definition. Either PICO_RP2040 or PICO_RP2350 must be defined.
#endif
#if defined(PICO_RP2350) && !defined(PICO_RP2350A)
#error Invalid RP2350 definition.  Need to set PICO_RP2350A=0/1 for A/B variant
#endif
#if defined(PICO_RP2350B)
#error Do not define PICO_RP2350B.  Use PICO_RP2350A=0 to indicate RP2350B.  See the SDK for more details
#endif

// Try and make the best of the old Arduino abs() macro.  When in C++, use
// the sane std::abs() call, but for C code use their macro since stdlib abs()
// is int but their macro "works" for everything (with potential side effects)
#ifdef abs
#undef abs
#endif // abs
#if defined(__cplusplus) && !defined(__riscv)
using std::abs;
using std::round;
#else
#define abs(x) ({ __typeof__(x) _x = (x); _x >= 0 ? _x : -_x; })
#define round(x) ({ __typeof__(x) _x = (x); _x >= 0 ? (long)(_x + 0.5) : (long)(_x - 0.5); })
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// For compatibility to many platforms and libraries
#define clockCyclesPerMicrosecond() ( F_CPU / 1000000L )
#define clockCyclesToMicroseconds(a) ( (a) / clockCyclesPerMicrosecond() )
#define microsecondsToClockCycles(a) ( (a) * clockCyclesPerMicrosecond() )

// Disable/re-enable all interrupts.  Safely handles nested disables
void interrupts();
void noInterrupts();

// Only implemented on some RP2350 boards, not the OG Pico 2
#ifdef RP2350_PSRAM_CS
void *pmalloc(size_t size);
void *pcalloc(size_t count, size_t size);
#else
[[deprecated("This chip does not have PSRAM, pmalloc will always fail")]] void *pmalloc(size_t size);
[[deprecated("This chip does not have PSRAM, pcalloc will always fail")]] void *pcalloc(size_t count, size_t size);
#endif

// AVR compatibility macros...naughty and accesses the HW directly
#define digitalPinToPort(pin)       (0)
#define digitalPinToBitMask(pin)    (1UL << (pin))
#define digitalPinToTimer(pin)      (0)
#define digitalPinToInterrupt(pin)  (pin)
#define NOT_AN_INTERRUPT            (-1)
#define portOutputRegister(port)    ((volatile uint32_t *)&(sio_hw->gpio_out))
#define portInputRegister(port)     ((volatile uint32_t *)&(sio_hw->gpio_in))
#define portModeRegister(port)      ((volatile uint32_t *)&(sio_hw->gpio_oe))
#define digitalWriteFast(pin, val)  (val ? sio_hw->gpio_set = (1 << pin) : sio_hw->gpio_clr = (1 << pin))
#define digitalReadFast(pin)        ((1 << pin) & sio_hw->gpio_in)
#define sei() interrupts()
#define cli() noInterrupts()

// ADC RP2040-specific calls
void analogReadResolution(int bits);
#ifdef __cplusplus
float analogReadTemp(float vref = 3.3);  // Returns core temp in Centigrade
#endif

// PWM RP2040-specific calls
void analogWriteFreq(uint32_t freq);
void analogWriteRange(uint32_t range);
void analogWriteResolution(int res);

// GPIO RP2040-specific calls
void setPinInvert(pin_size_t ulPin, bool invert);

#ifdef __cplusplus
} // extern "C"
#endif

// FreeRTOS potential calls
extern bool __isFreeRTOS;

// Ancient AVR defines
#define HAVE_HWSERIAL0
#define HAVE_HWSERIAL1
#define HAVE_HWSERIAL2

// PSTR/etc.
#ifndef FPSTR
#define FPSTR (const char *)
#endif

#ifndef PGM_VOID_P
#define PGM_VOID_P const void *
#endif

#ifdef __cplusplus

// emptyString is an ESP-ism, a constant string with ""
extern const String emptyString;

#ifdef USE_TINYUSB
// Needed for declaring Serial
#include "Adafruit_USBD_CDC.h"
#else
#include "SerialUSB.h"
#endif

#include "SerialUART.h"
#include "SerialSemi.h"
#include "RP2040Support.h"
#include "SerialPIO.h"
#include "Bootsel.h"

// Template which will evaluate at *compile time* to a single 32b number
// with the specified bits set.
template <size_t N>
constexpr uint64_t __bitset(const int (&a)[N], size_t i = 0U) {
    return i < N ? (1LL << a[i]) | __bitset(a, i + 1) : 0;
}
#endif

// Warn users trying to use Pico SDK's STDIO implementation
#include <pico/stdio.h> // Ensure it won't be re-included elsewhere
#undef stdio_uart_init
#define stdio_uart_init(...) static_assert(0, "stdio_uart_init is not supported or needed. Either use Serial.printf() or set the debug port in the IDE to Serial/1/2 and use printf().  See https://github.com/earlephilhower/arduino-pico/issues/1433#issuecomment-1540354673 and https://github.com/earlephilhower/arduino-pico/issues/1433#issuecomment-1546783109")
#undef stdio_init_all
#define stdio_init_all(...)  static_assert(0, "stdio_init_all is not supported or needed. Either use Serial.printf() or set the debug port in the IDE to Serial/1/2 and use printf().  See https://github.com/earlephilhower/arduino-pico/issues/1433#issuecomment-1540354673 and https://github.com/earlephilhower/arduino-pico/issues/1433#issuecomment-1546783109")
#undef stdio_usb_init
#define stdio_usb_init(...)  static_assert(0, "stdio_usb_init is not supported or needed. Either use Serial.printf() or set the debug port in the IDE to Serial/1/2 and use printf().  See https://github.com/earlephilhower/arduino-pico/issues/1433#issuecomment-1540354673 and https://github.com/earlephilhower/arduino-pico/issues/1433#issuecomment-1546783109")

// PSRAM decorator
#define PSRAM __attribute__((section("\".psram\"")))

// General GPIO/ADC layout info
#if defined(PICO_RP2350) && !PICO_RP2350A
#define __GPIOCNT 48
#define __FIRSTANALOGGPIO 40
#else
#define __GPIOCNT 30
#define __FIRSTANALOGGPIO 26
#endif

#ifdef __cplusplus
using namespace arduino;
#endif
