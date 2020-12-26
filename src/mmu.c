#include <stddef.h>
#include <stdint.h>
#include "mmu.h"

/* Raspberry Pi zero memory size less VC size */
#define PHYSICAL_MEM_MAX ((512*1024*1024) - 65536)

#define MT_DEVICE_NS  0x10412   // device no share (strongly ordered)
#define MT_DEVICE     0x10416   // device + shareable
#define MT_NORMAL     0x1040E   // normal cache + shareable
#define MT_NORMAL_NS  0x1040A   // normal cache no share
#define MT_NORMAL_XN  0x1041E   // normal cache + shareable and execute never

/* We have 1MB blocks, with 4096 entries
 * Covers 4GB of memory
 */
#define PAGE_TABLE_ENTRIES 4096
/* Each Block is 1Mb in size */
#define LEVEL1_BLOCKSIZE (1 << 20)

/* LEVEL1 TABLE ALIGNMENT 16K */
#define TLB_ALIGNMENT 16384

/*
 * PRIVATE INTERNAL MEMEOY DATA
 */
/* First Level Page Table for 1:1 mapping */
static size_t __attribute__((aligned(TLB_ALIGNMENT))) pageTableMap1to1[ PAGE_TABLE_ENTRIES ] = { 0 };
/* First Level Page Table for virtual mapping */
static size_t __attribute__((aligned(TLB_ALIGNMENT))) pageTableVirtualmap[ PAGE_TABLE_ENTRIES ] = { 0 };


/**
 * Sets up a default TLB table.
 *
 * This needs to be called by only once by one core on multi-core systems.
 * Each core can use the same default table.
 */
void MmuSetupPagetable (void)
{
    uint32_t base = 0;
    /* Set 1:1 mapping for all memory */

    /* APX = 0      AP[1:0] = b11 so read/write for privilege and user and cache_writeback */
    for( ; base < (PHYSICAL_MEM_MAX / LEVEL1_BLOCKSIZE); base++)
    {
        pageTableMap1to1[ base ] = base << 20 | MT_NORMAL;
    }

    /* Set some no cache strong order default values for rest of 4GB */
    for ( ; base < PAGE_TABLE_ENTRIES; base++ )
    {
        pageTableMap1to1[ base ] = base << 20 | MT_DEVICE_NS;
    }
}

/**
 * Enables the MMU system to the previously created TLB tables.
 */
void MmuEnable( void )
{
    EnableMmuTables( &pageTableMap1to1[ 0 ], &pageTableVirtualmap[ 0 ]);
}
