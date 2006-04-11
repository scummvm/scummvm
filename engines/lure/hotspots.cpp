/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "lure/hotspots.h"
#include "lure/decode.h"
#include "lure/palette.h"
#include "lure/disk.h"
#include "lure/res.h"
#include "lure/scripts.h"
#include "lure/room.h"
#include "lure/strings.h"
#include "lure/res_struct.h"
#include "lure/events.h"

namespace Lure {

Hotspot::Hotspot(HotspotData *res): _pathFinder(this) {
	_data = res;
	_anim = NULL;
	_frames = NULL;
	_numFrames = 0;
	_persistant = false;

	_hotspotId = res->hotspotId;
	_roomNumber = res->roomNumber;
	_startX = res->startX;
	_startY = res->startY;
	_destX = res->startX;
	_destY = res->startY;
	_destHotspotId = 0;
	_height = res->height;
	_width = res->width;
	_heightCopy = res->heightCopy;
	_widthCopy = res->widthCopy;
	_yCorrection = res->yCorrection;
	_talkX = res->talkX;
	_talkY = res->talkY;
	_layer = res->layer;
	_sequenceOffset = res->sequenceOffset;
	_tickCtr = res->tickTimeout;
	_actions = res->actions;
	_colourOffset = res->colourOffset;

	_override = Resources::getReference().getHotspotOverride(res->hotspotId);

	if (_data->animRecordId != 0)
		setAnimation(_data->animRecordId);

	_tickHandler = HotspotTickHandlers::getHandler(_data->tickProcOffset);
	_frameCtr = 0;
	_skipFlag = false;
	_pathfindCovered = false;
	_charRectY = 0;
}

// Special constructor used to create a voice hotspot

Hotspot::Hotspot(Hotspot *character, uint16 objType): _pathFinder(this) {
	_data = NULL;
	_anim = NULL;
	_frames = NULL;
	_numFrames = 0;
	_persistant = false;
	_hotspotId = 0xffff;
	_override = NULL;
	_colourOffset = 0;
	_destHotspotId = character->hotspotId();

	switch (objType) {
	case VOICE_ANIM_ID:
		_roomNumber = character->roomNumber();
		_destHotspotId = character->hotspotId();
		_startX = character->x() + character->talkX() + 12;
		_startY = character->y() + character->talkY() - 18;
		_destX = _startX;
		_destY = _startY;
		_layer = 1;
		_height = 18;
		_width = 32;
		_heightCopy = character->height() + 14;
		_widthCopy = 24;
		_layer = 2;
	
		_tickHandler = HotspotTickHandlers::getHandler(VOICE_TICK_PROC_ID);
		_tickCtr = 0;

		setAnimation(VOICE_ANIM_ID);
		break;

	default:
		break;
	}
}

Hotspot::~Hotspot() {
	if (_frames) delete _frames;
}

void Hotspot::setAnimation(uint16 newAnimId) {
	Resources &r = Resources::getReference();
	HotspotAnimData *tempAnim;
	if (newAnimId == 0) tempAnim = NULL;
	else tempAnim = r.getAnimation(newAnimId); 
	
	setAnimation(tempAnim);
}

void Hotspot::setAnimation(HotspotAnimData *newRecord) {
	Disk &r = Disk::getReference();
	if (_frames) {
		delete _frames;
		_frames = NULL;
	}
	_anim = NULL;
	_numFrames = 0;
	_frameNumber = 0;
	if (!newRecord) return;
	if (!r.exists(newRecord->animId)) return;

	_anim = newRecord;
	MemoryBlock *src = Disk::getReference().getEntry(_anim->animId);
	
	uint16 *numEntries = (uint16 *) src->data();
	uint16 *headerEntry = (uint16 *) (src->data() + 2);

	if ((*numEntries > 99) || (*numEntries == 0)) {
		// Wobbly, likely something wrong with the resoure
		_width = 1;
		_numFrames = 1;
		_frameNumber = 0;
		_frames = new Surface(1, 1);
		_frames->data().setBytes(_colourOffset, 0, 1);
		return;
	}

	// Calculate total needed size for output and create memory block to hold it
	uint32 totalSize = 0;
	for (uint16 ctr = 0; ctr < *numEntries; ++ctr, ++headerEntry) {
		totalSize += (*headerEntry + 31) / 32;
	}
	totalSize = (totalSize + 0x81) << 4;
	MemoryBlock *dest = Memory::allocate(totalSize);

	uint32 srcStart = (*numEntries + 1) * sizeof(uint16) + 6;
	AnimationDecoder::decode_data(src, dest, srcStart);

	_numFrames = *numEntries;
	_frameNumber = 0;
	
	_frames = new Surface(_width * _numFrames, _height);

	_frames->data().setBytes(_colourOffset, 0, _frames->data().size());

	byte *pSrc = dest->data() + 0x40;
	byte *pDest;
	headerEntry = (uint16 *) (src->data() + 2);
	MemoryBlock &mDest = _frames->data();

	for (uint16 frameNumCtr = 0; frameNumCtr < _numFrames; ++frameNumCtr, ++headerEntry) {

		if ((newRecord->flags & PIXELFLAG_HAS_TABLE) != 0) {
			// For animations with an offset table, set the source point for each frame
			uint16 frameOffset = *((uint16 *) (src->data() + ((frameNumCtr + 1) * sizeof(uint16)))) + 0x40;
			if ((uint32) frameOffset + _height * (_width / 2) > dest->size()) 
				error("Invalid frame offset in animation %x", newRecord->animRecordId);
			pSrc = dest->data() + frameOffset;
		}

		// Copy over the frame, applying the colour offset to each nibble
		for (uint16 yPos = 0; yPos < _height; ++yPos) {
			pDest = mDest.data() + (yPos * _numFrames + frameNumCtr) * _width;

			for (uint16 ctr = 0; ctr < _width / 2; ++ctr) {
				*pDest++ = _colourOffset + (*pSrc >> 4);
				*pDest++ = _colourOffset + (*pSrc & 0xf);
				++pSrc;
			}
		}
	}

	delete src;
	delete dest;
}

void Hotspot::copyTo(Surface *dest) {
	int16 xPos = _startX;
	int16 yPos = _startY;
	uint16 hWidth = _width;
	uint16 hHeight = _height;

	Rect r(_frameNumber * hWidth, 0, (_frameNumber + 1) * hWidth - 1, 
		hHeight - 1);

	if (yPos < 0) {
		if (yPos + hHeight <= 0) 
			// Completely off screen, so don't display
			return;

		// Reduce the source rectangle to only the on-screen portion
		r.top = -yPos;
		yPos = 0;
	}

	if (xPos < 0) {
		if (xPos + hWidth <= 0)
			// Completely off screen, so don't display
			return;

		// Reduce the source rectangle to only the on-screen portion
		r.left = -xPos;
		xPos = 0;
	}

	if (xPos >= FULL_SCREEN_WIDTH) 
		return;
	else if (xPos + hWidth > FULL_SCREEN_WIDTH)
		r.right = (_frameNumber * hWidth) + (FULL_SCREEN_WIDTH - xPos) - 1;
	if (yPos >= FULL_SCREEN_HEIGHT)
		return;
	else if (yPos + hHeight > FULL_SCREEN_HEIGHT)
		r.bottom = FULL_SCREEN_HEIGHT - yPos - 1;

	_frames->copyTo(dest, r, (uint16) xPos, (uint16) yPos, _colourOffset);
}

void Hotspot::incFrameNumber() {
	++_frameNumber;
	if (_frameNumber >= _numFrames) 
		_frameNumber = 0;
}

bool Hotspot::isActiveAnimation() {
	return ((_numFrames != 0) && (_layer != 0));
}

uint16 Hotspot::nameId() {
	if (_data == NULL)
		return 0;
	else 
		return _data->nameId;
}

void Hotspot::setPosition(int16 newX, int16 newY) {
	_startX = newX;
	_startY = newY;
	if (_data) {
		_data->startX = newX;
		_data->startY = newY;
	}
}

void Hotspot::setSize(uint16 newWidth, uint16 newHeight) {
	_width = newWidth;
	_height = newHeight;
}

bool Hotspot::executeScript() {
	if (_data->sequenceOffset == 0)
		return false;
	else
		return HotspotScript::execute(this);
}

void Hotspot::tick() {
	_tickHandler(*this);
}

void Hotspot::setTickProc(uint16 newVal) {
	if (_data)
		_data->tickProcOffset = newVal;
	_tickHandler = HotspotTickHandlers::getHandler(newVal);	
}

void Hotspot::walkTo(int16 endPosX, int16 endPosY, uint16 destHotspot) {
	if ((hotspotId() == PLAYER_ID) && (PATHFIND_COUNTDOWN != 0)) {
		// Show the clock cursor whilst pathfinding will be calculated
		Mouse &mouse = Mouse::getReference();
		mouse.setCursorNum(CURSOR_TIME_START, 0, 0);
	}

	_destX = endPosX;
	_destY = endPosY;
	_destHotspotId = destHotspot;
	_currentActions.clear();
	setCurrentAction(START_WALKING);
}

void Hotspot::setDirection(Direction dir) {
	_direction = dir;

	switch (dir) {
	case UP:
		setFrameNumber(_anim->upFrame);
		_charRectY = 4;
		break;
	case DOWN:
		setFrameNumber(_anim->downFrame);
		_charRectY = 4;
		break;
	case LEFT:
		setFrameNumber(_anim->leftFrame);
		_charRectY = 0;
		break;
	case RIGHT:
		setFrameNumber(_anim->rightFrame);
		_charRectY = 0;
		break;
	default:
		break;
	}
}

// Makes the character face the given hotspot

void Hotspot::faceHotspot(HotspotData *hotspot) {
	if (hotspot->hotspotId >= START_NONVISUAL_HOTSPOT_ID) {
		// Non visual hotspot
		// TODO:
	} else {
		// Visual hotspot
		int xp = x() - hotspot->startX;
		int yp = y() + heightCopy() - (hotspot->startY + hotspot->heightCopy);

		if (abs(yp) >= abs(xp)) {
			if (yp < 0) setDirection(DOWN);
			else setDirection(UP);
		} else {
			if (xp < 0) setDirection(LEFT);
			else setDirection(RIGHT);
		}
	}
}

// Sets or clears the hotspot as occupying an area in it's room's pathfinding data

void Hotspot::setOccupied(bool occupiedFlag) {
	if (occupiedFlag == _pathfindCovered) return;
	_pathfindCovered = occupiedFlag;

	int yp = (y() - 8 + heightCopy() - 4) >> 3;
	int widthVal = MAX((widthCopy() >> 3), 1);

	// Handle cropping for screen left
	int xp = (x() >> 3) - 16;
	if (xp < 0) {
		xp = -xp;
		widthVal -= xp;
		if (widthVal <= 0) return;
		xp = 0;
	}

	// Handle cropping for screen right
	int x2 = xp + widthVal;
	if (x2 > ROOM_PATHS_WIDTH) {
		++x2;
		widthVal -= x2;
		if (widthVal <= 0) return;
	}

	RoomPathsData &paths = Resources::getReference().getRoom(_roomNumber)->paths;
	if (occupiedFlag) {
		paths.setOccupied(xp, yp, widthVal);
	} else {
		paths.clearOccupied(xp, yp, widthVal);
	}
}

// walks the character a single step in a sequence defined by the walking list

bool Hotspot::walkingStep() {
	if (_pathFinder.isEmpty())	return true;

	// Check to see if the end of the next straight walking slice
	if (_pathFinder.stepCtr() >= _pathFinder.top().numSteps()) {
		// Move to next slice in walking sequence
		_pathFinder.stepCtr() = 0;
		_pathFinder.pop();
		if (_pathFinder.isEmpty())	return true;
	}

	if (_pathFinder.stepCtr() == 0)
		// At start of new slice, set the direction
		setDirection(_pathFinder.top().direction());

	MovementDataList *frameSet;
	switch (_pathFinder.top().direction()) {
	case UP:
		frameSet = &_anim->upFrames;
		break;
	case DOWN:
		frameSet = &_anim->downFrames;
		break;
	case LEFT:
		frameSet = &_anim->leftFrames;
		break;
	case RIGHT:
		frameSet = &_anim->rightFrames;
		break;
	default:
		return true;
	}

	int16 _xChange, _yChange;
	uint16 nextFrame;
	frameSet->getFrame(frameNumber(), _xChange, _yChange, nextFrame);
	setFrameNumber(nextFrame);
	setPosition(x() + _xChange, y() + _yChange);

	++_pathFinder.stepCtr();
	return false;
}

/*-------------------------------------------------------------------------*/
/* Hotspot action handling                                                 */
/*                                                                         */
/*-------------------------------------------------------------------------*/

uint16 validRoomExitHotspots[] = {0x2711, 0x2712, 0x2714, 0x2715, 0x2716, 0x2717,
	0x2718, 0x2719, 0x271A, 0x271E, 0x271F, 0x2720, 0x2721, 0x2722, 0x2725, 0x2726,
	0x2729, 0x272A, 0x272B, 0x272C, 0x272D, 0x272E, 0x272F, 0};

bool Hotspot::isRoomExit(uint16 id) {
	for (uint16 *p = &validRoomExitHotspots[0]; *p != 0; ++p) 
		if (*p == id) return true;
	return false;
}

HotspotPrecheckResult Hotspot::actionPrecheck(HotspotData *hotspot) {
	if ((hotspot->hotspotId == 0x420) || (hotspot->hotspotId == 0x436) ||
		(hotspot->hotspotId == 0x429)) {
		// TODO: figure out specific handling code
		actionPrecheck3(hotspot);
		return PC_0;
	} else {
		return actionPrecheck2(hotspot);
	}
}

HotspotPrecheckResult Hotspot::actionPrecheck2(HotspotData *hotspot) {
	ValueTableData fields = Resources::getReference().fieldList();

	if (hotspot->roomNumber != roomNumber()) {
		// Hotspot isn't in same room as character
		if (frameNumber() != 0) {
			Dialog::showMessage(0, hotspotId());
			setFrameNumber(0);
		}
		return PC_1;
	} else if (frameNumber() != 0) {
		// TODO: loc_883
		setFrameNumber(frameNumber() + 1);
		if (frameNumber() >= 6) {
			Dialog::showMessage(0xD, hotspotId());
			return PC_4;
		}

		if ((hotspot->hotspotId < 0x408)) {
			// TODO: Does other checks on HS[44] -> loc_886
			setFrameNumber(0);
			Dialog::showMessage(0xE, hotspotId());
			return PC_2;
		}
	}

	if (characterWalkingCheck(hotspot)) {
		return PC_INITIAL;
	} else {
		actionPrecheck3(hotspot);
		return PC_0;
	}
}

void Hotspot::actionPrecheck3(HotspotData *hotspot) {
	setFrameNumber(0);
	if (hotspot->hotspotId < 0x408) {
		// TODO: HS[44]=8, HS[42]=1E, HS[50]=ID
	}
}

// Checks to see whether a character needs to walk to the given hotspot

bool Hotspot::characterWalkingCheck(HotspotData *hotspot) {
	Resources &res = Resources::getReference();
	HotspotProximityList &list = res.proximityList();
	HotspotProximityList::iterator i;
	int16 xp, yp;

	// Get default position
	xp = hotspot->startX;
	yp = hotspot->startY + hotspot->heightCopy - 4;

	// Scan through the list for a proximity record
	for (i = list.begin(); i != list.end(); ++i) {
		HotspotProximityData *rec = *i;
		if (rec->hotspotId != hotspot->hotspotId) continue;

		xp = (int16) rec->x;
		yp = (int16) (rec->y & 0x7fff);

		// If the high bit of the Y position is clear, use standard handling
		// with the co-ordinates provided by the record
		if ((rec->y & 0x8000) == 0) 
			break;
		
		// Special handling for if hi-bit of Y is set
		if (((x() >> 3) != (xp >> 3)) ||
			((((y() + heightCopy()) >> 3) - 1) != (yp >> 3))) {
			walkTo(xp, yp);
			return true;
		} else {
			return false;
		}
	}

	// Default handling 
	if ((abs(x() - xp) < 8) && (abs(y() + heightCopy() - 1 - yp) < 19))
		return false;
	
	walkTo(xp, yp);
	return true;
}

void Hotspot::doAction(Action action, HotspotData *hotspot) {
	switch (action) {
	case GET:
		doGet(hotspot);
		break;
	case PUSH:
	case PULL:
	case OPERATE:
		doOperate(hotspot, action);
		break;
	case OPEN:
		doOpen(hotspot);
		break;
	case CLOSE:
		doClose(hotspot);
		break;
	case LOCK:
		doSimple(hotspot, LOCK);
		break;
	case UNLOCK:
		doSimple(hotspot, UNLOCK);
		break;
	case USE:
		doUse(hotspot);
		break;
	case GIVE:
		doGive(hotspot);
		break;
	case TALK_TO:
		doTalkTo(hotspot);
		break;
	case TELL:
		doTell(hotspot);
		break;
	case LOOK:
		doLook();
		break;
	case LOOK_AT:
		doLookAt(hotspot);
		break;
	case LOOK_THROUGH:
		doSimple(hotspot, LOOK_THROUGH);
		break;
	case ASK:
		doAsk(hotspot);
		break;
	case DRINK:
		doDrink();
		break;
	case STATUS:
		doStatus();
		break;
	case BRIBE:
		doBribe(hotspot);
		break;
	case EXAMINE:
		doExamine();
		break;
	default:
		doSimple(hotspot, action);
		break;
	}	
}

void Hotspot::doGet(HotspotData *hotspot) {
	Resources &res = Resources::getReference();

	HotspotPrecheckResult result = actionPrecheck(hotspot);
	if (result == PC_INITIAL) return;
	else if (result != PC_0) {
		stopWalking();
		return;
	}

	stopWalking();
	faceHotspot(hotspot);

	uint16 sequenceOffset = res.getHotspotAction(hotspot->actionsOffset, GET);
	if (sequenceOffset >= 0x8000) {
		Dialog::showMessage(sequenceOffset, hotspotId());
		return;
	} 
	
	if (sequenceOffset != 0) {
		uint16 result = Script::execute(sequenceOffset);

		if (result == 1) return;
		else if (result != 0) {
			Dialog::showMessage(result, hotspotId());
			return;
		}
	}

	// Move hotspot into characters's inventory
	hotspot->roomNumber = hotspotId();  

	if (hotspot->hotspotId < START_NONVISUAL_HOTSPOT_ID) {
		// Deactive hotspot animation
		Resources::getReference().deactivateHotspot(hotspot->hotspotId);
		// Remove any 'on the ground' description for the hotspot
		hotspot->descId2 = 0;
	}
}

void Hotspot::doOperate(HotspotData *hotspot, Action action) {
	Resources &res = Resources::getReference();
	uint16 sequenceOffset = res.getHotspotAction(hotspot->actionsOffset, action);

	if (sequenceOffset >= 0x8000) {
		Dialog::showMessage(sequenceOffset, hotspotId());
	} else if (sequenceOffset != 0) {
		uint16 result = Script::execute(sequenceOffset);
		if (result > 1)
			Dialog::showMessage(result, hotspotId());
	}
}

void Hotspot::doOpen(HotspotData *hotspot) {
	Resources &res = Resources::getReference();
	RoomExitJoinData *joinRec;

	if (isRoomExit(hotspot->hotspotId)) {
		joinRec = res.getExitJoin(hotspot->hotspotId);
		if (!joinRec->blocked) {
			// Room exit is already open
			Dialog::showMessage(4, hotspotId());
			// TODO: jmp loc_1102
			return;
		}
	}

	// TODO: Call to sub_107 and checking the results, then sub_110

	uint16 sequenceOffset = res.getHotspotAction(hotspot->actionsOffset, OPEN);
	if (sequenceOffset >= 0x8000) {
		// Message to display
		Dialog::showMessage(sequenceOffset, hotspotId());
	} else if (sequenceOffset != 0) {
		// Otherwise handle script
		uint16 result = Script::execute(sequenceOffset);

		if (result == 0) {
			joinRec = res.getExitJoin(hotspot->hotspotId);
			if (joinRec->blocked) {
				joinRec->blocked = 0;

				if (hotspotId() != PLAYER_ID) {
					// TODO: HS[44h]=3, HS[42h]W = 4
				}
			}
		} else if (result != 1) {
			// TODO: Figure out: if Hotspot-rec[60h] != 0, then set = 4
			Dialog::showMessage(result, hotspotId());
		}
	}
}

void Hotspot::doClose(HotspotData *hotspot) {
	Resources &res = Resources::getReference();
	RoomExitJoinData *joinRec;

	if (isRoomExit(hotspot->hotspotId)) {
		joinRec = res.getExitJoin(hotspot->hotspotId);
		if (joinRec->blocked) {
			// Room exit is already closed/blocked
			Dialog::showMessage(3, hotspotId());
			// TODO: jmp sub_129
			return;
		}
	}

	// TODO: Call to sub_107 and checking the results, then sub_110

	uint16 sequenceOffset = res.getHotspotAction(hotspot->actionsOffset, CLOSE);

	if (sequenceOffset >= 0x8000) {
		// Message to display
		Dialog::showMessage(sequenceOffset, hotspotId());
	} else if (sequenceOffset != 0) {
		// Otherwise handle script
		uint16 result = Script::execute(sequenceOffset);

		if (result != 0) {
			Dialog::showMessage(result, hotspotId());
		} else {
			joinRec = res.getExitJoin(hotspot->hotspotId);
			if (!joinRec->blocked) {
				// Close the door
				// TODO: Decode sub_183 - does check to see if door is 'jammed', but
				// a cursory inspection seems to indicate that the routine is more
				// concerned with checking if any character is blocking the door
//				if (!sub183(joinRec->0Dh) || !sub183(joinRec->0Fh)) {
//					Dialog::showMessage(2, hotspotId());
//				} else {
					joinRec->blocked = 1;
//				}
			}
		}
	}
}

void Hotspot::doUse(HotspotData *hotspot) {
	Resources &res = Resources::getReference();
//	uint16 usedId = res.fieldList().getField(USE_HOTSPOT_ID);
	uint16 sequenceOffset = res.getHotspotAction(hotspot->actionsOffset, USE);

	if (sequenceOffset >= 0x8000) {
		Dialog::showMessage(sequenceOffset, hotspotId());
	} else if (sequenceOffset == 0) {
		Dialog::showMessage(17, hotspotId());
	} else {
		uint16 result = Script::execute(sequenceOffset);
		if (result != 0) 
			Dialog::showMessage(result, hotspotId());
	}
}

void Hotspot::doGive(HotspotData *hotspot) {
	Resources &res = Resources::getReference();
	uint16 usedId = res.fieldList().getField(USE_HOTSPOT_ID);
	uint16 sequenceOffset = res.getHotspotAction(hotspot->actionsOffset, GIVE);

	if (sequenceOffset >= 0x8000) {
		Dialog::showMessage(sequenceOffset, hotspotId());
	} else {
		uint16 result = Script::execute(sequenceOffset);
		if (result == 0x3E7) {
			// TODO
		} else if (result == 0) {
			// Move item into character's inventory
			HotspotData *usedItem = res.getHotspot(usedId);
			usedItem->roomNumber = hotspotId();
		} else if (result > 1) {
			// TODO
		}
	}
}

void Hotspot::doTalkTo(HotspotData *hotspot) {
	// TODO: still some work at start
	if ((hotspot->hotspotId != 0x3EA) && ((hotspot->roomNumber != 28) || 
		(hotspot->hotspotId != 0x3EB))) {
		// sub_107 call and after check
	}

	// Validate character is in player's room - since currently you can activate
	// hotspots when you're not in the room
	if (hotspot->roomNumber != hotspot->roomNumber) return;

	Resources &res = Resources::getReference();
	uint16 sequenceOffset = res.getHotspotAction(hotspot->actionsOffset, TALK_TO);

	if (sequenceOffset >= 0x8000) {
		Dialog::showMessage(sequenceOffset, hotspotId());
	} else if (sequenceOffset == 0) {
		startTalk(hotspot);
	} else {
		uint16 result = Script::execute(sequenceOffset);

		if (result == 0) {
			// Start talking with character
			startTalk(hotspot);
		}
	}
}

void Hotspot::doTell(HotspotData *hotspot) {
	// TODO
}

void Hotspot::doLook() {
	Dialog::show(Room::getReference().descId());
}

void Hotspot::doLookAt(HotspotData *hotspot) {
	Resources &res = Resources::getReference();
	uint16 sequenceOffset = res.getHotspotAction(hotspot->actionsOffset, LOOK_AT);
	
	if (sequenceOffset >= 0x8000) {
		Dialog::showMessage(sequenceOffset, hotspotId());
	} else {
		if (sequenceOffset != 0) 
			sequenceOffset = Script::execute(sequenceOffset);

		if (sequenceOffset == 0) {
			uint16 descId = (hotspot->descId2 != 0) ? hotspot->descId2 : hotspot->descId;
			Dialog::show(descId);
		}
	}
}

void Hotspot::doAsk(HotspotData *hotspot) {
	// TODO
}

void Hotspot::doDrink() {
	Resources &res = Resources::getReference();
	uint16 usedId = res.fieldList().getField(USE_HOTSPOT_ID);
	HotspotData *hotspot = res.getHotspot(usedId);
	uint16 sequenceOffset = res.getHotspotAction(hotspot->actionsOffset, DRINK);

	if (sequenceOffset >= 0x8000) {
		Dialog::showMessage(sequenceOffset, hotspotId());
	} else if (sequenceOffset == 0) {
		Dialog::showMessage(22, hotspotId());
	} else {
		uint16 result = Script::execute(sequenceOffset);
		if (result == 0) {
			// Item has been drunk, so remove item from game
			hotspot->roomNumber = 0;
		} else if (result != 1) {
			Dialog::showMessage(result, hotspotId());
		}
	}
}

// doStatus
// Handle the status window

void Hotspot::doStatus() {
	char buffer[MAX_DESC_SIZE];
	uint16 numItems = 0;
	StringData &strings = StringData::getReference();
	Resources &resources = Resources::getReference();
	Room &room = Room::getReference();

	strings.getString(room.roomNumber(), buffer, NULL, NULL);
	strcat(buffer, "\n\nYou are carrying ");

	// Scan through the list and add in any items assigned to the player
	HotspotDataList &list = resources.hotspotData();
	HotspotDataList::iterator i;
	for (i = list.begin(); i != list.end(); ++i) {
		HotspotData *rec = *i;

		if (rec->roomNumber == PLAYER_ID) {
			if (numItems++ == 0) strcat(buffer, ": ");
			else strcat(buffer, ", ");
			strings.getString(rec->nameId, buffer + strlen(buffer), NULL, NULL);
		}
	}

	// If there were no items, add in the word 'nothing'
	if (numItems == 0) strcat(buffer, "nothing.");

	// If the player has money, add it in
	uint16 numGroats = resources.fieldList().numGroats();
	if (numGroats > 0) {
		sprintf(buffer + strlen(buffer), "\n\nYou have %d groat", numGroats);
		if (numGroats > 1) strcat(buffer, "s");
	}

	// Display the dialog
	Screen &screen = Screen::getReference();
	Mouse &mouse = Mouse::getReference();
	mouse.cursorOff();

	Surface *s = Surface::newDialog(INFO_DIALOG_WIDTH, buffer);
	s->copyToScreen(INFO_DIALOG_X, (FULL_SCREEN_HEIGHT-s->height())/2);

	Events::getReference().waitForPress();
	screen.update();
	mouse.cursorOn();
}

void Hotspot::doBribe(HotspotData *hotspot) {
	Dialog::show("Yet to do");
}

void Hotspot::doExamine() {
	Resources &res = Resources::getReference();
	uint16 usedId = res.fieldList().getField(USE_HOTSPOT_ID);
	HotspotData *hotspot = res.getHotspot(usedId);
	uint16 sequenceOffset = res.getHotspotAction(hotspot->actionsOffset, EXAMINE);
	
	if (sequenceOffset >= 0x8000) {
		Dialog::showMessage(sequenceOffset, hotspotId());
	} else {
		if (sequenceOffset != 0) 
			sequenceOffset = Script::execute(sequenceOffset);

		if (sequenceOffset == 0) {
			Dialog::show(hotspot->descId);
		}
	}
}

void Hotspot::doSimple(HotspotData *hotspot, Action action) {
	Resources &res = Resources::getReference();
	uint16 sequenceOffset = res.getHotspotAction(hotspot->actionsOffset, action);

	if (sequenceOffset >= 0x8000) {
		Dialog::showMessage(sequenceOffset, hotspotId());
	} else if (sequenceOffset != 0) {
		Script::execute(sequenceOffset);
	}
}

void Hotspot::startTalk(HotspotData *charHotspot) {
	Resources &res = Resources::getReference();
	uint16 talkIndex;

	setTickProc(TALK_TICK_PROC_ID);    // Set for providing talk listing

	// Get offset of talk set to use
	TalkHeaderData *headerEntry = res.getTalkHeader(charHotspot->hotspotId);
	uint16 talkOffset;

	// Calculate talk index to use
	if (charHotspot->nameId == STRANGER_ID)
		talkIndex = 0;
	else
		talkIndex = res.fieldList().getField(TALK_INDEX) + 1;
	talkOffset = headerEntry->getEntry(talkIndex);

	// Set the active talk data
	res.setTalkStartEntry(0);
	res.setTalkData(talkOffset);
	if (!res.getTalkData()) 
		error("Talk failed - invalid offset: Character=%xh, index=%d, offset=%xh",
			charHotspot->hotspotId, talkIndex, talkOffset);
}

/*------------------------------------------------------------------------*/

HandlerMethodPtr HotspotTickHandlers::getHandler(uint16 procOffset) {
	switch (procOffset) {
	case 0x4F82:
		return standardCharacterAnimHandler;
	case 0x7F3A:
		return standardAnimHandler;
	case 0x7207:
		return roomExitAnimHandler;
	case PLAYER_TICK_PROC_ID:
		return playerAnimHandler;
	case 0x7F69:
		return droppingTorchAnimHandler;
	case 0x8009:
		return fireAnimHandler;
	case TALK_TICK_PROC_ID:
		return talkAnimHandler;
	case 0x8241:
		return headAnimationHandler;
	default:
		return defaultHandler;
	}
}

void HotspotTickHandlers::defaultHandler(Hotspot &h) {
	// No handling done
}

void HotspotTickHandlers::standardAnimHandler(Hotspot &h) {
	if (h.tickCtr() > 0) 
		h.setTickCtr(h.tickCtr() - 1);
	else 
		h.executeScript();
}

void HotspotTickHandlers::standardCharacterAnimHandler(Hotspot &h) {

}

void HotspotTickHandlers::roomExitAnimHandler(Hotspot &h) {
	RoomExitJoinData *rec = Resources::getReference().getExitJoin(h.hotspotId());
	if (!rec) return;
	byte *currentFrame, *destFrame;

	if (rec->hotspot1Id == h.hotspotId()) {
		currentFrame = &rec->h1CurrentFrame; 
		destFrame = &rec->h1DestFrame;
	} else {
		currentFrame = &rec->h2CurrentFrame;
		destFrame = &rec->h2DestFrame;
	}
	
	if ((rec->blocked != 0) && (*currentFrame != *destFrame)) {
		// sub_178

		++*currentFrame;
		if (*currentFrame != *destFrame) {
			// cx=1 => sub_184 
		}
	} else if ((rec->blocked == 0) && (*currentFrame != 0)) {
		// sub_179
		if (*currentFrame == *destFrame) {
			// sub_184 and other stuff TODO
		}
		--*currentFrame;
	}

	h.setFrameNumber(*currentFrame);
}

void HotspotTickHandlers::playerAnimHandler(Hotspot &h) {
	Resources &res = Resources::getReference();
	Mouse &mouse = Mouse::getReference();
	RoomPathsData &paths = Resources::getReference().getRoom(h.roomNumber())->paths;
	PathFinder &pathFinder = h.pathFinder();
	CurrentActionStack &actions = h.currentActions();
	uint16 impingingList[MAX_NUM_IMPINGING];
	int numImpinging;
	Action hsAction;
	uint16 hotspotId;
	HotspotData *hotspot;

	// TODO: handle talk dialogs countdown if necessary
	
	numImpinging = Support::findIntersectingCharacters(h, impingingList);
	if (h.skipFlag()) {
		if (numImpinging > 0) 
			return;
		h.setSkipFlag(false);
	}

	// If a frame countdown is in progress, then decrement and exit
	if (h.frameCtr() > 0) {
		h.decrFrameCtr();
		return;
	}

	CurrentAction action = actions.action();

	switch (action) {
	case NO_ACTION:
		// Make sure there is no longer any destination
		h.setDestHotspot(0);
		break;

	case DISPATCH_ACTION:
		// Dispatch an action
		h.setDestHotspot(0);
		hsAction = actions.top().hotspotAction();
		hotspotId = actions.top().hotspotId();
		actions.pop();

		hotspot = res.getHotspot(hotspotId);
		h.doAction(hsAction, hotspot);
		break;

	case EXEC_HOTSPOT_SCRIPT:
		// A hotspot script is in progress for the player, so don't interrupt
		if (h.executeScript()) {
			// Script is finished, so pop of the execution action
			actions.pop();
		}
		break;

	case START_WALKING:
		// Start the player walking to the given destination
		h.setOccupied(false);        // clear pathfinding area

		// Reset the path finder / walking sequence
		pathFinder.reset(paths);

		// Set current action to processing walking path
		actions.pop();
		h.setCurrentAction(PROCESSING_PATH);
		// Deliberate fall through to processing walking path

	case PROCESSING_PATH:
		if (!pathFinder.process()) break;

		// Pathfinding is now complete 
		actions.pop();

		if (pathFinder.isEmpty()) {
			mouse.setCursorNum(CURSOR_ARROW);
			break;
		}

		if (mouse.getCursorNum() != CURSOR_CAMERA)
			mouse.setCursorNum(CURSOR_ARROW);
		h.setCurrentAction(WALKING);
		h.setPosition(h.x(), h.y() & 0xFFF8);
		
		// Deliberate fall through to walking

	case WALKING:
		// The character is currently moving
		if ((h.destHotspotId() != 0) && (h.destHotspotId() != 0xffff)) {
			// Player is walking to a room exit hotspot
			RoomExitJoinData *joinRec = res.getExitJoin(h.destHotspotId());
			if (joinRec->blocked) {
				// Exit now blocked, so stop walking
				actions.pop();
				break;
			}
		}

		if (h.walkingStep()) {
			// Walking done
			actions.pop();
		}
	
		// Check for whether need to change room
		Support::checkRoomChange(h);
		break;
	}
}

void HotspotTickHandlers::droppingTorchAnimHandler(Hotspot &h) {
	if (h.tickCtr() > 0) 
		h.setTickCtr(h.tickCtr() - 1);
	else {
		bool result = h.executeScript();
		if (result) {
			// Changeover to the fire on the straw
			Resources &res = Resources::getReference();
			res.deactivateHotspot(h.hotspotId());
			res.activateHotspot(0x41C);

			// Enable the fire and activate it's animation
			HotspotData *fire = res.getHotspot(0x418);
			fire->flags |= 0x80;
			fire->loadOffset = 0x7172; 
			res.activateHotspot(0x418);
		}
 	}
}

void HotspotTickHandlers::fireAnimHandler(Hotspot &h) {
	standardAnimHandler(h);
	h.setOccupied(true);
}

// Special variables used across multiple calls to talkAnimHandler
static TalkEntryData *_talkResponse;

void HotspotTickHandlers::talkAnimHandler(Hotspot &h) {
	// Talk handler
	Resources &res = Resources::getReference();
	Room &room = Room::getReference();
	ValueTableData &fields = res.fieldList();
	StringData &strings = StringData::getReference();
	Screen &screen = Screen::getReference();
	Mouse &mouse = Mouse::getReference();
	TalkSelections &talkSelections = res.getTalkSelections();
	TalkData *data = res.getTalkData();
	TalkEntryList &entries = data->entries; 
	char buffer[MAX_DESC_SIZE];
	Rect r;
	int lineNum, numLines;
	int selectedLine, responseNumber;
	bool showSelections, keepTalkingFlag;
	TalkEntryList::iterator i;
	TalkEntryData *entry;
	uint16 result, descId, charId;

	switch (res.getTalkState()) {
	case TALK_NONE:
		// Handle initial setup of talking options
		// Reset talk entry pointer list
		for (lineNum = 0; lineNum < MAX_TALK_SELECTIONS; ++lineNum)
			talkSelections[lineNum] = NULL;
		
		// Loop through list to find entries to display
		_talkResponse = NULL;
		numLines = 0;
		showSelections = false;

		i = entries.begin();
		for (lineNum = 0; lineNum < res.getTalkStartEntry(); ++lineNum)
			if (i != entries.end()) ++i;

		for (; i != entries.end(); ++i) {
			entry = *i;
			uint8 flags = (uint8) (entry->descId >> 14);
			if (flags == 3) 
				// Skip the entry
				continue;

			uint16 sequenceOffset = entry->preSequenceId & 0x3fff;
			bool showLine = sequenceOffset == 0;
			if (!showLine) 
				showLine = Script::execute(sequenceOffset) != 0;

			if (showLine) {
				talkSelections[numLines++] = entry;
				showSelections |= (entry->descId & 0x3fff) != TALK_MAGIC_ID;
			}

			if ((entry->preSequenceId & 0x8000) != 0) break;
		}

		if (showSelections && (numLines > 1))
			res.setTalkState(TALK_SELECT);
		else {
			res.setTalkState(TALK_RESPOND);
			res.setTalkSelection(1);
		}
		break;

	case TALK_SELECT:
		r.left = 0; r.right = FULL_SCREEN_WIDTH - 1;
		selectedLine = mouse.y() / MENUBAR_Y_SIZE;
		if ((selectedLine > MAX_TALK_SELECTIONS) || ((selectedLine != 0) && 
			!talkSelections[selectedLine-1]))
			selectedLine = 0;

		for (lineNum = 0; lineNum < MAX_TALK_SELECTIONS; ++lineNum) {
			if (!talkSelections[lineNum]) break;
			entry = talkSelections[lineNum];

			strings.getString(entry->descId & 0x3fff, buffer, NULL, NULL);

			// Clear line
			r.top = (lineNum + 1) * MENUBAR_Y_SIZE;
			r.bottom = r.top + MENUBAR_Y_SIZE - 1;
			screen.screen().fillRect(r, 0);

			// Display line
			byte colour = (lineNum+1 == selectedLine) ?
				DIALOG_WHITE_COLOUR : DIALOG_TEXT_COLOUR;
			screen.screen().writeString(r.left, r.top, buffer, false, colour);
		}

		if ((!mouse.lButton() && !mouse.rButton()) || (selectedLine == 0))
			break;

		// Set the talk response index to use
		res.setTalkSelection(selectedLine);
		res.setTalkState(TALK_RESPOND);
		break;

	case TALK_RESPOND:
		// Handle initial response to show the question in a talk dialog if needed
		selectedLine = res.getTalkSelection();
		entry = talkSelections[selectedLine-1];
		descId = entry->descId & 0x3fff;
		entry->descId |= 0x4000;

		if (descId != TALK_MAGIC_ID) {
			// Set up to display the question in a talk dialog
			room.setTalkDialog(PLAYER_ID, descId);
			res.setTalkState(TALK_RESPONSE_WAIT);
		} else {
			res.setTalkState(TALK_RESPOND_2);
		}
		break;

	case TALK_RESPOND_2:
		if (!_talkResponse) {
			// Handles bringing up the response talk dialog
			charId = fields.getField(ACTIVE_HOTSPOT_ID);
			selectedLine = res.getTalkSelection();
			entry = talkSelections[selectedLine-1];

			responseNumber = entry->postSequenceId;
			if ((responseNumber & 0x8000) != 0) 
				responseNumber = Script::execute(responseNumber & 0x7fff);

			do {
				_talkResponse = res.getTalkData()->getResponse(responseNumber);
				if (!_talkResponse->preSequenceId) break;
				responseNumber = Script::execute(_talkResponse->preSequenceId);
			} while (responseNumber != TALK_RESPONSE_MAGIC_ID);

			descId = _talkResponse->descId;
			if ((descId & 0x8000) != 0)
				descId = Script::execute(descId & 0x7fff);

			if (descId != TALK_MAGIC_ID) {
				room.setTalkDialog(charId, descId);
				res.setTalkState(TALK_RESPONSE_WAIT);
				return;
			} 
		}

		// Handle checking whether to keep talking
		result = _talkResponse->postSequenceId;
		if (result == 0xffff)
			keepTalkingFlag = false;
		else {
			if ((result & 0x8000) == 0) 
				keepTalkingFlag = true;
			else {
				result = Script::execute(result & 0x7fff);
				keepTalkingFlag = result != 0xffff;
			}
		}
			
		if (keepTalkingFlag) {
			// Reset for loading the next set of talking options
			res.setTalkStartEntry(result);
			res.setTalkState(TALK_NONE);
		} else {
			// End the conversation
			res.getActiveHotspot(PLAYER_ID)->setTickProc(PLAYER_TICK_PROC_ID);
			res.setTalkData(0);
			res.setCurrentAction(NONE);
			res.setTalkState(TALK_NONE);
		}
		break;

	case TALK_RESPONSE_WAIT:
		// Keeps waiting while a talk dialog is active
		break;
	}
}

void HotspotTickHandlers::headAnimationHandler(Hotspot &h) {
	Resources &res = Resources::getReference();
	Hotspot *character = res.getActiveHotspot(PLAYER_ID);
	uint16 frameNumber = 0;

	if (character->y() < 79) {
		//character = res.getActiveHotspot(RATPOUCH_ID);
		frameNumber = 1;
	} else {
		if (character->x() < 72) frameNumber = 0;
		else if (character->x() < 172) frameNumber = 1;
		else frameNumber = 2;
	}

	h.setFrameNumber(frameNumber);
}

/*-------------------------------------------------------------------------*/
/* Miscellaneous classes                                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/

// WalkingActionEntry class

// This method performs rounding of the number of steps depending on direciton

int WalkingActionEntry::numSteps() {
	switch (_direction) {
	case UP:
	case DOWN:
		return (_numSteps + 1) >> 1;

	case LEFT:
	case RIGHT:
		return (_numSteps + 3) >> 2;
	default:
		return 0;
	}
}

// PathFinder class

PathFinder::PathFinder(Hotspot *h) { 
	_hotspot = h;
	_list.clear(); 
	_stepCtr = 0; 
}

void PathFinder::reset(RoomPathsData &src) {
	_stepCtr = 0;
	_list.clear();
	src.decompress(_layer, _hotspot->widthCopy());
	_inProgress = false;
	_countdownCtr = PATHFIND_COUNTDOWN;
}

// Does the next stage of processing to figure out a path to take to a given
// destination. Returns true if the path finding has been completed

bool PathFinder::process() {
	bool returnFlag = _inProgress;
	// Check whether the pathfinding can be broken by the countdown counter
	bool breakFlag = (PATHFIND_COUNTDOWN != 0);
	_countdownCtr = PATHFIND_COUNTDOWN;
	int v;
	uint16 *pTemp;
	bool scanFlag = false;
	Direction currDirection = NO_DIRECTION;
	Direction newDirection;
	uint16 numSteps = 0, savedSteps = 0;
	bool altFlag;
	uint16 *pCurrent;

	if (!_inProgress) {
		// Following code only done during first call to method
		_inProgress = true;
		initVars();

		_xCurrent >>= 3; _yCurrent >>= 3;
		_xDestCurrent >>= 3; _yDestCurrent >>= 3;
		if ((_xCurrent == _xDestCurrent) && (_yCurrent == _yDestCurrent)) {
			// Very close move
			if (_xDestPos > 0) 
				add(RIGHT, _xDestPos);
			else if (_xDestPos < 0) 
				add(LEFT, -_xDestPos);

			goto final_step;
		}

		// Path finding

		_destX >>= 3;
		_destY >>= 3;
		_pSrc = &_layer[(_yCurrent + 1) * DECODED_PATHS_WIDTH + 1 + _xCurrent];
		_pDest = &_layer[(_yDestCurrent + 1) * DECODED_PATHS_WIDTH + 1 + _xDestCurrent];

		// Flag starting/ending cells
		*_pSrc = 1;
		_destOccupied = *_pDest != 0;
		_result = _destOccupied ? PF_DEST_OCCUPIED : PF_OK;
		*_pDest = 0;

		// Set up the current pointer, adjusting away from edges if necessary 

		if (_xCurrent >= _xDestCurrent) {
			_xChangeInc = -1;
			_xChangeStart = ROOM_PATHS_WIDTH;
		} else {
			_xChangeInc = 1;
			_xChangeStart = 1;
		}

		if (_yCurrent >= _yDestCurrent) {
			_yChangeInc = -1;
			_yChangeStart = ROOM_PATHS_HEIGHT;
		} else {
			_yChangeInc = 1;
			_yChangeStart = 1;
		}
	}

	// Major loop to populate data
	_cellPopulated = false;

	while (1) {
		// Loop through to process cells in the given area
		if (!returnFlag) _yCtr = 0;
		while (returnFlag || (_yCtr < ROOM_PATHS_HEIGHT)) {
			if (!returnFlag) _xCtr = 0;

			while (returnFlag || (_xCtr < ROOM_PATHS_WIDTH)) {
				if (!returnFlag) {
					processCell(&_layer[(_yChangeStart + _yCtr * _yChangeInc) * DECODED_PATHS_WIDTH +
						(_xChangeStart + _xCtr * _xChangeInc)]);
					if (breakFlag && (_countdownCtr <= 0)) return false;
				} else {
					returnFlag = false;
				}
				++_xCtr;
			}
			++_yCtr;
		}

		// If the destination cell has been filled in, then break out of loop
		if (*_pDest != 0) break;

		if (_cellPopulated) {
			// At least one cell populated, so go repeat loop
			_cellPopulated = false;
		} else {
			_result = PF_NO_PATH;
			scanFlag = true;
			break;
		}
	} 

	if (scanFlag || _destOccupied) {
		// Adjust the end point if necessary to stop character walking into occupied area

		// Restore destination's occupied state if necessary
		if (_destOccupied) {
			*_pDest = 0xffff;
			_destOccupied = false;
		}

		// Scan through lines
		v = 0xff;
		pTemp = _pDest;
		scanLine(_destX, -1, pTemp, v);
		scanLine(ROOM_PATHS_WIDTH - _destX, 1, pTemp, v);
		scanLine(_destY, -DECODED_PATHS_WIDTH, pTemp, v);
		scanLine(ROOM_PATHS_HEIGHT - _destY, DECODED_PATHS_WIDTH, pTemp, v); 

		if (pTemp == _pDest) {
			_result = PF_NO_WALK;
			clear();
			return true;
		}

		_pDest = pTemp;
	}

	// ****DEBUG****
	for (int ctr = 0; ctr < DECODED_PATHS_WIDTH * DECODED_PATHS_HEIGHT; ++ctr)
		Room::getReference().tempLayer[ctr] = _layer[ctr];

	// Determine the walk path by working backwards from the destination, adding in the 
	// walking steps in reverse order until source is reached

	for (int stageCtr = 0; stageCtr < 3; ++stageCtr) {
		altFlag = stageCtr == 1;
		pCurrent = _pDest;

		numSteps = 0;
		currDirection = NO_DIRECTION;
		while (1) {
			v = *pCurrent - 1;
			if (v == 0) break;
			
			newDirection = NO_DIRECTION;
			if (!altFlag && (currDirection != LEFT) && (currDirection != RIGHT)) {
				// Standard order direction checking
				if (*(pCurrent - DECODED_PATHS_WIDTH) == v) newDirection = DOWN;
				else if (*(pCurrent + DECODED_PATHS_WIDTH) == v) newDirection = UP;
				else if (*(pCurrent + 1) == v) newDirection = LEFT;
				else if (*(pCurrent - 1) == v) newDirection = RIGHT;
			} else {
				// Alternate order direction checking
				if (*(pCurrent + 1) == v) newDirection = LEFT;
				else if (*(pCurrent - 1) == v) newDirection = RIGHT;
				else if (*(pCurrent - DECODED_PATHS_WIDTH) == v) newDirection = DOWN;
				else if (*(pCurrent + DECODED_PATHS_WIDTH) == v) newDirection = UP;
			}
			if (newDirection == NO_DIRECTION) 
				error("Path finding process failed");

			// Process for the specified direction
			if (newDirection != currDirection) add(newDirection, 0);

			switch (newDirection) {
			case UP:
				pCurrent += DECODED_PATHS_WIDTH;
				break;

			case DOWN:
				pCurrent -= DECODED_PATHS_WIDTH;
				break;

			case LEFT:
				++pCurrent;
				break;

			case RIGHT:
				--pCurrent;
				break;

			default:
				break;
			}

			++numSteps;
			top().rawSteps() += 8;
			currDirection = newDirection;
		}

		if (stageCtr == 0) 
			// Save the number of steps needed
			savedSteps = numSteps;
		if ((stageCtr == 1) && (numSteps <= savedSteps))
			// Less steps were needed, so break out
			break;

		// Clear out any previously determined directions
		clear();
	}

	// Add a final move if necessary

	if (_result == PF_OK) {
		if (_xDestPos < 0) 
			addBack(LEFT, -_xDestPos);
		else if (_xDestPos > 0) 
			addBack(RIGHT, _xDestPos);
	}
	
final_step:
	if (_xPos < 0) add(LEFT, -_xPos);
	else if (_xPos > 0) add(RIGHT, _xPos);

	return true;
}

void PathFinder::processCell(uint16 *p) {
	// Only process cells that are still empty
	if (*p == 0) {
		uint16 vMax = 0xffff;
		uint16 vTemp;

		// Check the surrounding cells (up,down,left,right) for values
		// Up
		vTemp = *(p - DECODED_PATHS_WIDTH);
		if ((vTemp != 0) && (vTemp < vMax)) vMax = vTemp;
		// Down
		vTemp = *(p + DECODED_PATHS_WIDTH);
		if ((vTemp != 0) && (vTemp < vMax)) vMax = vTemp;
		// Left
		vTemp = *(p - 1);
		if ((vTemp != 0) && (vTemp < vMax)) vMax = vTemp;
		// Right
		vTemp = *(p + 1);
		if ((vTemp != 0) && (vTemp < vMax)) vMax = vTemp;

		if (vMax != 0xffff) {
			// A surrounding cell with a value was found
			++vMax;
			*p = vMax;
			_cellPopulated = true;
		}

		_countdownCtr -= 3;

	} else {
		--_countdownCtr;
	}
}

void PathFinder::scanLine(int numScans, int changeAmount, uint16 *&pEnd, int &v) {
	uint16 *pTemp = _pDest;

	for (int ctr = 1; ctr <= numScans; ++ctr) {
		pTemp += changeAmount;
		if ((*pTemp != 0) && (*pTemp != 0xffff)) {
			if ((v < ctr) || ((v == ctr) && (*pTemp >= *pEnd))) return;
			pEnd = pTemp;
			v = ctr;
			break;
		}
	}
}

void PathFinder::initVars() {
	int16 xRight;

	// Set up dest position, adjusting for walking off screen if necessary
	_destX = _hotspot->destX();
	_destY = _hotspot->destY();

	if (_destX < 10) _destX -= 50;
	if (_destX >= FULL_SCREEN_WIDTH-10) _destX += 50;

	_xPos = 0; _yPos = 0;
	_xDestPos = 0; _yDestPos = 0;

	_xCurrent = _hotspot->x();
	if (_xCurrent < 0) {
		_xPos = _xCurrent;
		_xCurrent = 0;
	}
	xRight = FULL_SCREEN_WIDTH - _hotspot->widthCopy() - 1;
	if (_xCurrent >= xRight) {
		_xPos = _xCurrent - xRight;
		_xCurrent = xRight;
	}

	_yCurrent = (_hotspot->y() & 0xf8) + _hotspot->heightCopy() - MENUBAR_Y_SIZE - 4;
	if (_yCurrent < 0) {
		_yPos = _yCurrent;
		_yCurrent = 0;
	}
	if (_yCurrent >= (FULL_SCREEN_HEIGHT - MENUBAR_Y_SIZE)) {
		_yPos = _yCurrent - (FULL_SCREEN_HEIGHT - MENUBAR_Y_SIZE);
		_yCurrent = FULL_SCREEN_HEIGHT - MENUBAR_Y_SIZE;
	}

	_xDestCurrent = _destX;
	if (_xDestCurrent < 0) {
		_xDestPos = _xDestCurrent;
		_xDestCurrent = 0;
	}
	xRight = FULL_SCREEN_WIDTH - _hotspot->widthCopy();
	if (_xDestCurrent >= xRight) {
		_xDestPos = _xDestCurrent - xRight;
		_xDestCurrent = xRight;
	}

	_yDestCurrent = _destY - 8;
	if (_yDestCurrent < 0)
		_yDestCurrent = 0;
	if (_yDestCurrent >= (FULL_SCREEN_HEIGHT - MENUBAR_Y_SIZE))
		_yDestCurrent = FULL_SCREEN_HEIGHT - MENUBAR_Y_SIZE - 1;

	// Subtract an amount from the countdown counter to compensate for
	// the time spent decompressing the walkable areas set for the room
	_countdownCtr -= 700;
}

/*-------------------------------------------------------------------------*/
/* Support methods                                                         */
/*                                                                         */
/*-------------------------------------------------------------------------*/

// finds a list of character animations whose base area are impinging 
// that of the specified character (ie. are bumping into them)

int Support::findIntersectingCharacters(Hotspot &h, uint16 *charList) {
	int numImpinging = 0;
	Resources &res = Resources::getReference();
	Rect r;

	r.left = h.x();
	r.right = h.x() + h.widthCopy();
	r.top = h.y() + h.heightCopy() - h.yCorrection() - h.charRectY();
	r.bottom = h.y() + h.heightCopy() + h.charRectY();

	HotspotList::iterator i;
	for (i = res.activeHotspots().begin(); i != res.activeHotspots().end(); ++i) {
		Hotspot &hotspot = **i;
		
		// Check for basic reasons to skip checking the animation
		if ((h.hotspotId() == hotspot.hotspotId()) || (hotspot.layer() == 0) ||
			(h.roomNumber() != hotspot.roomNumber()) || (h.hotspotId() >= 0x408) ||
			h.skipFlag()) continue;
		// TODO: See why si+ANIM_HOTSPOT_OFFSET compared aganst di+ANIM_VOICE_CTR

		if ((hotspot.x() > r.right) || (hotspot.x() + hotspot.widthCopy() >= r.left) ||
			(hotspot.y() + hotspot.heightCopy() + hotspot.charRectY() < r.top) ||
			(hotspot.y() + hotspot.heightCopy() - hotspot.charRectY() 
			- hotspot.yCorrection() >= r.bottom)) 
			continue;
		
		// Add hotspot Id to list
		if (numImpinging == MAX_NUM_IMPINGING)
			error("Exceeded maximum allowable number of impinging characters");
		*charList++ = hotspot.hotspotId();
		++numImpinging;
	}

	return numImpinging;
}

// Returns true if any other characters are intersecting the specified one

bool Support::checkForIntersectingCharacter(Hotspot &h) {
	uint16 tempList[MAX_NUM_IMPINGING];
	return findIntersectingCharacters(h, tempList) != 0;
}

// Check whether a character needs to change the room they're in

void Support::checkRoomChange(Hotspot &h) {
	int16 x = h.x() + (h.widthCopy() >> 1);
	int16 y = h.y() + h.heightCopy() - (h.yCorrection() >> 1);

	RoomData *roomData = Resources::getReference().getRoom(h.roomNumber());
	RoomExitData *exitRec = roomData->exits.checkExits(x, y);

	if (exitRec) {
		if (exitRec->sequenceOffset != 0xffff) {
			Script::execute(exitRec->sequenceOffset);
		} else {
			Support::characterChangeRoom(h, exitRec->roomNumber, 
				exitRec->x, exitRec->y, exitRec->direction);
		}
	}
}

void Support::characterChangeRoom(Hotspot &h, uint16 roomNumber, 
								  int16 newX, int16 newY, Direction dir) {
	ValueTableData &fields = Resources::getReference().fieldList();

	if (h.hotspotId() == PLAYER_ID) {
		// Room change code for the player

		h.setDirection(dir);
		PlayerNewPosition &p = fields.playerNewPos();
		p.roomNumber = roomNumber;
		p.position.x = newX;
		p.position.y = newY - 48;

		// TODO: Call sub_136, and if !ZF reset new room number back to 0 
	} else {
		// Any other character changing room
		if (checkForIntersectingCharacter(h)) {
			// Character is blocked, so abort room change
			h.currentActions().clear();
		} else {
			// Handle character room change
			h.setRoomNumber(roomNumber);
			h.setPosition((newX & 0xfff8) || 5, (newY - h.heightCopy()) & 0xfff8);
			h.setSkipFlag(true);
			h.setDirection(dir);

			h.currentActions().pop();
		}
	}
}

bool Support::charactersIntersecting(HotspotData *hotspot1, HotspotData *hotspot2) {
	return !((hotspot1->startX + hotspot1->widthCopy + 4 < hotspot2->startX) ||
		(hotspot2->startX + hotspot2->widthCopy + 4 < hotspot1->startX) ||
		(hotspot2->startY + hotspot2->heightCopy - hotspot2->yCorrection - 2 >=
			hotspot1->startY + hotspot1->heightCopy + 2) ||
		(hotspot2->startY + hotspot2->heightCopy + 2 < 
			hotspot1->startY + hotspot1->heightCopy - hotspot1->yCorrection - 2));
}

} // end of namespace Lure
