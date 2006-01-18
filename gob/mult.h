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

#pragma START_PACK_STRUCTS
typedef struct Mult_AnimData {
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
} GCC_PACK Mult_AnimData;

typedef struct Mult_Object {
	int32 *pPosX;
	int32 *pPosY;
	Mult_AnimData *pAnimData;
	int16 tick;
	int16 lastLeft;
	int16 lastRight;
	int16 lastTop;
	int16 lastBottom;
} Mult_Object;

// Mult
typedef struct Mult_StaticKey {
	int16 frame;
	int16 layer;
} GCC_PACK Mult_StaticKey;

typedef struct Mult_AnimKey {
	int16 frame;
	int16 layer;
	int16 posX;
	int16 posY;
	int16 order;
} GCC_PACK Mult_AnimKey;

typedef struct Mult_TextKey {
	int16 frame;
	int16 cmd;
	int16 unknown0[9];
	int16 index;
	int16 unknown1[2];
} GCC_PACK Mult_TextKey;

typedef struct Mult_PalKey {
	int16 frame;
	int16 cmd;
	int16 rates[4];
	int16 unknown0;
	int16 unknown1;
	int8 subst[16][4];
} GCC_PACK Mult_PalKey;

typedef struct Mult_PalFadeKey {
	int16 frame;
	int16 fade;
	int16 palIndex;
	int8 flag;
} GCC_PACK Mult_PalFadeKey;

typedef struct Mult_SndKey {
	int16 frame;
	int16 cmd;
	int16 freq;
	int16 channel;
	int16 repCount;
	int16 resId;
	int16 soundIndex;
} GCC_PACK Mult_SndKey;
#pragma END_PACK_STRUCTS

// Globals

extern Mult_Object *mult_objects;
extern int16 *mult_renderData;
extern int16 mult_objCount;
extern SurfaceDesc *mult_underAnimSurf;

extern char *mult_multData;
extern int16 mult_frame;
extern char mult_doPalSubst;
extern int16 mult_counter;
extern int16 mult_frameRate;

extern int32 *mult_animArrayX;
extern int32 *mult_animArrayY;

extern Mult_AnimData *mult_animArrayData;

extern int16 mult_index;

// Static keys
extern int16 mult_staticKeysCount;
extern Mult_StaticKey *mult_staticKeys;
extern int16 mult_staticIndices[10];

// Anim keys
extern Mult_AnimKey *mult_animKeys[4];
extern int16 mult_animKeysCount[4];
extern int16 mult_animLayer;
extern int16 mult_animIndices[10];

// Text keys
extern int16 mult_textKeysCount;
extern Mult_TextKey *mult_textKeys;

extern int16 mult_frameStart;

// Palette keys
extern int16 mult_palKeyIndex;
extern int16 mult_palKeysCount;
extern Mult_PalKey *mult_palKeys;
extern Color *mult_oldPalette;
extern Color mult_palAnimPalette[256];
extern int16 mult_palAnimKey;
extern int16 mult_palAnimIndices[4];
extern int16 mult_palAnimRed[4];
extern int16 mult_palAnimGreen[4];
extern int16 mult_palAnimBlue[4];

// Palette fading
extern Mult_PalFadeKey *mult_palFadeKeys;
extern int16 mult_palFadeKeysCount;
extern char mult_palFadingRed;
extern char mult_palFadingGreen;
extern char mult_palFadingBlue;

extern char mult_animDataAllocated;

extern char *mult_dataPtr;
extern int16 mult_staticLoaded[10];
extern int16 mult_animLoaded[10];
extern int16 mult_sndSlotsCount;

// Sound keys
extern int16 mult_sndKeysCount;
extern Mult_SndKey *mult_sndKeys;

void mult_zeroMultData(void);
void mult_loadMult(int16 resId);
void mult_freeMultKeys(void);
void mult_checkFreeMult(void);
void mult_playMult(int16 startFrame, int16 endFrame, char checkEscape,
    char handleMouse);
void mult_animate(void);
void mult_interGetObjAnimSize(void);
void mult_interInitMult(void);
void mult_freeMult(void);
void mult_interLoadMult(void);
void mult_freeAll(void);
void mult_initAll(void);
void mult_playSound(Snd_SoundDesc * soundDesc, int16 repCount, int16 freq,
    int16 channel);
void mult_playMult(int16 startFrame, int16 endFrame, char checkEscape,
    char handleMouse);
void mult_zeroMultData(void);
void mult_loadMult(int16 resId);
void mult_freeMultKeys(void);
void mult_checkFreeMult(void);

}				// End of namespace Gob

#endif	/* __MULT_H */
