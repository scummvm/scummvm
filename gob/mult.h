/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */
#ifndef GOB_MULT_H
#define GOB_MULT_H

#include "gob/sound.h"

namespace Gob {

class Mult {
public:
#pragma START_PACK_STRUCTS
	struct Mult_AnimData {
		int8 animation;
		int8 layer;
		int8 frame;
		int8 animType;
		int8 order;
		int8 isPaused;
		int8 isStatic;
		int8 maxTick;
		int8 unknown;
		int8 newLayer;
		int8 newAnimation;
		byte intersected;
		int8 newCycle;
	} GCC_PACK;

	struct Mult_Object {
		int32 *pPosX;
		int32 *pPosY;
		Mult_AnimData *pAnimData;
		int16 tick;
		int16 lastLeft;
		int16 lastRight;
		int16 lastTop;
		int16 lastBottom;
	};

	struct Mult_StaticKey {
		int16 frame;
		int16 layer;
	} GCC_PACK;

	struct Mult_AnimKey {
		int16 frame;
		int16 layer;
		int16 posX;
		int16 posY;
		int16 order;
	} GCC_PACK;

	struct Mult_TextKey {
		int16 frame;
		int16 cmd;
		int16 unknown0[9];
		int16 index;
		int16 unknown1[2];
	} GCC_PACK;

	struct Mult_PalKey {
		int16 frame;
		int16 cmd;
		int16 rates[4];
		int16 unknown0;
		int16 unknown1;
		int8 subst[16][4];
	} GCC_PACK;

	struct Mult_PalFadeKey {
		int16 frame;
		int16 fade;
		int16 palIndex;
		int8 flag;
	} GCC_PACK;

	struct Mult_SndKey {
		int16 frame;
		int16 cmd;
		int16 freq;
		int16 channel;
		int16 repCount;
		int16 resId;
		int16 soundIndex;
	} GCC_PACK;
#pragma END_PACK_STRUCTS

	// Globals

	Mult_Object *_objects;
	int16 *_renderData;
	int16 _objCount;
	Video::SurfaceDesc *_underAnimSurf;

	char *_multData;
	int16 _frame;
	char _doPalSubst;
	int16 _counter;
	int16 _frameRate;

	int32 *_animArrayX;
	int32 *_animArrayY;

	Mult_AnimData *_animArrayData;

	int16 _index;

	// Static keys
	int16 _staticKeysCount;
	Mult_StaticKey *_staticKeys;
	int16 _staticIndices[10];

	// Anim keys
	Mult_AnimKey *_animKeys[4];
	int16 _animKeysCount[4];
	int16 _animLayer;
	int16 _animIndices[10];

	// Text keys
	int16 _textKeysCount;
	Mult_TextKey *_textKeys;

	int16 _frameStart;

	// Palette keys
	int16 _palKeyIndex;
	int16 _palKeysCount;
	Mult_PalKey *_palKeys;
	Video::Color *_oldPalette;
	Video::Color _palAnimPalette[256];
	int16 _palAnimKey;
	int16 _palAnimIndices[4];
	int16 _palAnimRed[4];
	int16 _palAnimGreen[4];
	int16 _palAnimBlue[4];

	// Palette fading
	Mult_PalFadeKey *_palFadeKeys;
	int16 _palFadeKeysCount;
	char _palFadingRed;
	char _palFadingGreen;
	char _palFadingBlue;

	char _animDataAllocated;

	char *_dataPtr;
	int16 _staticLoaded[10];
	int16 _animLoaded[10];
	int16 _sndSlotsCount;

	// Sound keys
	int16 _sndKeysCount;
	Mult_SndKey *_sndKeys;

	void zeroMultData(void);
	void loadMult(int16 resId);
	void freeMultKeys(void);
	void checkFreeMult(void);
	void playMult(int16 startFrame, int16 endFrame, char checkEscape,
				  char handleMouse);
	void animate(void);
	void interGetObjAnimSize(void);
	void interInitMult(void);
	void freeMult(void);
	void interLoadMult(void);
	void freeAll(void);
	void initAll(void);
	void playSound(Snd::SoundDesc * soundDesc, int16 repCount, int16 freq,
				   int16 channel);

	Mult(GobEngine *vm);

protected:
	Video::Color _fadePal[5][16];
	GobEngine *_vm;

	char drawStatics(char stop);
	void drawAnims(void);
	void drawText(char *pStop, char *pStopNoClear);
	char prepPalAnim(char stop);
	void doPalAnim(void);
	char doFadeAnim(char stop);
	char doSoundAnim(char stop);
};

}				// End of namespace Gob

#endif	/* __MULT_H */
