// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2004 The ScummVM-Residual Team (www.scummvm.org)
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

#include "../stdafx.h"
#include "../bits.h"
#include "../debug.h"
#include "../timer.h"

#include "../mixer/mixer.h"
#include "../mixer/audiostream.h"

class ImuseSndMgr {
public:

#define MAX_IMUSE_SOUNDS 16

#define IMUSE_VOLGRP_VOICE 1
#define IMUSE_VOLGRP_SFX 2
#define IMUSE_VOLGRP_MUSIC 3

private:
	struct _region {
		int32 offset;		// offset of region
		int32 length;		// lenght of region
	};

	struct _jump {
		int32 offset;		// jump offset position
		int32 dest;			// jump to dest position
		byte hookId;		// id of hook
		int16 fadeDelay;	// fade delay in ms
	};
	
public:

	struct soundStruct {
		uint16 freq;		// frequency
		byte channels;		// stereo or mono
		byte bits;			// 8, 12, 16
		int numJumps;		// number of Jumps
		int numRegions;		// number of Regions
		_region *region;
		_jump *jump;
		bool endFlag;
		bool inUse;
		int32 offsetData;
		char name[32];
		int16 soundId;
		McmpMgr *_mcmpMgr;
		int type;
		int volGroupId;
		byte *ptr;
	};

private:

	soundStruct _sounds[MAX_IMUSE_SOUNDS];

	bool checkForProperHandle(soundStruct *soundHandle);
	soundStruct *allocSlot();
	void prepareSound(byte *ptr, soundStruct *sound);
	void countElements(byte *ptr, int &numRegions, int &numJumps);

public:

	ImuseSndMgr();
	~ImuseSndMgr();

	soundStruct *openSound(int32 soundId, const char *soundName, int soundType, int volGroupId, int disk);
	void closeSound(soundStruct *soundHandle);
	soundStruct *cloneSound(soundStruct *soundHandle);

	int getFreq(soundStruct *soundHandle);
	int getBits(soundStruct *soundHandle);
	int getChannels(soundStruct *soundHandle);
	bool isEndOfRegion(soundStruct *soundHandle, int region);
	int getNumRegions(soundStruct *soundHandle);
	int getNumJumps(soundStruct *soundHandle);
	int getRegionOffset(soundStruct *soundHandle, int region);
	int getJumpIdByRegionAndHookId(soundStruct *soundHandle, int region, int hookId);
	int getRegionIdByJumpId(soundStruct *soundHandle, int jumpId);
	int getJumpHookId(soundStruct *soundHandle, int number);
	int getJumpFade(soundStruct *soundHandle, int number);

	int32 getDataFromRegion(soundStruct *soundHandle, int region, byte **buf, int32 offset, int32 size);
};

#endif
