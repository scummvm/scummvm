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
#define MAX_IMUSE_SYNCS 4

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
		byte hookId;			// id of hook
		int16 fadeDelay;		// fade delay in ms
	};

	struct _sync {
		int32 size;		// size of sync
		byte *ptr;		// pointer to sync
	};
	
public:

	struct soundStruct {
		uint16 freq;			// frequency
		byte channels;		// stereo or mono
		byte bits;			// 8, 12, 16
		int8 numJumps;		// number of Jumps
		int8 numRegions;		// number of Regions
		int8 numSyncs;		// number of Syncs
		int32 offsetStop;	// end offset in source data
		bool endFlag;
		bool inUse;
		byte *allData;
		int32 offsetData;
		byte *resPtr;
		char name[15];
		int16 soundId;
		bool freeResPtr;
		BundleMgr *_bundle;
		_region region[MAX_IMUSE_REGIONS];
		_jump jump[MAX_IMUSE_JUMPS];
		_sync sync[MAX_IMUSE_SYNCS];
	};

private:

	soundStruct _sounds[MAX_IMUSE_SOUNDS];

	bool checkForProperHandle(soundStruct *soundHandle);
	int allocSlot();
	void prepareSound(byte *ptr, int slot);

	ScummEngine *_vm;
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
	int getRegionOffset(soundStruct *soundHandle, int region);
	int getJumpIdByRegionAndHookId(soundStruct *soundHandle, int region, int hookId);
	int getRegionIdByJumpId(soundStruct *soundHandle, int jumpId);
	int getJumpHookId(soundStruct *soundHandle, int number);
	int getJumpFade(soundStruct *soundHandle, int number);
	void getSyncSizeAndPtrById(soundStruct *soundHandle, int number, int32 &sync_size, byte **sync_ptr);

	int32 getDataFromRegion(soundStruct *soundHandle, int region, byte **buf, int32 offset, int32 size);
};

} // End of namespace Scumm

#endif
