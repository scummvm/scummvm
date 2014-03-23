/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef MADS_SEQUENCE_H
#define MADS_SEQUENCE_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/rect.h"
#include "mads/action.h"

namespace MADS {

class SpriteSlot;

enum SequenceSubEntryMode { SM_0 = 0, SM_1 = 1, SM_FRAME_INDEX = 2 };

enum SpriteAnimType { ANIMTYPE_NONE = 0, ANIMTYPE_CYCLED = 1, ANIMTYPE_REVERSIBLE = 2 };

#define SEQUENCE_ENTRY_SUBSET_MAX 5

struct SequenceSubEntries {
	int _count;
	SequenceSubEntryMode _mode[SEQUENCE_ENTRY_SUBSET_MAX];
	int _frameIndex[SEQUENCE_ENTRY_SUBSET_MAX];
	int8 _abortVal[SEQUENCE_ENTRY_SUBSET_MAX];
};

struct SequenceEntry {
	bool _active;
	int8 _spritesIndex;
	bool _flipped;

	int _frameIndex;
	int _frameStart;
	int _numSprites;

	SpriteAnimType _animType;
	int _frameInc;

	int _depth;
	int _scale;
	int _dynamicHotspotIndex;

	bool _nonFixed;
	uint32 _flags;

	Common::Point _msgPos;
	int _field18;
	int _field1A;
	int _field1C;
	int _field1E;
	int _field20;
	int _field22;
	int _triggerCountdown;
	bool _doneFlag;
	SequenceSubEntries _entries;
	AbortTimerMode _abortMode;

	ActionDetails _actionNouns;
	int _numTicks;
	int _extraTicks;
	uint32 _timeout;

	SequenceEntry();
};

class MADSEngine;

class SequenceList {
private:
	MADSEngine *_vm;
	Common::Array<SequenceEntry> _entries;
public:
	SequenceList(MADSEngine *vm);

	SequenceEntry &operator[](int index) { return _entries[index]; }
	void clear();
	bool addSubEntry(int index, SequenceSubEntryMode mode, int frameIndex, int abortVal);
	int add(int spriteListIndex, bool flipped, int frameIndex, int triggerCountdown, int delayTicks,
		int extraTicks, int numTicks, int msgX, int msgY, bool nonFixed, char scale, uint8 depth,
		int frameInc, SpriteAnimType animType, int numSprites, int frameStart);
	
	int addTimer(int time, int abortVal);
	void remove(int seqIndex);
	void setSpriteSlot(int seqIndex, SpriteSlot &spriteSlot);
	bool loadSprites(int seqIndex);
	void tick();
	void delay(uint32 priorFrameTime, uint32 currentTime);
	void setAnimRange(int seqIndex, int startVal, int endVal);
	void scan();
	void setDepth(int seqIndex, int depth);
	void setMsgPosition(int seqIndex, const Common::Point &pt);
	int addSpriteCycle(int srcSpriteIdx, bool flipped, int numTicks, 
		int triggerCountdown = 0, int timeoutTicks = 0, int extraTicks = 0);
	int addReverseSpriteCycle(int srcSpriteIdx, bool flipped, int numTicks,
		int triggerCountdown = 0, int timeoutTicks = 0, int extraTicks = 0);

	int startCycle(int srcSpriteIdx, bool flipped, int cycleIndex);
	int startReverseCycle(int srcSpriteIndex, bool flipped, int numTicks, int triggerCountdown, 
		int timeoutTicks, int extraTicks);
	void getTimeout(int spriteIdx, int arg2);
	void setScale(int spriteIdx, int scale);
};

} // End of namespace MADS

#endif /* MADS_SEQUENCE_H */
