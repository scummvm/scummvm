/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "lab/stddefines.h"

#include "lab/labfun.h"

namespace Lab {

#define PLAYBUFSIZE     65536L

#if defined(DOSCODE)
const char VERSION[] = "1.01";

char *AudioI_DriverName = "a32sbdg.dll";
#endif

extern bool MusicOn;

#if defined(DOSCODE)
static HTIMER server;
static HDRIVER hdriver;
static char *drvr;
static char *dll;
static drvr_desc *desc;
#endif
//static sound_buff firstblock, tempblock;
static int bufnum;
#if defined(DOSCODE)
static unsigned seg1;
static unsigned seg2;
static union REGS inregs, outregs;
#endif


bool EffectPlaying = false, ContMusic = false, DoMusic = false;
static char *CurMusic, *startMusic;
static uint32 StartMusicLen;


#if defined(DOSCODE)
static uint16 *mem, *head, *tail, counter;

#pragma off (check_stack)
void cdecl timer_callback(void) {

	head = (uint16 *)(0x41A);
	tail = (uint16 *)(0x41C);
	mem  = (uint16 *)(0x400 + *head);

	if (*tail > *head)
		counter = (*tail - *head) >> 1;
	else
		counter = (*head - *tail) >> 1;

	if (counter > 16)
		counter = 16;

	while (counter) {
		if ((*mem == 0x2e03) || (*mem == 0x300) || (*mem == 0x0003)) {
			*tail = *head;
			return;
		}

		mem++;
		counter--;
	}
}
#endif

void freeAudio(void) {
	if (!DoMusic)
		return;

#if defined(DOSCODE)
	AIL_release_timer_handle(server);

	AIL_shutdown(NULL);
#else
	//SDLWrapAudio();
#endif
}



bool initAudio(void) {
	if (!DoMusic)
		return true;

#if 0
#if defined(DOSCODE)

	AudioI_DriverName = "a32sbdg.dll";

	//
	// Allocate two 16K buffers from real-mode (lower 1MB) memory
	//
	// *buf1, *buf2 -> protected-mode pointers to buffers (sel:0000)
	// *seg1, *seg2 -> real-mode (physical) pointers to buffers (seg:0000)
	//
	// Note: DPMI calculations assume flat model near pointer offset 0 =
	// segment 0, offset 0 (Rational DOS4GW).  The reason -- our simple
	// file loader function can't use the far pointer formed by the selector
	// returned by the DPMI call.

	// Note that these examples do not implement out-of-memory error
	// checking
	//

	inregs.x.eax = 0x100;
	inregs.x.ebx = (16384 / 16);
	int386(0x31, &inregs, &outregs);

	seg1 = outregs.x.eax << 16;
	buf1 = (char *)(outregs.x.eax * 16);

	if (buf1 == NULL)
		return false;

	inregs.x.eax = 0x100;
	inregs.x.ebx = (16384 / 16);
	int386(0x31, &inregs, &outregs);

	seg2 = outregs.x.eax << 16;
	buf2 = (char *)(outregs.x.eax * 16);

	if (buf2 == NULL)
		return false;

	//
	// Load driver file
	//

	dll = FILE_read(AudioI_DriverName, NULL);

	if (dll == NULL) {
		return false;
	}

	drvr = DLL_load(dll, DLLMEM_ALLOC | DLLSRC_MEM, NULL);

	if (drvr == NULL) {
		return false;
	}

	free(dll);

	//
	// Initialize API before calling any Library functions
	//

	AIL_startup();

	hdriver = AIL_register_driver(drvr);

	if (hdriver == -1) {
		AIL_shutdown(NULL);
		return false;
	}

	//
	// Get driver type and factory default I/O parameters; exit if
	// driver is not capable of interpreting PCM sound data
	//

	desc = AIL_describe_driver(hdriver);

	if (desc->drvr_type != DSP_DRVR) {
		AIL_shutdown(NULL);
		return false;
	}

	if (!AIL_detect_device(hdriver, desc->default_IO, desc->default_IRQ, desc->default_DMA, desc->default_DRQ)) {
		desc->default_IRQ = 5;

		if (!AIL_detect_device(hdriver, desc->default_IO, desc->default_IRQ, desc->default_DMA, desc->default_DRQ)) {
			AIL_shutdown(NULL);
			return false;
		}
	}

	AIL_init_driver(hdriver, desc->default_IO, desc->default_IRQ, desc->default_DMA, desc->default_DRQ);

	//
	// Register a timer function; set up for 10-millisecond (100 Hz.)
	// callback intervals
	//

	server = AIL_register_timer(timer_callback);

	if (server != -1) {
		AIL_set_timer_period(server, 20000L);
		AIL_start_timer(server);
	}

#else
	// we allocate extra mempory for 16-bit samples
	buf1 = malloc(PLAYBUFSIZE);

	if (buf1 == NULL)
		return false;

	buf2 = malloc(PLAYBUFSIZE);

	if (buf2 == NULL)
		return false;

	if (!SDLInitAudio())
		return false;

#endif
#endif

	return true;
}


void initSampleRate(uint16 SampleSpeed) {
	flushAudio();

	if (SampleSpeed < 4000)
		SampleSpeed = 4000;

#if defined(DOSCODE)
	firstblock.sample_rate = 256 - (1000000L / SampleSpeed);
	firstblock.pack_type = 0 | 0x80;  // 8-bit mono sample
#else
	//firstblock.sample_rate = SampleSpeed;
	//firstblock.pack_type = AUDIO_S16; // SOUND_MONO | SOUND_16BIT;  // 16-bit mono sample
#endif
}




bool musicBufferEmpty(uint16 i) {
#if defined(NEWCODE)
	return (AIL_sound_buffer_status(hdriver, i) == DAC_DONE);
#else
	//return (SDLSoundBufferStatus(i) == DAC_DONE);
	return true;
#endif
}



void playMusicBlock(void *Ptr, uint32 Size, uint16 BufferNum, uint16 SampleSpeed) {
#if defined(DOSCODE)
	uint32 TempPtr;
	uint32 seg;
	char *buf;

	TempPtr = ((uint32) Ptr) / 16L;

	seg = TempPtr << 16;
	buf = (char *)(TempPtr * 16);

	if (SampleSpeed < 4000)
		SampleSpeed = 4000;

	firstblock.sample_rate = 256 - (1000000L / SampleSpeed);
	firstblock.pack_type = 0 | 0x80;  // 8-bit mono sample

	tempblock = firstblock;
	tempblock.sel_data = buf;
	tempblock.seg_data = seg;
	tempblock.len      = Size;

	AIL_register_sound_buffer(hdriver, BufferNum, &tempblock);
	AIL_format_sound_buffer(hdriver, &tempblock);

	AIL_start_digital_playback(hdriver);
	AIL_set_digital_playback_volume(hdriver, 127);
#else
#if 0

	if (SampleSpeed < 4000)
		SampleSpeed = 4000;

	firstblock.sample_rate = SampleSpeed;
	firstblock.pack_type = SOUND_MONO | SOUND_16BIT;  // 16-bit mono sample

	tempblock = firstblock;
	tempblock.sel_data = Ptr;
	tempblock.len      = Size;

	SDLPlayBuffer(BufferNum, &tempblock);
#endif
#endif
}


void updateSoundBuffers(void) {
	if (!DoMusic)
		return;

	if (!EffectPlaying)
		return;

#if defined(DOSCODE)

	for (int i = 0; i < 2; i++) {
		if ((AIL_sound_buffer_status(hdriver, i) == DAC_DONE) && firstblock.len) {
			tempblock.len = min(16384L, firstblock.len);
			firstblock.len -= tempblock.len;

			if (!(bufnum ^= 1)) {
				memcpy(buf1, CurMusic, (unsigned) tempblock.len);
				tempblock.sel_data = buf1;
				tempblock.seg_data = seg1;
			} else {
				memcpy(buf2, CurMusic, (unsigned) tempblock.len);
				tempblock.sel_data = buf2;
				tempblock.seg_data = seg2;
			}

			CurMusic += tempblock.len;

			AIL_register_sound_buffer(hdriver, i, &tempblock);
			AIL_format_sound_buffer(hdriver, &tempblock);

			AIL_start_digital_playback(hdriver);
			AIL_set_digital_playback_volume(hdriver, 127);
		}
	}

	//
	// Playback ends when no bytes are left in the source data and
	// the status of both buffers equals DAC_DONE
	//

	if (!firstblock.len) {
		if (ContMusic) {
			CurMusic = startMusic;
			firstblock.len = StartMusicLen;
		} else if ((AIL_sound_buffer_status(hdriver, 0) == DAC_DONE) &&
		           (AIL_sound_buffer_status(hdriver, 1) == DAC_DONE)) {
			flushAudio();
			EffectPlaying = false;
		}
	}

#else
#if 0

	for (int i = 0; i < 2; i++) {
		if ((SDLSoundBufferStatus(i) == DAC_DONE) && firstblock.len) {
			// use extra memory for 16-bit samples
			tempblock.len = min(PLAYBUFSIZE, firstblock.len);
			firstblock.len -= tempblock.len;

			if (!(bufnum ^= 1)) {
				memcpy(buf1, CurMusic, (unsigned) tempblock.len);
				tempblock.sel_data = buf1;
			} else {
				memcpy(buf2, CurMusic, (unsigned) tempblock.len);
				tempblock.sel_data = buf2;
			}

			CurMusic += tempblock.len;

			SDLPlayBuffer(i, &tempblock);
		}
	}

	//
	// Playback ends when no bytes are left in the source data and
	// the status of both buffers equals DAC_DONE
	//

	if (!firstblock.len) {
		if (ContMusic) {
			CurMusic = startMusic;
			firstblock.len = StartMusicLen;
		} else if ((SDLSoundBufferStatus(0) == DAC_DONE) &&
		           (SDLSoundBufferStatus(1) == DAC_DONE)) {
			flushAudio();
			EffectPlaying = false;
		}
	}

#endif
#endif
}



void flushAudio(void) {
	if (!DoMusic)
		return;

#if defined(DOSCODE)
	AIL_stop_digital_playback(hdriver);
#else
	//SDLStopPlayback();
#endif
	EffectPlaying = false;
}




void playMusic(uint16 SampleSpeed, uint16 Volume, uint32 Length, bool flush, void *Data) {
	if (!DoMusic)
		return;

	g_music->pauseBackMusic();

	if (flush)
		flushAudio();

	if (SampleSpeed < 4000)
		SampleSpeed = 4000;

#if defined(DOSCODE)
	firstblock.sample_rate = 256 - (1000000L / SampleSpeed);
	firstblock.pack_type = 0 | 0x80;  // 8-bit mono sample
#else
	//firstblock.sample_rate = SampleSpeed;
	//firstblock.pack_type = SOUND_MONO | SOUND_16BIT;  // 16-bit mono sample
#endif
	//firstblock.len = Length;
	bufnum = 0;

	//tempblock = firstblock;
	EffectPlaying = true;
	CurMusic = (char *)Data;
	startMusic = CurMusic;
	StartMusicLen = Length;

	updateSoundBuffers();
}

} // End of namespace Lab
