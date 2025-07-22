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
#include "macosx.h"
#include "macho-loader.h"
#include "nlist.h"
#include "../wii.h"

static int xnu_get_symtab(macho_sym_context_t *symContext) {
    phandle_t   memory_map;
    uint32_t*   prop;
    int         proplen;
    struct symtab_command *symTabCommand;

    //
    // Read the kernel symtab from the memory-map node.
    //
    memory_map = find_dev("/chosen/memory-map");
    if (!memory_map) {
        return 0;
    }
    prop = (uint32_t*)get_property(memory_map, "Kernel-__SYMTAB", &proplen);
    if (!prop) {
        return 0;
    }
    symTabCommand = (struct symtab_command*)prop[0];

    symContext->symbol_table    = (struct nlist*) symTabCommand->symoff;
    symContext->symbol_count    = symTabCommand->nsyms;
    symContext->string_table    = (const char*) symTabCommand->stroff;
    symContext->string_size     = symTabCommand->strsize;

    return 1;
}

//
// Patch the CPU type checking to force the 750CX branch to match Espresso.
//
static int xnu_patch_cpu_check(macho_sym_context_t *symContext) {
    unsigned long   kern_sym_start;
    char            *base;
    int             found;

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
    // Get _start function location.
    //
    kern_sym_start = macho_resolve_symbol(symContext, "__start");
    if (kern_sym_start == 0) {
        return 0;
    }
    base = (char*)kern_sym_start;

    //
    // Look for pattern at entry point (__start).
    //
    found = 0;
    for (int i = 0; i < 0x4000; i++, base++) {
        if (memcmp(base, cpuFind, sizeof (cpuFind)) == 0) {
            found = 1;
            break;
        }
    }

    if (!found) {
        printk("xnu_patch_cpu_check: failed to locate CPU check patch pattern\n");
        return 0;
    }

    memcpy(base, cpuReplace, sizeof (cpuReplace));
    printk("xnu_patch_cpu_check: patched CPU check for Espresso at %p\n", base);
    return 1;
}

//
// Patch PE_find_scc to return 0.
//
static int xnu_patch_find_scc(macho_sym_context_t *symContext) {
    unsigned long   kern_sym_pe_find_scc;
    uint32_t        *func;

    //
    // Get _PE_find_scc function location.
    //
    kern_sym_pe_find_scc = macho_resolve_symbol(symContext, "_PE_find_scc");
    if (kern_sym_pe_find_scc == 0) {
        return 0;
    }
    func = (uint32_t*)kern_sym_pe_find_scc;

    //
    // Patch to return 0. This function calls _get_io_base_addr which will panic.
    //
    func[0] = 0x38600000;
    func[1] = 0x4E800020;
    printk("xnu_patch_cpu_check: patched _PE_find_scc at %p\n", func);
    return 1;
}

//
// Patch the BAT setup to avoid conflicts with other BATs for the video DBAT3, and prevent DBAT2 from being filled.
//
static int xnu_patch_io_bats(macho_sym_context_t *symContext) {
    unsigned long   kern_sym_ppc_init;
    char            *base;
    int             found;

    static const char videoBatFind[] = {
        0xF0, 0x00,     // ... 0xF000
        0x41, 0x82      // beq ...
    };
    static const char videoBatRepl[] = {
        0xFF, 0xFF,     // ... 0xFFFF
        0x41, 0x82      // beq ...
    };
    static const char ioBatFind1[] = {
        0x4C, 0x00, 0x01, 0x2C,     // isync
        0x48                        // bl ...
    };
    static const char ioBatRepl1[] = {
        0x4C, 0x00, 0x01, 0x2C,     // isync
        0x60, 0x00, 0x00, 0x00      // nop
    };
    static const char ioBatFind2[] = {
        0x54, 0x63, 0x00, 0x06,     // rlwinm     r3, r3, 0x0, 0x0, 0x3
        0x7C, 0x03                  // cmpw r3, ...
    };
    static const char ioBatRepl2[] = {
        0x38, 0x60, 0x00, 0x00,     // li r3, 0x0000
        0x2C, 0x03, 0x00, 0x00,     // cmpwi r3, 0x0000
    };

    //
    // Get ppc_init function. TODO: Might need to adjust for 10.0 and 10.1.
    //
    kern_sym_ppc_init = macho_resolve_symbol(symContext, "_ppc_init");
    if (kern_sym_ppc_init == 0) {
        return 0;
    }
    base = (char*)kern_sym_ppc_init;

    //
    // Do video BAT patch. Start at 0x2 offset as patch starts halfway through an opcode.
    //
    found = 0;
    for (int i = 2; i < 0x1000; i += 4) {
        if (memcmp(&base[i], videoBatFind, sizeof (videoBatFind)) == 0) {
            memcpy(&base[i], videoBatRepl, sizeof (videoBatRepl));
            found = 1;
            break;
        }
    }

    if (!found) {
        printk("xnu_patch_io_bats: failed to patch video BAT pattern\n");
        return 0;
    }

    //
    // Do first I/O BAT patch. This calls _get_io_base_addr which will panic.
    //
    found = 0;
    for (int i = 0; i < 0x1000; i += 4) {
        if (memcmp(&base[i], ioBatFind1, sizeof (ioBatFind1)) == 0) {
            memcpy(&base[i], ioBatRepl1, sizeof (ioBatRepl1));
            found = 1;
            break;
        }
    }

    if (!found) {
        printk("xnu_patch_io_bats: failed to patch I/O BAT pattern\n");
        return 0;
    }

    //
    // Do second I/O BAT patch to prevent DBAT2 from being populated.
    //
    found = 0;
    for (int i = 0; i < 0x1000; i += 4) {
        if (memcmp(&base[i], ioBatFind2, sizeof (ioBatFind2)) == 0) {
            memcpy(&base[i], ioBatRepl2, sizeof (ioBatRepl2));
            found = 1;
            break;
        }
    }

    if (!found) {
        printk("xnu_patch_io_bats: failed to patch I/O BAT pattern\n");
        return 0;
    }

    return 1;
}

int xnu_patch(void) {
    macho_sym_context_t kernel_syms;
    uint32_t            xnu_version;

    //
    // Get the kernel symbol table.
    //
    if (!xnu_get_symtab(&kernel_syms)) {
        printk("Failed to get kernel symbol table\n");
        return 0;
    }

    xnu_version = xnu_read_darwin_version(&kernel_syms);
    if (xnu_version == 0) {
        return 0;
    }
    printk("XNU version: 0x%X\n", xnu_version);

    if (!xnu_patch_find_scc(&kernel_syms)) {
        return 0;
    }

    if (is_wii_cafe()) {
        if (!xnu_patch_cpu_check(&kernel_syms)) {
            return 0;
        }
    }

    if (xnu_match_darwin_version(xnu_version, 0, XNU_VERSION_JAGUAR_MAX)) {
        if (!xnu_patch_io_bats(&kernel_syms)) {
            return 0;
        }
    }
    
    return 1;
}