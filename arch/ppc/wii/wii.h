/*
 *   Creation Date: <2004/08/28 17:50:12 stepan>
 *   Time-stamp: <2004/08/28 17:50:12 stepan>
 *
 *	<wii.h>
 *
 *   Copyright (C) 2005 Stefan Reinauer
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   version 2
 *
 */

#ifndef _H_WII
#define _H_WII

/* vfd.c */
extern int		vfd_draw_str( const char *str );
extern void		vfd_close( void );

#define ESPRESSO_PVR_HIGH       0x70010000

//
// Speeds.
//
#define WII_RVL_CPU_FREQ        729000000
#define WII_RVL_BUS_FREQ        243000000
#define WII_CAFE_CPU_FREQ       1243125000
#define WII_CAFE_BUS_FREQ       248625000

//
// Device tree.
//
#define WII_DEVICE_TREE_BROADWAY_BASE           0x0C003000
#define WII_DEVICE_TREE_BROADWAY_LENGTH         0x8
#define WII_DEVICE_TREE_ESPRESSO_BASE           0x0C000000
#define WII_DEVICE_TREE_ESPRESSO_LENGTH         0x100

#define WII_DEVICE_TREE_HOLLYWOOD_BASE          0x0D000030
#define WII_DEVICE_TREE_HOLLYWOOD_LENGTH        0x10
#define WII_DEVICE_TREE_HOLLYWOOD_IRQ           14
#define WII_DEVICE_TREE_LATTE_BASE              0x0D800440
#define WII_DEVICE_TREE_LATTE_LENGTH            0x48
#define WII_DEVICE_TREE_LATTE_IRQ               24

#include "kernel.h"

#endif   /* _H_WII */
