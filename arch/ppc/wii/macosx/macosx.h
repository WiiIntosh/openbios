/*
 *	<macosx.h>
 *
 *   Copyright (C) 2025 John Davis
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   version 2
 *
 */

#ifndef _H_MACOSX
#define _H_MACOSX

#include "boot_args.h"

extern unsigned long macho_get_top(void *macho);
extern int macosx_patch_bootx(char *base, unsigned long length);
extern int macosx_check_bootx(void);
extern boot_args_ptr macosx_get_boot_args(void);
extern int macosx_inject_mkext(void *device_tree);
extern int macosx_patch(void);

#endif
