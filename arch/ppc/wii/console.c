/*
 *      <console.c>
 *
 *      Simple text console
 *
 *   Copyright (C) 2025 John Davis
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation
 *
 */

#include "config.h"
#include "libopenbios/bindings.h"
#include "libopenbios/console.h"
#include "drivers/drivers.h"

#ifdef CONFIG_DEBUG_CONSOLE
/* ******************************************************************
 *      common functions, implementing simple concurrent console
 * ****************************************************************** */


//
// Wii CAFE console functions.
//
static int cafe_putchar(int c) {
  //
  // Send character over IPC to be displayed on gamepad console.
  //
  out_be32((volatile unsigned int*)CAFE_LATTE_IPCPPCMSG, CAFE_CMD_PRINT | ((uint32_t)c));
  out_be32((volatile unsigned int*)CAFE_LATTE_IPCPPCCTRL, 0x1);

  while (in_be32((volatile unsigned int*)CAFE_LATTE_IPCPPCCTRL) & 0x1);
  return c;
}

static int cafe_availchar(void) {
  return 0;
}

static int cafe_getchar(void) {
  return 0;
}

struct _console_ops cafe_console_ops = {
    .putchar = cafe_putchar,
    .availchar = cafe_availchar,
    .getchar = cafe_getchar
};

#endif	// CONFIG_DEBUG_CONSOLE
