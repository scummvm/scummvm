/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef ANIMATION_H
#define ANIMATION_H

// Uncomment this if you are using libmpeg2 0.3.1.
// #define USE_MPEG2_0_3_1

#ifdef _MSC_VER
typedef int8 int8_t;
typedef int16 int16_t;
typedef int32 int32_t;

typedef uint8 uint8_t;
typedef uint16 uint16_t;
typedef uint32 uint32_t; 
#else
#include <inttypes.h>
#endif

#ifdef USE_MPEG2
extern "C" {
	#include <mpeg2dec/mpeg2.h>
}

#ifdef USE_MPEG2_0_3_1
typedef int mpeg2_state_t;
typedef sequence_t mpeg2_sequence_t;
#define STATE_BUFFER -1
#endif

#endif

#include "sword1/screen.h"
#include "sword1/sound.h"

#include "sound/audiostream.h"

namespace Sword1 {


#ifdef BACKEND_8BIT
#define SQR(x) ((x) * (x))
#define SHIFT 3
#else
#define SHIFT 1
#endif

#define BITDEPTH (1 << (8 - SHIFT))
#define ROUNDADD (1 << (SHIFT - 1))

#define BUFFER_SIZE 4096

class AnimationState {
private:
	Screen *_scr;
	SoundMixer *_snd;
	OSystem *_sys;

	uint framenum;
	uint32 ticks;

#ifdef USE_MPEG2
	mpeg2dec_t *decoder;
	const mpeg2_info_t *info;
#endif

	File *mpgfile;
	File *sndfile;

	byte buffer[BUFFER_SIZE];

	PlayingSoundHandle bgSound;
	AudioStream *bgSoundStream;

#ifdef BACKEND_8BIT
	int palnum;
	int maxPalnum;

	byte lookup[2][BITDEPTH * BITDEPTH * BITDEPTH];
	byte *lut;
	byte *lut2;
	int lutcalcnum;

	int curpal;
	int cr;
	int pos;

	struct {
		uint cnt;
		uint end;
		byte pal[4 * 256];
	} palettes[50];
#else
	static NewGuiColor *lookup;
	NewGuiColor * overlay;
#endif

public:

	AnimationState(Screen *scr, SoundMixer *snd, OSystem *sys);
	~AnimationState();

	bool init(const char *name);
	bool decodeFrame();

private:

#ifdef BACKEND_8BIT
	void buildLookup(int p, int lines);
	bool checkPaletteSwitch();
#else
	void buildLookup(void);
	void plotYUV(NewGuiColor *lut, int width, int height, byte *const *dat);
#endif
};

class MoviePlayer {
private:
	Screen *_scr;
	SoundMixer *_snd;
	OSystem *_sys;

public:
	MoviePlayer(Screen *scr, SoundMixer *snd, OSystem *sys) : _scr(scr), _snd(snd), _sys(sys) {}
	void play(const char *filename);
};

} // End of namespace Sword2

#endif
