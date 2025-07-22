/*
 *	<macosx.c>
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
#include "libc/vsprintf.h"
#include "boot_args.h"
#include "device_tree.h"
#include "macho-loader.h"
#include "macosx.h"
#include "mkext.h"

#define BOOTX_VERBOSE

extern void flush_dcache_range(char *start, char *stop);
extern void flush_icache_range(char *start, char *stop);

static phandle_t obp_devopen(const char *path) {
    phandle_t ph;

    push_str(path);
    fword("open-dev");
    ph = POP();

    return ph;
}

static int obp_devread(phandle_t ph, char *buf, int nbytes) {
    int ret;

    PUSH((int)buf);
    PUSH(nbytes);
    push_str("read");
    PUSH(ph);
    fword("$call-method");
    ret = POP();

    return ret;
}

static int obp_devseek(phandle_t ph, int hi, int lo) {
    int ret;

    PUSH(lo);
    PUSH(hi);
    push_str("seek");
    PUSH(ph);
    fword("$call-method");
    ret = POP();

    return ret;
}

boot_args_ptr macosx_get_boot_args(void) {
    phandle_t   memory_map;
    uint32_t*   prop;
    int         proplen;

    //
    // Read the boot args location from the memory-map node.
    //
    memory_map = find_dev("/chosen/memory-map");
    if (!memory_map) {
        return 0;
    }
    prop = (uint32_t*)get_property(memory_map, "BootArgs", &proplen);
    if (!prop || (prop[1] != sizeof (boot_args))) {
        return 0;
    }

    return (boot_args_ptr)(prop[0]);
}

int macosx_inject_mkext(void *device_tree) {

    return 0;
}

#define kMacOSXSignature (0x4D4F5358)

int macosx_patch(void) {
    //
    // BootX generally lays out things in memory in the following order:
    //
    // Kernel
    // Drivers
    // Boot arguments
    // Flattened devicetree
    //
    boot_args_ptr   xnu_boot_args;
    DTEntry         dtEntry;
    void            *mkextPtr;
    mkext_header    mkextHeader;
    void            *mkextBuffer;
    unsigned long   prop[2];
    char            mkextName[32];
    void            *newDT;
    unsigned long   newDTSize;
    phandle_t       ph;
    int ret;

    //
    // Get the boot arguments and devicetree.
    //
    xnu_boot_args = macosx_get_boot_args();
    if (!xnu_boot_args) {
        printk("Failed to get boot args!\n");
    }

    printk("BootArgs: %p, top of kernel: 0x%lx\n", xnu_boot_args, xnu_boot_args->topOfKernelData);
    printk("Devicetree: %p, length: 0x%lx\n", xnu_boot_args->deviceTreeP, xnu_boot_args->deviceTreeLength);
    DTInit(xnu_boot_args->deviceTreeP, xnu_boot_args->deviceTreeLength);

    //
    // Add a new DriversPackage property to the memory map.
    // This will be located where the devicetree currently is.
    //
    if (DTLookupEntry(0, "/chosen/memory-map", &dtEntry) != kSuccess) {
        return 1;
    }

    //
    // Read the MKEXT header.
    //
    ph = obp_devopen("hd:3,\\Wii.mkext");
    if (ph == 0) {
        printk("failed to open mkext\n");
    }
    ret = obp_devseek(ph, 0, 0);
    if (ret !=  0) {
        printk("failed to seek mkext\n");
    }

    ret = obp_devread(ph, (char*)&mkextHeader, sizeof (mkextHeader));
    if (ret != sizeof (mkextHeader)) {
        printk("failed to read mkext header\n");
    }

    mkextBuffer = malloc(mkextHeader.length);
    if (!mkextBuffer) {
        printk("failed to allocate mkext\n");
    }
    ret = obp_devseek(ph, 0, 0);
    if (ret !=  0) {
        printk("failed to seek mkext\n");
    }
    ret = obp_devread(ph, mkextBuffer, mkextHeader.length);
    if (ret != mkextHeader.length) {
        printk("failed to read all mkext\n");
    }

    // TODO: do adler check.


    mkextPtr = xnu_boot_args->deviceTreeP;
    prop[0] = (unsigned long)mkextPtr;
    prop[1] = mkextHeader.length;
    sprintf(mkextName, "DriversPackage-%lx", prop[0]);
    printk("MKEXT (%s) will be at %p, length: %x\n", mkextName, mkextPtr, mkextHeader.length);

    if (DTAddProperty(dtEntry, mkextName, prop, sizeof (prop), &newDT, &newDTSize) != kSuccess) {
        return 1;
    }

    //
    // Copy the mkext.
    //
   memcpy(mkextPtr, mkextBuffer, mkextHeader.length);
   free(mkextBuffer);
  //  flush_dcache_range(mkextPtr, ((char*)mkextPtr) + Wii_mkext_len); // is this even needed.

    //
    // Copy the new devicetree after the mkext.
    //
    xnu_boot_args->deviceTreeP      = ((char*)xnu_boot_args->deviceTreeP) + ((prop[1] + 0xFFF) & ~(0xFFF));
    xnu_boot_args->deviceTreeLength = newDTSize;
    memcpy(xnu_boot_args->deviceTreeP, newDT, newDTSize);
    //flush_dcache_range((char*)xnu_boot_args->deviceTreeP, ((char*)xnu_boot_args->deviceTreeP) + newDTSize);
    free(newDT);

    //
    // Fix the DeviceTree property.
    //
    DTInit(xnu_boot_args->deviceTreeP, xnu_boot_args->deviceTreeLength);
    if (DTLookupEntry(0, "/chosen/memory-map", &dtEntry) != kSuccess) {
        return 1;
    }
    prop[0] = (uint32_t)xnu_boot_args->deviceTreeP;
    prop[1] = newDTSize;
    DTSetProperty(dtEntry, "DeviceTree", prop);

    //
    // Adjust top of kernel address.
    //
    xnu_boot_args->topOfKernelData = (unsigned long)xnu_boot_args->deviceTreeP + ((newDTSize + 0xFFF) & ~(0xFFF));

    printk("New top of kernel: 0x%lx\n", xnu_boot_args->topOfKernelData);
    printk("New devicetree: %p, length: 0x%lx\n", xnu_boot_args->deviceTreeP, xnu_boot_args->deviceTreeLength);


    xnu_boot_args->PhysicalDRAM[0].size = 0x02000000; // 32MB MEM1
    xnu_boot_args->PhysicalDRAM[1].base = 0x10000000; // 2016MB MEM2
    xnu_boot_args->PhysicalDRAM[1].size = 0x7E000000;

    //flush_dcache_range((char*)xnu_boot_args, ((char*)xnu_boot_args) + sizeof (boot_args));

   // *((uint32_t*)(((uint8_t*)0) + 0x00095bc4)) = 0x4E800020; // 10.4.6
  //*((uint32_t*)(((uint8_t*)0) + 0x00095668)) = 0x4E800020; // 10.4.6 
  //*((uint32_t*)(((uint8_t*)0) + 0x000c7bdc)) = 0x4E800020; // 10.4.6


    xnu_patch();

    unsigned long gOFMSRSave;
    __asm__ volatile("mfmsr %0" : "=r" (gOFMSRSave));

      // Turn off translations
  unsigned long msr = 0x00001000;
  __asm__ volatile("sync");
  __asm__ volatile("mtmsr %0" : : "r" (msr));
  __asm__ volatile("isync");

    flush_dcache_range((char*)0x4000, (char*)xnu_boot_args->topOfKernelData);
        flush_icache_range((char*)0x4000, (char*)xnu_boot_args->topOfKernelData);

    __asm__ volatile("isync");
  __asm__ volatile("sync");
  __asm__ volatile("eieio");

 // (*(void (*)())0x000923c0)((uint32_t)xnu_boot_args, kMacOSXSignature);

    // Restore translations
  __asm__ volatile("sync");
  __asm__ volatile("mtmsr %0" : : "r" (gOFMSRSave));
  __asm__ volatile("isync");

      flush_dcache_range((char*)0x4000, (char*)xnu_boot_args->topOfKernelData);
        flush_icache_range((char*)0x4000, (char*)xnu_boot_args->topOfKernelData);
  __asm__ volatile("isync");
  __asm__ volatile("sync");
  __asm__ volatile("eieio");


    printk("all flushed 0X%X\n", *((uint32_t*)0x00382bb4));

   // *((uint32_t*)(((uint8_t*)0)  + 0x00085d00)) = 0x4E800020; // 10.2 
   // *((uint32_t*)(((uint8_t*)0) + 0x0008588c)) = 0x4E800020; // 10.2  
  //  *((uint32_t*)(((uint8_t*)0) + 0x000a182c)) = 0x4E800020; // 10.2

    return 0;
}
