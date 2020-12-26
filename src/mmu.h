#ifndef _MMU_H
#define _MMU_H

#ifdef __cplusplus
extern "C" {
#endif

void MmuSetupPagetable( void );

void MmuEnable( void );

void EnableMmuTables( void* map1to1, void* virtualmap );

#ifdef __cplusplus
}
#endif

#endif
