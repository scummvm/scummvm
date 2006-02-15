// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#ifndef IMUSE_SNDMGR_H
#define IMUSE_SNDMGR_H

#include "stdafx.h"
#include "bits.h"
#include "debug.h"
#include "timer.h"

#include "mixer/mixer.h"
#include "mixer/audiostream.h"

class McmpMgr;
class Block;

class ImuseSndMgr {
public:

// MAX_IMUSE_SOUNDS needs to be hardcoded, ask aquadran
#define MAX_IMUSE_SOUNDS    16

// The numbering below fixes talking to Domino in his office
// and it also allows Manny to get the info for Mercedes
// Colomar, without this the game hangs at these points!
#define IMUSE_VOLGRP_BGND   0
#define IMUSE_VOLGRP_ACTION 1
#define IMUSE_VOLGRP_SFX    2
#define IMUSE_VOLGRP_MUSIC  3
#define IMUSE_VOLGRP_VOICE  4

private:
	struct Region {
		int32 offset;		// offset of region
		int32 length;		// lenght of region
	};

	struct Jump {
		int32 offset;		// jump offset position
		int32 dest;			// jump to dest position
		byte hookId;		// id of hook
		int16 fadeDelay;	// fade delay in ms
	};
	
public:

	struct SoundStruct {
		uint16 freq;		// frequency
		byte channels;		// stereo or mono
		byte bits;			// 8, 12, 16
		int numJumps;		// number of Jumps
		int numRegions;		// number of Regions
		Region *region;
		Jump *jump;
		bool endFlag;
		bool inUse;
		char name[32];
		McmpMgr *mcmpMgr;
		Block *blockRes;
		int volGroupId;
		byte *resPtr;
		bool mcmpData;
	};

private:

	SoundStruct _sounds[MAX_IMUSE_SOUNDS];

	bool checkForProperHandle(SoundStruct *soundHandle);
	SoundStruct *allocSlot();
	void parseSoundHeader(byte *ptr, SoundStruct *sound, int &headerSize);
	void countElements(byte *ptr, int &numRegions, int &numJumps);

public:

	ImuseSndMgr();
	~ImuseSndMgr();

	SoundStruct *openSound(const char *soundName, int volGroupId);
	void closeSound(SoundStruct *soundHandle);
	SoundStruct *cloneSound(SoundStruct *soundHandle);

	int getFreq(SoundStruct *soundHandle);
	int getBits(SoundStruct *soundHandle);
	int getChannels(SoundStruct *soundHandle);
	bool isEndOfRegion(SoundStruct *soundHandle, int region);
	int getNumRegions(SoundStruct *soundHandle);
	int getNumJumps(SoundStruct *soundHandle);
	int getRegionOffset(SoundStruct *soundHandle, int region);
	int getJumpIdByRegionAndHookId(SoundStruct *soundHandle, int region, int hookId);
	int getRegionIdByJumpId(SoundStruct *soundHandle, int jumpId);
	int getJumpHookId(SoundStruct *soundHandle, int number);
	int getJumpFade(SoundStruct *soundHandle, int number);

	int32 getDataFromRegion(SoundStruct *soundHandle, int region, byte **buf, int32 offset, int32 size);
};

#endif
