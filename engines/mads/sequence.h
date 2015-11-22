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

#ifndef MADS_SEQUENCE_H
#define MADS_SEQUENCE_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/rect.h"
#include "mads/action.h"

namespace MADS {

class SpriteSlot;

enum SequenceTrigger {
	SEQUENCE_TRIGGER_EXPIRE = 0,	// Trigger when the sequence finishes
	SEQUENCE_TRIGGER_LOOP = 1,		// Trigger when the sequence loops
	SEQUENCE_TRIGGER_SPRITE = 2		// Trigger when sequence reaches specific sprite
};

enum SpriteAnimType { ANIMTYPE_NONE = 0, ANIMTYPE_CYCLED = 1, ANIMTYPE_PING_PONG = 2, ANIMTYPE_STAMP = 9 };

#define SEQUENCE_ENTRY_SUBSET_MAX 5

struct SequenceSubEntries {
	int _count;
	SequenceTrigger _mode[SEQUENCE_ENTRY_SUBSET_MAX];
	int _frameIndex[SEQUENCE_ENTRY_SUBSET_MAX];
	int _trigger[SEQUENCE_ENTRY_SUBSET_MAX];
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

	Common::Point _position;
	Common::Point _posDiff;
	Common::Point _posSign;
	Common::Point _posAccum;
	int _triggerCountdown;
	bool _doneFlag;
	SequenceSubEntries _entries;
	TriggerMode _triggerMode;

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
	bool addSubEntry(int index, SequenceTrigger mode, int frameIndex, int trigger);
	int add(int spriteListIndex, bool flipped, int frameIndex, int triggerCountdown, int delayTicks,
		int extraTicks, int numTicks, int msgX, int msgY, bool nonFixed, int scale, int depth,
		int frameInc, SpriteAnimType animType, int numSprites, int frameStart);

	int addTimer(int timeout, int endTrigger);
	void remove(int seqIndex);
	int findByTrigger(int trigger);
	void setSpriteSlot(int seqIndex, SpriteSlot &spriteSlot);
	bool loadSprites(int seqIndex);
	void tick();
	void delay(uint32 priorFrameTime, uint32 currentTime);
	void setAnimRange(int seqIndex, int startVal, int endVal);
	void scan();
	void setDepth(int seqIndex, int depth);
	void setPosition(int seqIndex, const Common::Point &pt);
	int addSpriteCycle(int srcSpriteIdx, bool flipped, int numTicks,
		int triggerCountdown = 0, int timeoutTicks = 0, int extraTicks = 0);
	int addReverseSpriteCycle(int srcSpriteIdx, bool flipped, int numTicks,
		int triggerCountdown = 0, int timeoutTicks = 0, int extraTicks = 0);

	int startCycle(int srcSpriteIdx, bool flipped, int cycleIndex);
	int startPingPongCycle(int srcSpriteIndex, bool flipped, int numTicks,
		int triggerCountdown = 0, int timeoutTicks = 0, int extraTicks = 0);
	void updateTimeout(int destSeqIndex, int srcSeqIndex);
	void setScale(int spriteIdx, int scale);
	void setMsgLayout(int seqIndex);
	void setDone(int seqIndex);
	void setMotion(int seqIndex, int flags, int deltaX, int deltaY);

	int addStampCycle(int srcSpriteIdx, bool flipped, int sprite);
	void setSeqPlayer(int idx, bool flag);
};

} // End of namespace MADS

#endif /* MADS_SEQUENCE_H */
