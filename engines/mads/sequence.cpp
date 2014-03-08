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

#include "common/scummsys.h"
#include "mads/mads.h"
#include "mads/assets.h"
#include "mads/sequence.h"
#include "mads/scene.h"

namespace MADS {

	SequenceEntry::SequenceEntry() {
		_spritesIndex = 0;
		_flipped = 0;
		_frameIndex = 0;
		_frameStart = 0;
		_numSprites = 0;
		_animType = ANIMTYPE_NONE;
		_frameInc = 0;
		_depth = 0;
		_scale = 0;
		_dynamicHotspotIndex = -1;
		_triggerCountdown = 0;
		_doneFlag = 0;
		_entries._count = 0;
		_abortMode = ABORTMODE_0;
		_numTicks = 0;
		_extraTicks = 0;
		_timeout = 0;
	}

	/*------------------------------------------------------------------------*/

#define SEQUENCE_LIST_SIZE 30

SequenceList::SequenceList(MADSEngine *vm) : _vm(vm) {
	// IMPORTANT: Preallocate timer slots. Note that sprite slots refer to entries 
	// in this list by index, so we can't just add or delete entries later
	for (int i = 0; i < SEQUENCE_LIST_SIZE; ++i) {
		SequenceEntry rec;
		rec._active = false;
		rec._dynamicHotspotIndex = -1;
		_entries.push_back(rec);
	}
}

void SequenceList::clear() {
	for (uint i = 0; i < _entries.size(); ++i) {
		_entries[i]._active = false;
		_entries[i]._dynamicHotspotIndex = -1;
	}
}

bool SequenceList::addSubEntry(int index, SequenceSubEntryMode mode, int frameIndex, int abortVal) {
	if (_entries[index]._entries._count >= TIMER_ENTRY_SUBSET_MAX)
		return true;

	int subIndex = _entries[index]._entries._count++;
	_entries[index]._entries._mode[subIndex] = mode;
	_entries[index]._entries._frameIndex[subIndex] = frameIndex;
	_entries[index]._entries._abortVal[subIndex] = abortVal;

	return false;
}

int SequenceList::add(int spriteListIndex, bool flipped, int frameIndex, int triggerCountdown, int delayTicks, int extraTicks, int numTicks,
	int msgX, int msgY, bool nonFixed, char scale, uint8 depth, int frameInc, SpriteAnimType animType, int numSprites,
	int frameStart) {
	Scene &scene = _vm->_game->_scene;

	// Find a free slot
	uint seqIndex = 0;
	while ((seqIndex < _entries.size()) && _entries[seqIndex]._active)
		++seqIndex;
	if (seqIndex == _entries.size())
		error("TimerList full");

	if (frameStart <= 0)
		frameStart = 1;
	if (numSprites == 0)
		numSprites = scene._sprites[spriteListIndex]->getCount();
	if (frameStart == numSprites)
		frameInc = 0;

	// Set the list entry fields
	_entries[seqIndex]._active = true;
	_entries[seqIndex]._spritesIndex = spriteListIndex;
	_entries[seqIndex]._flipped = flipped;
	_entries[seqIndex]._frameIndex = frameIndex;
	_entries[seqIndex]._frameStart = frameStart;
	_entries[seqIndex]._numSprites = numSprites;
	_entries[seqIndex]._animType = animType;
	_entries[seqIndex]._frameInc = frameInc;
	_entries[seqIndex]._depth = depth;
	_entries[seqIndex]._scale = scale;
	_entries[seqIndex]._nonFixed = nonFixed;
	_entries[seqIndex]._msgPos.x = msgX;
	_entries[seqIndex]._msgPos.y = msgY;
	_entries[seqIndex]._numTicks = numTicks;
	_entries[seqIndex]._extraTicks = extraTicks;

	_entries[seqIndex]._timeout = scene._frameStartTime + delayTicks;

	_entries[seqIndex]._triggerCountdown = triggerCountdown;
	_entries[seqIndex]._doneFlag = false;
	_entries[seqIndex]._field13 = 0;
	_entries[seqIndex]._dynamicHotspotIndex = -1;
	_entries[seqIndex]._entries._count = 0;
	_entries[seqIndex]._abortMode = _vm->_game->_abortTimersMode2;

	_entries[seqIndex]._actionNouns = _vm->_game->_scene._action._activeAction;

	return seqIndex;
}

int SequenceList::addTimer(int time, int abortVal) {
	Scene &scene = _vm->_game->_scene;
	uint seqIndex;
	for (seqIndex = 0; seqIndex < _entries.size(); ++seqIndex) {
		if (!_entries[seqIndex]._active)
			break;
	}
	assert(seqIndex < (int)_entries.size());

	SequenceEntry &se = _entries[seqIndex];
	se._active = true;
	se._spritesIndex = -1;
	se._numTicks = time;
	se._extraTicks = 0;
	se._timeout = scene._frameStartTime + time;
	se._triggerCountdown = true;
	se._doneFlag = false;
	se._entries._count = 0;
	se._abortMode = _vm->_game->_abortTimersMode2;
	se._actionNouns = _vm->_game->_scene._action._activeAction;
	addSubEntry(seqIndex, SM_0, 0, abortVal);

	return seqIndex;
}

void SequenceList::remove(int seqIndex) {
	Scene &scene = _vm->_game->_scene;

	if (_entries[seqIndex]._active) {
		if (_entries[seqIndex]._dynamicHotspotIndex >= 0)
			scene._dynamicHotspots.remove(_entries[seqIndex]._dynamicHotspotIndex);
	}

	_entries[seqIndex]._active = false;
	scene._spriteSlots.deleteTimer(seqIndex);
}

void SequenceList::setSpriteSlot(int seqIndex, SpriteSlot &spriteSlot) {
	Scene &scene = _vm->_game->_scene;
	SequenceEntry &timerEntry = _entries[seqIndex];
	SpriteAsset &spriteSet = *scene._sprites[timerEntry._spritesIndex];

	spriteSlot._spriteType = spriteSet.isBackground() ? ST_BACKGROUND : ST_FOREGROUND;
	spriteSlot._seqIndex = seqIndex;
	spriteSlot._spritesIndex = timerEntry._spritesIndex;
	spriteSlot._frameNumber = (timerEntry._flipped ? 0x8000 : 0) | timerEntry._frameIndex;
	spriteSlot._depth = timerEntry._depth;
	spriteSlot._scale = timerEntry._scale;

	if (!timerEntry._nonFixed) {
		spriteSlot._position = timerEntry._msgPos;
	} else {
		spriteSlot._position = spriteSet.getFrame(timerEntry._frameIndex - 1)->_pos;
	}
}

bool SequenceList::loadSprites(int seqIndex) {
	Scene &scene = _vm->_game->_scene;
	SequenceEntry &seqEntry = _entries[seqIndex];
	int slotIndex;
	bool result = false;
	int idx = -1;

	scene._spriteSlots.deleteTimer(seqIndex);
	if (seqEntry._doneFlag) {
		remove(seqIndex);
		return false;
	}

	if (seqEntry._spritesIndex == -1) {
		// Doesn't have an associated sprite anymore, so mark as done
		seqEntry._doneFlag = true;
	}
	else if ((slotIndex = scene._spriteSlots.add()) >= 0) {
		SpriteSlot &spriteSlot = scene._spriteSlots[slotIndex];
		setSpriteSlot(seqIndex, spriteSlot);

		int x2 = 0, y2 = 0;

		if ((seqEntry._field13 != 0) || (seqEntry._dynamicHotspotIndex >= 0)) {
			SpriteAsset &spriteSet = *scene._sprites[seqEntry._spritesIndex];
			MSprite *frame = spriteSet.getFrame(seqEntry._frameIndex - 1);
			int width = frame->getWidth() * seqEntry._scale / 200;
			int height = frame->getHeight() * seqEntry._scale / 100;

			warning("frame size %d x %d", width, height);

			// TODO: Missing stuff here, and I'm not certain about the dynamic hotspot stuff below

			if (seqEntry._dynamicHotspotIndex >= 0) {
				DynamicHotspot &dynHotspot = scene._dynamicHotspots[seqEntry._dynamicHotspotIndex];

				dynHotspot._bounds.left = MAX(x2 - width, 0);
				dynHotspot._bounds.right = MAX(x2 - width, 319) - dynHotspot._bounds.left + 1;
				dynHotspot._bounds.top = MAX(y2 - height, 0);
				dynHotspot._bounds.bottom = MIN(y2, 155) - dynHotspot._bounds.top;

				scene._dynamicHotspots._changed = true;
			}
		}

		// Frame adjustments
		if (seqEntry._frameStart != seqEntry._numSprites)
			seqEntry._frameIndex += seqEntry._frameInc;

		if (seqEntry._frameIndex >= seqEntry._frameStart) {
			if (seqEntry._frameIndex > seqEntry._numSprites) {
				result = true;
				if (seqEntry._animType == ANIMTYPE_CYCLED) {
					//  back to the starting frame (cyclic)
					seqEntry._frameIndex = seqEntry._frameStart;
				}
				else {
					// Switch into reverse mode
					seqEntry._frameIndex = seqEntry._numSprites - 1;
					seqEntry._frameInc = -1;
				}
			}
		}
		else {
			// Currently in reverse mode and moved past starting frame
			result = true;

			if (seqEntry._animType == ANIMTYPE_CYCLED)
			{
				// Switch back to forward direction again
				seqEntry._frameIndex = seqEntry._frameStart + 1;
				seqEntry._frameInc = 1;
			}
			else {
				// Otherwise reset back to last sprite for further reverse animating
				seqEntry._frameIndex = seqEntry._numSprites;
			}
		}

		if (result && (seqEntry._triggerCountdown != 0)) {
			if (--seqEntry._triggerCountdown == 0)
				seqEntry._doneFlag = true;
		}
	}
	else {
		// Out of sprite display slots, so mark entry as done
		seqEntry._doneFlag = true;
	}

	if (seqEntry._entries._count > 0) {
		for (int i = 0; i <= seqEntry._entries._count; ++i) {
			switch (seqEntry._entries._mode[i]) {
			case SM_0:
			case SM_1:
				if (((seqEntry._entries._mode[i] == SM_0) && seqEntry._doneFlag) ||
					((seqEntry._entries._mode[i] == SM_1) && result))
					idx = i;
				break;

			case SM_FRAME_INDEX: {
				int v = seqEntry._entries._frameIndex[i];
				if ((v == seqEntry._frameIndex) || (v == 0))
					idx = i;
				break;
			}

			default:
				break;
			}
		}
	}

	if (idx >= 0) {
		_vm->_game->_abortTimers = seqEntry._entries._abortVal[idx];
		_vm->_game->_abortTimersMode = seqEntry._abortMode;
	}

	return result;
}

/**
* Handles counting down entries in the timer list for action
*/
void SequenceList::tick() {
	Scene &scene = _vm->_game->_scene;
	for (uint idx = 0; idx < _entries.size(); ++idx) {
		if ((_vm->_game->_abortTimers2 == 0) && (_vm->_game->_abortTimers != 0))
			break;

		SequenceEntry &seqEntry = _entries[idx];
		uint32 currentTimer = scene._frameStartTime;

		if (!seqEntry._active || (currentTimer < seqEntry._timeout))
			continue;

		// Set the next timeout for the timer entry
		seqEntry._timeout = currentTimer + seqEntry._numTicks;

		// Action the sprite
		if (loadSprites(idx)) {
			seqEntry._timeout += seqEntry._extraTicks;
		}
	}
}

void SequenceList::delay(uint32 priorFrameTime, uint32 currentTime) {
	for (uint idx = 0; idx < _entries.size(); ++idx) {
		if (_entries[idx]._active) {
			_entries[idx]._timeout += currentTime - priorFrameTime;
		}
	}
}

void SequenceList::setAnimRange(int seqIndex, int startVal, int endVal) {
	Scene &scene = _vm->_game->_scene;	
	SequenceEntry &seqEntry = _entries[seqIndex];
	SpriteAsset &spriteSet = *scene._sprites[seqEntry._spritesIndex];
	int numSprites = spriteSet.getCount();
	int tempStart = startVal, tempEnd = endVal;

	switch (startVal) {
	case -2:
		tempStart = numSprites;
		break;
	case -1:
		tempStart = 1;
		break;
	}

	switch (endVal) {
	case -2:
	case 0:
		tempEnd = numSprites;
		break;
	case -1:
		tempEnd = 1;
		break;
	default:
		tempEnd = numSprites;
		break;
	}

	seqEntry._frameStart = tempStart;
	seqEntry._numSprites = tempEnd;

	seqEntry._frameIndex = (seqEntry._frameInc < 0) ? tempStart : tempEnd;
}

void SequenceList::scan() {
	Scene &scene = _vm->_game->_scene;

	for (uint i = 0; i < _entries.size(); ++i) {
		if (!_entries[i]._active && (_entries[i]._spritesIndex != -1)) {
			int idx = scene._spriteSlots.add();
			setSpriteSlot(i, scene._spriteSlots[idx]);
		}
	}
}

/**
* Sets the depth of the specified entry in the sequence list
*/
void SequenceList::setDepth(int seqIndex, int depth) {
	_entries[seqIndex]._depth = depth;
}

void SequenceList::setMsgPosition(int seqIndex, const Common::Point &pt) {
	_entries[seqIndex]._msgPos = pt;
	_entries[seqIndex]._nonFixed = false;
}

int SequenceList::addSpriteCycle(int srcSpriteIdx, bool flipped, int numTicks, int triggerCountdown, int timeoutTicks, int extraTicks) {
	Scene &scene = _vm->_game->_scene;
	MSprite *spriteFrame = scene._sprites[srcSpriteIdx]->getFrame(0);
	int depth = scene._depthSurface.getDepth(Common::Point(spriteFrame->_pos.x + 
		(spriteFrame->w / 2), spriteFrame->_pos.y + (spriteFrame->h / 2)));

	return add(srcSpriteIdx, flipped, 1, triggerCountdown, timeoutTicks, extraTicks, numTicks, 0, 0,
		true, 100, depth - 1, 1, ANIMTYPE_CYCLED, 0, 0);
}

int SequenceList::startCycle(int srcSpriteIndex, bool flipped, int cycleIndex) {
	int result = addSpriteCycle(srcSpriteIndex, flipped, INDEFINITE_TIMEOUT, 0, 0, 0);
	if (result >= 0)
		setAnimRange(result, cycleIndex, cycleIndex);

	return result;
}

int SequenceList::startReverseCycle(int srcSpriteIndex, bool flipped, int numTicks, 
		int triggerCountdown, int timeoutTicks, int extraTicks) {
	SpriteAsset *sprites = _vm->_game->_scene._sprites[srcSpriteIndex];
	MSprite *frame = sprites->getFrame(0);
	int depth = _vm->_game->_scene._depthSurface.getDepth(Common::Point(
		frame->_pos.x + frame->w / 2, frame->_pos.y + frame->h / 2));

	return add(srcSpriteIndex, flipped, 1, triggerCountdown, timeoutTicks, extraTicks,
		numTicks, 0, 0, true, 100, depth - 1, 1, ANIMTYPE_REVERSIBLE, 0, 0);
}

} // End of namespace 
