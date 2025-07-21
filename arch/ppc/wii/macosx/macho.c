/*
 *	<macho.c>
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
#include "macho-loader.h"
#include "macosx.h"
#include "nlist.h"

//
// Gets the highest virtual address in the Mach-O binary.
//
unsigned long macho_get_top(void *macho) {
    struct mach_header      *machHeader;
    char                    *commandPtr;
    struct load_command     *loadCommand;
    struct segment_command  *segCommand; 
    unsigned long           machTop;

    //
    // Check the header.
    //
    machHeader = (struct mach_header*)macho;
    if (machHeader->magic != MH_MAGIC) {
        printk("macho_get_top: Not a macho at %p\n", macho);
        return 0;
    }

    //
    // Iterate through commands and get highest virtual address.
    //
    machTop = 0;
    commandPtr = (char*)(machHeader + 1);
    for (int i = 0; i < machHeader->ncmds; i++) {
        loadCommand = (struct load_command*)commandPtr;

        if (loadCommand->cmd == LC_SEGMENT) {
            segCommand = (struct segment_command*)loadCommand;
            if ((segCommand->vmaddr + segCommand->vmsize) > machTop) {
                machTop = segCommand->vmaddr + segCommand->vmsize;
            }
        }

        commandPtr += loadCommand->cmdsize; 
    }

    return machTop;
}


unsigned long macho_resolve_symbol(macho_sym_context_t *context, const char *symbolName) {
    const char    *symStr;

    for (unsigned long i = 0; i < context->symbol_count; i++) {
        //
        // Get the symbol string for the symbol.
        //
        symStr = (const char*)(context->string_table + context->symbol_table[i].n_un.n_strx);
        if (strcmp(symbolName, symStr) == 0) {
        printk("macho_resolve_symbol: found symbol '%s' at 0x%lX\n", symbolName, context->symbol_table[i].n_value);
        return context->symbol_table[i].n_value;
        }
    }

    printk("macho_resolve_symbol: failed to locate symbol '%s'\n", symbolName);
    return 0;
}