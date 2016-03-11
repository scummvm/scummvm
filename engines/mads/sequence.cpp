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
	_triggerMode = SEQUENCE_TRIGGER_DAEMON;
	_numTicks = 0;
	_extraTicks = 0;
	_timeout = 0;
	_active = false;
	_nonFixed = false;
	_flags = 0;
	for (int i = 0; i < 5; ++i)
		_entries._mode[i] = SEQUENCE_TRIGGER_EXPIRE;

	_entries._count = 0;
	_actionNouns._verbId = VERB_NONE;
	_actionNouns._objectNameId = -1;
	_actionNouns._indirectObjectId = -1;

	Common::fill(&_entries._frameIndex[0], &_entries._frameIndex[SEQUENCE_ENTRY_SUBSET_MAX], 0);
	Common::fill(&_entries._trigger[0], &_entries._trigger[SEQUENCE_ENTRY_SUBSET_MAX], 0);
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

bool SequenceList::addSubEntry(int index, SequenceTrigger mode, int frameIndex, int trigger) {
	if (_entries[index]._entries._count >= SEQUENCE_ENTRY_SUBSET_MAX)
		return true;

	int subIndex = _entries[index]._entries._count++;
	_entries[index]._entries._mode[subIndex] = mode;
	_entries[index]._entries._frameIndex[subIndex] = frameIndex;
	_entries[index]._entries._trigger[subIndex] = trigger;

	return false;
}

int SequenceList::add(int spriteListIndex, bool flipped, int frameIndex, int triggerCountdown, int delayTicks, int extraTicks, int numTicks,
		int msgX, int msgY, bool nonFixed, int scale, int depth, int frameInc, SpriteAnimType animType, int numSprites,
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
	_entries[seqIndex]._position.x = msgX;
	_entries[seqIndex]._position.y = msgY;
	_entries[seqIndex]._numTicks = numTicks;
	_entries[seqIndex]._extraTicks = extraTicks;

	_entries[seqIndex]._timeout = scene._frameStartTime + delayTicks;

	_entries[seqIndex]._triggerCountdown = triggerCountdown;
	_entries[seqIndex]._doneFlag = false;
	_entries[seqIndex]._flags = 0;
	_entries[seqIndex]._dynamicHotspotIndex = -1;
	_entries[seqIndex]._entries._count = 0;
	_entries[seqIndex]._triggerMode = _vm->_game->_triggerSetupMode;

	_entries[seqIndex]._actionNouns = _vm->_game->_scene._action._activeAction;

	return seqIndex;
}

int SequenceList::addTimer(int timeout, int endTrigger) {
	Scene &scene = _vm->_game->_scene;
	uint seqIndex;
	for (seqIndex = 0; seqIndex < _entries.size(); ++seqIndex) {
		if (!_entries[seqIndex]._active)
			break;
	}
	assert(seqIndex < _entries.size());

	SequenceEntry &se = _entries[seqIndex];
	se._active = true;
	se._spritesIndex = -1;
	se._numTicks = timeout;
	se._extraTicks = 0;
	se._timeout = scene._frameStartTime + timeout;
	se._triggerCountdown = true;
	se._doneFlag = false;
	se._entries._count = 0;
	se._triggerMode = _vm->_game->_triggerSetupMode;
	se._actionNouns = _vm->_game->_scene._action._activeAction;
	addSubEntry(seqIndex, SEQUENCE_TRIGGER_EXPIRE, 0, endTrigger);

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

int SequenceList::findByTrigger(int trigger) {
	for (uint idx = 0; idx < _entries.size(); ++idx) {
		if (_entries[idx]._active) {
			for (int subIdx = 0; subIdx < _entries[idx]._entries._count; ++subIdx) {
				if (_entries[idx]._entries._trigger[subIdx] == trigger)
					return idx;
			}
		}
	}

	return -1;
}

void SequenceList::setSpriteSlot(int seqIndex, SpriteSlot &spriteSlot) {
	Scene &scene = _vm->_game->_scene;
	SequenceEntry &timerEntry = _entries[seqIndex];
	SpriteAsset &spriteSet = *scene._sprites[timerEntry._spritesIndex];

	spriteSlot._flags = spriteSet.isBackground() ? IMG_DELTA : IMG_UPDATE;
	spriteSlot._seqIndex = seqIndex;
	spriteSlot._spritesIndex = timerEntry._spritesIndex;
	spriteSlot._frameNumber = timerEntry._flipped ? -timerEntry._frameIndex : timerEntry._frameIndex;
	spriteSlot._depth = timerEntry._depth;
	spriteSlot._scale = timerEntry._scale;

	if (!timerEntry._nonFixed) {
		spriteSlot._position = timerEntry._position;
	} else {
		MSprite *sprite = spriteSet.getFrame(timerEntry._frameIndex - 1);
		spriteSlot._position = sprite->_offset;
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
	} else if ((slotIndex = scene._spriteSlots.add()) >= 0) {
		SpriteSlot &spriteSlot = scene._spriteSlots[slotIndex];
		setSpriteSlot(seqIndex, spriteSlot);

		if ((seqEntry._flags != 0) || (seqEntry._dynamicHotspotIndex >= 0)) {
			SpriteAsset &spriteSet = *scene._sprites[seqEntry._spritesIndex];
			MSprite *frame = spriteSet.getFrame(seqEntry._frameIndex - 1);
			int width = frame->w * seqEntry._scale / 200;
			int height = frame->h * seqEntry._scale / 100;
			Common::Point pt = spriteSlot._position;

			// Handle sprite movement, if present
			if (seqEntry._flags & 1) {
				seqEntry._posAccum.x += seqEntry._posDiff.x;
				if (seqEntry._posAccum.x >= 100) {
					int v = seqEntry._posAccum.x / 100;
					seqEntry._position.x += v * seqEntry._posSign.x;
					seqEntry._posAccum.x -= v * 100;
				}

				seqEntry._posAccum.y += seqEntry._posDiff.y;
				if (seqEntry._posAccum.y >= 100) {
					int v = seqEntry._posAccum.y / 100;
					seqEntry._position.y += v * seqEntry._posSign.y;
					seqEntry._posAccum.y -= v * 100;
				}
			}

			if (seqEntry._flags & 2) {
				// Check for object having moved off-screen
				if ((pt.x + width) < 0 || (pt.x + width) >= MADS_SCREEN_WIDTH ||
						pt.y < 0 || (pt.y - height) >= MADS_SCENE_HEIGHT) {
					result = true;
					seqEntry._doneFlag = true;
				}
			}

			if (seqEntry._dynamicHotspotIndex >= 0) {
				DynamicHotspot &dynHotspot = scene._dynamicHotspots[seqEntry._dynamicHotspotIndex];

				dynHotspot._bounds.left = MAX(pt.x - width, 0);
				dynHotspot._bounds.top = MAX(pt.y - height, 0);
				dynHotspot._bounds.right = dynHotspot._bounds.left + width + 1;
				dynHotspot._bounds.bottom = dynHotspot._bounds.top + height + 1;

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
				} else {
					// Switch into reverse mode
					seqEntry._frameIndex = seqEntry._numSprites - 1;
					seqEntry._frameInc = -1;
				}
			}
		} else {
			// Currently in reverse mode and moved past starting frame
			result = true;

			if (seqEntry._animType == ANIMTYPE_CYCLED)
			{
				// Switch back to forward direction again
				seqEntry._frameIndex = seqEntry._frameStart + 1;
				seqEntry._frameInc = 1;
			} else {
				// Otherwise reset back to last sprite for further reverse animating
				seqEntry._frameIndex = seqEntry._numSprites;
			}
		}

		if (result && (seqEntry._triggerCountdown != 0)) {
			if (--seqEntry._triggerCountdown == 0)
				seqEntry._doneFlag = true;
		}
	} else {
		// Out of sprite display slots, so mark entry as done
		seqEntry._doneFlag = true;
	}

	for (int i = 0; i < seqEntry._entries._count; ++i) {
		switch (seqEntry._entries._mode[i]) {
		case SEQUENCE_TRIGGER_EXPIRE:
		case SEQUENCE_TRIGGER_LOOP:
			if (((seqEntry._entries._mode[i] == SEQUENCE_TRIGGER_EXPIRE) && seqEntry._doneFlag) ||
				((seqEntry._entries._mode[i] == SEQUENCE_TRIGGER_LOOP) && result))
				idx = i;
			break;

		case SEQUENCE_TRIGGER_SPRITE: {
			int v = seqEntry._entries._frameIndex[i];
			if ((v == seqEntry._frameIndex) || (v == 0))
				idx = i;
			break;
		}

		default:
			break;
		}
	}

	if (idx >= 0) {
		_vm->_game->_trigger = seqEntry._entries._trigger[idx];
		_vm->_game->_triggerMode = seqEntry._triggerMode;

		if (seqEntry._triggerMode != SEQUENCE_TRIGGER_DAEMON)
			scene._action._activeAction = seqEntry._actionNouns;
	}

	return result;
}

/**
* Handles counting down entries in the timer list for action
*/
void SequenceList::tick() {
	Scene &scene = _vm->_game->_scene;
	for (uint idx = 0; idx < _entries.size(); ++idx) {
		if ((_vm->_game->_fx == 0) && (_vm->_game->_trigger != 0))
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
	int tempStart, tempEnd;

	switch (startVal) {
	case -2:
		tempStart = numSprites;
		break;
	case -1:
		tempStart = 1;
		break;
	default:
		tempStart = startVal;
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
		tempEnd = endVal;
		break;
	}

	seqEntry._frameStart = tempStart;
	seqEntry._numSprites = tempEnd;

	seqEntry._frameIndex = (seqEntry._frameInc >= 0) ? tempStart : tempEnd;
}

void SequenceList::scan() {
	Scene &scene = _vm->_game->_scene;

	for (uint i = 0; i < _entries.size(); ++i) {
		if (_entries[i]._active && (_entries[i]._spritesIndex != -1)) {
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

void SequenceList::setPosition(int seqIndex, const Common::Point &pt) {
	_entries[seqIndex]._position = pt;
	_entries[seqIndex]._nonFixed = false;
}

int SequenceList::addSpriteCycle(int srcSpriteIdx, bool flipped, int numTicks, int triggerCountdown, int timeoutTicks, int extraTicks) {
	Scene &scene = _vm->_game->_scene;
	MSprite *spriteFrame = scene._sprites[srcSpriteIdx]->getFrame(0);
	int depth = scene._depthSurface.getDepth(Common::Point(
		spriteFrame->_offset.x + (spriteFrame->w / 2),
		spriteFrame->_offset.y + (spriteFrame->h / 2)));

	return add(srcSpriteIdx, flipped, 1, triggerCountdown, timeoutTicks, extraTicks, numTicks, 0, 0,
		true, 100, depth - 1, 1, ANIMTYPE_CYCLED, 0, 0);
}

int SequenceList::addReverseSpriteCycle(int srcSpriteIdx, bool flipped, int numTicks,
		int triggerCountdown, int timeoutTicks, int extraTicks) {
	Scene &scene = _vm->_game->_scene;

	SpriteAsset *asset = scene._sprites[srcSpriteIdx];
	MSprite *spriteFrame = asset->getFrame(0);
	int depth = scene._depthSurface.getDepth(Common::Point(
		spriteFrame->_offset.x + (spriteFrame->w / 2),
		spriteFrame->_offset.y + (spriteFrame->h / 2)));

	return add(srcSpriteIdx, flipped, asset->getCount(), triggerCountdown, timeoutTicks, extraTicks,
		numTicks, 0, 0, true, 100, depth - 1, -1, ANIMTYPE_CYCLED, 0, 0);
}


int SequenceList::startCycle(int srcSpriteIndex, bool flipped, int cycleIndex) {
	int result = addSpriteCycle(srcSpriteIndex, flipped, INDEFINITE_TIMEOUT, 0, 0, 0);
	if (result >= 0)
		setAnimRange(result, cycleIndex, cycleIndex);

	return result;
}

int SequenceList::startPingPongCycle(int srcSpriteIndex, bool flipped, int numTicks,
		int triggerCountdown, int timeoutTicks, int extraTicks) {
	SpriteAsset *sprites = _vm->_game->_scene._sprites[srcSpriteIndex];
	MSprite *frame = sprites->getFrame(0);
	int depth = _vm->_game->_scene._depthSurface.getDepth(Common::Point(
		frame->_offset.x + frame->w / 2, frame->_offset.y + frame->h / 2));

	return add(srcSpriteIndex, flipped, 1, triggerCountdown, timeoutTicks,
		extraTicks, numTicks, 0, 0, true, 100, depth - 1, 1, ANIMTYPE_PING_PONG, 0, 0);
}

void SequenceList::updateTimeout(int srcSeqIndex, int destSeqIndex) {
	Player &player = _vm->_game->_player;
	int timeout;

	if (srcSeqIndex >= 0)
		timeout = _entries[srcSeqIndex]._timeout;
	else
		timeout = player._priorTimer + player._ticksAmount;

	if (destSeqIndex >= 0)
		_entries[destSeqIndex]._timeout = timeout;
	else
		player._priorTimer = timeout - player._ticksAmount;

}

void SequenceList::setScale(int spriteIdx, int scale) {
	_entries[spriteIdx]._scale = scale;
}

void SequenceList::setMsgLayout(int seqIndex) {
	Player &player = _vm->_game->_player;
	int yp = player._playerPos.y + (player._centerOfGravity * player._currentScale) / 100;
	setPosition(seqIndex, Common::Point(player._playerPos.x, yp));
	setDepth(seqIndex, player._currentDepth);
	setScale(seqIndex, player._currentScale);
	updateTimeout(-1, seqIndex);
}

void SequenceList::setDone(int seqIndex) {
	_entries[seqIndex]._doneFlag = true;
	_entries[seqIndex]._timeout = _vm->_game->_player._priorTimer;
}

void SequenceList::setMotion(int seqIndex, int flags, int deltaX, int deltaY) {
	SequenceEntry &se = _entries[seqIndex];
	se._flags = flags | 1;

	// Set the direction sign for movement
	if (deltaX > 0) {
		se._posSign.x = 1;
	} else if (deltaX < 0) {
		se._posSign.x = -1;
	} else {
		se._posSign.x = 0;
	}

	if (deltaY > 0) {
		se._posSign.y = 1;
	} else if (deltaY < 0) {
		se._posSign.y = -1;
	} else {
		se._posSign.y = 0;
	}

	se._posDiff.x = ABS(deltaX);
	se._posDiff.y = ABS(deltaY);
	se._posAccum.x = se._posAccum.y = 0;
}

int SequenceList::addStampCycle(int srcSpriteIdx, bool flipped, int sprite) {
	int id;

	id = addSpriteCycle(srcSpriteIdx, flipped, 32767, 0, 0, 0);
	if (id >= 0) {
		setAnimRange(id, sprite, sprite);
		_entries[id]._animType = ANIMTYPE_STAMP;
	}
	return (id);
}

void SequenceList::setSeqPlayer(int idx, bool flag) {
	Player &player = _vm->_game->_player;
	int yp = player._playerPos.y + (player._centerOfGravity * player._currentScale) / 100;
	setPosition(idx, Common::Point(player._playerPos.x, yp));
	setDepth(idx, player._currentDepth);
	setScale(idx, player._currentScale);

	if (flag)
		_vm->_game->syncTimers(SYNC_SEQ, idx, SYNC_PLAYER, 0);
}

} // End of namespace
