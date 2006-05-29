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
 * $URL$
 * $Id$
 *
 */
#ifndef GOB_MULT_H
#define GOB_MULT_H

#include "gob/gob.h"
#include "gob/sound.h"
#include "gob/video.h"
#include "gob/goblin.h"

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
		int8 somethingAnimation; // New in GOB2
		int8 somethingLayer;     // New in GOB2
		int8 somethingFrame;     // New in GOB2
		int8 someFlag;           // New in GOB2
		int8 state;              // New in GOB2
		int8 field_E;            // New in GOB2
		int8 field_F;            // New in GOB2
		int8 field_10;           // New in GOB2
		int8 field_12;           // New in GOB2
		int8 field_13;           // New in GOB2
		int8 field_14;           // New in GOB2
		int8 field_15;           // New in GOB2
		int8 field_17;           // New in GOB2
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
		int8 someFlag;                     // New in GOB2
		int16 somethingLeft;               // New in GOB2
		int16 somethingTop;                // New in GOB2
		int16 somethingRight;              // New in GOB2
		int16 somethingBottom;             // New in GOB2
		int8 goblinX;                      // New in GOB2
		int8 goblinY;                      // New in GOB2
		int8 destX;                        // New in GOB2
		int8 destY;                        // New in GOB2
		int8 gobDestX;                     // New in GOB2
		int8 gobDestY;                     // New in GOB2
		int8 nearestWayPoint;              // New in GOB2
		int8 nearestDest;                  // New in GOB2
		Goblin::Gob2_State **goblinStates; // New in GOB2
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
	Mult_Object **_renderData2;
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

	int8 *_orderArray;

	void zeroMultData(void);
	void checkFreeMult(void);
	void interGetObjAnimSize(void);
	void freeMult(void);
	void freeAll(void);
	void initAll(void);

	virtual void setMultData(uint16 multindex) = 0;
	virtual void multSub(uint16 multindex) = 0;
	virtual void loadMult(int16 resId) = 0;
	virtual void playMult(int16 startFrame, int16 endFrame, char checkEscape,
				  char handleMouse) = 0;
	virtual void animate(void) = 0;
	virtual void playSound(Snd::SoundDesc * soundDesc, int16 repCount,
				  int16 freq, int16 channel) = 0;
	virtual void freeMultKeys(void) = 0;

	Mult(GobEngine *vm);
	virtual ~Mult() {};

protected:
	Video::Color _fadePal[5][16];
	GobEngine *_vm;

	virtual char drawStatics(char stop) = 0;
	virtual char drawAnims(char stop) = 0;
	virtual void drawText(char *pStop, char *pStopNoClear) = 0;
	virtual char prepPalAnim(char stop) = 0;
	virtual void doPalAnim(void) = 0;
	virtual char doFadeAnim(char stop) = 0;
	virtual char doSoundAnim(char stop, int16 frame) = 0;
};

class Mult_v1 : public Mult {
public:
	Mult_v1(GobEngine *vm);
	virtual ~Mult_v1() {};

	virtual void setMultData(uint16 multindex);
	virtual void multSub(uint16 multindex);
	virtual void loadMult(int16 resId);
	virtual void playMult(int16 startFrame, int16 endFrame, char checkEscape,
				  char handleMouse);
	virtual void animate(void);
	virtual void playSound(Snd::SoundDesc * soundDesc, int16 repCount,
				  int16 freq, int16 channel);
	virtual void freeMultKeys(void);

protected:
	virtual char drawStatics(char stop);
	virtual char drawAnims(char stop);
	virtual void drawText(char *pStop, char *pStopNoClear);
	virtual char prepPalAnim(char stop);
	virtual void doPalAnim(void);
	virtual char doFadeAnim(char stop);
	virtual char doSoundAnim(char stop, int16 frame);
};

class Mult_v2 : public Mult_v1 {
public:
#pragma START_PACK_STRUCTS
	struct Mult_Data {
		int16 palFadeKeysCount;
		Mult_PalFadeKey *palFadeKeys;

		int16 palKeysCount;
		Mult_PalKey *palKeys;

		int16 staticKeysCount;
		Mult_StaticKey *staticKeys;
		int8 staticCount;
		int16 staticIndices[10];
		int16 staticLoaded[10];

		int16 animKeysCount[4];
		Mult_AnimKey *animKeys[4];
		int8 animCount;
		int16 animIndices[10];
		int16 animLoaded[10];
		int16 animKeysIndices1[4]; // Not sure with these
		int16 animKeysIndices2[4]; // "

		int16 textKeysCount;
		Mult_TextKey *textKeys;

		int16 sndKeysCount;
		Mult_SndKey *sndKeys;

		int16 sndSlotsCount;
		int16 sndSlot[60];
		int16 frameRate;      

		Video::Color fadePal[5][16];
		int16 field_124[4][4]; // Not sure here either
		int16 palAnimIndices[4]; // Not sure here either
		int16 frameStart;

		int8 field_156;
		int16 field_157[4];
		int16 field_15F[4][4];
		int16 field_17F[4][4];

		int16 somepointer05size[4];
		char *somepointer05[4]; // Seems to be similar to staticKeys/animKeys
		int16 somepointer05indices[4];
		char *somepointer09; // ?
		char *somepointer10; // ?
		char *execPtr;
	} GCC_PACK;
#pragma END_PACK_STRUCTS

	Mult_Data *_multData2; // TODO: This'll be _multData once every function using it
	                       //       in GOB2 is done
												 // TODO: Maybe changing Mult_v1::_multData to struct Mult_Data as well?
												 //       Could help minimizing code dup...
	Mult_Data *_multDatas[8];

	Mult_v2(GobEngine *vm);
	virtual ~Mult_v2() {};

	virtual void setMultData(uint16 multindex);
	virtual void multSub(uint16 multindex);
	virtual void loadMult(int16 resId);
	virtual void playMult(int16 startFrame, int16 endFrame, char checkEscape,
				  char handleMouse);
	virtual void animate(void);
	virtual void playSound(Snd::SoundDesc * soundDesc, int16 repCount,
				  int16 freq, int16 channel);
	virtual void freeMultKeys(void);

protected:
	virtual char drawStatics(char stop);
	virtual char drawAnims(char stop);
	virtual void drawText(char *pStop, char *pStopNoClear);
	virtual char prepPalAnim(char stop);
	virtual void doPalAnim(void);
	virtual char doFadeAnim(char stop);
	virtual char doSoundAnim(char stop, int16 frame);

	void sub_62DD(int16 index);
	void sub_6A35(void);
};

}				// End of namespace Gob

#endif	/* __MULT_H */
