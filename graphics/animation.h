/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2004 The ScummVM project
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

#ifndef GRAPHICS_ANIMATION_H
#define GRAPHICS_ANIMATION_H

#include "common/scummsys.h"
#include "sound/mixer.h"

class AudioStream;

// Uncomment this if you are using libmpeg2 0.3.1.
// #define USE_MPEG2_0_3_1

#ifdef USE_MPEG2
typedef int8 int8_t;
typedef int16 int16_t;
typedef int32 int32_t;

typedef uint8 uint8_t;
typedef uint16 uint16_t;
typedef uint32 uint32_t; 

extern "C" {
	#include <mpeg2dec/mpeg2.h>
}

#ifdef USE_MPEG2_0_3_1
typedef int mpeg2_state_t;
typedef sequence_t mpeg2_sequence_t;
#define STATE_BUFFER -1
#endif

#endif

#ifdef BACKEND_8BIT
#define SQR(x) ((x) * (x))
#define SHIFT 3
#else
#define SHIFT 1
#endif

#define BITDEPTH (1 << (8 - SHIFT))
#define ROUNDADD (1 << (SHIFT - 1))

#define BUFFER_SIZE 4096


namespace Graphics {

class BaseAnimationState {
protected:
	const int MOVIE_WIDTH;
	const int MOVIE_HEIGHT;
	
	SoundMixer *_snd;
	OSystem *_sys;

	uint framenum;
	uint frameskipped;
	uint32 ticks;

#ifdef USE_MPEG2
	mpeg2dec_t *decoder;
	const mpeg2_info_t *info;
#endif

	File *mpgfile;

	byte buffer[BUFFER_SIZE];

	PlayingSoundHandle bgSound;
	AudioStream *bgSoundStream;

#ifdef BACKEND_8BIT
	int palnum;
	int maxPalnum;

	byte lookup[2][(BITDEPTH+1) * (BITDEPTH+1) * (BITDEPTH+1)];
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
	OverlayColor *overlay;
	int bitFormat;
	int16 *colortab;
	uint16 *rgb_2_pix;
#endif

public:
	BaseAnimationState(SoundMixer *snd, OSystem *sys, int width, int height);
	virtual ~BaseAnimationState();

	bool init(const char *name, void *audioArg = NULL);
	bool decodeFrame();

#ifndef BACKEND_8BIT
	void buildLookup();
#endif

protected:
	bool checkPaletteSwitch();
	virtual void drawYUV(int width, int height, byte *const *dat) = 0;
	virtual AudioStream *createAudioStream(const char *name, void *arg);

#ifdef BACKEND_8BIT
	void buildLookup(int p, int lines);
	virtual void setPalette(byte *pal) = 0;
#else
	void plotYUV(int width, int height, byte *const *dat);
#endif
};


} // End of namespace Graphics

#endif
