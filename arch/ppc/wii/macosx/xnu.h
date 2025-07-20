/*
 *	<xnu.h>
 *
 *   Copyright (C) 2025 John Davis
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   version 2
 *
 */

#ifndef _H_XNU
#define _H_XNU

extern int xnu_patch(void);

extern int xnu_match_darwin_version(uint32_t kernelVersion, uint32_t minVerison, uint32_t maxVersion);
extern uint32_t xnu_read_darwin_version(const char *kernel, uint32_t kernelSize);

#endif
