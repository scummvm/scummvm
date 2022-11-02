/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SCUMM_IMUSE_DIGI_SNDMGR_H
#define SCUMM_IMUSE_DIGI_SNDMGR_H


#include "common/scummsys.h"

namespace Audio {
class SeekableAudioStream;
}

namespace Common {
class SeekableReadStream;
}

namespace Scumm {

class ScummEngine;
class BundleMgr;
class BundleDirCache;

class ImuseDigiSndMgr {
public:

#define MAX_IMUSE_SOUNDS 16

#define IMUSE_RESOURCE 1
#define IMUSE_BUNDLE 2

#define IMUSE_VOLGRP_VOICE 1
#define IMUSE_VOLGRP_SFX 2
#define IMUSE_VOLGRP_MUSIC 3

public:

	struct SoundDesc {
		bool inUse;
		bool scheduledForDealloc;

		byte *resPtr;
		int resSize;
		int resCurOffset;
		char name[15];
		int16 soundId;
		BundleMgr *bundle;
	};

private:

	SoundDesc _sounds[MAX_IMUSE_SOUNDS];
	bool checkForProperHandle(SoundDesc *soundDesc);
	SoundDesc *allocSlot();

	ScummEngine *_vm;
	byte _disk;
	BundleDirCache *_cacheBundleDir;

	bool openMusicBundle(SoundDesc *sound, int &disk);
	bool openVoiceBundle(SoundDesc *sound, int &disk);

public:

	ImuseDigiSndMgr(ScummEngine *scumm);
	~ImuseDigiSndMgr();

	SoundDesc *openSound(int32 soundId, const char *soundName, int soundType, int volGroupId, int disk);
	void closeSound(SoundDesc *soundDesc);
	void closeSoundById(int soundId);
	SoundDesc *findSoundById(int soundId);
	SoundDesc *getSounds();
	void scheduleSoundForDeallocation(int soundId);

};

} // End of namespace Scumm

#endif
