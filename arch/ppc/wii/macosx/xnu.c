/*
 *	<xnu.c>
 *
 *   Copyright (C) 2025 John Davis
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   version 2
 *
 */

#include "config.h"
#include "libopenbios/bindings.h"
#include "xnu.h"
#include "../wii.h"

#define XNU_REGION_START        0x4000

//
// Patch the CPU type checking to force the 750CX branch to match Espresso.
//
static void patch_xnu_cpu_check(char *base, uint32_t length, uint32_t kernelVersion) {
  int       found;

  static const char cpuFind[] = {
        0xFF, 0xFF, 0x0F, 0x00,     // Mask
        0x00, 0x08,                 // PROCESSOR_VERSION_750
        0x02, 0x00                  // Version for mask
  };
  static const char cpuReplace[] = {
        0xFF, 0xFF, 0x00, 0x00,     // Mask
        0x70, 0x01,                 // High bits of Espressor PVR
        0x00, 0x00                  // Version for mask
  };

  //
  // Only need to patch on Espresso.
  //
  if (!is_wii_cafe()) {
    return;
  }

  //
  // Look for pattern at entry point (__start).
  //
  found = 0;
  for (int i = 0; i < length; i++, base++) {
    if (memcmp(base, cpuFind, sizeof (cpuFind)) == 0) {
      found = 1;
      break;
    }
  }

  if (!found) {
    printk("Failed to locate CPU check patch pattern\n");
    return;
  }

  memcpy(base, cpuReplace, sizeof (cpuReplace));
  printk("Patched CPU check for Espresso at %p\n", base);
}

int xnu_patch(void) {
    phandle_t   memory_map;
    uint32_t*   prop;
    int         proplen;
    char        *kernel_header_base;
    uint32_t    kernel_header_len;
    char        *kernel_text_base;
    uint32_t    kernel_text_len;
    char        *kernel_data_base;
    uint32_t    kernel_data_len;
    uint32_t    xnu_version;

    //
    // Read the kernel sizes from the memory-map node.
    //
    memory_map = find_dev("/chosen/memory-map");
    if (!memory_map) {
        return 1;
    }
    prop = (uint32_t*)get_property(memory_map, "Kernel-__HEADER", &proplen);
    if (!prop) {
        return 1;
    }
    kernel_header_base = (char*)prop[0];
    kernel_header_len = prop[1];
    prop = (uint32_t*)get_property(memory_map, "Kernel-__TEXT", &proplen);
    if (!prop) {
        return 1;
    }
    kernel_text_base = (char*)prop[0];
    kernel_text_len = prop[1];
    prop = (uint32_t*)get_property(memory_map, "Kernel-__DATA", &proplen);
    if (!prop) {
        return 1;
    }
    kernel_data_base = (char*)prop[0];
    kernel_data_len = prop[1];

    printk("XNU header: %p, len %x\n", kernel_header_base, kernel_header_len);
    printk("XNU text: %p, len %x\n", kernel_text_base, kernel_text_len);
    printk("XNU data: %p, len %x\n", kernel_data_base, kernel_data_len);

    // Get the kernel version.
    xnu_version = xnu_read_darwin_version(kernel_text_base, kernel_text_len + kernel_data_len);
    printk("XNU version: 0x%X\n", xnu_version);

    patch_xnu_cpu_check(kernel_text_base, kernel_text_len, xnu_version);
    return 0;
}