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

//////////////////////////////////////////////////////////////////////
// Simple ARM7 stub (sends RTC, TSC, and X/Y data to the ARM 9)
// -- joat
// -- modified by Darkain and others
//////////////////////////////////////////////////////////////////////

#include <nds.h>

#include <bios.h>
#include <arm7/touch.h>
#include <arm7/clock.h>
#include <arm7/audio.h>
#include <system.h>
#include <stdlib.h>
#include <string.h>
#include <registers_alt.h> // Needed for SOUND_CR
#include <NDS/scummvm_ipc.h>
//////////////////////////////////////////////////////////////////////
#ifdef USE_DEBUGGER
#include <dswifi7.h>
#endif

vu8 *soundData;

vu8 *soundBuffer;
vu8 *arm9Buffer;
bool soundFilled[4];

int playingSection;

int temp;

int adpcmBufferNum = 0;

s8 getFreeSoundChannel() {
	for (int i = 0; i < 16; i++) {
		if ( (SCHANNEL_CR(i) & SCHANNEL_ENABLE) == 0 )
			return i;
	}
	return -1;
}

void startSound(int sampleRate, const void *data, uint32 bytes, u8 channel = 0, u8 vol = 0x7F, u8 pan = 63, u8 format = 0) {
	channel = getFreeSoundChannel();

	if (channel > 1)
		channel = 1;

	bytes &= ~7; // Multiple of 4 bytes!

	SCHANNEL_CR(channel) = 0;
	SCHANNEL_TIMER(channel)  = SOUND_FREQ(sampleRate);
	SCHANNEL_SOURCE(channel) = (uint32)data;
	SCHANNEL_LENGTH(channel) = (bytes & 0x7FFFFFFF) >> 2;
	SCHANNEL_REPEAT_POINT(channel) = 0;

	SCHANNEL_CR(channel + 2) = 0;
	SCHANNEL_TIMER(channel + 2)  = SOUND_FREQ(sampleRate);
	SCHANNEL_SOURCE(channel + 2) = (uint32)data;
	SCHANNEL_LENGTH(channel + 2) = (bytes & 0x7FFFFFFF) >> 2;
	SCHANNEL_REPEAT_POINT(channel + 2) = 0;

	uint32 flags = SCHANNEL_ENABLE | SOUND_VOL(vol) | SOUND_PAN(pan);

	switch (format) {
	case 1: {
		flags |= SOUND_FORMAT_8BIT;
		flags |= SOUND_REPEAT;
		break;
	}

	case 0: {
		flags |= SOUND_FORMAT_16BIT;
		flags |= SOUND_REPEAT;
		break;
	}

	case 2: {
		flags |= SOUND_FORMAT_ADPCM;
		flags |= SOUND_ONE_SHOT;

		SCHANNEL_SOURCE(channel) = (unsigned int)IPC->adpcm.buffer[0];
		SCHANNEL_LENGTH(channel) = ((bytes + 4) & 0x7FFFFFFF) >> 2;

		SCHANNEL_CR(channel + 1) = 0;
		SCHANNEL_SOURCE(channel + 1) = (unsigned int)IPC->adpcm.buffer[0];
		SCHANNEL_LENGTH(channel + 1) = ((bytes + 4) & 0x7FFFFFFF) >> 2;
		SCHANNEL_TIMER(channel + 1)  = SOUND_FREQ(sampleRate);
		SCHANNEL_REPEAT_POINT(channel + 1) = 0;
		SCHANNEL_CR(channel + 1) = flags;
		temp = bytes;
		adpcmBufferNum = 0;
		break;
	}
	}

	soundData = (vu8 *)data;

	SCHANNEL_CR(channel)     = flags;
	SCHANNEL_CR(channel + 2) = flags;

	if (channel == 0) {
		for (volatile int i = 0; i < 16384 * 2; i++) {
			// Delay loop - this makes everything stay in sync!
		}

		TIMER0_CR = 0;
		TIMER0_DATA = SOUND_FREQ(sampleRate) * 2;
		TIMER0_CR = TIMER_ENABLE | TIMER_DIV_1;

		TIMER1_CR = 0;
		TIMER1_DATA = 65536 - ((bytes & 0x7FFFFFFF) >> 3); // Trigger four times during the length of the buffer
		TIMER1_CR = TIMER_ENABLE | TIMER_IRQ_REQ | TIMER_CASCADE;

		playingSection = 0;
	} else {
		for (volatile int i = 0; i < 16384 * 2; i++) {
			// Delay loop - this makes everything stay in sync!
		}

		TIMER2_CR = 0;
		TIMER2_DATA = SOUND_FREQ(sampleRate) * 2;
		TIMER2_CR = TIMER_ENABLE | TIMER_DIV_1;

		TIMER3_CR = 0;
		TIMER3_DATA = 65536 - ((bytes & 0x7FFFFFFF) >> 3); // Trigger four times during the length of the buffer
		TIMER3_CR = TIMER_ENABLE | TIMER_IRQ_REQ | TIMER_CASCADE;

		IPC->streamPlayingSection = 0;
	}
}

void stopSound(int chan) {
	SCHANNEL_CR(chan) = 0;
}

void InterruptTimer1() {
	IPC->fillNeeded[playingSection] = true;
	soundFilled[playingSection] = false;

	if (playingSection == 3) {
		playingSection = 0;
	} else {
		playingSection++;
	}

	IPC->playingSection = playingSection;
}

void InterruptTimer3() {
	while (IPC->adpcm.semaphore); // Wait for buffer to become free if needed
	IPC->adpcm.semaphore = true; // Lock the buffer structure to prevent clashing with the ARM7

	IPC->streamFillNeeded[IPC->streamPlayingSection] = true;

	if (IPC->streamPlayingSection == 3) {
		IPC->streamPlayingSection = 0;
	} else {
		IPC->streamPlayingSection++;
	}

	IPC->adpcm.semaphore = false;
}

void VblankHandler() {
	// sound code  :)
	TransferSound *snd = IPC->soundData;
	IPC->soundData = 0;
	if (snd) {
		for (int i = 0; i < snd->count; i++) {
			s8 chan = getFreeSoundChannel();
			if (snd->data[i].rate > 0) {
				if (chan >= 0) {
					startSound(snd->data[i].rate, snd->data[i].data, snd->data[i].len, chan, snd->data[i].vol, snd->data[i].pan, snd->data[i].format);
				}
			} else {
				stopSound(-snd->data[i].rate);
			}
		}
	}
}

//---------------------------------------------------------------------------------
void VcountHandler() {
//---------------------------------------------------------------------------------
	inputGetAndSend();
}

volatile bool exitflag = false;

//---------------------------------------------------------------------------------
void powerButtonCB() {
//---------------------------------------------------------------------------------
	exitflag = true;
}

int main(int argc, char ** argv) {
	// enable sound
	SOUND_CR = SOUND_ENABLE | SOUND_VOL(0x7F);
	IPC->soundData = 0;

	for (int r = 0; r < 8; r++) {
		IPC->adpcm.arm7Buffer[r] = (u8 *)malloc(512);
	}

	for (int r = 0; r < 4; r++) {
		soundFilled[r] = false;
	}

	readUserSettings();
	ledBlink(0);

	irqInit();
	// Start the RTC tracking IRQ
	initClockIRQ();
	fifoInit();
	touchInit();

//	mmInstall(FIFO_MAXMOD);

	SetYtrigger(80);

//	installWifiFIFO();
//	installSoundFIFO();

	installSystemFIFO();

	irqSet(IRQ_VCOUNT, VcountHandler);
	irqSet(IRQ_VBLANK, VblankHandler);

	irqEnable(IRQ_VBLANK | IRQ_VCOUNT | IRQ_NETWORK);

	irqSet(IRQ_TIMER1, InterruptTimer1);
	irqEnable(IRQ_TIMER1);

	irqSet(IRQ_TIMER3, InterruptTimer3);
	irqEnable(IRQ_TIMER3);

	setPowerButtonCB(powerButtonCB);

	// Keep the ARM7 mostly idle
	while (!exitflag) {
		if ( 0 == (REG_KEYINPUT & (KEY_SELECT | KEY_START | KEY_L | KEY_R))) {
			exitflag = true;
		}

		swiWaitForVBlank();
	}

	return 0;
}
