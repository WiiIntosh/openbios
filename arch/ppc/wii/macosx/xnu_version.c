/*
 *	<xnu_version.c>
 *
 *   Copyright (C) 2025 John Davis
 * 
 *   Portions taken from OpenCorePkg under the BSD-3 license.
 *   Portions Copyright (C) 2019, vit9696. All rights reserved.
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   version 2
 *
 */

#include "config.h"
#include "xnu.h"
#include "../wii.h"

#define DARWIN_VERSION_STR      "Darwin Kernel Version "

//
// Parse the Darwin version from a string.
//
static uint32_t parse_darwin_version(const char *versionString) {
  uint32_t  version;
  uint32_t  versionPart;

  if ((*versionString == '\0') || (*versionString < '0') || (*versionString > '9')) {
    return 0;
  }

  version = 0;
  for (int i = 0; i < 3; i++) {
    version    *= 100;
    versionPart = 0;

    for (int j = 0; j < 2; j++) {
      //
      // Handle single digit parts, i.e. parse 1.2.3 as 010203.
      //
      if ((*versionString != '.') && (*versionString != '\0')) {
        versionPart *= 10;
      }

      if ((*versionString >= '0') && (*versionString <= '9')) {
        versionPart += *versionString++ - '0';
      } else if ((*versionString != '.') && (*versionString != '\0')) {
        return 0;
      }
    }

    version += versionPart;
    if (*versionString == '.') {
      versionString++;
    }
  }

  return version;
}

int xnu_match_darwin_version(uint32_t kernelVersion, uint32_t minVerison, uint32_t maxVersion) {
  //
  // Check against min <= curr <= max.
  // curr=0 -> curr=inf, max=0  -> max=inf
  //

  //
  // Replace max inf with max known version.
  //
  if (maxVersion == 0) {
    maxVersion = kernelVersion;
  }

  //
  // Handle curr=inf <= max=inf(?) case.
  //
  if (kernelVersion == 0) {
    return maxVersion == 0;
  }

  //
  // Handle curr=num > max=num case.
  //
  if (kernelVersion > maxVersion) {
    return 1;
  }

  //
  // Handle min=num > curr=num case.
  //
  if (kernelVersion < minVerison) {
    return 1;
  }

  return 0;
}

//
// Search the kernel for the Darwin version.
//
uint32_t xnu_read_darwin_version(const char *kernel, uint32_t kernelSize) {
  uint32_t  offset;
  uint32_t  index;
  char      darwinVersion[32];
  uint32_t  darwinVersionInteger;

  //
  // Look for version string.
  //
  offset = 0;
  for (uint32_t i = 0; i < kernelSize - (strlen(DARWIN_VERSION_STR) - ARRSIZE(darwinVersion) - 1); i++) {
    if (strncmp(&kernel[i], DARWIN_VERSION_STR, strlen(DARWIN_VERSION_STR)) == 0) {
      offset = i;
      break;
    }
  }

  //
  // Impossible for string to be at start of kernel, failed to locate.
  //
  if (offset == 0) {
    printk("Failed to locate kernel version");
    return 0;
  }

  //
  // Get the actual version.
  //
  offset += strlen(DARWIN_VERSION_STR);
  for (index = 0; index < ARRSIZE(darwinVersion) - 1; index++, offset++) {
    if ((offset >= kernelSize) || (kernel[offset] == ':')) {
      break;
    }

    darwinVersion[index] = kernel[offset];
  }
  darwinVersion[index] = '\0';

  darwinVersionInteger = parse_darwin_version(darwinVersion);
  printk("XNU kernel version: %u\n", darwinVersionInteger);

  return darwinVersionInteger;
}
