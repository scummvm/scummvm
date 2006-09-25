/*
	io_m3sd.h  by SaTa.

	Hardware Routines for reading an SD card
	using the M3 SD

	This software is completely free. No warranty is provided.
	If you use it, please give me credit and email me about your
	project at chishm@hotmail.com

	See gba_nds_fat.txt for help and license details.
*/

#ifndef IO_M3SD_H
#define IO_M3SD_H

// 'M3SD'
#define DEVICE_TYPE_M3SD 0x4453334D

#include "disc_io.h"

// export interface
extern LPIO_INTERFACE M3SD_GetInterface(void) ;

#endif	// define IO_M3SD_H
