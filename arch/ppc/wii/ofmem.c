/*
 *   Creation Date: <1999/11/07 19:02:11 samuel>
 *   Time-stamp: <2004/01/07 19:42:36 samuel>
 *
 *	<ofmem.c>
 *
 *	OF Memory manager
 *
 *   Copyright (C) 1999-2004 Samuel Rydh (samuel@ibrium.se)
 *   Copyright (C) 2004 Stefan Reinauer
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation
 *
 */

#include "config.h"
#include "libopenbios/bindings.h"
#include "libc/string.h"
#include "libopenbios/ofmem.h"
#include "kernel.h"
#include "mmutypes.h"
#include "asm/processor.h"
#include "wii.h"

#define BIT(n)		(1U << (31 - (n)))

/* called from assembly */
extern void dsi_exception(void);
extern void isi_exception(void);
extern void setup_mmu(void);

/****************************************************************
 * Memory usage (before of_quiesce is called) - Wii / Revolution
 *
 *			Physical
 *
 *	0x00000000	Exception vectors
 *	0x00004000	Free space
 *  0x017FFFFF  End of MEM1 free space
 *  0x10000000  Start of MEM2 free space
 *  0x11000000  Open Firmware (us)
 *  0x11100000  OF allocations
 *  0x111F0000  PTE Hash
 *  0x11200000  Free space
 *  0x11600000  Start of MEM2-MEM1 expansion region
 *  0x13DFFFFF  End of MEM2-MEM1 expansion region
 *  0x13E00000  XFB and misc region in MEM2
 *  0x13FFFFFF  End of MEM2
 * 
 *			Virtual
 *
 *	0x00000000	Exception vectors
 *	0x00004000	Free space
 *  0x03FFFFFF  End of free space
 *  0x11000000  Open Firmware (us)
 *  0x11100000  OF allocations
 *  0x111F0000  PTE Hash
 *  0x13E00000  XFB and misc region in MEM2
 *  0x13FFFFFF  End of MEM2
 *
 * Allocations grow downwards from 0x11100000
 *
 ****************************************************************/

/****************************************************************
 * Memory usage (before of_quiesce is called) - Wii U / Cafe
 *
 *			Physical
 *
 *	0x00000000	Exception vectors
 *	0x00004000	Free space
 *  0x01FFFFFF  End of MEM1 free space
 *  0x10000000  Start of MEM2 free space
 *  0x11000000  Open Firmware (us)
 *  0x11100000  OF allocations
 *  0x111F0000  PTE Hash
 *  0x11200000  Free space
 *  0x11600000  Start of MEM2-MEM1 expansion region
 *  0x175FFFFF  End of MEM2-MEM1 expansion region
 *  0x17600000  Free space
 *  0x7E000000  GFX memory
 *  0x7FFFFFFF  End of MEM2
 * 
 *			Virtual
 *
 *	0x00000000	Exception vectors
 *	0x00004000	Free space
 *  0x07FFFFFF  End of free space
 *  0x11000000  Open Firmware (us)
 *  0x11100000  OF allocations
 *  0x111F0000  PTE Hash
 *  0x7E000000  GFX memory
 *  0x7FFFFFFF  End of MEM2
 *
 * Allocations grow downwards from 0x11100000
 *
 ****************************************************************/

#define HASH_BITS		15
#define HASH_SIZE		(2 << HASH_BITS)
#define SEGR_BASE		0x0400

#define FREE_BASE_1		0x00004000
#define MEM_HOLE_BASE	0x01800000
#define FREE_BASE_2		0x10000000

#define OF_CODE_START	0x11000000
#define OF_MALLOC_BASE	&_end

#define HASH_BASE		(0x11200000 - HASH_SIZE)

static ofmem_t s_ofmem;

#define IO_BASE			0x80000000
#define OFMEM (&s_ofmem)

#define WII_IO_BASE		0x0C000000
#define WII_MEM2_BASE	0x10000000

#define CAFE_GFX_BASE   0x8F000000

static inline unsigned long get_hash_base(void) {
    return HASH_BASE;
}

static inline unsigned long get_hash_size(void) {
    return HASH_SIZE;
}

static ucell get_heap_top(void) {
    return HASH_BASE;
}

static inline size_t ALIGN_SIZE(size_t x, size_t a) {
    return (x + a - 1) & ~(a - 1);
}

ofmem_t* ofmem_arch_get_private(void) {
    return OFMEM;
}

void* ofmem_arch_get_malloc_base(void) {
    return OF_MALLOC_BASE;
}

ucell ofmem_arch_get_heap_top(void) {
    return get_heap_top();
}

ucell ofmem_arch_get_virt_top(void) {
    return IO_BASE;
}

void ofmem_arch_unmap_pages(ucell virt, ucell size) {
    /* kill page mappings in provided range */
}

void ofmem_arch_map_pages(phys_addr_t phys, ucell virt, ucell size, ucell mode) {
    /* none yet */
}

ucell ofmem_arch_get_iomem_base(void) {
    /* Currently unused */
    return 0;
}

ucell ofmem_arch_get_iomem_top(void) {
    /* Currently unused */
    return 0;
}

retain_t *ofmem_arch_get_retained(void) {
    /* not implemented */
    return NULL;
}

int ofmem_arch_get_physaddr_cellsize(void) {
    return 1;
}

int ofmem_arch_encode_physaddr(ucell *p, phys_addr_t value) {
    int n = 0;
    p[n++] = value;
    return n;
}

/* Return size of a single MMU package translation property entry in cells */
int ofmem_arch_get_translation_entry_size(void) {
    return 3 + ofmem_arch_get_physaddr_cellsize();
}

/* Generate translation property entry for PPC.
 * According to the platform bindings for PPC
 * (http://www.openfirmware.org/1275/bindings/ppc/release/ppc-2_1.html#REF34579)
 * a translation property entry has the following layout:
 *
 *      virtual address
 *      length
 *      physical address
 *      mode
 */
void ofmem_arch_create_translation_entry(ucell *transentry, translation_t *t) {
    int i = 0;

    transentry[i++] = t->virt;
    transentry[i++] = t->size;
    i += ofmem_arch_encode_physaddr(&transentry[i], t->phys);
    transentry[i++] = t->mode;
}

/* Return the size of a memory available entry given the phandle in cells */
int ofmem_arch_get_available_entry_size(phandle_t ph) {
    if (ph == s_phandle_memory) {
        return 1 + ofmem_arch_get_physaddr_cellsize();
    } else {
        return 1 + 1;
    }
}

/* Generate memory available property entry for PPC */
void ofmem_arch_create_available_entry(phandle_t ph, ucell *availentry, phys_addr_t start, ucell size) {
    int i = 0;

    if (ph == s_phandle_memory) {
        i += ofmem_arch_encode_physaddr(availentry, start);
    } else {
    availentry[i++] = start;
    }
    
    availentry[i] = size;
}

/************************************************************************/
/*	OF private allocations						*/
/************************************************************************/

/* Private functions for mapping between physical/virtual addresses */
phys_addr_t va2pa(unsigned long va) {
    return (phys_addr_t)va;
}

unsigned long pa2va(phys_addr_t pa) {
    return (unsigned long)pa;
}

void *malloc(size_t size) {
    return ofmem_malloc(size);
}

void free(void *ptr) {
    ofmem_free(ptr);
}

void *realloc(void *ptr, size_t size) {
    return ofmem_realloc(ptr, size);
}


/************************************************************************/
/*	misc								*/
/************************************************************************/

ucell ofmem_arch_default_translation_mode(phys_addr_t phys) {
	//
	// Wii I/O regions.
	//
	if ((phys >= WII_IO_BASE) && (phys < WII_MEM2_BASE))
		return 0x6a;	/* WIm GxPp, I/O */
    if ((is_wii_cafe() && (phys >= CAFE_GFX_BASE)) || (is_wii_rvl() && (phys >= RVL_XFB_BASE)))
        return 0x6a;	/* WIm GxPp, I/O */
    /* XXX: Guard bit not set as it should! */
	if( phys < IO_BASE || phys >= 0xffc00000)
		return 0x02;	/*0xa*/	/* wim GxPp */
	return 0x6a;		/* WIm GxPp, I/O */
}

ucell ofmem_arch_io_translation_mode(phys_addr_t phys) {
    return 0x6a;		/* WIm GxPp, I/O */
}

/************************************************************************/
/*	page fault handler						*/
/************************************************************************/

static phys_addr_t ea_to_phys(unsigned long ea, ucell *mode) {
    ucell phys;

    /* hardcode our translation needs */
    if( ea >= OF_CODE_START && ea < FREE_BASE_2 ) {
        *mode = ofmem_arch_default_translation_mode( ea );
        return ea;
    }

    phys = ofmem_translate(ea, mode);
    if( phys == (ucell)-1 ) {
        phys = ea;
        *mode = ofmem_arch_default_translation_mode( phys );

        /* print_virt_range(); */
        /* print_phys_range(); */
        /* print_trans(); */
    }
    return phys;
}

static void hash_page(ucell ea, ucell phys, ucell mode) {
    static int next_grab_slot=0;
    unsigned long *upte, cmp, hash1;
    int i, vsid, found;
    mPTE_t *pp;

    vsid = (ea>>28) + SEGR_BASE;
    cmp = BIT(0) | (vsid << 7) | ((ea & 0x0fffffff) >> 22);

    hash1 = vsid;
    hash1 ^= (ea >> 12) & 0xffff;
    hash1 &= (get_hash_size() - 1) >> 6;

    pp = (mPTE_t*)(get_hash_base() + (hash1 << 6));
    upte = (unsigned long*)pp;

    /* replace old translation */
    for( found=0, i=0; !found && i<8; i++ )
        if( cmp == upte[i*2] )
            found=1;

    /* otherwise use a free slot */
    for( i=0; !found && i<8; i++ )
        if( !pp[i].v )
            found=1;

    /* out of slots, just evict one */
    if( !found ) {
        i = next_grab_slot + 1;
        next_grab_slot = (next_grab_slot + 1) % 8;
    }
    i--;
    upte[i*2] = cmp;
    upte[i*2+1] = (phys & ~0xfff) | mode;

    asm volatile( "tlbie %0"  :: "r"(ea) );
}

void dsi_exception(void) {
    unsigned long dar, dsisr;
    ucell mode;
    ucell phys;

    asm volatile("mfdar %0" : "=r" (dar) : );
    asm volatile("mfdsisr %0" : "=r" (dsisr) : );

    phys = ea_to_phys(dar, &mode);
    hash_page(dar, phys, mode);
}

void isi_exception(void) {
    unsigned long nip, srr1;
    ucell mode;
    ucell phys;

    asm volatile("mfsrr0 %0" : "=r" (nip) : );
    asm volatile("mfsrr1 %0" : "=r" (srr1) : );

    phys = ea_to_phys(nip, &mode);
    hash_page(nip, phys, mode);
}

/************************************************************************/
/*	init / cleanup							*/
/************************************************************************/

void setup_mmu(void) {
    unsigned long sdr1 = HASH_BASE | ((HASH_SIZE-1) >> 16);
    unsigned long sr_base = (0x20 << 24) | SEGR_BASE;
    unsigned long msr;
    int i;

    asm volatile("mtsdr1 %0" :: "r" (sdr1) );
    for( i=0; i<16; i++ ) {
        int j = i << 28;
        asm volatile("mtsrin %0,%1" :: "r" (sr_base + i), "r" (j) );
    }
    asm volatile("mfmsr %0" : "=r" (msr) : );
    msr |= MSR_IR | MSR_DR;
    asm volatile("mtmsr %0" :: "r" (msr) );
}

void ofmem_init(void) {
    ofmem_t *ofmem = OFMEM;
    memset(ofmem, 0, sizeof (*ofmem));

    if (is_wii_rvl()) {
        ofmem->ramsize = 0x4000000; // 64MB virtual.
    } else if (is_wii_cafe()) {
        ofmem->ramsize = 0x8000000; // 128MB virtual.
    }

    ofmem_claim_phys(0, FREE_BASE_1, 0);
    ofmem_claim_virt(0, FREE_BASE_1, 0);

    if (is_wii_rvl()) {
        ofmem_map(0x4000, 0x4000, 0x01800000 - 0x4000, 0);
        ofmem_map(0x11600000, 0x01800000, 0x02800000, 0);
    } else if (is_wii_cafe()) {
        ofmem_map(0x4000, 0x4000, 0x02000000 - 0x4000, -1);
        ofmem_map(0x11600000, 0x02000000, 0x06000000, -1);
    }
}
