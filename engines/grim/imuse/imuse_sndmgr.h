/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GRIM_IMUSE_SNDMGR_H
#define GRIM_IMUSE_SNDMGR_H

#include "audio/mixer.h"
#include "audio/audiostream.h"

namespace Grim {

class McmpMgr;

class ImuseSndMgr {
public:

// MAX_IMUSE_SOUNDS needs to be hardcoded, ask aquadran
#define MAX_IMUSE_SOUNDS    16

// The numbering below fixes talking to Domino in his office
// and it also allows Manny to get the info for Mercedes
// Colomar, without this the game hangs at these points!
#define IMUSE_VOLGRP_BGND   0
#define IMUSE_VOLGRP_SFX    1
#define IMUSE_VOLGRP_VOICE  2
#define IMUSE_VOLGRP_MUSIC  3
#define IMUSE_VOLGRP_ACTION 4

private:
	struct Region {
		int32 offset;       // offset of region
		int32 length;       // lenght of region
	};

	struct Jump {
		int32 offset;       // jump offset position
		int32 dest;         // jump to dest position
		byte hookId;        // id of hook
		int16 fadeDelay;    // fade delay in ms
	};

public:

	struct SoundDesc {
		uint16 freq;        // frequency
		byte channels;      // stereo or mono
		byte bits;          // 8, 12, 16
		int numJumps;       // number of Jumps
		int numRegions;     // number of Regions
		Region *region;
		Jump *jump;
		bool endFlag;
		bool inUse;
		char name[32];
		McmpMgr *mcmpMgr;
		int type;
		int volGroupId;
		bool mcmpData;
		uint32 headerSize;
		Common::SeekableReadStream *inStream;
	};

private:

	SoundDesc _sounds[MAX_IMUSE_SOUNDS];
	bool _demo;

	bool checkForProperHandle(SoundDesc *soundDesc);
	SoundDesc *allocSlot();
	void parseSoundHeader(SoundDesc *sound, int &headerSize);
	void countElements(SoundDesc *sound);

public:

	ImuseSndMgr(bool demo);
	~ImuseSndMgr();

	SoundDesc *openSound(const char *soundName, int volGroupId);
	void closeSound(SoundDesc *sound);
	SoundDesc *cloneSound(SoundDesc *sound);

	int getFreq(SoundDesc *sound);
	int getBits(SoundDesc *sound);
	int getChannels(SoundDesc *sound);
	bool isEndOfRegion(SoundDesc *sound, int region);
	int getNumRegions(SoundDesc *sound);
	int getNumJumps(SoundDesc *sound);
	int getRegionOffset(SoundDesc *sound, int region);
	int getRegionLength(SoundDesc *sound, int region);
	int getJumpIdByRegionAndHookId(SoundDesc *sound, int region, int hookId);
	int getRegionIdByJumpId(SoundDesc *sound, int jumpId);
	int getJumpHookId(SoundDesc *sound, int number);
	int getJumpFade(SoundDesc *sound, int number);

	int32 getDataFromRegion(SoundDesc *sound, int region, byte **buf, int32 offset, int32 size);
};

} // end of namespace Grim

#endif
