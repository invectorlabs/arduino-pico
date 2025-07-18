// -------------------------------------------------- //
// This file is autogenerated by pioasm; do not edit! //
// -------------------------------------------------- //

#pragma once

#if !PICO_NO_HARDWARE
#include "hardware/pio.h"
#endif

// ----- //
// servo //
// ----- //

#define servo_wrap_target 0
#define servo_wrap 7
#define servo_pio_version 0

#define servo_offset_halt 2u

static const uint16_t servo_program_instructions[] = {
    //     .wrap_target
    0x9080, //  0: pull   noblock         side 0
    0xa027, //  1: mov    x, osr
    0x0022, //  2: jmp    !x, 2
    0xa046, //  3: mov    y, isr
    0x00a6, //  4: jmp    x != y, 6
    0x1807, //  5: jmp    7               side 1
    0xa042, //  6: nop
    0x0084, //  7: jmp    y--, 4
    //     .wrap
};

#if !PICO_NO_HARDWARE
static const struct pio_program servo_program = {
    .instructions = servo_program_instructions,
    .length = 8,
    .origin = -1,
    .pio_version = servo_pio_version,
#if PICO_PIO_VERSION > 0
    .used_gpio_ranges = 0x0
#endif
};

static inline pio_sm_config servo_program_get_default_config(uint offset) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + servo_wrap_target, offset + servo_wrap);
    sm_config_set_sideset(&c, 2, true, false);
    return c;
}

static inline void servo_program_init(PIO pio, uint sm, uint offset, uint pin) {
    pio_gpio_init(pio, pin);
    pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, true);
    pio_sm_config c = servo_program_get_default_config(offset);
    sm_config_set_sideset_pins(&c, pin);
    pio_sm_init(pio, sm, offset, &c);
}

#endif

