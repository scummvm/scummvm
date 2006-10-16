/*
	io_m3sd.c 

	Hardware Routines for reading a Secure Digital card
	using the M3 SD
	
	Some code based on M3 SD drivers supplied by M3Adapter.
	Some code written by SaTa may have been unknowingly used.

 Copyright (c) 2006 Michael "Chishm" Chisholm
	
 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.
  3. The name of the author may not be used to endorse or promote products derived
     from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
	2006-07-25 - Chishm
		* Improved startup function that doesn't delay hundreds of seconds
		  before reporting no card inserted.
		* Fixed writeData function to timeout on error
		* writeSectors function now wait until the card is ready before continuing with a transfer

	2006-08-05 - Chishm
		* Tries multiple times to get a Relative Card Address at startup
		
	2006-08-07 - Chishm
		* Moved the SD initialization to a common function
*/

#include "io_m3sd.h"
#include "io_sd_common.h"
#include "io_m3_common.h"
//#include "common.h"
#include "disc_io.h"
#include <stdio.h>

#define BYTES_PER_READ 512

//---------------------------------------------------------------
// M3SD register addresses

#define REG_M3SD_DIR	(*(vu16*)0x08800000)	// direction control register
#define REG_M3SD_DAT	(*(vu16*)0x09000000)	// SD data line, 8 bits at a time
#define REG_M3SD_CMD	(*(vu16*)0x09200000)	// SD command byte
#define REG_M3SD_ARGH	(*(vu16*)0x09400000)	// SD command argument, high halfword
#define REG_M3SD_ARGL	(*(vu16*)0x09600000)	// SD command argument, low halfword
#define REG_M3SD_STS	(*(vu16*)0x09800000)	// command and status register

//---------------------------------------------------------------
// Send / receive timeouts, to stop infinite wait loops
#define NUM_STARTUP_CLOCKS 100	// Number of empty (0xFF when sending) bytes to send/receive to/from the card
#define TRANSMIT_TIMEOUT 20000	// Time to wait for the M3 to respond to transmit or receive requests
#define RESPONSE_TIMEOUT 256	// Number of clocks sent to the SD card before giving up
#define WRITE_TIMEOUT	3000	// Time to wait for the card to finish writing

//---------------------------------------------------------------
// Variables required for tracking SD state
static u32 _M3SD_relativeCardAddress = 0;	// Preshifted Relative Card Address

//---------------------------------------------------------------
// Internal M3 SD functions

static inline void _M3SD_unlock (void) {
	_M3_changeMode (M3_MODE_MEDIA);
}

static inline bool _M3SD_waitOnBusy (void) {
	int i = 0;
	while ( (REG_M3SD_STS & 0x01) == 0x00) {
		i++;
		if (i >= TRANSMIT_TIMEOUT) {
			return false;
		}
	}
	return true;
}

static inline bool _M3SD_waitForDataReady (void) {
	int i = 0;
	while ( (REG_M3SD_STS & 0x40) == 0x00) {
		i++;
		if (i >= TRANSMIT_TIMEOUT) {
			return false;
		}
	}
	return true;
}


static bool _M3SD_sendCommand (u16 command, u32 argument) {
	REG_M3SD_STS = 0x8;
	REG_M3SD_CMD = 0x40 + command;		// Include the start bit
	REG_M3SD_ARGH = argument >> 16;
	REG_M3SD_ARGL = argument;
	// The CRC7 of the command is calculated by the M3
	
	REG_M3SD_DIR=0x29;
	if (!_M3SD_waitOnBusy()) {
		REG_M3SD_DIR=0x09;
		return false;
	}
	REG_M3SD_DIR=0x09;
	return true;
}

static bool _M3SD_sendByte (u8 byte) {
	int i = 0;
	REG_M3SD_DAT = byte;
	REG_M3SD_DIR = 0x03;
	REG_M3SD_STS = 0x01;
	while ((REG_M3SD_STS & 0x04) == 0) {
		i++;
		if (i >= TRANSMIT_TIMEOUT) {
			return false;
		}
	}
	return true;
}

static u8 _M3SD_getByte (void) {
	int i;
	// Request 8 bits of data from the SD's CMD pin
	REG_M3SD_DIR = 0x02;
	REG_M3SD_STS = 0x02;
	// Wait for the data to be ready
	i = 0;
	while ((REG_M3SD_STS & 0x08) == 0) {
		i++;
		if (i >= TRANSMIT_TIMEOUT) {
			// Return an empty byte if a timeout occurs
			return 0xFF;
		}
	}
	i = 0;
	while ((REG_M3SD_STS & 0x08) != 0) {
		i++;
		if (i >= TRANSMIT_TIMEOUT) {
			// Return an empty byte if a timeout occurs
			return 0xFF;
		}
	}
	// Return the data
	return (REG_M3SD_DAT & 0xff);
}

// Returns the response from the SD card to a previous command.
static bool _M3SD_getResponse (u8* dest, u32 length) {
	u32 i;	
	u8 dataByte;
	int shiftAmount;
	
	// Wait for the card to be non-busy
	for (i = 0; i < RESPONSE_TIMEOUT; i++) {
		dataByte = _M3SD_getByte();
		if (dataByte != SD_CARD_BUSY) {
			break;
		}
	}
	
	if (dest == NULL) {
		return true;
	}
	
	// Still busy after the timeout has passed
	if (dataByte == 0xff) {
		return false;
	}
	
	// Read response into buffer
	for ( i = 0; i < length; i++) {
		dest[i] = dataByte;
		dataByte = _M3SD_getByte();
	}
	// dataByte will contain the last piece of the response
	
	// Send 16 more clocks, 8 more than the delay required between a response and the next command
	i = _M3SD_getByte();
	i = _M3SD_getByte();
	
	// Shift response so that the bytes are correctly aligned
	// The register may not contain properly aligned data
	for (shiftAmount = 0; ((dest[0] << shiftAmount) & 0x80) != 0x00; shiftAmount++) {
		if (shiftAmount >= 7) {
			return false;
		}
	}
	
	for (i = 0; i < length - 1; i++) {
		dest[i] = (dest[i] << shiftAmount) | (dest[i+1] >> (8-shiftAmount));
	}
	// Get the last piece of the response from dataByte
	dest[i] = (dest[i] << shiftAmount) | (dataByte >> (8-shiftAmount));

	return true;
}


static inline bool _M3SD_getResponse_R1 (u8* dest) {
	return _M3SD_getResponse (dest, 6);
}

static inline bool _M3SD_getResponse_R1b (u8* dest) {
	return _M3SD_getResponse (dest, 6);
}

static inline bool _M3SD_getResponse_R2 (u8* dest) {
	return _M3SD_getResponse (dest, 17);
}

static inline bool _M3SD_getResponse_R3 (u8* dest) {
	return _M3SD_getResponse (dest, 6);
}

static inline bool _M3SD_getResponse_R6 (u8* dest) {
	return _M3SD_getResponse (dest, 6);
}

static void _M3SD_sendClocks (u32 numClocks) {
	while (numClocks--) {
		_M3SD_sendByte(0xff);
	}
}

static void _M3SD_getClocks (u32 numClocks) {
	while (numClocks--) {
		_M3SD_getByte();
	}
}

bool _M3SD_cmd_6byte_response (u8* responseBuffer, u8 command, u32 data) {
	_M3SD_sendCommand (command, data);
	return _M3SD_getResponse (responseBuffer, 6);
}

bool _M3SD_cmd_17byte_response (u8* responseBuffer, u8 command, u32 data) {
	_M3SD_sendCommand (command, data);
	return _M3SD_getResponse (responseBuffer, 17);
}

static bool _M3SD_initCard (void) {
	// Give the card time to stabilise
	_M3SD_sendClocks (NUM_STARTUP_CLOCKS);
	
	// Reset the card
	if (!_M3SD_sendCommand (GO_IDLE_STATE, 0)) {
		return false;
	}

	_M3SD_getClocks (NUM_STARTUP_CLOCKS);

	// Card is now reset, including it's address
	_M3SD_relativeCardAddress = 0;
	
	// Init the card
	return _SD_InitCard (_M3SD_cmd_6byte_response, 
				_M3SD_cmd_17byte_response,
				true,
				&_M3SD_relativeCardAddress);
}

static bool _M3SD_readData (void* buffer) {
	u32 i;
	u8* buff_u8 = (u8*)buffer;
	u16* buff = (u16*)buffer;
	u16 temp;

	REG_M3SD_DIR = 0x49;
	if (!_M3SD_waitForDataReady()) {
		REG_M3SD_DIR = 0x09;
		return false;
	}
	REG_M3SD_DIR = 0x09;
	
	REG_M3SD_DIR =  0x8;
	REG_M3SD_STS = 0x4;
	
	i = REG_M3SD_DIR;
	// Read data
	i=256;
	if ((u32)buff_u8 & 0x01) {
		while(i--)
		{
			temp = REG_M3SD_DIR;
			*buff_u8++ = temp & 0xFF;
			*buff_u8++ = temp >> 8;
		}
	} else {
		while(i--)
			*buff++ = REG_M3SD_DIR; 
	}
	// Read end checksum
	i = REG_M3SD_DIR + REG_M3SD_DIR + REG_M3SD_DIR + REG_M3SD_DIR;
	
	return true;
}

static void _M3SD_clkout (void) {
	REG_M3SD_DIR = 0x4;
	REG_M3SD_DIR = 0xc;
/*	__asm volatile (
	"ldr 	r1, =0x08800000		\n"	
	"mov 	r0, #0x04			\n"
	"strh	r0, [r1]			\n"
	"mov	r0, r0				\n"
	"mov	r0, r0				\n"
	"mov    r0, #0x0c			\n"
	"strh	r0, [r1]			\n"
	:					// Outputs
	:					// Inputs
	: "r0", "r1"		// Clobber list
	);*/
}

static void _M3SD_clkin (void) {
	REG_M3SD_DIR = 0x0;
	REG_M3SD_DIR = 0x8;
/*	__asm volatile (
	"ldr 	r1, =0x08800000		\n"	
	"mov 	r0, #0x00			\n"
	"strh	r0, [r1]			\n"
	"mov	r0, r0				\n"
	"mov	r0, r0				\n"
	"mov    r0, #0x08			\n"
	"strh	r0, [r1]			\n"
	:					// Outputs
	:					// Inputs
	: "r0", "r1"		// Clobber list
	);*/
}

static bool _M3SD_writeData (u8* data, u8* crc) {
	int i;
	u8 temp;

	do {
		_M3SD_clkin();
	} while ((REG_M3SD_DAT & 0x100) == 0);
	
	REG_M3SD_DAT = 0;	// Start bit
	
	_M3SD_clkout();
	
	for (i = 0; i < BYTES_PER_READ; i++) {
		temp = (*data++);
		REG_M3SD_DAT = temp >> 4;
		_M3SD_clkout();
		REG_M3SD_DAT = temp;
		_M3SD_clkout();
	}
	
	if (crc != NULL) {
		for (i = 0; i < 8; i++) {
			temp = (*crc++);
			REG_M3SD_DAT = temp >> 4;
			_M3SD_clkout();
			REG_M3SD_DAT = temp;
			_M3SD_clkout();
		}
	}

	i = 32;
	while (i--) {
		temp += 2;		// a NOP to stop the compiler optimising out the loop
	}
	
	for (i = 0; i  < 32; i++) {
		REG_M3SD_DAT = 0xff;
		_M3SD_clkout();
	}
	
	do {
		_M3SD_clkin();
	} while ((REG_M3SD_DAT & 0x100) == 0);
	
	return true;
}

//---------------------------------------------------------------
// Functions needed for the external interface

bool _M3SD_startUp (void) {
	_M3SD_unlock();
	return _M3SD_initCard();
}

bool _M3SD_isInserted (void) {
	u8 responseBuffer [6];
	// Make sure the card receives the command
	if (!_M3SD_sendCommand (SEND_STATUS, 0)) {
		return false;
	}
	// Make sure the card responds
	if (!_M3SD_getResponse_R1 (responseBuffer)) {
		return false;
	}
	// Make sure the card responded correctly
	if (responseBuffer[0] != SEND_STATUS) {
		return false;
	}
	return true;
}

bool _M3SD_readSectors (u32 sector, u32 numSectors, void* buffer) {
	u32 i;
	u8* dest = (u8*) buffer;
	u8 responseBuffer[6];
	
	if (numSectors == 1) {
		// If it's only reading one sector, use the (slightly faster) READ_SINGLE_BLOCK
		if (!_M3SD_sendCommand (READ_SINGLE_BLOCK, sector * BYTES_PER_READ)) {
			return false;
		}

		if (!_M3SD_readData (buffer)) {
			return false;
		}

	} else {
		// Stream the required number of sectors from the card
		if (!_M3SD_sendCommand (READ_MULTIPLE_BLOCK, sector * BYTES_PER_READ)) {
			return false;
		}
	
		for(i=0; i < numSectors; i++, dest+=BYTES_PER_READ) {
			if (!_M3SD_readData(dest)) {
				return false;
			}
			REG_M3SD_STS = 0x8;
		}
	
		// Stop the streaming
		_M3SD_sendCommand (STOP_TRANSMISSION, 0);
		_M3SD_getResponse_R1b (responseBuffer);
	}

	return true;
}

bool _M3SD_writeSectors (u32 sector, u32 numSectors, const void* buffer) {
	u8 crc[8];
	u8 responseBuffer[6];
	u32 offset = sector * BYTES_PER_READ;
	u8* data = (u8*) buffer;
	int i;
	// Precalculate the data CRC
	_SD_CRC16 ( data, BYTES_PER_READ, crc);
	
	while (numSectors--) {
		// Send a single sector write command
		_M3SD_sendCommand (WRITE_BLOCK, offset);
		if (!_M3SD_getResponse_R1 (responseBuffer)) {
			return false;
		}
	
		REG_M3SD_DIR = 0x4;
		REG_M3SD_STS = 0x0;
	
		// Send the data
		if (! _M3SD_writeData( data, crc)) {
			return false;
		}
		
		if (numSectors > 0) {
			offset += BYTES_PER_READ;
			data += BYTES_PER_READ;
			// Calculate the next CRC while waiting for the card to finish writing
			_SD_CRC16 ( data, BYTES_PER_READ, crc);
		}
		
		// Wait for the card to be ready for the next transfer
		i = WRITE_TIMEOUT;
		responseBuffer[3] = 0;
		do {
			_M3SD_sendCommand (SEND_STATUS, _M3SD_relativeCardAddress);
			_M3SD_getResponse_R1 (responseBuffer);
			i--;
			if (i <= 0) {
				return false;
			}
		} while (((responseBuffer[3] & 0x1f) != ((SD_STATE_TRAN << 1) | READY_FOR_DATA)));
	}
	
	return true;

}

bool _M3SD_clearStatus (void) {
	return _M3SD_initCard ();
}

bool _M3SD_shutdown (void) {
	_M3_changeMode (M3_MODE_ROM);
	return true;
}

IO_INTERFACE _io_m3sd = {
	DEVICE_TYPE_M3SD,
	FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_SLOT_GBA,
	(FN_MEDIUM_STARTUP)&_M3SD_startUp,
	(FN_MEDIUM_ISINSERTED)&_M3SD_isInserted,
	(FN_MEDIUM_READSECTORS)&_M3SD_readSectors,
	(FN_MEDIUM_WRITESECTORS)&_M3SD_writeSectors,
	(FN_MEDIUM_CLEARSTATUS)&_M3SD_clearStatus,
	(FN_MEDIUM_SHUTDOWN)&_M3SD_shutdown
} ;

LPIO_INTERFACE M3SD_GetInterface(void) {
	return &_io_m3sd ;
} ;

