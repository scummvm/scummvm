/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef IMUSE_DIGI_SNDMGR_H
#define IMUSE_DIGI_SNDMGR_H

#include "stdafx.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "scumm/imuse_digi/dimuse_bndmgr.h"

namespace Scumm {

class ScummEngine;
class BundleMgr;

class ImuseDigiSndMgr {
public:

#define MAX_IMUSE_SOUNDS 16
#define MAX_IMUSE_JUMPS 80
#define MAX_IMUSE_REGIONS 85
#define MAX_IMUSE_MARKERS 60

#define IMUSE_RESOURCE 1
#define IMUSE_BUNDLE 2
#define IMUSE_VOICE 1
#define IMUSE_SFX 2
#define IMUSE_MUSIC 3

private:
	struct _region {
		int32 offset;		// offset of region
		int32 length;		// lenght of region
	};

	struct _jump {
		int32 offset;		// jump offset position
		int32 dest;			// jump to dest position
		int hookId;			// id of hook
		int fadeDelay;		// fade delay in ms
	};

	struct _marker {
		char name[256];		// name of marker
	};
	
public:

	struct soundStruct {
		int freq;			// frequency
		int channels;		// stereo or mono
		int bits;			// 8, 12, 16
		int numJumps;		// number of Jumps
		int numRegions;		// number of Regions
		int numMarkers;		// number of Markers
		int32 offsetStop;	// end offset in source data
		bool endFlag;
		bool inUse;
		byte *allData;
		int32 offsetData;
		byte *resPtr;
		char name[15];
		int soundId;
		bool freeResPtr;
		BundleMgr *_bundle;
		_region region[MAX_IMUSE_REGIONS];
		_marker marker[MAX_IMUSE_MARKERS];
		_jump jump[MAX_IMUSE_JUMPS];
	};

private:

	soundStruct _sounds[MAX_IMUSE_SOUNDS];

	bool checkForProperHandle(soundStruct *soundHandle);
	int allocSlot();
	void prepareSound(byte *ptr, int slot);

	ScummEngine *_scumm;
	OSystem::MutexRef _mutex;
	byte _disk;
	BundleDirCache *_cacheBundleDir;

	bool openMusicBundle(int slot);
	bool openVoiceBundle(int slot);

public:

	ImuseDigiSndMgr(ScummEngine *scumm);
	~ImuseDigiSndMgr();
	
	soundStruct * openSound(int32 soundId, const char *soundName, int soundType, int soundGroup);
	void closeSound(soundStruct *soundHandle);

	int getFreq(soundStruct *soundHandle);
	int getBits(soundStruct *soundHandle);
	int getChannels(soundStruct *soundHandle);
	bool isEndOfRegion(soundStruct *soundHandle, int region);
	int getNumRegions(soundStruct *soundHandle);
	int getNumJumps(soundStruct *soundHandle);
	int getNumMarkers(soundStruct *soundHandle);
	int getJumpIdByRegion(soundStruct *soundHandle, int number);
	int getJumpDestRegionId(soundStruct *soundHandle, int number);
	int getJumpHookId(soundStruct *soundHandle, int number);
	int getJumpFade(soundStruct *soundHandle, int number);
	char *getMarker(soundStruct *soundHandle, int number);

	int32 getDataFromRegion(soundStruct *soundHandle, int region, byte **buf, int32 offset, int32 size);
};

} // End of namespace Scumm

#endif
