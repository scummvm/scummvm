/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2005 The ScummVM project
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
#include "sound/audiostream.h"
#include "scumm/imuse_digi/dimuse_bndmgr.h"

namespace Scumm {

class ScummEngine;
class BundleMgr;

class ImuseDigiSndMgr {
public:

#define MAX_IMUSE_SOUNDS 16

#define IMUSE_RESOURCE 1
#define IMUSE_BUNDLE 2

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

	struct _sync {
		int32 size;			// size of sync
		byte *ptr;			// pointer to sync
	};
	
public:

	struct soundStruct {
		uint16 freq;		// frequency
		byte channels;		// stereo or mono
		byte bits;			// 8, 12, 16
		int numJumps;		// number of Jumps
		int numRegions;		// number of Regions
		int numSyncs;		// number of Syncs
		_region *region;
		_jump *jump;
		_sync *sync;
		bool endFlag;
		bool inUse;
		byte *allData;
		int32 offsetData;
		byte *resPtr;
		char name[15];
		int16 soundId;
		BundleMgr *bundle;
		int type;
		int volGroupId;
		int disk;
		AudioStream *compressedStream;
		bool compressed;
		char lastFileName[24];
	};

private:

	soundStruct _sounds[MAX_IMUSE_SOUNDS];

	bool checkForProperHandle(soundStruct *soundHandle);
	soundStruct *allocSlot();
	void prepareSound(byte *ptr, soundStruct *sound);
	void prepareSoundFromRMAP(Common::File *file, soundStruct *sound, int32 offset, int32 size);

	ScummEngine *_vm;
	byte _disk;
	BundleDirCache *_cacheBundleDir;

	bool openMusicBundle(soundStruct *sound, int disk);
	bool openVoiceBundle(soundStruct *sound, int disk);

	void countElements(byte *ptr, int &numRegions, int &numJumps, int &numSyncs);

public:

	ImuseDigiSndMgr(ScummEngine *scumm);
	~ImuseDigiSndMgr();

	soundStruct *openSound(int32 soundId, const char *soundName, int soundType, int volGroupId, int disk);
	void closeSound(soundStruct *soundHandle);
	soundStruct *cloneSound(soundStruct *soundHandle);

	bool isCompressed(soundStruct *soundHandle);
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
