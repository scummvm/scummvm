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

#include "graphics/animation.h"

#include "sound/mixer.h"


namespace Sword2 {

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
	Sword2Engine *_vm;
	SoundMixer *_snd;
	OSystem *_sys;

	uint framenum;
	uint frameskipped;
	int ticks;

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
	static OverlayColor *lookup;
	OverlayColor *overlay;
#endif

public:

	AnimationState(Sword2Engine *vm);
	~AnimationState();

	bool init(const char *name);
	bool decodeFrame();

#ifndef BACKEND_8BIT
	void drawTextObject(SpriteInfo *s, uint8 *src);
	void clearDisplay();
	void updateDisplay(void);
#endif

private:

#ifdef BACKEND_8BIT
	void buildLookup(int p, int lines);
	bool checkPaletteSwitch();
	void setPalette(byte *pal);
#else
	void buildLookup(void);
	void plotYUV(OverlayColor *lut, int width, int height, byte *const *dat);
#endif
};

struct MovieInfo {
	char name[9];
	uint frames;
};
 
class MoviePlayer {
private:
	Sword2Engine *_vm;
	SoundMixer *_snd;
	OSystem *_sys;

	uint8 *_textSurface;

	static struct MovieInfo _movies[];

	void openTextObject(MovieTextObject *obj);
	void closeTextObject(MovieTextObject *obj);
	void drawTextObject(AnimationState *anim, MovieTextObject *obj);

	int32 playDummy(const char *filename, MovieTextObject *text[], uint8 *musicOut);

public:
	MoviePlayer(Sword2Engine *vm);
	int32 play(const char *filename, MovieTextObject *text[], uint8 *musicOut);
};

} // End of namespace Sword2

#endif
