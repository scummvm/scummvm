/*
	io_dldi.h 

	Reserved space for new drivers
	
	This software is completely free. No warranty is provided.
	If you use it, please give me credit and email me about your
	project at chishm@hotmail.com

	See gba_nds_fat.txt for help and license details.
*/

#ifndef IO_DLDI_H
#define IO_DLDI_H

// 'DLDI'
#define DEVICE_TYPE_DLDD 0x49444C44

#include "disc_io.h"
#ifdef NDS
#include <nds/memory.h>
#endif

extern IO_INTERFACE _io_dldi;

// export interface
static inline LPIO_INTERFACE DLDI_GetInterface(void) {
#ifdef NDS
	WAIT_CR &= ~(ARM9_OWNS_ROM | ARM9_OWNS_CARD);
#endif // defined NDS
	return &_io_dldi;
}

#endif	// define IO_DLDI_H
