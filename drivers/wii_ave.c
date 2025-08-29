/*
 *   OpenBIOS Wii A/V encoder support
 *
 *   (C) 2025 John Davis
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   version 2
 *
 */

/*
	BootMii - a Free Software replacement for the Nintendo/BroadOn bootloader.
	low-level video support for the BootMii UI

Copyright (C) 2008, 2009	Hector Martin "marcan" <marcan@marcansoft.com>
Copyright (C) 2009			Haxx Enterprises <bushing@gmail.com>
Copyright (c) 2009		Sven Peter <svenpeter@gmail.com>

# This code is licensed to you under the terms of the GNU GPL, version 2;
# see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

Some routines and initialization constants originally came from the
"GAMECUBE LOW LEVEL INFO" document and sourcecode released by Titanik
of Crazy Nation and the GC Linux project.
*/

#include "config.h"
#include "libopenbios/bindings.h"
#include "timer.h"
#include "wii_ave.h"

#define HOLLYWOOD_GPIO_BASE     0x0D0000C0
#define HOLLYWOOD_GPIO_REG_OUT  0x00
#define HOLLYWOOD_GPIO_REG_DIR  0x04
#define HOLLYWOOD_GPIO_REG_IN   0x08

#define GPIO_PIN_AVE_SCL        0x4000
#define GPIO_PIN_AVE_SDA        0x8000

#define AVE_I2C_ADDR            0xE0

//
// Read/write registers.
//
static inline uint32_t gpio_read32(uint32_t offset) {
  return in_be32((volatile unsigned*)(HOLLYWOOD_GPIO_BASE + offset));
}
static inline void gpio_write32(uint32_t offset, uint32_t data) {
  out_be32((volatile unsigned*)(HOLLYWOOD_GPIO_BASE + offset), data);
}

//
// AVE data.
//
static const uint8_t ave_gamma[0x21] = {
    0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00,
    0x10, 0x00, 0x10, 0x00, 0x10, 0x20, 0x40, 0x60,
    0x80, 0xA0, 0xEB, 0x10, 0x00, 0x20, 0x00, 0x40,
    0x00, 0x60, 0x00, 0x80, 0x00, 0xA0, 0x00, 0xEB,
    0x00
};

static uint8_t ave_macrovision[0x1A];

//
// Set I2C bus direction.
//
static void i2c_set_direction(int out) {
    uint32_t gpio = gpio_read32(HOLLYWOOD_GPIO_REG_DIR);
    gpio &= ~(GPIO_PIN_AVE_SDA);
    gpio |= GPIO_PIN_AVE_SCL;

    if (out) {
        gpio |= GPIO_PIN_AVE_SDA;
    }
    gpio_write32(HOLLYWOOD_GPIO_REG_DIR, gpio);
}

static void i2c_set_scl(int scl) {
    uint32_t gpio = gpio_read32(HOLLYWOOD_GPIO_REG_OUT) & ~(GPIO_PIN_AVE_SCL);

    if (scl) {
        gpio |= GPIO_PIN_AVE_SCL;
    }
    gpio_write32(HOLLYWOOD_GPIO_REG_OUT, gpio);
}

static void i2c_set_sda(int sda) {
    uint32_t gpio = gpio_read32(HOLLYWOOD_GPIO_REG_OUT) & ~(GPIO_PIN_AVE_SDA);

    if (sda) {
        gpio |= GPIO_PIN_AVE_SDA;
    }
    gpio_write32(HOLLYWOOD_GPIO_REG_OUT, gpio);
}

static int i2c_get_sda(void) {
    return gpio_read32(HOLLYWOOD_GPIO_REG_IN) & GPIO_PIN_AVE_SDA;
}

//
// Send address of the AVE to the I2C bus.
//
static int i2c_send_slave_addr(char addr) {
    i2c_set_sda(0);
    udelay(2);

    i2c_set_scl(0);
    for (int i = 0; i < 8; i++) {
        i2c_set_sda(addr & 0x80);
        udelay(2);
        i2c_set_scl(1);
        udelay(2);
        i2c_set_scl(0);
        addr <<= 1;
    }

    i2c_set_direction(0);
    udelay(2);

    i2c_set_scl(1);
    udelay(2);

    if (i2c_get_sda()) {
        return 0;
    }

    i2c_set_sda(0);
    i2c_set_direction(1);
    i2c_set_scl(0);
    return 1;
}

static int i2c_send_data(char addr, void *data, size_t length) {
    i2c_set_direction(1);
    i2c_set_scl(1);
    i2c_set_sda(1);
    udelay(4);

    if (!i2c_send_slave_addr(addr)) {
        return 0;
    }

    i2c_set_direction(1);
    for (int i = 0; i < length; i++) {
        char byte = ((char*)data)[i];
        for (int j = 0; j < 8; j++) {
            i2c_set_sda(byte & 0x80);
            udelay(2);
            i2c_set_scl(1);
            udelay(2);
            i2c_set_scl(0);
            byte <<= 1;
        }

        i2c_set_direction(0);
        udelay(2);
        i2c_set_scl(1);
        udelay(2);

        if (i2c_get_sda()) {
            return 0;
        }

        i2c_set_sda(0);
        i2c_set_direction(1);
        i2c_set_scl(0);
    }

    i2c_set_direction(1);
    i2c_set_sda(0);
    udelay(2);
    i2c_set_sda(1);
    udelay(2);
    return 1;
}

//
// Writes an 8-bit value to an AVE register.
//
static int ave_write8(char reg, uint8_t data) {
  char buffer[2];

  buffer[0] = reg;
  buffer[1] = data;
  return i2c_send_data(AVE_I2C_ADDR, buffer, sizeof (buffer));
}

//
// Writes an 16-bit value to an AVE register.
//
static int ave_write16(char reg, uint16_t data) {
  char buffer[3];

  buffer[0] = reg;
  buffer[1] = (data >> 8) & 0xFF;
  buffer[1] = data & 0xFF;
  return i2c_send_data(AVE_I2C_ADDR, buffer, sizeof (buffer));
}

//
// Writes an 32-bit value to an AVE register.
//
static int ave_write32(char reg, uint32_t data) {
  char buffer[5];

  buffer[0] = reg;
  buffer[1] = (data >> 24) & 0xFF;
  buffer[2] = (data >> 16) & 0xFF;
  buffer[3] = (data >> 8) & 0xFF;
  buffer[4] = data & 0xFF;
  return i2c_send_data(AVE_I2C_ADDR, buffer, sizeof (buffer));
}

//
// Writes data to an AVE register.
//
static int ave_write(char reg, const void *data, size_t length) {
  char buffer[0x100];

  buffer[0] = reg;
  memcpy(&buffer[1], data, length);
  return i2c_send_data(AVE_I2C_ADDR, buffer, length + 1);
}

//
// Initializes the Wii AV encoder.
//
int ave_init(int component) {
    memset(ave_macrovision, 0, sizeof (ave_macrovision));

    ave_write8(0x6A, 1);
    ave_write8(0x65, 3);

    ave_write8(0x01, component ? (1 << 5) : 0);

    ave_write8(0x00, 0);
	ave_write16(0x71, 0x8E8E);
	ave_write8(0x02, 7);
	ave_write16(0x05, 0x0000);
	ave_write16(0x08, 0x0000);
	ave_write32(0x7A, 0x00000000);

    ave_write(0x40, ave_macrovision, sizeof (ave_macrovision));
    ave_write8(0x0A, 0);
    ave_write8(0x03, 1);
    ave_write(0x10, ave_gamma, sizeof (ave_gamma));
    ave_write8(0x04, 1);

   // ave_write32(0x7A, 0x00000000);
   // ave_write16(0x08, 0x0000);
   // ave_write8(0x03, 1);

    return 0;
}
