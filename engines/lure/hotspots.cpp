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

Hotspot::Hotspot(HotspotData *res) {
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
}

// Special constructor used to create a voice hotspot

Hotspot::Hotspot(Hotspot *character, uint16 objType) {
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

	for (uint16 frameCtr = 0; frameCtr < _numFrames; ++frameCtr, ++headerEntry) {
		
		// Copy over the frame, applying the colour offset to each nibble
		for (uint16 yPos = 0; yPos < _height; ++yPos) {
			pDest = mDest.data() + (yPos * _numFrames + frameCtr) * _width;

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


void Hotspot::walkTo(int16 endPosX, int16 endPosY, uint16 destHotspot, bool immediate) {
	_destX = endPosX;
	_destY = endPosY - _height;

	_destHotspotId = destHotspot;
	if (immediate) 
		setPosition(_destX, _destY);
}

void Hotspot::setDirection(Direction dir) {
	switch (dir) {
	case UP:
		setFrameNumber(_anim->upFrame);
		break;
	case DOWN:
		setFrameNumber(_anim->downFrame);
		break;
	case LEFT:
		setFrameNumber(_anim->leftFrame);
		break;
	case RIGHT:
		setFrameNumber(_anim->rightFrame);
		break;
	default:
		break;
	}
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
	int16 xPos = h.x();
	int16 yPos = h.y();
	if ((xPos == h.destX()) && (yPos == h.destY())) return;
	HotspotAnimData &anim = h.anim();
	int16 xDiff = h.destX() - h.x();
	int16 yDiff = h.destY() - h.y();

	int16 xChange, yChange;
	uint16 nextFrame;
	MovementDataList *moves;

	if ((yDiff < 0) && (xDiff <= 0)) moves = &anim.upFrames;
	else if (xDiff < 0) moves = &anim.leftFrames;
	else if (yDiff > 0) moves = &anim.downFrames;
	else moves = &anim.rightFrames;

	// Get movement amount and next frame number
	moves->getFrame(h.frameNumber(), xChange, yChange, nextFrame);
	xPos += xChange; yPos += yChange;

	// Make sure that the move amount doesn't overstep the destination X/Y
	if ((yDiff < 0) && (yPos < h.destY())) yPos = h.destY();
	else if ((xDiff < 0) && (xPos < h.destX())) xPos = h.destX();
	else if ((yDiff > 0) && (yPos > h.destY())) yPos = h.destY();
	else if ((xDiff > 0) && (xPos > h.destX())) xPos = h.destX();

	// Check to see if player has entered an exit area
	RoomData *roomData = Resources::getReference().getRoom(h.roomNumber());
	Room &room = Room::getReference();
	bool charInRoom = room.roomNumber() == h.roomNumber();
	RoomExitData *exitRec = roomData->exits.checkExits(xPos, yPos + h.height());

	if (!exitRec) {
		h.setPosition(xPos, yPos);
		h.setFrameNumber(nextFrame);
	} else {
		h.setRoomNumber(exitRec->roomNumber);
		h.walkTo(exitRec->x, exitRec->y, 0, true);
		if (exitRec->direction != NO_DIRECTION)
			h.setDirection(exitRec->direction);
		if (charInRoom)
			room.setRoomNumber(exitRec->roomNumber, false);
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
	// TODO: figure out remainder of method
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

} // end of namespace Lure
