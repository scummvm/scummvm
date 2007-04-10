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
#include "lure/game.h"
#include "common/endian.h"

namespace Lure {

Hotspot::Hotspot(HotspotData *res): _pathFinder(this) {
	Resources &resources = Resources::getReference();

	_data = res;
	_anim = NULL;
	_frames = NULL;
	_numFrames = 0;
	_persistant = false;

	_hotspotId = res->hotspotId;
	_originalId = res->hotspotId;
	_roomNumber = res->roomNumber;
	_startX = res->startX;
	_startY = res->startY;
	_destX = res->startX;
	_destY = res->startY;
	_destHotspotId = 0;
	_frameWidth = res->width;
	_frameStartsUsed = false;
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

	_override = resources.getHotspotOverride(res->hotspotId);
	setAnimation(_data->animRecordId);
	_tickHandler = HotspotTickHandlers::getHandler(_data->tickProcOffset);
	_nameBuffer[0] = '\0';

	_frameCtr = 0;
	_skipFlag = false;
	_charRectY = 0;
	_voiceCtr = 0;
	_blockedOffset = 0;
	_exitCtr = 0;
	_walkFlag = true;
	_startRoomNumber = 0;

	if (_data->npcSchedule != 0) {
		CharacterScheduleEntry *entry = resources.charSchedules().getEntry(_data->npcSchedule);
		_currentActions.addFront(DISPATCH_ACTION, entry, _roomNumber);
	}
}

// Special constructor used to create a voice hotspot

Hotspot::Hotspot(Hotspot *character, uint16 objType): _pathFinder(this) {
	assert(character);
	_originalId = objType;
	_data = NULL;
	_anim = NULL;
	_frames = NULL;
	_numFrames = 0;
	_persistant = false;
	_hotspotId = 0xffff;
	_override = NULL;
	_colourOffset = 0;
	_destHotspotId = character->hotspotId();
	_blockedOffset = 0;
	_exitCtr = 0;
	_voiceCtr = 0;
	_walkFlag = false;

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
		_yCorrection = 1;

		_tickCtr = 0;
		_voiceCtr = 40;

		_tickHandler = HotspotTickHandlers::getHandler(VOICE_TICK_PROC_ID);
		setAnimation(VOICE_ANIM_ID);
		break;

	case PUZZLED_ANIM_ID:
	case EXCLAMATION_ANIM_ID:
		_roomNumber = character->roomNumber();
		_hotspotId = 0xfffe;
		_startX = character->x() + character->talkX() + 12;
		_startY = character->y() + character->talkY() - 20;
		_width = 32;
		_height = 18;
		_widthCopy = 19;
		_heightCopy = 18 + character->heightCopy();
		_layer = 1;
		_persistant = false;
		_yCorrection = 1;
		_voiceCtr = CONVERSE_COUNTDOWN_SIZE;

		_destHotspotId = character->hotspotId();
		_tickHandler = HotspotTickHandlers::getHandler(PUZZLED_TICK_PROC_ID);
		setAnimation(VOICE_ANIM_ID);
		setFrameNumber(objType == PUZZLED_ANIM_ID ? 1 : 2);
		
		character->setFrameCtr(_voiceCtr);
		break;

	default:
		break;
	}

	_frameWidth = _width;
	_frameStartsUsed = false;
	_nameBuffer[0] = '\0';
}

Hotspot::~Hotspot() {
	if (_frames) delete _frames;
}

void Hotspot::setAnimation(uint16 newAnimId) {
	Resources &r = Resources::getReference();
	HotspotAnimData *tempAnim;
	_animId = newAnimId;
	if (newAnimId == 0) 
		tempAnim = NULL;
	else {
		tempAnim = r.getAnimation(newAnimId); 
		assert(tempAnim != NULL);
	}
	
	setAnimation(tempAnim);
}

struct SizeOverrideEntry {
	uint16 animId;
	uint16 width, height;
};

static const SizeOverrideEntry sizeOverrides[] = {
	{BLACKSMITH_STANDARD, 32, 48},
	{BLACKSMITH_HAMMERING_ANIM_ID, 48, 47},
	{0, 0, 0}
};

void Hotspot::setAnimation(HotspotAnimData *newRecord) {
	Disk &r = Disk::getReference();
	uint16 tempWidth, tempHeight;
	int16 xStart;

	if (_frames) {
		delete _frames;
		_frames = NULL;
	}
	_anim = NULL;
	_numFrames = 0;
	_frameNumber = 0;
	if (!newRecord) return;
	if (!r.exists(newRecord->animId)) return;

	// Scan for any size overrides - some animations get their size set after decoding, but
	// we want it in advance so we can decode the animation straight to a graphic surface
	const SizeOverrideEntry *p = &sizeOverrides[0];
	while ((p->animId != 0) && (p->animId != newRecord->animId)) ++p;
	if (p->animId != 0)
		setSize(p->width, p->height);

	_anim = newRecord;
	MemoryBlock *src = Disk::getReference().getEntry(_anim->animId);
	
	uint16 *numEntries = (uint16 *) src->data();
	uint16 *headerEntry = (uint16 *) (src->data() + 2);
	assert((*numEntries >= 1) && (*numEntries < 100));

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
	
	// Special handling need
	if (newRecord->animRecordId == SERF_ANIM_ID) {
		_frameStartsUsed = true;
		_frames = new Surface(416, 27);
	}
	else {
		_frames = new Surface(_width * _numFrames, _height);
		_frameStartsUsed = false;
	}
	_frames->data().setBytes(_colourOffset, 0, _frames->data().size());

	byte *pSrc = dest->data() + 0x40;
	byte *pDest;
	headerEntry = (uint16 *) (src->data() + 2);
	MemoryBlock &mDest = _frames->data();
	uint16 frameOffset = 0x40; 
	uint16 *offsetPtr = (uint16 *) src->data();

	tempWidth = _width;
	tempHeight = _height;

	for (uint16 frameNumCtr = 0; frameNumCtr < _numFrames; ++frameNumCtr, ++headerEntry) {

		if ((newRecord->flags & PIXELFLAG_HAS_TABLE) != 0) {
			// For animations with an offset table, set the source pointer
			pSrc = dest->data() + frameOffset;
		}
		
		if (newRecord->animRecordId == SERF_ANIM_ID) {
			// Save the start of each frame for serf, since the size varies 
			xStart = (frameNumCtr == 0) ? 0 : _frameStarts[frameNumCtr - 1] + tempWidth;
			_frameStarts[frameNumCtr] = xStart;

			// Switch statement to handle varying size for different frames
			switch (frameNumCtr) {
			case 3:
				tempWidth = 48;
				tempHeight = 25;
				break;
			case 4:
				tempHeight = 26;
				break;
			case 5:
				tempWidth = 32;
				break;
			case 6:
				tempHeight = 27;
				break;
			case 7:
				tempWidth = 16;
				break;
			default:
				break;
			}
		} else {
			// Set the X Start based on the frame size
			xStart = frameNumCtr * _width;
		}

		// Copy over the frame, applying the colour offset to each nibble
		for (uint16 yPos = 0; yPos < tempHeight; ++yPos) {
			pDest = mDest.data() + yPos * _frames->width() + xStart;

			for (uint16 xPos = 0; xPos < tempWidth / 2; ++xPos) {
				*pDest++ = _colourOffset + (*pSrc >> 4);
				*pDest++ = _colourOffset + (*pSrc & 0xf);
				++pSrc;
			}
		}

		if ((newRecord->flags & PIXELFLAG_HAS_TABLE) != 0) 
			frameOffset += (*++offsetPtr >> 1);
	}

	delete src;
	delete dest;
}

void Hotspot::copyTo(Surface *dest) {
	int16 xPos = _startX;
	int16 yPos = _startY;
	uint16 hWidth = _frameWidth;
	uint16 hHeight = _height;

	Rect r(_frameNumber * hWidth, 0, (_frameNumber + 1) * hWidth - 1, hHeight - 1);
	if (_frameStartsUsed) {
		assert(_frameNumber < MAX_NUM_FRAMES);
		r.left = _frameStarts[_frameNumber];
		r.right = (_frameNumber == _numFrames - 1) ? _frames->width() - 1 :
			_frameStarts[_frameNumber + 1] - 1;
		r.bottom = _height - 1;
	}

	// Handle clipping for X position
	if (xPos < 0) {
		if (xPos + hWidth <= 0)
			// Completely off screen, so don't display
			return;

		// Reduce the source rectangle to only the on-screen portion
		r.left += -xPos;
		xPos = 0;
	}
	else if (xPos >= FULL_SCREEN_WIDTH) 
		return;
	else if (xPos + hWidth > FULL_SCREEN_WIDTH)
		r.right = r.left + (FULL_SCREEN_WIDTH - xPos - 1);

	// Handle clipping for Y position
	if (yPos < 0) {
		if (yPos + hHeight <= MENUBAR_Y_SIZE) 
			// Completely off screen, so don't display
			return;

		// Reduce the source rectangle to only the on-screen portion
		r.top += -(yPos + MENUBAR_Y_SIZE);
		yPos = MENUBAR_Y_SIZE;
	}
	else if (yPos >= FULL_SCREEN_HEIGHT)
		return;
	else if (yPos + hHeight > FULL_SCREEN_HEIGHT)
		r.bottom = r.top + (FULL_SCREEN_HEIGHT - yPos - 1);

	// Final check to make sure there is anything to display
	if ((r.top >= r.bottom) || (r.left >= r.right))
		return;

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

const char *Hotspot::getName()
{
	// If name hasn't been loaded yet, then do so
	if (!_nameBuffer[0] && (nameId() != 0))
		StringData::getReference().getString(nameId(), _nameBuffer);

	return &_nameBuffer[0];
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
	_frameWidth = newWidth;
	_height = newHeight;
}

bool Hotspot::executeScript() {
	if (_data->sequenceOffset == 0xffff)
		return false;
	else
		return HotspotScript::execute(this);
}

void Hotspot::tick() {
	debugC(ERROR_BASIC, kLureDebugAnimations, "Hotspot %xh tick begin", _hotspotId);
	_tickHandler(*this);
	debugC(ERROR_BASIC, kLureDebugAnimations, "Hotspot %xh tick end", _hotspotId);
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
	_currentActions.addFront(START_WALKING, _roomNumber);
}

void Hotspot::stopWalking() {
	_voiceCtr = 0;
	setActionCtr(0);
	_currentActions.clear();
	Room::getReference().setCursorState(CS_NONE);
}

void Hotspot::endAction() {
	Room &room = Room::getReference();

	_voiceCtr = 0;
	setActionCtr(0);
	if (_hotspotId == PLAYER_ID)
		//Room::getReference().setCursorState(CS_NONE);  **DEBUG**
		room.setCursorState((CursorState) ((int) room.cursorState() & 2));

	if (_currentActions.top().hasSupportData()) 
		_currentActions.top().setSupportData(_currentActions.top().supportData().next());
}

void Hotspot::setDirection(Direction dir) {
	if (_numFrames == 0) return;
	uint8 newFrameNumber = 0;

	switch (dir) {
	case UP:
		newFrameNumber = _anim->upFrame;
		_charRectY = 4;
		break;
	case DOWN:
		newFrameNumber = _anim->downFrame;
		_charRectY = 4;
		break;
	case LEFT:
		newFrameNumber = _anim->leftFrame;
		_charRectY = 0;
		break;
	case RIGHT:
		newFrameNumber = _anim->rightFrame;
		_charRectY = 0;
		break;
	default:
		break;
	}

	setFrameNumber(newFrameNumber);
	_direction = dir;
}

// Makes the character face the given hotspot

void Hotspot::faceHotspot(HotspotData *hotspot) {
	if (hotspot->hotspotId >= START_NONVISUAL_HOTSPOT_ID) {
		// Non visual hotspot
		setDirection(hotspot->nonVisualDirection());

	} else {
		// Visual hotspot
		int xp = x() - hotspot->startX;
		int yp = y() + heightCopy() - (hotspot->startY + hotspot->heightCopy);

		if (ABS(yp) >= ABS(xp)) {
			if (yp < 0) setDirection(DOWN);
			else setDirection(UP);
		} else {
			if (xp < 0) setDirection(RIGHT);
			else setDirection(LEFT);
		}
	}

	if (hotspotId() == PLAYER_ID) {
		Room::getReference().update();
		Screen::getReference().update();
	}
}

void Hotspot::faceHotspot(uint16 id) {
	HotspotData *hotspot = Resources::getReference().getHotspot(id);
	assert(hotspot != NULL);
	faceHotspot(hotspot);
}

// Sets a character walking to a random destination position

void Hotspot::setRandomDest() {
	Resources &res = Resources::getReference();
	RoomData *roomData = res.getRoom(roomNumber());
	Common::Rect &rect = roomData->walkBounds;
	Common::RandomSource rnd;
	int tryCtr = 0;
	int16 xp, yp;

	if (_currentActions.isEmpty())
		_currentActions.addFront(START_WALKING, roomNumber());
	else
		_currentActions.top().setAction(START_WALKING);
	_walkFlag = true;

	while (tryCtr ++ <= 20) {
		xp = rect.left + rnd.getRandomNumber(rect.right - rect.left);
		yp = rect.left + rnd.getRandomNumber(rect.right - rect.left);
		setDestPosition(xp, yp);
		setDestHotspot(0);

		if (!roomData->paths.isOccupied(xp, yp) && !roomData->paths.isOccupied(xp, yp)) 
			break;
	}
}

// Sets or clears the hotspot as occupying an area in its room's pathfinding data

void Hotspot::setOccupied(bool occupiedFlag) {
	if ((coveredFlag() != VB_INITIAL) &&
		(occupiedFlag == (coveredFlag() == VB_TRUE)))
		return;
	setCoveredFlag(occupiedFlag ? VB_TRUE : VB_FALSE);

	int xp = x() >> 3;
	int yp = (y() - 8 + heightCopy() - 4) >> 3;
	int widthVal = MAX((widthCopy() >> 3), 1);

	// Handle cropping for screen left
	if (xp < 0) {
		xp = -xp;
		widthVal -= xp;
		if (widthVal <= 0) return;
		xp = 0;
	}

	// Handle cropping for screen right
	int x2 = xp + widthVal - ROOM_PATHS_WIDTH - 1;
	if (x2 >= 0) {
		widthVal -= (x2 + 1);
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

void Hotspot::updateMovement() {
	assert(_data != NULL);
	if (_currentActions.action() == EXEC_HOTSPOT_SCRIPT) {
		if (_data->coveredFlag) {
			// Reset position and direction
			resetPosition();
		} else {
			// Make sure the cell occupied by character is covered
			_data->coveredFlag = VB_TRUE;
			setOccupied(true);
		}
	}
}

void Hotspot::updateMovement2(CharacterMode value) {
	setCharacterMode(value);
	updateMovement();
}

void Hotspot::resetPosition() {
	setPosition(x() & 0xf8 | 5, y());
	setDirection(direction());
}

void Hotspot::converse(uint16 destCharacterId, uint16 messageId, bool standStill) {
	assert(_data);
	_data->talkDestCharacterId = destCharacterId;
	_data->talkMessageId = messageId;
	_data->talkCountdown = CONVERSE_COUNTDOWN_SIZE;

	if ((destCharacterId != 0) && (destCharacterId != NOONE_ID)) {
		// Talking to a destination - add in any talk countdown from the destination,
		// in case the destination is already in process of talking
		HotspotData *hotspot = Resources::getReference().getHotspot(destCharacterId);
		_data->talkCountdown += hotspot->talkCountdown;
	}

	if (standStill) {
		setDelayCtr(_data->talkCountdown);
		_data->characterMode = CHARMODE_CONVERSING;
		//TODO: HS[3Eh]=use_hotspot_id, HS[40h]=active_hotspot_id
	}
}

void Hotspot::showMessage(uint16 messageId, uint16 destCharacterId) {
	Resources &res = Resources::getReference();
	MemoryBlock *data = res.messagesData();
	Hotspot *hotspot;
	uint16 *v = (uint16 *) data->data();
	uint16 v2, idVal;
	messageId &= 0x7fff;

	// Skip through header to find table for given character
	while (READ_LE_UINT16(v) != hotspotId()) v += 2;

	// Scan through secondary list
	++v;
	v = (uint16 *) (data->data() + READ_LE_UINT16(v));
	v2 = 0;
	while ((idVal = READ_LE_UINT16(v)) != 0xffff) {
		++v;
		if (READ_LE_UINT16(v) == messageId) break;
		++v;
	}

	// default response if a specific response not found
	if (idVal == 0xffff) idVal = 0x8c4; 

	if (idVal == 0x76) {
		// Special code id for showing the puzzled talk bubble
		hotspot = new Hotspot(this, PUZZLED_ANIM_ID);
		res.addHotspot(hotspot);

	} else if (idVal == 0x120) {
		// Special code id for showing the exclamation talk bubble
		hotspot = new Hotspot(this, EXCLAMATION_ANIM_ID);
		res.addHotspot(hotspot);

	} else if (idVal >= 0x8000) {
		// Handle string display
		idVal &= 0x7fff;
		hotspot = res.getActiveHotspot(res.fieldList().getField(ACTIVE_HOTSPOT_ID));
		const char *itemName = (hotspot == NULL) ? NULL : hotspot->getName();

		Dialog::show(idVal, itemName, this->getName());
		
	} else if (idVal != 0) {
		// Handle message as a talking dialog (the character talking to themselves)
		converse(destCharacterId, idVal);
	}
}

void Hotspot::handleTalkDialog() {
	assert(_data);
	Resources &res = Resources::getReference();
	ValueTableData &fields = res.fieldList();
	Room &room = Room::getReference();

	// Return if no talk dialog is necessary
	if (_data->talkCountdown == 0) return;
	debugC(ERROR_DETAILED, kLureDebugAnimations, "Talk countdown = %d", _data->talkCountdown);

	if (_data->talkCountdown == CONVERSE_COUNTDOWN_SIZE) {
		// Time to set up the dialog for the character
		--_data->talkCountdown;
		debugC(ERROR_DETAILED, kLureDebugAnimations, "Talk dialog opening");
		startTalkDialog();

		if ((_data->talkDestCharacterId != NOONE_ID) && (_data->talkDestCharacterId != 0) &&
			(_hotspotId < FIRST_NONCHARACTER_ID)) {
			// Speaking to a hotspot
			fields.setField(ACTIVE_HOTSPOT_ID, _data->talkDestCharacterId);

			// Face the character to the hotspot
			HotspotData *destHotspot = res.getHotspot(_data->talkDestCharacterId);
			assert(destHotspot != NULL);
			faceHotspot(destHotspot);

			// If the hotspot is also a character, then face it to the speaker
			if (_data->talkDestCharacterId < FIRST_NONCHARACTER_ID) {
				Hotspot *charHotspot = res.getActiveHotspot(_data->talkDestCharacterId);
				if (charHotspot != NULL)
					charHotspot->faceHotspot(resource());
			}
		}

	} else if ((fields.flags() & GAMEFLAG_FAST_TEXTSPEED) != 0) {
		// Fast text speed
		--_data->talkCountdown;
	} else if ((fields.flags() & (GAMEFLAG_8 | GAMEFLAG_4)) != 0) {
		fields.flags() |= GAMEFLAG_4;
		--_data->talkCountdown;
	} else {
		--_data->talkCountdown;
		fields.flags() -= GAMEFLAG_4;
	}

	if (_data->talkCountdown == 0) {
		// Talking is finish - stop talking and free voice animation
		debugC(ERROR_DETAILED, kLureDebugAnimations, "Talk dialog close");
		room.setTalkDialog(0, 0, 0, 0);
		res.setTalkingCharacter(0);
	}

	debugC(ERROR_DETAILED, kLureDebugAnimations, "Talk handler method end");
}

void Hotspot::startTalkDialog() {
	assert(_data);
	Room &room = Room::getReference();

	if (room.roomNumber() != roomNumber()) return;
	room.setTalkDialog(hotspotId(), _data->talkDestCharacterId, _data->useHotspotId, 
		_data->talkMessageId);
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
	Resources &res = Resources::getReference();
	ValueTableData &fields = res.fieldList();

	if ((hotspot->hotspotId == 0x420) || (hotspot->hotspotId == 0x436) ||
		(hotspot->hotspotId == 0x429)) {
		// TODO: figure out specific handling code - sub_213
		if (0)
			return PC_INITIAL;
	} else if (hotspot->roomNumber != roomNumber()) {
		// loc_884
		if (actionCtr() == 0) 
			converse(0, hotspotId());
		setActionCtr(0);
		return PC_NOT_IN_ROOM;
	} else if (actionCtr() != 0) {
		// loc_883
		setActionCtr(actionCtr() + 1);
		if (actionCtr() >= 6) {
			warning("actionCtr exceeded");
			setActionCtr(0);
			converse(0, 0xD);
			return PC_EXCESS;
		} 

		if ((hotspot->hotspotId >= FIRST_NONCHARACTER_ID) || 
			(hotspot->characterMode == CHARMODE_8) ||
			(hotspot->characterMode == CHARMODE_4) ||
			(hotspot->characterMode == CHARMODE_7)) {
			// loc_880
			if (characterWalkingCheck(hotspot))
				return PC_INITIAL;
		} else {
			// loc_886
			setActionCtr(0);
			converse(0, 0xE);
			return PC_UNKNOWN;
		}
	} else {
		setActionCtr(1);
		if ((hotspot->hotspotId >= FIRST_NONCHARACTER_ID) ||
			((hotspot->actionHotspotId != _hotspotId) && (hotspot->characterMode == CHARMODE_4))) {
			// loc_880
			if (characterWalkingCheck(hotspot))
				return PC_INITIAL;

		} else if (hotspot->actionHotspotId != _hotspotId) {
			if (fields.getField(88) == 2) {
				// loc_882
				hotspot->v2b = 0x2A;
				hotspot->useHotspotId = _hotspotId;
				return PC_INITIAL;
			} else {
				converse(NOONE_ID, 5);
				setDelayCtr(4);
			}
		} 
	}

	// loc_888
	setActionCtr(0);
	if (hotspot->hotspotId < FIRST_NONCHARACTER_ID) {
		hotspot->characterMode = CHARMODE_8;
		hotspot->delayCtr = 30;
		hotspot->actionHotspotId = hotspot->hotspotId;
	}
	return PC_EXECUTE;
}

bool Hotspot::characterWalkingCheck(HotspotData *hotspot) {
	int16 xp, yp;

	if (hotspot == NULL) {
		// DEBUG for now - hardcoded value for 3E7h (NULL)
		xp = 78; yp = 162;
		_walkFlag = true;
	}
	else if ((hotspot->walkX == 0) && (hotspot->walkY == 0)) {
		// The hotspot doesn't have any walk co-ordinates
		xp = hotspot->startX;
		yp = hotspot->startY + hotspot->heightCopy - 4;
		_walkFlag = false;
	} else {
		xp = hotspot->walkX;
		yp = hotspot->walkY & 0x7fff;
		_walkFlag = true;

		if ((hotspot->walkY & 0x8000) != 0) {
			// Special handling for walking
//			if (((xp >> 3) != (x() >> 3)) ||
//				((((y() + heightCopy()) >> 3) - 1) != (yp >> 3))) {
			if ((ABS(xp - x()) > 8) || 
				(ABS(yp - (y() + heightCopy())) > 8)) {
				walkTo(xp, yp);
				return true;
			} else {
				return false;
			}
		}
	}

	// Default walking handling
	// TODO: ANIM[27h] = 1 if hotspot has walk co-ordinates
	if ((ABS(x() - xp) >= 8) ||
		(ABS(y() + heightCopy() - yp - 1) >= 19)) {
		walkTo(xp, yp);
		return true;
	}

	return false;
}

bool Hotspot::doorCloseCheck(uint16 doorId) {
	Resources &res = Resources::getReference();
	Hotspot *doorHotspot = res.getActiveHotspot(doorId);
	if (!doorHotspot) {
		warning("Hotspot %xh is not currently active", doorId);
		return true;
	}

	Rect bounds(doorHotspot->x(), doorHotspot->y() + doorHotspot->heightCopy() 
		- doorHotspot->yCorrection() - doorHotspot->charRectY(),
		doorHotspot->x() + doorHotspot->widthCopy(),
		doorHotspot->y() + doorHotspot->heightCopy() + doorHotspot->charRectY());

	// Loop through active hotspots
	HotspotList::iterator i;
	HotspotList &lst = res.activeHotspots();
	for (i = lst.begin(); i != lst.end(); ++i) {
		Hotspot *hsCurrent = *i;

		// Skip entry if it's the door or the character
		if ((hsCurrent->hotspotId() == hotspotId()) ||
			(hsCurrent->hotspotId() == doorHotspot->hotspotId())) 
			continue;

		// Skip entry if it doesn't meet certain criteria
		if ((hsCurrent->layer() == 0) ||
			(hsCurrent->roomNumber() != doorHotspot->roomNumber()) ||
			(hsCurrent->hotspotId() < PLAYER_ID) ||
			((hsCurrent->hotspotId() >= 0x408) && (hsCurrent->hotspotId() < 0x2710))) 
			continue;

		// Also skip entry if special Id
		if ((hsCurrent->hotspotId() == 0xfffe) || (hsCurrent->hotspotId() == 0xffff))
			continue;

		// Check the dimensions of the animation
		if ((hsCurrent->x() < bounds.right) &&
			((hsCurrent->x() + hsCurrent->widthCopy()) > bounds.left) &&
			((hsCurrent->y() + hsCurrent->heightCopy() + hsCurrent->charRectY()) >= bounds.top) &&
			((hsCurrent->y() + hsCurrent->heightCopy() - hsCurrent->charRectY()
			- hsCurrent->yCorrection()) > bounds.bottom)) {
			// Return false - the door can't be closed
			return false;
		}
	}

	// No blocking characters, so return true that the door can be closed
	return true;
}

/*-------------------------------------------------------------------------*/

typedef void (Hotspot::*ActionProcPtr)(HotspotData *hotspot);

void Hotspot::doAction() {
	CurrentActionEntry &entry = _currentActions.top();
	HotspotData *hotspot = NULL;

	if (!entry.hasSupportData() || (entry.supportData().action() == NONE)) {
		doAction(NONE, NULL);
	} else {
		if (entry.supportData().numParams() > 0)
			hotspot = Resources::getReference().getHotspot(entry.supportData().param(0));
		doAction(entry.supportData().action(), hotspot);
	}
}

void Hotspot::doAction(Action action, HotspotData *hotspot) {
	ActionProcPtr actionProcList[NPC_JUMP_ADDRESS + 1] = {
		&Hotspot::doNothing, 
		&Hotspot::doGet, 
		NULL, 
		&Hotspot::doOperate, 
		&Hotspot::doOperate, 
		&Hotspot::doOperate, 
		&Hotspot::doOpen, 
		&Hotspot::doClose,
		&Hotspot::doLockUnlock, 
		&Hotspot::doLockUnlock, 
		&Hotspot::doUse, 
		&Hotspot::doGive, 
		&Hotspot::doTalkTo, 
		&Hotspot::doTell, 
		NULL,
		&Hotspot::doLook, 
		&Hotspot::doLookAt, 
		&Hotspot::doLookThrough, 
		&Hotspot::doAsk, 
		NULL, 
		&Hotspot::doDrink,
		&Hotspot::doStatus, 
		&Hotspot::doGoto, 
		&Hotspot::doReturn, 
		&Hotspot::doBribe, 
		&Hotspot::doExamine, 
		NULL, NULL,
		&Hotspot::npcSetRoomAndBlockedOffset, 
		&Hotspot::npcHeySir, 
		&Hotspot::npcExecScript, 
		&Hotspot::npcResetPausedList, 
		&Hotspot::npcSetRandomDest,
		&Hotspot::npcWalkingCheck, 
		&Hotspot::npcSetSupportOffset,
		&Hotspot::npcSupportOffsetConditional,
		&Hotspot::npcDispatchAction, 
		&Hotspot::npcUnknown3, 
		&Hotspot::npcPause, 
		&Hotspot::npcStartTalking,
		&Hotspot::npcJumpAddress};

	(this->*actionProcList[action])(hotspot);
}

void Hotspot::doNothing(HotspotData *hotspot) {
	_currentActions.pop();
	if (!_currentActions.isEmpty())
	{
		setBlockedFlag(false);
		currentActions().top().setAction(DISPATCH_ACTION);
	}
	else if (hotspotId() == PLAYER_ID) {
		Room::getReference().setCursorState(CS_NONE);
	}
}

void Hotspot::doGet(HotspotData *hotspot) {
	Resources &res = Resources::getReference();
	HotspotPrecheckResult result = actionPrecheck(hotspot);

	if (result == PC_INITIAL) return;
	else if (result != PC_EXECUTE) {
		endAction();
		return;
	}

	faceHotspot(hotspot);
	endAction();

	uint16 sequenceOffset = res.getHotspotAction(hotspot->actionsOffset, GET);
	if (sequenceOffset >= 0x8000) {
		showMessage(sequenceOffset);
		return;
	} 
	
	if (sequenceOffset != 0) {
		uint16 execResult = Script::execute(sequenceOffset);

		if (execResult == 1) return;
		else if (execResult != 0) {
			showMessage(execResult);
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

void Hotspot::doOperate(HotspotData *hotspot) {
	Resources &res = Resources::getReference();
	Action action = _currentActions.top().supportData().action();

	HotspotPrecheckResult result = actionPrecheck(hotspot);
	if (result == PC_INITIAL) return;
	else if (result != PC_EXECUTE) {
		endAction();
		return;
	}

	setActionCtr(0);
	faceHotspot(hotspot);
	endAction();

	uint16 sequenceOffset = res.getHotspotAction(hotspot->actionsOffset, action);
	if (sequenceOffset >= 0x8000) {
		showMessage(sequenceOffset);
	} else {
		sequenceOffset = Script::execute(sequenceOffset);
		if (sequenceOffset > 1)
			showMessage(sequenceOffset);
	}
}

void Hotspot::doOpen(HotspotData *hotspot) {
	Resources &res = Resources::getReference();
	RoomExitJoinData *joinRec;

	if (isRoomExit(hotspot->hotspotId)) {
		joinRec = res.getExitJoin(hotspot->hotspotId);
		if (!joinRec->blocked) {
			// Room exit is already open
			showMessage(4);
			endAction();
			return;
		}
	}

	HotspotPrecheckResult result = actionPrecheck(hotspot);
	if (result == PC_INITIAL) return;
	else if (result != PC_EXECUTE) {
		endAction();
		return;
	}

	faceHotspot(hotspot);
	setActionCtr(0);
	endAction();

	uint16 sequenceOffset = res.getHotspotAction(hotspot->actionsOffset, OPEN);
	if (sequenceOffset >= 0x8000) {
		// Message to display
		showMessage(sequenceOffset);
		return;
	}

	if (sequenceOffset != 0) {
		sequenceOffset = Script::execute(sequenceOffset);

		if (sequenceOffset == 1) return;
		if (sequenceOffset != 0) {
			if (_exitCtr != 0) 
				_exitCtr = 4;
			showMessage(sequenceOffset);
			return;
		}
	}

	joinRec = res.getExitJoin(hotspot->hotspotId);
	if (joinRec->blocked) {
		joinRec->blocked = 0;

		if (hotspotId() != PLAYER_ID) {
			// TODO: HS[44h]=3, HS[42h]W = 4
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
			showMessage(3);
			endAction();
			return;
		}
	}

	HotspotPrecheckResult result = actionPrecheck(hotspot);
	if (result == PC_INITIAL) return;
	else if (result != PC_EXECUTE) {
		endAction();
		return;
	}

	faceHotspot(hotspot);
	setActionCtr(0);
	endAction();

	uint16 sequenceOffset = res.getHotspotAction(hotspot->actionsOffset, CLOSE);
	if (sequenceOffset >= 0x8000) {
		// Message to display
		showMessage(sequenceOffset);
		return;
	} else if (sequenceOffset != 0) {
		// Otherwise handle script
		sequenceOffset = Script::execute(sequenceOffset);

		if (sequenceOffset != 0) {
			showMessage(sequenceOffset);
			return;
		}
	}

	joinRec = res.getExitJoin(hotspot->hotspotId);
	if (!joinRec->blocked) {
		// Close the door
		if (!doorCloseCheck(joinRec->hotspot1Id) ||
			!doorCloseCheck(joinRec->hotspot2Id)) {
			// A character is preventing the door from closing
			showMessage(2);
		} else {
			// Flag the door as closed
			joinRec->blocked = 1;
		}
	}
}

void Hotspot::doUse(HotspotData *hotspot) {
	Resources &res = Resources::getReference();
	uint16 usedId = _currentActions.top().supportData().param(1);
	HotspotData *usedHotspot = res.getHotspot(usedId);
	ValueTableData &fields = res.fieldList();
	fields.setField(ACTIVE_HOTSPOT_ID, hotspot->hotspotId);
	fields.setField(USE_HOTSPOT_ID, usedHotspot->hotspotId);
	_data->useHotspotId = usedId;

	if (usedHotspot->roomNumber != hotspotId()) {
		// Item to be used is not in character's inventory - say "What???"
		endAction();
		showMessage(0xF);
		return;
	}

	HotspotPrecheckResult result = actionPrecheck(hotspot);
	if (result == PC_INITIAL) return;
	else if (result != PC_EXECUTE) {
		endAction();
		return;
	}

	faceHotspot(hotspot);
	endAction();

	// TODO: If character=3E9h, HS[-1]=28h, HS[1Fh]=50h

	uint16 sequenceOffset = res.getHotspotAction(hotspot->actionsOffset, USE);

	if (sequenceOffset >= 0x8000) {
		showMessage(sequenceOffset);
	} else if (sequenceOffset == 0) {
		showMessage(17);
	} else {
		sequenceOffset = Script::execute(sequenceOffset);
		if (sequenceOffset != 0) 
			showMessage(sequenceOffset);
	}
}

uint16 giveTalkIds[6] = {0xCF5E, 0xCF14, 0xCF90, 0xCFAA, 0xCFD0, 0xCFF6};

void Hotspot::doGive(HotspotData *hotspot) {
	Resources &res = Resources::getReference();
	uint16 usedId = _currentActions.top().supportData().param(1);
	HotspotData *usedHotspot = res.getHotspot(usedId);
	ValueTableData &fields = res.fieldList();
	fields.setField(ACTIVE_HOTSPOT_ID, hotspot->hotspotId);
	fields.setField(USE_HOTSPOT_ID, usedId);
	_data->useHotspotId = usedId;

	if (usedHotspot->roomNumber != hotspotId()) {
		// Item to be used is not in character's inventory - say "What???"
		endAction();
		showMessage(0xF);
		return;
	}

	HotspotPrecheckResult result = actionPrecheck(hotspot);
	if (result == PC_INITIAL) return;
	else if (result != PC_EXECUTE) {
		endAction();
		return;
	}

	faceHotspot(hotspot);
	endAction();

	if ((hotspot->hotspotId != PRISONER_ID) || (usedId != BOTTLE_HOTSPOT_ID)) 
		showMessage(7);
	
	uint16 sequenceOffset = res.getHotspotAction(hotspot->actionsOffset, GIVE);

	if (sequenceOffset >= 0x8000) {
		showMessage(sequenceOffset);
	} else if (sequenceOffset != 0) {
		sequenceOffset = Script::execute(sequenceOffset);
		if (sequenceOffset == NOONE_ID) {
			// Start a conversation based on the index of field #6
			uint16 index = fields.getField(GIVE_TALK_INDEX);
			assert(index < 6);
			startTalk(hotspot, giveTalkIds[index]);

		} else if (sequenceOffset == 0) {
			// Move item into character's inventory
			HotspotData *usedItem = res.getHotspot(usedId);
			usedItem->roomNumber = hotspotId();
		} else if (sequenceOffset > 1) {
			showMessage(result);
		}
	}
}

void Hotspot::doTalkTo(HotspotData *hotspot) {
	Resources &res = Resources::getReference();
	ValueTableData &fields = res.fieldList();
	fields.setField(ACTIVE_HOTSPOT_ID, hotspot->hotspotId);
	fields.setField(USE_HOTSPOT_ID, hotspot->hotspotId);

	if ((hotspot->hotspotId != SKORL_ID) && ((hotspot->roomNumber != 28) || 
		(hotspot->hotspotId != 0x3EB))) {

		HotspotPrecheckResult result = actionPrecheck(hotspot);
		if (result == PC_INITIAL) return;
		else if (result != PC_EXECUTE) {
			endAction();
			return;
		}
	}

	faceHotspot(hotspot);
	endAction();

	uint16 sequenceOffset = res.getHotspotAction(hotspot->actionsOffset, TALK_TO);
	if (sequenceOffset >= 0x8000) {
		showMessage(sequenceOffset);
		return;
	}

	if (sequenceOffset != 0) {
		uint16 result = Script::execute(sequenceOffset);

		if (result != 0) {
			endAction();
			return;
		}
	}

	// Start talking with character
	startTalk(hotspot, getTalkId(hotspot));
}

void Hotspot::doTell(HotspotData *hotspot) {
	Resources &res = Resources::getReference();
	ValueTableData &fields = res.fieldList();
	fields.setField(ACTIVE_HOTSPOT_ID, hotspot->hotspotId);
	fields.setField(USE_HOTSPOT_ID, hotspot->hotspotId);
	Hotspot *character = res.getActiveHotspot(hotspot->hotspotId);
	assert(character);

	HotspotPrecheckResult hsResult = actionPrecheck(hotspot);
	if (hsResult == PC_INITIAL) return;
	else if (hsResult != PC_EXECUTE) {
		endAction();
		return;
	}

	converse(hotspot->hotspotId, 0x7C);

	uint16 sequenceOffset = res.getHotspotAction(hotspot->actionsOffset, TELL);
	if (sequenceOffset >= 0x8000) {
		showMessage(sequenceOffset);
	} else if (sequenceOffset != 0) {
		uint16 result = Script::execute(sequenceOffset);

		if (result == 0) {
			// Build up sequence of commands for character to follow
			CharacterScheduleEntry &cmdData = _currentActions.top().supportData();
			character->setStartRoomNumber(character->roomNumber());
			character->currentActions().clear();
			character->setBlockedFlag(false);

			for (int index = 1; index < cmdData.numParams(); index += 3) {
				character->currentActions().addBack((Action) cmdData.param(index),
					character->roomNumber(), cmdData.param(index + 1), cmdData.param(index + 2));
			}
		}
	}

	endAction();
}

void Hotspot::doLook(HotspotData *hotspot) {
	endAction();
	Dialog::show(Room::getReference().descId());
}

uint16 hotspotLookAtList[] = {0x411, 0x412, 0x41F, 0x420, 0x421, 0x422, 0x426, 
	0x427, 0x428, 0x429, 0x436, 0x437, 0};

void Hotspot::doLookAt(HotspotData *hotspot) {
	Resources &res = Resources::getReference();
	uint16 sequenceOffset = res.getHotspotAction(hotspot->actionsOffset, LOOK_AT);

	if (hotspot->hotspotId >= FIRST_NONCHARACTER_ID) {
		// Check if the hotspot appears in the list of hotspots that don't
		// need to be walked to before being looked at
		uint16 *tempId = &hotspotLookAtList[0];
		while ((*tempId != 0) && (*tempId != hotspot->hotspotId)) ++tempId;
		if (!*tempId) {
			// Hotspot wasn't in the list
			HotspotPrecheckResult result = actionPrecheck(hotspot);
			if (result == PC_INITIAL) return;
			else if (result != PC_EXECUTE) {
				endAction();
				return;
			}
		}
	}

	faceHotspot(hotspot);
	setActionCtr(0);
	endAction();

	if (sequenceOffset >= 0x8000) {
		showMessage(sequenceOffset);
	} else {
		if (sequenceOffset != 0) 
			sequenceOffset = Script::execute(sequenceOffset);

		if (sequenceOffset == 0) {
			uint16 descId = (hotspot->descId2 != 0) ? hotspot->descId2 : hotspot->descId;
			Dialog::show(descId);
		}
	}
}

void Hotspot::doLookThrough(HotspotData *hotspot) {
	Resources &res = Resources::getReference();
	uint16 sequenceOffset = res.getHotspotAction(hotspot->actionsOffset, LOOK_THROUGH);

	if (hotspot->hotspotId >= FIRST_NONCHARACTER_ID) {
		// Check if the hotspot appears in the list of hotspots that don't
		// need to be walked to before being looked at
		uint16 *tempId = &hotspotLookAtList[0];
		while ((*tempId != 0) && (*tempId != hotspot->hotspotId)) ++tempId;
		if (!*tempId) {
			// Hotspot wasn't in the list
			HotspotPrecheckResult result = actionPrecheck(hotspot);
			if (result == PC_INITIAL) return;
			else if (result != PC_EXECUTE) {
				endAction();
				return;
			}
		}
	}

	faceHotspot(hotspot);
	setActionCtr(0);
	endAction();

	if (sequenceOffset >= 0x8000) {
		showMessage(sequenceOffset);
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
	Resources &res = Resources::getReference();
	ValueTableData &fields = res.fieldList();
	uint16 usedId = _currentActions.top().supportData().param(1);
	Hotspot *destCharacter = res.getActiveHotspot(hotspot->hotspotId);
	assert(destCharacter);
	HotspotData *usedHotspot = res.getHotspot(usedId);
	fields.setField(ACTIVE_HOTSPOT_ID, hotspot->hotspotId);
	fields.setField(USE_HOTSPOT_ID, usedId);
	_data->useHotspotId = usedId;

	HotspotPrecheckResult result = actionPrecheck(hotspot);
	if (result == PC_INITIAL) return;
	else if (result != PC_EXECUTE) {
		endAction();
		return;
	}

	faceHotspot(hotspot);
	endAction();
	showMessage(9, hotspot->hotspotId); // CHARACTER, DO YOU HAVE ITEM?

	// Get the action and handle the reply
	uint16 sequenceOffset = res.getHotspotAction(hotspot->actionsOffset, ASK);

	if (sequenceOffset >= 0x8000) {
		destCharacter->showMessage(sequenceOffset, hotspotId());
	} else if (sequenceOffset != 0) {
		sequenceOffset = Script::execute(sequenceOffset);

		if (sequenceOffset == 0) {
			// Give item to character
			usedHotspot->roomNumber = hotspotId();
			destCharacter->showMessage(32, hotspotId());
		} else if (sequenceOffset != 1) {
			destCharacter->showMessage(sequenceOffset, hotspotId());
		}
	}
}

void Hotspot::doDrink(HotspotData *hotspot) {
	Resources &res = Resources::getReference();
	ValueTableData &fields = res.fieldList();
	fields.setField(ACTIVE_HOTSPOT_ID, hotspot->hotspotId);
	fields.setField(USE_HOTSPOT_ID, hotspot->hotspotId);

	endAction();

	// Make sure item is in character's inventory
	if (hotspot->roomNumber != hotspotId()) {
		showMessage(0xF);
		return;
	}

	uint16 sequenceOffset = res.getHotspotAction(hotspot->actionsOffset, DRINK);

	if (sequenceOffset >= 0x8000) {
		showMessage(sequenceOffset);
	} else if (sequenceOffset == 0) {
		showMessage(22);
	} else {
		uint16 result = Script::execute(sequenceOffset);
		if (result == 0) {
			// Item has been drunk, so remove item from game
			hotspot->roomNumber = 0;
		} else if (result != 1) {
			showMessage(result);
		}
	}
}

// doStatus
// Handle the status window

void Hotspot::doStatus(HotspotData *hotspot) {
	char buffer[MAX_DESC_SIZE];
	uint16 numItems = 0;
	StringData &strings = StringData::getReference();
	Resources &resources = Resources::getReference();
	Room &room = Room::getReference();
	
	room.update();
	endAction();

	strings.getString(room.roomNumber(), buffer);
	strcat(buffer, "\n\nYou are carrying ");

	// Scan through the list and add in any items assigned to the player
	HotspotDataList &list = resources.hotspotData();
	HotspotDataList::iterator i;
	for (i = list.begin(); i != list.end(); ++i) {
		HotspotData *rec = *i;

		if (rec->roomNumber == PLAYER_ID) {
			if (numItems++ == 0) strcat(buffer, ": ");
			else strcat(buffer, ", ");
			strings.getString(rec->nameId, buffer + strlen(buffer));
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

// doGoto
// Sets the room for the character to go to

void Hotspot::doGoto(HotspotData *hotspot) {
	_exitCtr = 0;
	_blockedOffset = 0;
	_currentActions.top().setRoomNumber(_currentActions.top().supportData().param(0));
	endAction();
}

void Hotspot::doReturn(HotspotData *hotspot) {
	currentActions().top().setRoomNumber(startRoomNumber());
	endAction();
}

uint16 bribe_hotspot_list[] = {0x421, 0x879, 0x3E9, 0x8C7, 0x429, 0x8D1,
	0x422, 0x8D4, 0x420, 0x8D6, 0x42B, 0x956, 0x3F2, 0xBE6, 0};

void Hotspot::doBribe(HotspotData *hotspot) {
	Resources &res = Resources::getReference();
	ValueTableData &fields = res.fieldList();
	fields.setField(ACTIVE_HOTSPOT_ID, hotspot->hotspotId);
	fields.setField(USE_HOTSPOT_ID, hotspot->hotspotId);

	HotspotPrecheckResult result = actionPrecheck(hotspot);
	if (result == PC_INITIAL) return;
	else if (result != PC_EXECUTE) {
		endAction();
		return;
	}
	
	uint16 *tempId = &bribe_hotspot_list[0];
	uint16 sequenceOffset = 0x14B;     // Default sequence offset
	while (*tempId != 0) {
		if (*tempId++ == hotspotId()) {
			sequenceOffset = *tempId;
			if ((sequenceOffset & 0x8000) != 0)
				sequenceOffset = Script::execute(sequenceOffset & 0x7fff);
			break;
		}
		++tempId;                      // Move over entry's sequence offset
	}
	
	// TODO: call to talk_setup
	faceHotspot(hotspot);
	setActionCtr(0);
	endAction();

	sequenceOffset = res.getHotspotAction(hotspot->actionsOffset, BRIBE);
	if (sequenceOffset != 0) {
		sequenceOffset = Script::execute(sequenceOffset);
		if (sequenceOffset != 0) return;
	}

	// TODO: talk_record_index
	showMessage(sequenceOffset);
}

void Hotspot::doExamine(HotspotData *hotspot) {
	Resources &res = Resources::getReference();
	ValueTableData &fields = res.fieldList();
	fields.setField(ACTIVE_HOTSPOT_ID, hotspot->hotspotId);
	fields.setField(USE_HOTSPOT_ID, hotspot->hotspotId);

	endAction();
	uint16 sequenceOffset = res.getHotspotAction(hotspot->actionsOffset, EXAMINE);

	if (sequenceOffset >= 0x8000) {
		showMessage(sequenceOffset);
	} else {
		if (sequenceOffset != 0) 
			sequenceOffset = Script::execute(sequenceOffset);

		if (sequenceOffset == 0) {
			Dialog::show(hotspot->descId);
		}
	}
}

void Hotspot::doLockUnlock(HotspotData *hotspot) {
	Action action = _currentActions.top().supportData().action();
	Resources &res = Resources::getReference();
	ValueTableData &fields = res.fieldList();
	fields.setField(ACTIVE_HOTSPOT_ID, hotspot->hotspotId);
	fields.setField(USE_HOTSPOT_ID, hotspot->hotspotId);

	HotspotPrecheckResult result = actionPrecheck(hotspot);
	if (result == PC_INITIAL) return;
	else if (result != PC_EXECUTE) {
		endAction();
		return;
	}

	faceHotspot(hotspot);
	endAction();
	
	uint16 sequenceOffset = res.getHotspotAction(hotspot->actionsOffset, action);

	if (sequenceOffset >= 0x8000) {
		showMessage(sequenceOffset);
	} else {
		if (sequenceOffset != 0) 
			Script::execute(sequenceOffset);
	}
}

void Hotspot::npcSetRoomAndBlockedOffset(HotspotData *hotspot) {
	CharacterScheduleEntry &entry = _currentActions.top().supportData();
	_exitCtr = 0;

	_blockedOffset = entry.param(1);
	_currentActions.top().setRoomNumber(entry.param(0));
	endAction();
}

void Hotspot::npcHeySir(HotspotData *hotspot) {
	Resources &res = Resources::getReference();

	// If player is performing an action, wait until it's done
	Hotspot *playerHotspot = res.getActiveHotspot(PLAYER_ID);
	if (!playerHotspot->currentActions().isEmpty()) {
		setDelayCtr(12);
		setCharacterMode(CHARMODE_PAUSED);
		setActionCtr(0);
		return;
	}

	// TODO: Check storage of hotspot Id in data_1090/data_1091=0

	// Get the npc to say "Hey Sir" to player
	showMessage(0x22, PLAYER_ID);

	// Get the character to remain in place for a while
	setDelayCtr(130);
	setCharacterMode(CHARMODE_4);

	// Set the talk override to the specified Id
	CharacterScheduleEntry &entry = _currentActions.top().supportData();
	_data->talkOverride = entry.param(0);

	doNothing(hotspot);
}

void Hotspot::npcExecScript(HotspotData *hotspot) {
	CharacterScheduleEntry &entry = _currentActions.top().supportData();
	uint16 offset = entry.param(0);
	endAction();
	Script::execute(offset);
}

void Hotspot::npcResetPausedList(HotspotData *hotspot) {
	Resources &res = Resources::getReference();
	setCharacterMode(CHARMODE_HESITATE);
	setDelayCtr(IDLE_COUNTDOWN_SIZE + 1);

	res.pausedList().reset(hotspotId());
	endAction();
}

void Hotspot::npcSetRandomDest(HotspotData *hotspot) {
	endAction();
	setRandomDest();
}

void Hotspot::npcWalkingCheck(HotspotData *hotspot) {
	Resources &res = Resources::getReference();
	ValueTableData &fields = res.fieldList();
	CharacterScheduleEntry &entry = _currentActions.top().supportData();
	uint16 hId = entry.param(0);

	endAction();
	fields.setField(USE_HOTSPOT_ID, hId);
	fields.setField(ACTIVE_HOTSPOT_ID, hId);

	if ((hId < PLAYER_ID) || (hotspot->roomNumber == _roomNumber)) {
		characterWalkingCheck(hotspot);
	}
}

void Hotspot::npcSetSupportOffset(HotspotData *hotspot) {
	CharacterScheduleEntry &entry = _currentActions.top().supportData();
	uint16 entryId = entry.param(0);

	CharacterScheduleEntry *newEntry = Resources::getReference().
		charSchedules().getEntry(entryId, entry.parent());
	_currentActions.top().setSupportData(newEntry);
}

void Hotspot::npcSupportOffsetConditional(HotspotData *hotspot) {
	Resources &res = Resources::getReference();
	CharacterScheduleEntry &entry = _currentActions.top().supportData();
	CharacterScheduleEntry *newEntry;
	uint16 scriptOffset = entry.param(0);
	uint16 entryId = entry.param(1);

	if (Script::execute(scriptOffset) == 0) {
		// Not succeeded, get next entry
		newEntry = entry.next();
	} else {
		// Get entry specified by parameter 1
		newEntry = res.charSchedules().getEntry(entryId, entry.parent());
	}

	_currentActions.top().setSupportData(newEntry);
	HotspotData *hotspotData = (newEntry->numParams() == 0) ? NULL : 
		res.getHotspot(newEntry->param(0));
	doAction(newEntry->action(), hotspotData);
}

void Hotspot::npcDispatchAction(HotspotData *hotspot) {
	Resources &res = Resources::getReference();
	ValueTableData &fields = res.fieldList();
	CharacterScheduleEntry &entry = _currentActions.top().supportData();

	fields.setField(USE_HOTSPOT_ID, entry.param(0));
	fields.setField(ACTIVE_HOTSPOT_ID, entry.param(0));

	HotspotPrecheckResult result = actionPrecheck(hotspot);
	if (result == PC_EXECUTE) {
		endAction();
	} else if (result != PC_INITIAL) {
		CharacterScheduleEntry *newEntry = Resources::getReference().
			charSchedules().getEntry(entry.param(0), entry.parent());
		_currentActions.top().setSupportData(newEntry);
		
		HotspotData *hotspotData = (newEntry->numParams() == 0) ? NULL : 
			res.getHotspot(newEntry->param(0));
		doAction(newEntry->action(), hotspotData);
	}
}

void Hotspot::npcUnknown3(HotspotData *hotspot) {
	warning("npcUnknown3: Not yet implemented");
	endAction();
}

void Hotspot::npcPause(HotspotData *hotspot) {
	uint16 delayAmount = _currentActions.top().supportData().param(0);
	endAction();

	setCharacterMode(CHARMODE_PAUSED);
	setDelayCtr(delayAmount);
}

void Hotspot::npcStartTalking(HotspotData *hotspot) {
	CharacterScheduleEntry &entry = _currentActions.top().supportData();
	uint16 stringId = entry.param(0);
	uint16 destHotspot = entry.param(1);

	converse(destHotspot, stringId, false);
	endAction();
}

void Hotspot::npcJumpAddress(HotspotData *hotspot) {
	Resources &res = Resources::getReference();
	ValueTableData &fields = res.fieldList();
	int procIndex = _currentActions.top().supportData().param(0);
	HotspotData *player;
	CharacterScheduleEntry *newEntry;
	endAction();

	switch (procIndex) {
	case 0:
		if (fields.getField(OLD_ROOM_NUMBER) == 19) {
			fields.setField(TALK_INDEX, 24);
			res.getHotspot(0x3F1)->nameId = 0x154;
			Dialog::show(0xAB9);
		}
		break;

	case 1:
		player = res.getHotspot(PLAYER_ID);
		newEntry = res.charSchedules().getEntry(JUMP_ADDR_2_SUPPORT_ID, NULL);
		_currentActions.top().setSupportData(newEntry);
		break;

	default:
		error("Hotspot::npcJumpAddress - invalid method index %d", procIndex);
		break;
	}
}

/*------------------------------------------------------------------------*/

uint16 Hotspot::getTalkId(HotspotData *charHotspot) {
	Resources &res = Resources::getReference();
	uint16 talkIndex;
	TalkHeaderData *headerEntry;

	// If the hotspot has a talk data override, return it
	if (charHotspot->talkOverride != 0)
	{
		// Has an override, so return it and reset back to zero
		uint16 result = charHotspot->talkOverride;
		charHotspot->talkOverride = 0;
		return result;
	}

	// Get offset of talk set to use
	headerEntry = res.getTalkHeader(charHotspot->hotspotId);

	// Calculate talk index to use
	if (charHotspot->nameId == STRANGER_ID)
		talkIndex = 0;
	else
		talkIndex = res.fieldList().getField(TALK_INDEX) + 1;

	return headerEntry->getEntry(talkIndex);
}

void Hotspot::startTalk(HotspotData *charHotspot, uint16 id) {
	Resources &res = Resources::getReference();

	// Set for providing talk listing
	setTickProc(TALK_TICK_PROC_ID);    
	
	// Signal the character that they're being talked to
	charHotspot->talkDestCharacterId = _hotspotId;
	_data->talkDestCharacterId = charHotspot->hotspotId;
	
	// Set the active talk data
	res.setTalkStartEntry(0);
	res.setTalkData(id);
	if (!res.getTalkData()) 
		error("Talk failed - invalid offset: Character=%xh, offset=%xh",
			charHotspot->hotspotId, id);
}

void Hotspot::saveToStream(Common::WriteStream *stream) {
	_currentActions.saveToStream(stream);
	_pathFinder.saveToStream(stream);

	stream->writeUint16LE(_roomNumber);
	stream->writeSint16LE(_startX);
	stream->writeSint16LE(_startY);
	stream->writeSint16LE(_destX);
	stream->writeSint16LE(_destY);
	stream->writeUint16LE(_frameWidth);
	stream->writeUint16LE(_height);
	stream->writeUint16LE(_width);
	stream->writeUint16LE(_heightCopy);
	stream->writeUint16LE(_widthCopy);
	stream->writeUint16LE(_yCorrection);
	stream->writeUint16LE(_talkX);
	stream->writeUint16LE(_talkY);
	stream->writeByte(_layer);
	stream->writeUint16LE(_sequenceOffset);
	stream->writeUint16LE(_tickCtr);
	stream->writeUint32LE(_actions);
	stream->writeByte(_colourOffset);
	stream->writeUint16LE(_animId);
	stream->writeUint16LE(_frameNumber);

	stream->writeUint16LE(_frameCtr);
	stream->writeByte(_skipFlag);
	stream->writeUint16LE(_charRectY);
	stream->writeUint16LE(_voiceCtr);
	stream->writeUint16LE(_blockedOffset);
	stream->writeUint16LE(_exitCtr);
	stream->writeByte(_walkFlag);
	stream->writeUint16LE(_startRoomNumber);
}

void Hotspot::loadFromStream(Common::ReadStream *stream) {
	_currentActions.loadFromStream(stream);
	_pathFinder.loadFromStream(stream);

	_roomNumber = stream->readUint16LE();
	_startX = stream->readSint16LE();
	_startY = stream->readSint16LE();
	_destX = stream->readSint16LE();
	_destY = stream->readSint16LE();
	_frameWidth = stream->readUint16LE();
	_height = stream->readUint16LE();
	_width = stream->readUint16LE();
	_heightCopy = stream->readUint16LE();
	_widthCopy = stream->readUint16LE();
	_yCorrection = stream->readUint16LE();
	_talkX = stream->readUint16LE();
	_talkY = stream->readUint16LE();
	_layer = stream->readByte();
	_sequenceOffset = stream->readUint16LE();
	_tickCtr = stream->readUint16LE();
	_actions = stream->readUint32LE();
	_colourOffset = stream->readByte();
	setAnimation(stream->readUint16LE());
	setFrameNumber(stream->readUint16LE());

	_frameCtr = stream->readUint16LE();
	_skipFlag = stream->readByte() != 0;
	_charRectY = stream->readUint16LE();
	_voiceCtr = stream->readUint16LE();
	_blockedOffset = stream->readUint16LE();
	_exitCtr = stream->readUint16LE();
	_walkFlag = stream->readByte() != 0;
	_startRoomNumber = stream->readUint16LE();
}

/*------------------------------------------------------------------------*/

HandlerMethodPtr HotspotTickHandlers::getHandler(uint16 procOffset) {
	switch (procOffset) {
	case STANDARD_CHARACTER_TICK_PROC:
		return standardCharacterAnimHandler;
	case VOICE_TICK_PROC_ID:
		return voiceBubbleAnimHandler;
	case PUZZLED_TICK_PROC_ID:
		return puzzledAnimHandler;
	case 0x7207:
		return roomExitAnimHandler;
	case PLAYER_TICK_PROC_ID:
		return playerAnimHandler;
	case 0x7C14:
		return followerAnimHandler;
	case 0x7EFA:
		return skorlAnimHandler;
	case 0x7F37:
		return standardAnimHandler2;
	case 0x7F3A:
		return standardAnimHandler;
	case 0x7F69:
		return droppingTorchAnimHandler;
	case 0x7FA1:
		return playerSewerExitAnimHandler;
	case 0x8009:
		return fireAnimHandler;
	case 0x8180:
		return goewinCaptiveAnimHandler;
	case 0x81B3:
		return prisonerAnimHandler;
	case 0x81F3:
		return catrionaAnimHandler;
	case 0x820E:
		return morkusAnimHandler;
	case 0x8241:
		return headAnimHandler;
	case 0x82A0:
		return sellerAnimHandler;
	case 0x85ce:
		return skorlGaurdAnimHandler;
	case 0x882A:
		return rackSerfAnimHandler;
	case TALK_TICK_PROC_ID:
		return talkAnimHandler;
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

void HotspotTickHandlers::standardAnimHandler2(Hotspot &h) {
	h.handleTalkDialog();
	standardAnimHandler(h);
}

void HotspotTickHandlers::standardCharacterAnimHandler(Hotspot &h) {
	Resources &res = Resources::getReference();
	ValueTableData &fields = res.fieldList();
	RoomPathsData &paths = Resources::getReference().getRoom(h.roomNumber())->paths;
	PathFinder &pathFinder = h.pathFinder();
	CurrentActionStack &actions = h.currentActions();
	uint16 impingingList[MAX_NUM_IMPINGING];
	int numImpinging;
	bool bumpedPlayer;

	if (h.currentActions().action() != WALKING) {
		char buffer[MAX_DESC_SIZE];
		h.currentActions().list(buffer);
		debugC(ERROR_DETAILED, kLureDebugAnimations, "Hotspot standard character p=(%d,%d,%d) bs=%d\n%s", 
			h.x(), h.y(), h.roomNumber(), h.blockedState(), buffer);
	}

	// Handle any active talk dialog
	h.handleTalkDialog();

	// Handle any active hotspot the character is using (for example, if the player is
	// talking to a character, this stops them from moving for the duration)
	if (h.useHotspotId() != 0) {
		debugC(ERROR_DETAILED, kLureDebugAnimations, "Use Hotspot Id = %xh, v2b = %d",
			h.useHotspotId(), h.v2b());
		if (h.v2b() == 0x2A) {
			fields.setField(ACTIVE_HOTSPOT_ID, h.v2b());
			fields.setField(USE_HOTSPOT_ID, h.useHotspotId());
			Script::execute(h.script());
			h.setUseHotspotId(0);
		} else {
			h.updateMovement();
			return;
		}
	}

	// If a frame countdown is in progress, then decrement and exit
	if (h.frameCtr() > 0) {
		debugC(ERROR_DETAILED, kLureDebugAnimations, "Frame ctr = %d", h.frameCtr());
		h.decrFrameCtr();
		return;
	}

	debugC(ERROR_DETAILED, kLureDebugAnimations, "Hotspot standard character point 2");
	numImpinging = Support::findIntersectingCharacters(h, impingingList);
	bumpedPlayer = (numImpinging == 0) ? false :
		Support::isCharacterInList(impingingList, numImpinging, PLAYER_ID);

	// Check for character having just changed room
	debugC(ERROR_DETAILED, kLureDebugAnimations, "Hotspot standard character point 3");
	if (h.skipFlag()) {
		debugC(ERROR_DETAILED, kLureDebugAnimations, "Skip flag was set");

		if (numImpinging > 0) {
			// Scan to check if the character has bumped into player
			Hotspot *player = res.getActiveHotspot(PLAYER_ID);

			if (bumpedPlayer && (player->characterMode() == CHARMODE_IDLE)) {
				// Signal the player to move out of the way automatically
				player->setBlockedState(BS_INITIAL);
				player->setDestHotspot(0);
				player->setRandomDest();

				Room::getReference().setCursorState(CS_BUMPED);
				debugC(ERROR_DETAILED, kLureDebugAnimations, "Player bumped code starting");

			} else {
				// Signal the character to pause briefly to allow bumped
				// character time to start moving out of the way
				h.setDelayCtr(10);
				h.setCharacterMode(CHARMODE_PAUSED);
				debugC(ERROR_DETAILED, kLureDebugAnimations, "Pausing after bumping into character");
			}
			return;
		}

		h.setSkipFlag(false);
	}

	// TODO: Handling of any set Tick Script Offset, as well as certain other
	// as of yet unknown hotspot flags

	debugC(ERROR_DETAILED, kLureDebugAnimations, "Hotspot standard character point 4");
	if (h.pauseCtr() != 0) {
		debugC(ERROR_DETAILED, kLureDebugAnimations, "pause ctr = %d", h.pauseCtr());
		h.updateMovement();
		h.pathFinder().clear();
		if (h.pauseCtr() > 1) {
			res.pausedList().scan(h);
			return;
		} else {
			h.setPauseCtr(0);
			if (h.characterMode() == CHARMODE_NONE) {
				h.pathFinder().clear();
				return;
			}
		}
	}

	debugC(ERROR_DETAILED, kLureDebugAnimations, "Hotspot standard character point 5");
	if (h.characterMode() != CHARMODE_NONE) {
		debugC(ERROR_DETAILED, kLureDebugAnimations, "char mode = %d, delay ctr = %d", 
			h.characterMode(), h.delayCtr());

		if (h.characterMode() == CHARMODE_6) {
			// TODO: Figure out what mode 6 is
			h.updateMovement();
			if (bumpedPlayer) return;
		} else {
			// All other character modes
			if (h.delayCtr() > 0) {
				// There is some countdown left to do
				bool decrementFlag = true; 

				if (!decrementFlag) {
					HotspotData *hotspot = res.getHotspot(0); // TODO: HS[50h]
					decrementFlag = (hotspot->roomNumber != h.roomNumber()) ? false :
						Support::charactersIntersecting(hotspot, h.resource());
				}			

				if (decrementFlag) {
					h.setDelayCtr(h.delayCtr() - 1);
					return;
				}
			}
		}

		h.resource()->actionHotspotId = 0;
		CharacterMode currentMode = h.characterMode();
		h.setCharacterMode(CHARMODE_NONE);
		h.pathFinder().clear();

		if ((currentMode == CHARMODE_4) || (currentMode == CHARMODE_7)) {
			// TODO: HS[33h]=0
			h.showMessage(1);
		}
		return;
	}

	debugC(ERROR_DETAILED, kLureDebugAnimations, "Hotspot standard character point 6");
	CurrentAction action = actions.action();

	switch (action) {
	case NO_ACTION:
		h.setCharacterMode(CHARMODE_IDLE);
		break;

	case DISPATCH_ACTION:
		// Dispatch an action
		debugC(ERROR_DETAILED, kLureDebugAnimations, "Hotspot standard character dispatch action");

		if (actions.top().roomNumber() == 0)
			actions.top().setRoomNumber(h.roomNumber());
		if (actions.top().roomNumber() == h.roomNumber()) {
			// NPC in correct room for action
			h.setSkipFlag(false);
			h.doAction();
		} else {
			// NPC in wrong room for action
			npcRoomChange(h);
			debugC(ERROR_DETAILED, kLureDebugAnimations, "Hotspot standard character change room request");
		}
		break;

	case EXEC_HOTSPOT_SCRIPT:
		// A hotspot script is in progress for the player, so don't interrupt
		debugC(ERROR_DETAILED, kLureDebugAnimations, "Hotspot standard character exec hotspot script");
		if (h.executeScript()) {
			// Script is finished
			actions.top().setAction(DISPATCH_ACTION);
		}
		break;

	case START_WALKING:
		// Start the player walking to the given destination
		
		debugC(ERROR_DETAILED, kLureDebugAnimations, 
			"Hotspot standard character exec start walking => (%d,%d)",
			h.destX(), h.destY());
		h.setOccupied(false);  
		pathFinder.reset(paths);
		h.currentActions().top().setAction(PROCESSING_PATH);

		// Deliberate fall through to processing walking path

	case PROCESSING_PATH:
		// Handle processing pathfinding
		debugC(ERROR_DETAILED, kLureDebugAnimations, "Hotspot standard character processing path");
		res.pausedList().scan(h);
		
		if (!pathFinder.process()) break;

		debugC(ERROR_DETAILED, kLureDebugAnimations, 
			"pathFinder done: result = %d", pathFinder.result());

		// Post-processing checks
		if ((pathFinder.result() == PF_OK) || 
			((h.destHotspotId() == 0) && (pathFinder.result() == PF_DEST_OCCUPIED))) {
			// Standard processing
			debugC(ERROR_DETAILED, kLureDebugAnimations, "Standard result handling");

			h.setBlockedState(BS_NONE);
			if (h.pathFinder().isEmpty()) {
				// No path was defined
				h.currentActions().top().setAction(DISPATCH_ACTION);
				return;
			}
			h.currentActions().top().setAction(WALKING);
			h.setPosition(h.x(), h.y() & 0xfff8);
		} else if (h.blockedState() == BS_FINAL) {
			// If this point is reached, the character twice hasn't found a walking path
			debugC(ERROR_DETAILED, kLureDebugAnimations, "Character is hopelessly blocked");

			res.pausedList().reset(h.hotspotId());
			h.updateMovement();

			assert(!h.currentActions().isEmpty());
			h.currentActions().pop();

			h.setBlockedFlag(false);
			h.setBlockedState(BS_NONE);
			h.setCharacterMode(CHARMODE_PAUSED);
			h.setDelayCtr(2);

			if (h.currentActions().isEmpty() || 
				(h.currentActions().top().roomNumber() != h.roomNumber()))
				h.setDestHotspot(0xffff);

			if (bumpedPlayer)
				h.setCharacterMode(CHARMODE_6);

		} else {
			debugC(ERROR_DETAILED, kLureDebugAnimations, "Character is blocked from moving");
			CharacterScheduleEntry *newEntry = res.charSchedules().getEntry(RETURN_SUPPORT_ID);
			assert(newEntry);
			
			// Increment the blocked state 
			h.setBlockedState((BlockedState) ((int) h.blockedState() + 1));
			if (!h.blockedFlag())
			{
				// Not already handling blocked, so add a new dummy action so that the new
				// action set below will not replace the existing one
				h.currentActions().addFront(DISPATCH_ACTION, 0);
				h.setBlockedFlag(true);
			}

			// Set the current action
			CurrentActionEntry &entry = h.currentActions().top();
			entry.setAction(DISPATCH_ACTION);
			entry.setSupportData(newEntry);
			entry.setRoomNumber(h.roomNumber());
		} 
		
		// If the top action is now walking, deliberately fall through to the case entry;
		// otherwise break out to exit method
		if (h.currentActions().isEmpty() || h.currentActions().top().action() != WALKING)
			break;

	case WALKING:
		// The character is currently moving
		debugC(ERROR_DETAILED, kLureDebugAnimations, "Hotspot standard character walking");
		h.setOccupied(false);  

		// If the character is walking to an exit hotspot, make sure it's still open
		if ((h.destHotspotId() != 0) && (h.destHotspotId() != 0xffff)) {
			// Player is walking to a room exit hotspot
			RoomExitJoinData *joinRec = res.getExitJoin(h.destHotspotId());
			if (joinRec->blocked) {
				// Exit now blocked, so stop walking
				actions.top().setAction(DISPATCH_ACTION);
				h.setOccupied(true);
				break;
			}
		}

		if (res.pausedList().check(h.hotspotId(), numImpinging, impingingList) == 0) {
			if (h.walkingStep()) 
				// Walking done
				h.currentActions().top().setAction(DISPATCH_ACTION);
			 
			if (h.destHotspotId() != 0) {
				// Walking to an exit, check for any required room change
				Support::checkRoomChange(h);
			}
		}

		h.setOccupied(true);
		break;
	}
	debugC(ERROR_DETAILED, kLureDebugAnimations, "Hotspot standard character point 7");
}

void HotspotTickHandlers::voiceBubbleAnimHandler(Hotspot &h) {
	Resources &res = Resources::getReference();
	debugC(ERROR_DETAILED, kLureDebugAnimations, 
		"Voice Bubble anim handler: char = %xh, ctr = %d, char speaking ctr = %d", 
		h.hotspotId(), h.voiceCtr(), 
		res.getActiveHotspot(res.getTalkingCharacter())->resource()->talkCountdown);

	if (h.voiceCtr() != 0) 
		h.setVoiceCtr(h.voiceCtr() - 1);

	if (h.voiceCtr() != 0) {
		// Countdown not yet ended
		Hotspot *charHotspot = res.getActiveHotspot(res.getTalkingCharacter());
		if (charHotspot->roomNumber() == h.roomNumber()) {
			// Character is still in the same room as when it began speaking
			if (charHotspot->resource()->talkCountdown != 0) {
				// Character still talking
				if (!res.checkHotspotExtent(charHotspot->resource())) {
					// Set voice bubble off screen to hide it
					h.setPosition(h.x(), -100);
				} else {
					// Keep voice bubble in track with character
					h.setPosition(charHotspot->x() + charHotspot->talkX() + 12,
						charHotspot->y() + charHotspot->talkY() - 18);
				}
				return;
			}
		}
	}

	// End of voice time, so unload
	res.deactivateHotspot(&h);
	return;
}

void HotspotTickHandlers::puzzledAnimHandler(Hotspot &h) {
	Resources &res = Resources::getReference();
	HotspotData *charHotspot = res.getHotspot(h.destHotspotId());
	assert(charHotspot);

	h.setVoiceCtr(h.voiceCtr() - 1);
	if ((charHotspot->roomNumber != h.roomNumber()) || (h.voiceCtr() == 0) ||
		!res.checkHotspotExtent(charHotspot)) {
		// Remove the animation
		res.deactivateHotspot(&h);
		return;
	}

	h.setPosition(charHotspot->startX + charHotspot->talkX + 12,
		charHotspot->startY + charHotspot->talkY - 20);
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
		// Closing the door
		h.setOccupied(true);

		++*currentFrame;
		if (*currentFrame == *destFrame) {
			// TODO: play closed door sound
		}
	} else if ((rec->blocked == 0) && (*currentFrame != 0)) {
		// Opening the door
		h.setOccupied(false);

		--*currentFrame;
		if (*currentFrame == *destFrame) {
			//TODO: Check against script val 88 and play sound
		}
	}

	h.setFrameNumber(*currentFrame);
}

void HotspotTickHandlers::playerAnimHandler(Hotspot &h) {
	Resources &res = Resources::getReference();
	Room &room = Room::getReference();
	Mouse &mouse = Mouse::getReference();
	RoomPathsData &paths = Resources::getReference().getRoom(h.roomNumber())->paths;
	PathFinder &pathFinder = h.pathFinder();
	CurrentActionStack &actions = h.currentActions();
	ValueTableData &fields = res.fieldList();
	uint16 impingingList[MAX_NUM_IMPINGING];
	int numImpinging;
	Action hsAction;
	uint16 hotspotId;
	HotspotData *hotspot;
	char buffer[MAX_DESC_SIZE];

	h.currentActions().list(buffer);
	debugC(ERROR_DETAILED, kLureDebugAnimations, 
		"Hotspot player anim handler p=(%d,%d,%d) bs=%d\n%s", 
		h.x(), h.y(), h.roomNumber(), h.blockedState(), buffer);

	h.handleTalkDialog();

	// If a frame countdown is in progress, then decrement and exit
	if (h.frameCtr() > 0) {
		debugC(ERROR_DETAILED, kLureDebugAnimations, "Frame countdown = %d", h.frameCtr());
		h.decrFrameCtr();
		return;
	}

	numImpinging = Support::findIntersectingCharacters(h, impingingList);

	if (h.skipFlag()) {
		debugC(ERROR_DETAILED, kLureDebugAnimations, "Skip flag set: numImpinging = %d", numImpinging);
		if (numImpinging > 0) 
			return;
		h.setSkipFlag(false);
	}
	// TODO: HS[58h] check

	if (h.pauseCtr() > 0) {
		debugC(ERROR_DETAILED, kLureDebugAnimations, "Pause countdown = %d", h.pauseCtr());
		h.updateMovement();
		h.pathFinder().clear();

		if (h.pauseCtr() == 1) {
			h.setPauseCtr(0);
			if (h.characterMode() == 0) {
				h.setOccupied(false);
				return;
			}
		} else {
			res.pausedList().scan(h);
			return;
		}
	}

	if ((h.characterMode() != CHARMODE_NONE) && (h.characterMode() != CHARMODE_IDLE)) {
		if (h.delayCtr() != 0) {
			debugC(ERROR_DETAILED, kLureDebugAnimations, "Delay countdown = %d", h.delayCtr());
			h.updateMovement();
			h.pathFinder().clear();
			h.setDelayCtr(h.delayCtr() - 1);
			return;
		}

		debugC(ERROR_DETAILED, kLureDebugAnimations, "Character mode = %d", h.characterMode());
		h.setOccupied(false);
		h.setCharacterMode(CHARMODE_NONE);
		if (fields.playerPendingPos().isSet) {
			// Start walking to the previously set destination
			fields.playerPendingPos().isSet = false;
			h.setDestPosition(fields.playerPendingPos().pos.x, fields.playerPendingPos().pos.y);
			h.currentActions().addFront(START_WALKING, h.roomNumber());
			h.setWalkFlag(false);
		}
		return;
	}

	CurrentAction action = actions.action();

	switch (action) {
	case NO_ACTION:
		// Make sure there is no longer any destination
		h.setDestHotspot(0);
		h.updateMovement2(CHARMODE_IDLE);
		strcpy(room.statusLine(), "");
		break;

	case DISPATCH_ACTION:
		// Dispatch an action
		h.setDestHotspot(0);

		hotspot = NULL;
		if (actions.top().hasSupportData()) {
			hsAction = actions.top().supportData().action();

			if (actions.top().supportData().numParams() > 0) {
				hotspotId = actions.top().supportData().param(0);
				hotspot = res.getHotspot(hotspotId);
			} 
		} else {
			hsAction = NONE;
		}

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
		h.setOccupied(false);  

		// Reset the path finder / walking sequence
		pathFinder.reset(paths);

		// Set current action to processing walking path
		actions.pop();
		h.currentActions().addFront(PROCESSING_PATH, h.roomNumber());
		// Deliberate fall through to processing walking path

	case PROCESSING_PATH:
		h.setCharacterMode(CHARMODE_NONE);
		res.pausedList().scan(h);

		if (!pathFinder.process()) break;

		// Pathfinding is now complete 
		pathFinder.list(buffer);
		debugC(ERROR_DETAILED, kLureDebugAnimations, 
			"Pathfind processing done; result=%d, walkFlag=%d\n%s", 
			pathFinder.result(), h.walkFlag(), buffer);

		if ((pathFinder.result() != PF_OK) && 
			(h.walkFlag() || (pathFinder.result() != PF_DEST_OCCUPIED))) { 
			
			debugC(ERROR_DETAILED, kLureDebugAnimations, "Blocked state checking");
			if (h.blockedState() == BS_FINAL) {
				res.pausedList().reset(h.hotspotId());
				h.setBlockedState(BS_NONE);
				h.currentActions().pop();
				h.setCharacterMode(CHARMODE_6);
				h.setDelayCtr(7);
				return;

			} else if (h.blockedState() != BS_NONE) {
				fields.playerPendingPos().pos.x = h.destX();
				fields.playerPendingPos().pos.y = h.destY();
				fields.playerPendingPos().isSet = true;
				h.setBlockedState((BlockedState) ((int) h.blockedState() + 1));
				h.setRandomDest();
				return;
			}
		}

		h.setCharacterMode(CHARMODE_NONE);
		h.setPosition(h.x(), h.y() & 0xFFF8);

		if (pathFinder.isEmpty()) {
			mouse.setCursorNum(CURSOR_ARROW);
			break;
		}

		h.currentActions().top().setAction(WALKING);
		if (mouse.getCursorNum() != CURSOR_CAMERA)
			mouse.setCursorNum(CURSOR_ARROW);
		
		// Deliberate fall through to walking

	case WALKING:
		// The character is currently moving
		h.setOccupied(false);

		if ((h.destHotspotId() != 0) && (h.destHotspotId() != 0xffff)) {
			// Player is walking to a room exit hotspot
			RoomExitJoinData *joinRec = res.getExitJoin(h.destHotspotId());
			if (joinRec->blocked) {
				// Exit now blocked, so stop walking
				actions.pop();
				h.setOccupied(true);
				break;
			}
		}

		if (res.pausedList().check(PLAYER_ID, numImpinging, impingingList) == 0) {
			if (h.walkingStep()) {
				// Walking done
				if (room.cursorState() == CS_BUMPED)
					room.setCursorState(CS_NONE);
				if (fields.playerPendingPos().isSet) {
					h.setCharacterMode(CHARMODE_6);
					h.setDelayCtr(IDLE_COUNTDOWN_SIZE);
					return;
				}

				h.currentActions().top().setAction(DISPATCH_ACTION);
			}

			// Check for whether need to change room
			if (Support::checkRoomChange(h))
				// Player changinge room - break now to avoid resetting occupied status
				break;
		}
		h.setOccupied(true);
		break;
	}

	debugC(ERROR_DETAILED, kLureDebugAnimations, "Hotspot player anim handler end");
}

struct RoomTranslationRecord {
	uint8 srcRoom;
	uint8 destRoom;
};

RoomTranslationRecord roomTranslations[] = {
	{0x1E, 0x13}, {0x07, 0x08}, {0x1C, 0x12}, {0x26, 0x0F}, 
	{0x27, 0x0F}, {0x28, 0x0F}, {0x29, 0x0F}, {0x22, 0x0A}, 
	{0x23, 0x13}, {0x24, 0x14}, {0x31, 0x2C}, {0x2F, 0x2C},
	{0, 0}};

void HotspotTickHandlers::followerAnimHandler(Hotspot &h) {
	static int countdownCtr = 0;
	Resources &res = Resources::getReference();
	ValueTableData &fields = res.fieldList();
	Hotspot *player = res.getActiveHotspot(PLAYER_ID);

	if ((fields.getField(37) == 0) && h.currentActions().isEmpty()) {
		if (h.roomNumber() != player->roomNumber()) {
			// Character in different room than player
			if (h.hotspotId() == GOEWIN_ID) 
				h.currentActions().addFront(DISPATCH_ACTION, player->roomNumber());
			else {
				// Scan through the translation list for an alternate destination room
				RoomTranslationRecord *p = &roomTranslations[0];
				while ((p->srcRoom != 0) && (p->srcRoom != player->roomNumber()))
					++p;
				h.currentActions().addFront(DISPATCH_ACTION, 
					(p->srcRoom != 0) ? p->destRoom : player->roomNumber());
			}
		}
	}

	// If some action is in progress, do standard handling
	if (h.characterMode() != CHARMODE_IDLE) {
		standardCharacterAnimHandler(h);
		return;
	}

	if (fields.wanderingCharsLoaded()) {
		// Start Ratpouch to sewer exit to meet player
		fields.wanderingCharsLoaded() = false;
		h.setBlockedFlag(false);
		CharacterScheduleEntry *newEntry = res.charSchedules().getEntry(RETURN_SUPPORT_ID);
		h.currentActions().addFront(DISPATCH_ACTION, newEntry, 7);
		h.setActionCtr(0);

		standardCharacterAnimHandler(h);
		return;
	}

	// Handle any pause countdown
	if (countdownCtr > 0) {
		--countdownCtr;
		standardCharacterAnimHandler(h);
		return;
	}

	// Handle selecting a random action for the character to do
	RandomActionSet *set = res.randomActions().getRoom(h.roomNumber());
	if (!set) {
		standardCharacterAnimHandler(h);
		return;
	}

	Common::RandomSource rnd;
	RandomActionType actionType;
	uint16 scheduleId;
	int actionIndex = rnd.getRandomNumber(set->numActions() - 1);
	set->getEntry(actionIndex, actionType, scheduleId);

	if (actionType == REPEAT_ONCE_DONE)	{
		// Repeat once random action that's already done, so don't repeat it
		standardCharacterAnimHandler(h);
		return;
	}

	// For repeat once actions, make sure the character is in the same room as the player
	if (actionType == REPEAT_ONCE) {
		if (player->roomNumber() != h.roomNumber()) {
			// Not in the same room, so don't do the action
			standardCharacterAnimHandler(h);
			return;
		}
		
		// Flag the action as having been done, so it won't be repeated
		set->setDone(actionIndex);
	}

	if (scheduleId == 0) {
		// No special schedule to perform, so simply set a random action
		h.setRandomDest();
	} else {
		// Prepare the follower to standard the specified schedule
		CharacterScheduleEntry *newEntry = res.charSchedules().getEntry(scheduleId);
		assert(newEntry);
		h.currentActions().addFront(DISPATCH_ACTION, newEntry, h.roomNumber());

		// Set a random delay before beginning the action
		countdownCtr = rnd.getRandomNumber(32);
	}

	standardCharacterAnimHandler(h);
}

void HotspotTickHandlers::skorlAnimHandler(Hotspot &h) {
	Resources &res = Resources::getReference();
	ValueTableData &fields = res.fieldList();
	HotspotData *player = res.getHotspot(PLAYER_ID);

	if ((fields.getField(11) != 0) && !h.skipFlag() && 
		(h.roomNumber() == player->roomNumber)) {
		// TODO: Need extra check on data_669

		if (Support::charactersIntersecting(h.resource(), player)) {
			// Skorl has caught the player
			Game::getReference().setState(GS_RESTORE_RESTART | GS_CAUGHT);
		}
	}

	standardCharacterAnimHandler(h);
}

void HotspotTickHandlers::droppingTorchAnimHandler(Hotspot &h) {
	if (h.frameCtr() > 0) 
		h.setFrameCtr(h.frameCtr() - 1);
	else {
		bool result = h.executeScript();
		if (result) {
			// Changeover to the fire on the straw
			Resources &res = Resources::getReference();
			res.deactivateHotspot(h.hotspotId());
			res.activateHotspot(0x41C);

			// Enable the fire and activate its animation
			HotspotData *fire = res.getHotspot(0x418);
			fire->flags |= 0x80;
			fire->loadOffset = 0x7172; 
			res.activateHotspot(0x418);
		}
 	}
}

void HotspotTickHandlers::playerSewerExitAnimHandler(Hotspot &h) {
	if (h.frameCtr() > 0)
	{
		h.decrFrameCtr();
	}
	else if (h.executeScript())
	{
		Resources &res = Resources::getReference();

		// Deactive the dropping animation
		h.setLayer(0);
		res.deactivateHotspot(h.hotspotId());

		// Position the player
		Hotspot *playerHotspot = res.getActiveHotspot(PLAYER_ID);
		playerHotspot->setPosition(FULL_SCREEN_WIDTH / 2, (FULL_SCREEN_HEIGHT - MENUBAR_Y_SIZE) / 2);
		playerHotspot->setDirection(DOWN);
		playerHotspot->setCharacterMode(CHARMODE_NONE);

		// Setup Ratpouch
		Hotspot *ratpouchHotspot = res.getActiveHotspot(RATPOUCH_ID);
		ratpouchHotspot->setCharacterMode(CHARMODE_NONE);
		ratpouchHotspot->setDelayCtr(0);
		ratpouchHotspot->setActions(0x821C00);
	}
}

void HotspotTickHandlers::fireAnimHandler(Hotspot &h) {
	standardAnimHandler(h);
	h.setOccupied(true);
}

void HotspotTickHandlers::goewinCaptiveAnimHandler(Hotspot &h) {
	if (h.actionCtr() > 0) {
		if (h.executeScript()) {
			h.setTickProc(STANDARD_CHARACTER_TICK_PROC);
			h.setActionCtr(0);
		}
	}
}

void HotspotTickHandlers::prisonerAnimHandler(Hotspot &h) {
	ValueTableData &fields = Resources::getReference().fieldList();
	Common::RandomSource rnd;

	h.handleTalkDialog();
	if (h.frameCtr() > 0) {
		h.setFrameCtr(h.frameCtr() - 1);
		return;
	}

	if (h.actionCtr() != 0) {
		if (h.executeScript() == 0) {
			h.setActionCtr(0);
			h.setScript(0x3E0);
		}
		return;
	}

	if ((fields.getField(PRISONER_DEAD) == 0) && (rnd.getRandomNumber(65536) >= 6)) {
		h.setActionCtr(1);
		h.setScript(0x3F6);
	}
}

void HotspotTickHandlers::catrionaAnimHandler(Hotspot &h) {
	h.handleTalkDialog();
	if (h.frameCtr() > 0) {
		h.decrFrameCtr();
	} else {
		h.executeScript();
		h.setFrameCtr(h.actionCtr());
	}
}

void HotspotTickHandlers::morkusAnimHandler(Hotspot &h) {
	h.handleTalkDialog();
	if (h.frameCtr() > 0) {
		h.decrFrameCtr();
		return;
	}

	if (h.executeScript()) {
		// Script is done - set new script to one of two alternates randomly
		Common::RandomSource rnd;
		h.setScript(rnd.getRandomNumber(100) >= 50 ? 0x54 : 0); 
		h.setFrameCtr(20 + rnd.getRandomNumber(63));
	}
}

// Special variables used across multiple calls to talkAnimHandler
static TalkEntryData *_talkResponse;
static uint16 talkDestCharacter;

void HotspotTickHandlers::talkAnimHandler(Hotspot &h) {
	// Talk handler
	Resources &res = Resources::getReference();
	StringData &strings = StringData::getReference();
	Screen &screen = Screen::getReference();
	Mouse &mouse = Mouse::getReference();
	TalkSelections &talkSelections = res.getTalkSelections();
	TalkData *data = res.getTalkData();
	TalkEntryList &entries = data->entries; 
	Hotspot *charHotspot;
	char buffer[MAX_DESC_SIZE];
	Rect r;
	int lineNum, numLines;
	int selectedLine, responseNumber;
	bool showSelections, keepTalkingFlag;
	TalkEntryList::iterator i;
	TalkEntryData *entry;
	uint16 result, descId;

	debugC(ERROR_DETAILED, kLureDebugAnimations, "Player talk anim handler state = %d", res.getTalkState());
	switch (res.getTalkState()) {
	case TALK_NONE:
		talkDestCharacter = h.resource()->talkDestCharacterId;
		assert(talkDestCharacter != 0);
		// Fall through to TALK_START

	case TALK_START:
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
			if (!showLine) {
				debugC(ERROR_DETAILED, kLureDebugAnimations, 
					"Checking whether to display line: script=%xh, descId=%d",
					sequenceOffset, entry->descId);
				showLine = Script::execute(sequenceOffset) != 0;
			}

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

			strings.getString(entry->descId & 0x3fff, buffer);

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

		// Get the original question for display
		selectedLine = res.getTalkSelection();
		entry = talkSelections[selectedLine-1];
		descId = entry->descId & 0x3fff;
		entry->descId |= 0x4000;
		debugC(ERROR_DETAILED, kLureDebugAnimations, "Talk line set: line=#%d, desc=%xh",
			selectedLine, descId);

		// Get the response the destination character will say
		if (descId != TALK_MAGIC_ID) 
			// Set up to display the question and response in talk dialogs
			h.converse(talkDestCharacter, descId, false);
		res.setTalkState(TALK_RESPOND_2);
		break;

	case TALK_RESPOND_2:
		// Wait until the question dialog is no longer active
		h.handleTalkDialog();
		debugC(ERROR_DETAILED, kLureDebugAnimations, "Player talk dialog countdown %d", 
			h.resource()->talkCountdown);

		if (res.getTalkingCharacter() != 0)
			return;

		selectedLine = res.getTalkSelection();
		entry = talkSelections[selectedLine-1];

		responseNumber = entry->postSequenceId;
		debugC(ERROR_DETAILED, kLureDebugAnimations, "Post sequence Id = %xh", responseNumber);

		if ((responseNumber & 0x8000) != 0) {
			responseNumber = Script::execute(responseNumber & 0x7fff);
			debugC(ERROR_DETAILED, kLureDebugAnimations, "Post sequence Id = %xh", responseNumber);
		}

		do {
			_talkResponse = res.getTalkData()->getResponse(responseNumber);
			debugC(ERROR_DETAILED, kLureDebugAnimations, "Character response pre id = %xh",
				_talkResponse->preSequenceId);

			if (!_talkResponse->preSequenceId) break;
			responseNumber = Script::execute(_talkResponse->preSequenceId);
			debugC(ERROR_DETAILED, kLureDebugAnimations, "Character response new response = %d",
				responseNumber);
		} while (responseNumber != TALK_RESPONSE_MAGIC_ID);

		descId = _talkResponse->descId;
		if ((descId & 0x8000) != 0)
			descId = Script::execute(descId & 0x7fff);

		if (descId != TALK_MAGIC_ID) {
			charHotspot = res.getActiveHotspot(talkDestCharacter);

			if (charHotspot != NULL)
				charHotspot->converse(PLAYER_ID, descId, true);
		} 
		res.setTalkState(TALK_RESPONSE_WAIT);
		break;

	case TALK_RESPONSE_WAIT:
		// Wait until the character's response has finished being displayed
		charHotspot = res.getActiveHotspot(talkDestCharacter);
		debugC(ERROR_DETAILED, kLureDebugAnimations, "Player talk dialog countdown %d", 
			(charHotspot) ? charHotspot->resource()->talkCountdown : 0);

		if (res.getTalkingCharacter() != 0) 
			return;

		result = _talkResponse->postSequenceId;
		debugC(ERROR_DETAILED, kLureDebugAnimations, "Character response post id = %xh", result);

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

		debugC(ERROR_DETAILED, kLureDebugAnimations, "Keep Talking flag = %d", keepTalkingFlag);

		if (keepTalkingFlag) {
			// Reset for loading the next set of talking options
			res.setTalkStartEntry(result);
			res.setTalkState(TALK_START);
		} else {
			// End the conversation
			res.getActiveHotspot(PLAYER_ID)->setTickProc(PLAYER_TICK_PROC_ID);
			if (charHotspot)
				charHotspot->setUseHotspotId(0);
			res.setTalkData(0);
			res.setCurrentAction(NONE);
			res.setTalkState(TALK_NONE);
		}
		break;
	}
}

void HotspotTickHandlers::headAnimHandler(Hotspot &h) {
	Resources &res = Resources::getReference();
	Hotspot *character = res.getActiveHotspot(PLAYER_ID);
	uint16 frameNumber = 0;

	if (character->y() < 79) {
		// TODO: 
		//character = res.getActiveHotspot(RATPOUCH_ID);
		frameNumber = 1;
	} else {
		if (character->x() < 72) frameNumber = 0;
		else if (character->x() < 172) frameNumber = 1;
		else frameNumber = 2;
	}

	h.setFrameNumber(frameNumber);
}

void HotspotTickHandlers::sellerAnimHandler(Hotspot &h) {
	h.handleTalkDialog();
	if (h.frameCtr() > 0) {
		h.decrFrameCtr();
		return;
	}
	
	// TODO: Decode remainder of sellers tick proc
}

void HotspotTickHandlers::skorlGaurdAnimHandler(Hotspot &h) {
	h.handleTalkDialog();

	// Set the frame number
	h.setFrameNumber(h.actionCtr());
}

void HotspotTickHandlers::rackSerfAnimHandler(Hotspot &h) {
	Resources &res = Resources::getReference();

	// Handle any talking
	h.handleTalkDialog();

	if (h.frameCtr() > 0) {
		h.decrFrameCtr();
		return;
	}

	switch (h.actionCtr()) {
	case 1:
		h.setScript(RACK_SERF_SCRIPT_ID_1);
		h.setActionCtr(2);
		break;

	case 2:
		if (HotspotScript::execute(&h))
			h.setActionCtr(0);
		break;

	case 3:
		h.setScript(RACK_SERF_SCRIPT_ID_2);
		h.setActionCtr(4);
		h.setLayer(2);

	case 4:
		if (HotspotScript::execute(&h)) {
			h.setLayer(255);
			res.deactivateHotspot(h.hotspotId());

			HotspotData *ratpouchData = res.getHotspot(RATPOUCH_ID);
			ratpouchData->roomNumber = 4;
			Hotspot *newHotspot = res.activateHotspot(RATPOUCH_ID);
			newHotspot->converse(PLAYER_ID, 0x9C, true);
		}
		break;

	default:
		break;
	}
}

/*-------------------------------------------------------------------------*/

// support method for the standard character tick proc routine - it gets called
// when the character is in the wrong room designated for an action, and is
// responsible for starting the character walking to the correct exit

void HotspotTickHandlers::npcRoomChange(Hotspot &h) {
	Resources &res = Resources::getReference();

	// Increment number of times an exit has been attempted
	h.setExitCtr(h.exitCtr() + 1);
	if (h.exitCtr() >= 5) {
		// Failed to exit room too many times
		h.setExitCtr(0);
		if (h.currentActions().size() > 1) {
			// Pending items on stack
			if (h.startRoomNumber() != 0) {
				if (h.currentActions().top().supportData().id() != RETURN_SUPPORT_ID) {
					h.currentActions().top().supportData().setDetails(RETURN, 0);
				}
			}
			h.currentActions().top().setRoomNumber(h.roomNumber());

		} else if (h.blockedOffset() != 0) {
			// Only current action on stack - and there is a block handler 
			CharacterScheduleEntry *entry = res.charSchedules().getEntry(h.blockedOffset());
			h.currentActions().top().setSupportData(entry);
			h.currentActions().top().setRoomNumber(h.roomNumber());
		}

		return;
	}

	// Get room exit coordinates
	RoomExitCoordinateData &exitData = res.coordinateList().getEntry(
		h.roomNumber()).getData(h.currentActions().top().roomNumber());

	if (h.hotspotId() != RATPOUCH_ID) {
		// Count up the number of characters in the room
		HotspotList &list = res.activeHotspots();
		HotspotList::iterator i;
		int numCharacters = 0;

		for (i = list.begin(); i != list.end(); ++i) {
			if ((h.roomNumber() == (exitData.roomNumber & 0xff)) && (h.layer() != 0) &&
				(h.hotspotId() >= PLAYER_ID) && (h.hotspotId() < FIRST_NONCHARACTER_ID)) 
				++numCharacters;
		}

		if (numCharacters >= 4) {
			uint16 dataId = res.getCharOffset(0);
			CharacterScheduleEntry *entry = res.charSchedules().getEntry(dataId);
			h.currentActions().addFront(DISPATCH_ACTION, entry, h.roomNumber());
			
			return;
		}
	}

	h.setDestPosition(exitData.x, exitData.y);
	h.setDestHotspot(res.exitHotspots().getHotspot(h.roomNumber(), exitData.hotspotIndexId));

	if (h.destHotspotId() != 0xffff) {
		RoomExitJoinData *joinRec = res.getExitJoin(h.destHotspotId());

		if (joinRec->blocked) {
			// The room exit is blocked - so add an opening action
			h.currentActions().addFront(OPEN, h.roomNumber(), h.destHotspotId(), 0);
			h.setBlockedFlag(false);
			return;
		}
	}

	// No exit hotspot, or it has one that's not blocked. So start the walking
	h.currentActions().top().setAction(START_WALKING);  
	h.setWalkFlag(true);
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

void PathFinder::clear() {
	_stepCtr = 0;
	_list.clear();
	_inProgress = false;
	_countdownCtr = PATHFIND_COUNTDOWN;
}

void PathFinder::reset(RoomPathsData &src) {
	clear();
	src.decompress(_layer, _hotspot->widthCopy());
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

			_inProgress = false;
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
	_inProgress = false;

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
	if (_hotspot->hotspotId() == PLAYER_ID) {
		for (int ctr = 0; ctr < DECODED_PATHS_WIDTH * DECODED_PATHS_HEIGHT; ++ctr)
			Room::getReference().tempLayer[ctr] = _layer[ctr];
	}

	// Determine the walk path by working backwards from the destination, adding in the 
	// walking steps in reverse order until source is reached
	int stageCtr;
	for (stageCtr = 0; stageCtr < 3; ++stageCtr) {
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
	if (_xPos < 0) add(RIGHT, -_xPos);
	else if (_xPos > 0) add(LEFT, _xPos);

	return true;
}

void PathFinder::list(char *buffer) {
	if (buffer) {
		sprintf(buffer, "Pathfinder::list\n");
		buffer += strlen(buffer);
	}
	else {
		printf("Pathfinder::list\n");
	}
	
	ManagedList<WalkingActionEntry *>::iterator i;
	for (i = _list.begin(); i != _list.end(); ++i) {
		WalkingActionEntry *e = *i;
		if (buffer) {
			sprintf(buffer, "Direction=%d, numSteps=%d\n", e->direction(), e->numSteps());
			buffer += strlen(buffer);
		}
		else
			printf("Direction=%d, numSteps=%d\n", e->direction(), e->numSteps());
	}
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

	_result = PF_OK;

	// Subtract an amount from the countdown counter to compensate for
	// the time spent decompressing the walkable areas set for the room
	_countdownCtr -= 700;
}

void PathFinder::saveToStream(Common::WriteStream *stream) {
	// Note: current saving process only handles the PathFinder correctly
	// if all pathfinding is done in one go (ie. multiple calls pathfinding
	// isn't supported)

	ManagedList<WalkingActionEntry *>::iterator i;
	for (i = _list.begin(); i != _list.end(); ++i) {
		WalkingActionEntry *entry = *i;
		stream->writeByte(entry->direction());
		stream->writeSint16LE(entry->rawSteps());
	}
	stream->writeByte(0xff);
	stream->writeByte(_result);
	stream->writeSint16LE(_stepCtr);
}

void PathFinder::loadFromStream(Common::ReadStream *stream) {
	_inProgress = false;
	_list.clear();
	uint8 direction;
	while ((direction = stream->readByte()) != 0xff) {
		int steps = stream->readSint16LE();
		_list.push_back(new WalkingActionEntry((Direction) direction, steps));
	}
	_result = (PathFinderResult)stream->readByte();
	_stepCtr = stream->readSint16LE();
}

// Current action entry class methods

CurrentActionEntry::CurrentActionEntry(CurrentAction newAction, uint16 roomNum) {
	_action = newAction; 
	_supportData = NULL; 
	_dynamicSupportData = false;
	_roomNumber = roomNum;
}

CurrentActionEntry::CurrentActionEntry(CurrentAction newAction, CharacterScheduleEntry *data, uint16 roomNum) { 
	assert(data->parent() != NULL);
	_action = newAction; 
	_supportData = data; 
	_dynamicSupportData = false;
	_roomNumber = roomNum;
}

CurrentActionEntry::CurrentActionEntry(Action newAction, uint16 roomNum, uint16 param1, uint16 param2) {
	_action = DISPATCH_ACTION;
	_dynamicSupportData = true;
	_supportData = new CharacterScheduleEntry();
	uint16 params[2] = {param1, param2};
	_supportData->setDetails2(newAction, 2, params);
	_roomNumber = roomNum;
}

void CurrentActionEntry::setSupportData(uint16 entryId) {
	CharacterScheduleEntry &entry = supportData();

	CharacterScheduleEntry *newEntry = Resources::getReference().
		charSchedules().getEntry(entryId, entry.parent());
	setSupportData(newEntry);
}

void CurrentActionEntry::saveToStream(WriteStream *stream) {
	debugC(ERROR_DETAILED, kLureDebugAnimations, "Saving hotspot action entry dyn=%d id=%d",
		hasSupportData(), hasSupportData() ? supportData().id() : 0);
	stream->writeByte((uint8) _action);
	stream->writeUint16LE(_roomNumber);
	stream->writeByte(hasSupportData());
	if (hasSupportData()) {
		// Handle the support data
		stream->writeByte(_dynamicSupportData);
		if (_dynamicSupportData)
		{
			// Write out the dynamic data
			stream->writeSint16LE(supportData().numParams());
			for (int index = 0; index < supportData().numParams(); ++index)
				stream->writeUint16LE(supportData().param(index));
		}
		else
		{
			// Write out the Id for the static entry
			stream->writeSint16LE(supportData().id());
		}
	}
	debugC(ERROR_DETAILED, kLureDebugAnimations, "Finished saving hotspot action entry");
}

CurrentActionEntry *CurrentActionEntry::loadFromStream(ReadStream *stream) {
	Resources &res = Resources::getReference();
	uint8 actionNum = stream->readByte();
	if (actionNum == 0xff) return NULL;
	CurrentActionEntry *result;

	uint16 roomNumber = stream->readUint16LE();
	bool hasSupportData = stream->readByte() != 0;

	if (!hasSupportData) {
		// An entry that doesn't have support data
		result = new CurrentActionEntry(
			(CurrentAction) actionNum, roomNumber);
	} else {
		// Handle support data for the entry
		bool dynamicData = stream->readByte() != 0;
		if (dynamicData)
		{
			// Load action entry that has dynamic data
			result = new CurrentActionEntry(
				(CurrentAction) actionNum, roomNumber);
			result->_supportData = new CharacterScheduleEntry();
			Action action = (Action) stream->readByte();
			int numParams = stream->readSint16LE();
			uint16 *paramList = new uint16[numParams];
			for (int index = 0; index < numParams; ++index)
				paramList[index] = stream->readUint16LE();
				
			result->_supportData->setDetails2(action, numParams, paramList);
			delete paramList;
		}
		else
		{
			// Load action entry with an NPC schedule entry
			uint16 entryId = stream->readUint16LE();
			CharacterScheduleEntry *entry = res.charSchedules().getEntry(entryId);
			result = new CurrentActionEntry((CurrentAction) actionNum, roomNumber);
			result->setSupportData(entry);
		}
	}

	return result;
}

void CurrentActionStack::list(char *buffer) {
	ManagedList<CurrentActionEntry *>::iterator i;

	if (buffer) {
		sprintf(buffer, "CurrentActionStack::list num_actions=%d\n", size());
		buffer += strlen(buffer);
	}
	else
		printf("CurrentActionStack::list num_actions=%d\n", size());

	for (i = _actions.begin(); i != _actions.end(); ++i) {
		CurrentActionEntry *entry = *i;
		if (buffer) {
			sprintf(buffer, "style=%d room#=%d", entry->action(), entry->roomNumber());
			buffer += strlen(buffer);
		}
		else
			printf("style=%d room#=%d", entry->action(), entry->roomNumber());
	
		if (entry->hasSupportData()) {
			CharacterScheduleEntry &rec = entry->supportData();

			if (buffer) {
				sprintf(buffer, ", action=%d params=", rec.action());
				buffer += strlen(buffer);
			}
			else
				printf(", action=%d params=", rec.action());

			if (rec.numParams() == 0) 
				if (buffer) {
					strcat(buffer, "none");
					buffer += strlen(buffer);
				}
				else
					printf("none");
			else {
				for (int ctr = 0; ctr < rec.numParams(); ++ctr) {
					if (ctr != 0) {
						if (buffer) {
							strcpy(buffer, ", ");
							buffer += strlen(buffer);
						}
						else
							printf(", ");
					}

					if (buffer) {
						sprintf(buffer, "%d", rec.param(ctr));
						buffer += strlen(buffer);
					} else 
						printf("%d", rec.param(ctr));
				}
			}
		}
		if (buffer) {
			sprintf(buffer, "\n");
			buffer += strlen(buffer);
		}
		else
			printf("\n");
	}
}

void CurrentActionStack::saveToStream(WriteStream *stream) {
	ManagedList<CurrentActionEntry *>::iterator i;

	debugC(ERROR_DETAILED, kLureDebugAnimations, "Saving hotspot action stack");
	char buffer[MAX_DESC_SIZE];
	list(buffer);
	debugC(ERROR_DETAILED, kLureDebugAnimations, "%s", buffer);

	for (i = _actions.begin(); i != _actions.end(); ++i)
	{
		CurrentActionEntry *rec = *i;
		rec->saveToStream(stream);
	}
	stream->writeByte(0xff);      // End of list marker
	debugC(ERROR_DETAILED, kLureDebugAnimations, "Finished saving hotspot action stack");
}

void CurrentActionStack::loadFromStream(ReadStream *stream) {
	CurrentActionEntry *rec;

	_actions.clear();
	while ((rec = CurrentActionEntry::loadFromStream(stream)) != NULL)
		_actions.push_back(rec);
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
	uint16 hotspotY;

	r.left = h.x();
	r.right = h.x() + h.widthCopy();
	r.top = h.y() + h.heightCopy() - h.yCorrection() - h.charRectY();
	r.bottom = h.y() + h.heightCopy() + h.charRectY();

	HotspotList::iterator i;
	for (i = res.activeHotspots().begin(); i != res.activeHotspots().end(); ++i) {
		Hotspot &hotspot = **i;
		
		// Check for basic reasons to skip checking the animation
		if ((h.hotspotId() == hotspot.hotspotId()) || (hotspot.layer() == 0) ||
			(h.roomNumber() != hotspot.roomNumber()) || 
			(hotspot.hotspotId() >= FIRST_NONCHARACTER_ID) ||
			hotspot.skipFlag()) continue;
		// TODO: See why si+ANIM_HOTSPOT_OFFSET compared aganst di+ANIM_VOICE_CTR

		hotspotY = hotspot.y() + hotspot.heightCopy();
		if ((hotspot.x() >= r.right) || (hotspot.x() + hotspot.widthCopy() <= r.left) ||
			(hotspotY + hotspot.charRectY() <= r.top) ||
			(hotspotY - hotspot.charRectY() - hotspot.yCorrection() >= r.bottom))
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

bool Support::checkRoomChange(Hotspot &h) {
	int16 x = h.x() + (h.widthCopy() >> 1);
	int16 y = h.y() + h.heightCopy() - (h.yCorrection() >> 1);

	RoomData *roomData = Resources::getReference().getRoom(h.roomNumber());
	RoomExitData *exitRec = roomData->exits.checkExits(x, y);

	if (exitRec) {
		// End the current walking sequence
		if (exitRec->sequenceOffset != 0xffff) {
			Script::execute(exitRec->sequenceOffset);
		} else {
			Support::characterChangeRoom(h, exitRec->roomNumber, 
				exitRec->x, exitRec->y, exitRec->direction);
		}
	}

	return (exitRec != NULL);
}

void Support::characterChangeRoom(Hotspot &h, uint16 roomNumber, 
								  int16 newX, int16 newY, Direction dir) {
	Resources &res = Resources::getReference();
	Room &room = Room::getReference();
	ValueTableData &fields = res.fieldList();

	if (h.hotspotId() == PLAYER_ID) {
		// Room change code for the player
		if (room.cursorState() != CS_NONE) return;

		h.setDirection(dir);
		PlayerNewPosition &p = fields.playerNewPos();
		p.roomNumber = roomNumber;
		p.position.x = newX;
		p.position.y = newY - 48;

		// TODO: Double-check.. is it impinging in leaving room (right now) or entering room
		if (checkForIntersectingCharacter(h)) {
			fields.playerPendingPos().pos.x = h.destX();
			fields.playerPendingPos().pos.y = h.destY();
			fields.playerPendingPos().isSet = true;
			Room::getReference().setCursorState(CS_BUMPED);
			h.setActionCtr(0);
			h.setBlockedState((BlockedState) ((int) h.blockedState() + 1));
			h.setDestHotspot(0);
			h.setRandomDest();
			p.roomNumber = 0;
		}

	} else {
		// Any other character changing room

		if (checkForIntersectingCharacter(h)) {
			// Character is blocked, so add a handler for handling it
			uint16 dataId = res.getCharOffset(0);
			CharacterScheduleEntry *entry = res.charSchedules().getEntry(dataId);
			h.currentActions().addFront(DISPATCH_ACTION, entry, h.roomNumber());
		} else {
			// Handle character room change
			h.setRoomNumber(roomNumber);
			h.setPosition((newX & 0xfff8) | 5, (newY - h.heightCopy()) & 0xfff8);
			h.setSkipFlag(true);
			h.setDirection(dir);

			h.setExitCtr(0);
			h.currentActions().top().setAction(DISPATCH_ACTION);
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

bool Support::isCharacterInList(uint16 *lst, int numEntries, uint16 charId) {
	while (numEntries-- > 0) 
		if (*lst++ == charId) return true;
	return false;
}

void HotspotList::saveToStream(WriteStream *stream) {
	HotspotList::iterator i;
	for (i = begin(); i != end(); ++i)
	{
		Hotspot *hotspot = *i;
		debugC(ERROR_INTERMEDIATE, kLureDebugAnimations, "Saving hotspot %xh", hotspot->hotspotId());
		bool dynamicObject = hotspot->hotspotId() != hotspot->originalId();
		stream->writeUint16LE(hotspot->originalId());
		stream->writeByte(dynamicObject);
		stream->writeUint16LE(hotspot->destHotspotId());
		hotspot->saveToStream(stream);

		debugC(ERROR_DETAILED, kLureDebugAnimations, "Saved hotspot %xh", hotspot->hotspotId());
	}
	stream->writeUint16LE(0);
}

void HotspotList::loadFromStream(ReadStream *stream) {
	Resources &res = Resources::getReference();
	Hotspot *hotspot;

	clear();
	uint16 hotspotId = stream->readUint16LE();
	while (hotspotId != 0)
	{
		debugC(ERROR_INTERMEDIATE, kLureDebugAnimations, "Loading hotspot %xh", hotspotId);
		bool dynamicObject = stream->readByte() != 0;
		uint16 destHotspotId = stream->readUint16LE();

		if (dynamicObject) {
			// Add in a dynamic object (such as a floating talk bubble)
			Hotspot *destHotspot = res.getActiveHotspot(destHotspotId);
			assert(destHotspot);
			hotspot = new Hotspot(destHotspot, hotspotId);
			res.addHotspot(hotspot);
		}
		else
		{
			hotspot = res.activateHotspot(hotspotId);
		}
		assert(hotspot);

		hotspot->loadFromStream(stream);
		debugC(ERROR_DETAILED, kLureDebugAnimations, "Loaded hotspot %xh", hotspotId);

		hotspotId = stream->readUint16LE();
	}
}

} // end of namespace Lure
