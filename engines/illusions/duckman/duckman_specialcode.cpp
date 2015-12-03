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

#include "illusions/duckman/illusions_duckman.h"
#include "illusions/duckman/duckman_screenshakereffects.h"
#include "illusions/duckman/duckman_specialcode.h"
#include "illusions/duckman/duckman_inventory.h"
#include "illusions/duckman/propertytimers.h"
#include "illusions/duckman/scriptopcodes_duckman.h"
#include "illusions/actor.h"
#include "illusions/dictionary.h"
#include "illusions/resources/fontresource.h"
#include "illusions/resources/scriptresource.h"
#include "illusions/sound.h"
#include "illusions/specialcode.h"
#include "illusions/textdrawer.h"
#include "illusions/time.h"
#include "illusions/updatefunctions.h"

#include "engines/util.h"

namespace Illusions {

// Duckman_SpecialCode

DuckmanSpecialCode::DuckmanSpecialCode(IllusionsEngine_Duckman *vm)
	: _vm(vm) {

	_propertyTimers = new PropertyTimers(_vm);
	_inventory = new DuckmanInventory(_vm);
	_wasCursorHoldingElvisPoster = false;
	_counter = 0;
	_savedTempMasterSfxVolume = 16;
	_lastRandomSoundIndex = 6;
}

DuckmanSpecialCode::~DuckmanSpecialCode() {
	delete _propertyTimers;
	delete _inventory;
}

typedef Common::Functor1Mem<OpCall&, void, DuckmanSpecialCode> SpecialCodeFunctionDM;
#define SPECIAL(id, func) _specialCodeMap[id] = new SpecialCodeFunctionDM(this, &DuckmanSpecialCode::func);

void DuckmanSpecialCode::init() {
	// TODO
	SPECIAL(0x00160001, spcStartScreenShaker);
	SPECIAL(0x00160002, spcSetCursorHandMode);
	SPECIAL(0x00160003, spcResetChinesePuzzle);
	SPECIAL(0x00160004, spcAddChinesePuzzleAnswer);
	SPECIAL(0x00160005, spcOpenInventory);
	SPECIAL(0x00160007, spcPutBackInventoryItem);
	SPECIAL(0x00160008, spcClearInventorySlot);
	SPECIAL(0x0016000A, spcAddPropertyTimer);
	SPECIAL(0x0016000B, spcSetPropertyTimer);
	SPECIAL(0x0016000C, spcRemovePropertyTimer);
	SPECIAL(0x0016000E, spcInitTeleporterPosition);
	SPECIAL(0x0016000F, spcUpdateTeleporterPosition);
	SPECIAL(0x00160010, spcCenterNewspaper);
	SPECIAL(0x00160012, spcStopScreenShaker);
	SPECIAL(0x00160013, spcIncrCounter);
	SPECIAL(0x00160014, spcUpdateObject272Sequence);
	SPECIAL(0x00160017, spcPlayRandomSound);
	SPECIAL(0x0016001A, spcHoldGlowingElvisPoster);
	SPECIAL(0x0016001B, spcStartCredits);
	SPECIAL(0x0016001C, spcSetCursorInventoryMode);
	SPECIAL(0x0016001D, spcCenterCurrentScreenText);
	SPECIAL(0x0016001E, spcSetDefaultTextCoords);
	SPECIAL(0x0016001F, spcSetTextDuration);
	SPECIAL(0x00160020, spcSetTempMasterSfxVolume);
	SPECIAL(0x00160021, spcRestoreTempMasterSfxVolume);
}

#undef SPECIAL

void DuckmanSpecialCode::run(uint32 specialCodeId, OpCall &opCall) {
	SpecialCodeMapIterator it = _specialCodeMap.find(specialCodeId);
	if (it != _specialCodeMap.end()) {
		(*(*it)._value)(opCall);
	} else {
		debug("DuckmanSpecialCode::run() Unimplemented special code %08X", specialCodeId);
		_vm->notifyThreadId(opCall._threadId);
	}
}

void DuckmanSpecialCode::spcStartScreenShaker(OpCall &opCall) {
	ARG_BYTE(effect);
	const ScreenShakeEffect *shakerEffect = getScreenShakeEffect(effect);
	_vm->startScreenShaker(shakerEffect->_pointsCount, shakerEffect->_duration, shakerEffect->_points, opCall._threadId);
}

void DuckmanSpecialCode::spcSetCursorHandMode(OpCall &opCall) {
	ARG_BYTE(mode);
	_vm->setCursorHandMode(mode);
	_vm->notifyThreadId(opCall._threadId);
}

void DuckmanSpecialCode::spcResetChinesePuzzle(OpCall &opCall) {
	_vm->_scriptResource->_properties.set(0x000E0018, false);
	_vm->_scriptResource->_properties.set(0x000E0019, false);
	_chinesePuzzleIndex = 0;
	_vm->notifyThreadId(opCall._threadId);
}

void DuckmanSpecialCode::spcAddChinesePuzzleAnswer(OpCall &opCall) {
	ARG_BYTE(answer);
	_chinesePuzzleAnswers[_chinesePuzzleIndex++] = answer;
	if (_chinesePuzzleIndex == 3) {
		_vm->_scriptResource->_properties.set(0x000E0018, true);
		if ((_chinesePuzzleAnswers[0] == 7 && _chinesePuzzleAnswers[1] == 2 && _chinesePuzzleAnswers[2] == 5) ||
			(_chinesePuzzleAnswers[0] == 5 && _chinesePuzzleAnswers[1] == 2 && _chinesePuzzleAnswers[2] == 7))
			_vm->_scriptResource->_properties.set(0x000E0019, true);
		else if ((_chinesePuzzleAnswers[0] == 7 && _chinesePuzzleAnswers[1] == 2 && _chinesePuzzleAnswers[2] == 1) ||
			(_chinesePuzzleAnswers[0] == 1 && _chinesePuzzleAnswers[1] == 2 && _chinesePuzzleAnswers[2] == 7))
			_vm->_scriptResource->_properties.set(0x000E00A0, true);
	}
	_vm->notifyThreadId(opCall._threadId);
}

void DuckmanSpecialCode::spcOpenInventory(OpCall &opCall) {
	_inventory->openInventory();
	_vm->notifyThreadId(opCall._threadId);
}

void DuckmanSpecialCode::spcPutBackInventoryItem(OpCall &opCall) {
	_inventory->putBackInventoryItem();
	_vm->notifyThreadId(opCall._threadId);
}

void DuckmanSpecialCode::spcClearInventorySlot(OpCall &opCall) {
	ARG_UINT32(objectId);
	_inventory->clearInventorySlot(objectId);
	_vm->notifyThreadId(opCall._threadId);
}

void DuckmanSpecialCode::spcAddPropertyTimer(OpCall &opCall) {
	ARG_UINT32(propertyId);
	_propertyTimers->addPropertyTimer(propertyId);
	_vm->notifyThreadId(opCall._threadId);
}

void DuckmanSpecialCode::spcSetPropertyTimer(OpCall &opCall) {
	ARG_INT16(propertyNum);
	ARG_INT16(duration);
	_propertyTimers->setPropertyTimer(propertyNum | 0xE0000, duration);
	_vm->notifyThreadId(opCall._threadId);
}

void DuckmanSpecialCode::spcRemovePropertyTimer(OpCall &opCall) {
	ARG_UINT32(propertyId);
	_propertyTimers->removePropertyTimer(propertyId);
	_vm->notifyThreadId(opCall._threadId);
}

void DuckmanSpecialCode::spcInitTeleporterPosition(OpCall &opCall) {
	_teleporterPosition.x = 4;
	_teleporterPosition.y = 3;
	updateTeleporterProperties();
	_vm->_scriptResource->_properties.set(0x000E007A, false);
	_vm->notifyThreadId(opCall._threadId);
}

void DuckmanSpecialCode::spcUpdateTeleporterPosition(OpCall &opCall) {
	ARG_BYTE(direction);
	int16 deltaX = 0;
	int16 deltaY = 0;
	uint32 sequenceId = 0;
	
	Control *control = _vm->getObjectControl(0x400C0);
	switch (direction) {
	case 1:
		if (_teleporterPosition.y > 1) {
			deltaY = -1;
			sequenceId = 0x60386;
		}
		break;
	case 4:
		if (_teleporterPosition.x < 4) {
			deltaX = 1;
			sequenceId = 0x60387;
		}
		break;
	case 0x10:
		if (_teleporterPosition.y < 3) {
			deltaY = 1;
			sequenceId = 0x60385;
		}
		break;
	case 0x40:
		if (_teleporterPosition.x > 1) {
			deltaX = -1;
			sequenceId = 0x60388;
		}
		break;
	default:
		break;
	}
	
	if (sequenceId) {
		control->startSequenceActor(sequenceId, 2, opCall._threadId);
		_teleporterPosition.x += deltaX;
		_teleporterPosition.y += deltaY;
		updateTeleporterProperties();
		_vm->_scriptResource->_properties.set(0x000E007A, false);
	} else {
		_vm->notifyThreadId(opCall._threadId);
	}

	_vm->notifyThreadId(opCall._threadId);
}

void DuckmanSpecialCode::spcCenterNewspaper(OpCall &opCall) {
	Control *control = _vm->getObjectControl(0x40017);
	control->_flags |= 8;
	control->_actor->_position.x = 160;
	control->_actor->_position.y = 100;
	_vm->notifyThreadId(opCall._threadId);
}

void DuckmanSpecialCode::spcStopScreenShaker(OpCall &opCall) {
	_vm->stopScreenShaker();
	_vm->notifyThreadId(opCall._threadId);
}

void DuckmanSpecialCode::spcIncrCounter(OpCall &opCall) {
	ARG_BYTE(maxCount);
	ARG_BYTE(incr);
	_vm->_scriptResource->_properties.set(0x000E0088, false);
	if (incr) {
		_counter += incr;
		if (_counter >= maxCount)
			_vm->_scriptResource->_properties.set(0x000E0088, true);
	} else {
		_counter = 0;
	}
	_vm->notifyThreadId(opCall._threadId);
}

void DuckmanSpecialCode::spcUpdateObject272Sequence(OpCall &opCall) {
	byte flags = 0;
	uint32 sequenceId;
	if (_vm->_scriptResource->_properties.get(0x000E0085))
		flags |= 1;
	if (_vm->_scriptResource->_properties.get(0x000E0083))
		flags |= 2;
	if (_vm->_scriptResource->_properties.get(0x000E0084))
		flags |= 4;
	switch (flags) {
	case 0:
		sequenceId = 0x603C1;
		break;
	case 1:
		sequenceId = 0x603BF;
		break;
	case 2:
		sequenceId = 0x603C2;
		break;
	case 3:
		sequenceId = 0x603C0;
		break;
	case 4:
		sequenceId = 0x603C3;
		break;
	case 5:
		sequenceId = 0x603C5;
		break;
	case 6:
		sequenceId = 0x603C4;
		break;
	case 7:
		sequenceId = 0x603C6;
		break;
	default:
		sequenceId = 0x603C1;
		break;
	}
	Control *control = _vm->getObjectControl(0x40110);
	control->startSequenceActor(sequenceId, 2, opCall._threadId);
}

void DuckmanSpecialCode::spcPlayRandomSound(OpCall &opCall) {
	static const uint32 kRandomSoundIds[] = {
		0x00090084, 0x00090085, 0x00090086, 0x00090087, 0x00090088, 0x00090089
	};
	int16 soundIndex;
	do {
		soundIndex = _vm->getRandom(ARRAYSIZE(kRandomSoundIds));
	} while (soundIndex == _lastRandomSoundIndex);
	_vm->_soundMan->playSound(kRandomSoundIds[soundIndex], 255, 0);
	_lastRandomSoundIndex = soundIndex;
	_vm->notifyThreadId(opCall._threadId);
}

void DuckmanSpecialCode::spcHoldGlowingElvisPoster(OpCall &opCall) {
	const uint32 kPosterObjectId = 0x40072;
	const uint32 kPosterSequenceId = 0x60034;
	ARG_BYTE(mode);
	switch (mode) {
	case 0:
		if (_vm->_cursor._objectId == kPosterObjectId) {
			_wasCursorHoldingElvisPoster = true;
			_inventory->addInventoryItem(_vm->_cursor._objectId);
			_vm->stopCursorHoldingObject();
		} else {
			_wasCursorHoldingElvisPoster = false;
		}
		break;
	case 1:
		if (_wasCursorHoldingElvisPoster) {
			_inventory->clearInventorySlot(kPosterObjectId);
			_vm->_cursor._objectId = kPosterObjectId;
			_vm->_cursor._sequenceId2 = kPosterSequenceId;
			_vm->_cursor._field14[_vm->_cursor._actorIndex - 1] = true;
		}
		break;
	}
	_vm->notifyThreadId(opCall._threadId);
}

void DuckmanSpecialCode::spcStartCredits(OpCall &opCall) {
	ARG_BYTE(mode);
	if (mode == 0)
		startCredits();
	_vm->notifyThreadId(opCall._threadId);
}

void DuckmanSpecialCode::spcSetCursorInventoryMode(OpCall &opCall) {
	ARG_BYTE(mode);
	ARG_BYTE(value);
	_vm->setCursorInventoryMode(mode, value);
	_vm->notifyThreadId(opCall._threadId);
}

void DuckmanSpecialCode::spcCenterCurrentScreenText(OpCall &opCall) {
	WidthHeight dimensions;
	_vm->getDefaultTextDimensions(dimensions);
	Common::Point pt(160, dimensions._height / 2 + 8);
	_vm->setDefaultTextPosition(pt);
	_vm->notifyThreadId(opCall._threadId);
}

void DuckmanSpecialCode::spcSetDefaultTextCoords(OpCall &opCall) {
	_vm->setDefaultTextCoords();
	_vm->notifyThreadId(opCall._threadId);
}

void DuckmanSpecialCode::spcSetTextDuration(OpCall &opCall) {
	ARG_INT16(kind);
	ARG_INT16(duration);
	_vm->setTextDuration(kind, duration);
	_vm->notifyThreadId(opCall._threadId);
}

void DuckmanSpecialCode::spcSetTempMasterSfxVolume(OpCall &opCall) {
	ARG_INT16(sfxVolume);
	// TODO _savedTempMasterSfxVolume = _vm->getMasterSfxVolume();
	// TODO _vm->setMasterSfxVolume(sfxVolume);
	_vm->notifyThreadId(opCall._threadId);
}

void DuckmanSpecialCode::spcRestoreTempMasterSfxVolume(OpCall &opCall) {
	// TODO _vm->setMasterSfxVolume(_savedTempMasterSfxVolume);
	_savedTempMasterSfxVolume = 16;
	_vm->notifyThreadId(opCall._threadId);
}

void DuckmanSpecialCode::updateTeleporterProperties() {
	_vm->_scriptResource->_properties.set(0x000E0074, _teleporterPosition.x == 4 && _teleporterPosition.y == 2);
	_vm->_scriptResource->_properties.set(0x000E0075, _teleporterPosition.x == 4 && _teleporterPosition.y == 3);
	_vm->_scriptResource->_properties.set(0x000E0076, _teleporterPosition.x == 3 && _teleporterPosition.y == 3);
	_vm->_scriptResource->_properties.set(0x000E0077, _teleporterPosition.x == 2 && _teleporterPosition.y == 2);
	_vm->_scriptResource->_properties.set(0x000E0078, _teleporterPosition.x == 1 && _teleporterPosition.y == 1);	
}

void DuckmanSpecialCode::startCredits() {
	static const struct { uint32 objectId; int scrollPosY; } kCreditsItems[] = {
		{0x40136,   0}, {0x40137,  16}, {0x40138,  32}, {0x40139,  48},
		{0x4013A,  64}, {0x4013B,  80}, {0x4013C,  96}, {0x4013D, 112}
	};
	_creditsCurrText = (char*)_vm->_resSys->getResource(0x190052)->_data;
	_creditsItems.clear();
	for (uint i = 0; i < ARRAYSIZE(kCreditsItems);  ++i) {
		CreditsItem creditsItem;
		creditsItem.objectId = kCreditsItems[i].objectId;
		creditsItem.scrollPosY = kCreditsItems[i].scrollPosY;
		creditsItem.scrollPosIndex = 0;
		creditsItem.active = false;
		_creditsItems.push_back(creditsItem);
	}
	uint32 currSceneId = _vm->getCurrentScene();
	_vm->_updateFunctions->add(0, currSceneId, new Common::Functor1Mem<uint, int, DuckmanSpecialCode>(this, &DuckmanSpecialCode::updateCredits));
	_creditsNextUpdateTicks = getCurrentTime();
	_creditsLastUpdateTicks = _creditsNextUpdateTicks - 4;
}

int DuckmanSpecialCode::updateCredits(uint flags) {

	if (_vm->_pauseCtr > 0) {
		_creditsNextUpdateTicks = getCurrentTime() + 4;
		return 1;
	}

	if (flags & 1) {
		_vm->_scriptResource->_properties.set(0x000E0096, true);
		_lastCreditsItemIndex = -1;
		_creditsEndReached = false;
		return 2;
	}

	if (!isTimerExpired(_creditsLastUpdateTicks, _creditsNextUpdateTicks)) {
		return 1;
	}

	bool creditsRunning = false;
	int index = 0;
	for (CreditsItems::iterator it = _creditsItems.begin(); it != _creditsItems.end(); ++it, ++index) {
		CreditsItem &creditsItem = *it;
		Control *control = _vm->getObjectControl(creditsItem.objectId);
		if (!creditsItem.active && creditsItem.scrollPosY == 0 && !_creditsEndReached) {
			creditsItem.active = true;
			creditsItem.scrollPosIndex = 0;
			control->fillActor(0);
			char *text = readNextCreditsLine();
			if (!strncmp(text, "&&&END", 6)) {
				creditsItem.active = false;
				_creditsEndReached = true;
			} else {
				uint16 wtext[128];
				charToWChar(text, wtext, ARRAYSIZE(wtext));

				FontResource *font = _vm->_dict->findFont(0x120001); 
				TextDrawer textDrawer;
				WidthHeight dimensions;
				uint16 *outText;
				control->getActorFrameDimensions(dimensions);
				textDrawer.wrapText(font, wtext, &dimensions, Common::Point(0, 0), 2, outText);
				textDrawer.drawText(_vm->_screen, control->_actor->_surface, 0, 0);
				control->_actor->_flags |= 0x4000;

				_lastCreditsItemIndex = index;
			}
		}
		if (creditsItem.active) {
			if (_creditsEndReached && _creditsItems[_lastCreditsItemIndex].scrollPosIndex > 53) {
				creditsItem.active = false;
				creditsItem.scrollPosY = -1;
			} else {
				creditsRunning = true;
				control->_actor->_position = getCreditsItemPosition(creditsItem.scrollPosIndex);
				++creditsItem.scrollPosIndex;
				if (getCreditsItemPosition(creditsItem.scrollPosIndex).x < 0)
					creditsItem.active = false;
			}
		}
		if (creditsItem.scrollPosY > 0)
			--creditsItem.scrollPosY;
	}
	_creditsLastUpdateTicks = _creditsNextUpdateTicks;
	_creditsNextUpdateTicks = getCurrentTime() + 4;

	if (!creditsRunning) {
		_vm->_scriptResource->_properties.set(0x000E0096, true);
		_lastCreditsItemIndex = -1;
		_creditsEndReached = false;
		return 2;
	}

	return 1;
}

char *DuckmanSpecialCode::readNextCreditsLine() {
	static char line[256];
	char *dest = line;
	char *src = _creditsCurrText;
	do {
		if (*src == 10 || *src == 13) {
			src += 2;
			*dest = 0;
			break;
		}
		*dest++ = *src++;
	} while (1);
	_creditsCurrText = src;
	return line;
}

Common::Point DuckmanSpecialCode::getCreditsItemPosition(int index) {
	static const struct { int16 x, y; } kCreditsItemsPoints[] = {
		{159, 200}, {158, 195}, {157, 190}, {156, 185}, {156, 180}, {157, 176}, 
		{158, 172}, {159, 168}, {161, 164}, {162, 161}, {163, 158}, {163, 155}, 
		{162, 152}, {161, 149}, {159, 147}, {158, 144}, {157, 142}, {156, 140}, 
		{156, 138}, {157, 136}, {158, 134}, {159, 132}, {161, 130}, {162, 128}, 
		{163, 127}, {163, 126}, {162, 125}, {161, 124}, {159, 123}, {158, 122}, 
		{157, 121}, {156, 120}, {156, 119}, {157, 118}, {158, 117}, {159, 116}, 
		{161, 115}, {162, 114}, {163, 113}, {163, 112}, {162, 111}, {161, 110}, 
		{159, 109}, {158, 108}, {157, 107}, {156, 106}, {156, 105}, {157, 104}, 
		{158, 103}, {159, 102}, {161, 101}, {162, 100}, {163,  99}, {163,  98}, 
		{162,  97}, {161,  96}, {159,  95}, {158,  94}, {157,  93}, {156,  92}, 
		{156,  91}, {157,  90}, {158,  89}, {159,  88}, {161,  87}, {162,  86}, 
		{163,  85}, {163,  84}, {162,  83}, {161,  82}, {159,  81}, {158,  80}, 
		{157,  79}, {156,  78}, {156,  77}, {157,  76}, {158,  75}, {159,  74},
		{161,  73}, {162,  72}, {163,  71}, {163,  70}, {162,  69}, {161,  68}, 
		{159,  67}, {158,  66}, {157,  64}, {156,  62}, {156,  60}, {157,  58}, 
		{158,  56}, {159,  54}, {161,  52}, {162,  50}, {163,  40}, {163,  40}, 
		{162,  40}, {161,  40}, {159,  40}, {158,  40}, {157,  40}, {156,  40}, 
		{156,  40}, {157,  40}, {158,  40}, {159,  40}, {161,  40}, {162,  40}, 
		{163,  40}, {163,  40}, {162,  40}, {161,  40}, {159,  40}, {158,  40}, 
		{157,  40}, {156,  40}, {156,  40}, {157,  40}, {158,  40}, {159,  40}, 
		{161,  40}, {162,  40}, {163,  40}, {163,  40}, {162,  40}, {161,  40}, 
		{159,  40}, {158,  40}, { -1,  -1} 
	};

	if (index < 0 || index >= ARRAYSIZE(kCreditsItemsPoints))
		return Common::Point(-1, -1);
	return Common::Point(kCreditsItemsPoints[index].x, kCreditsItemsPoints[index].y);
}

void DuckmanSpecialCode::charToWChar(char *text, uint16 *wtext, uint size) {
	while (*text != 0 && size > 1) {
		*wtext++ = (byte)*text++;
		--size;
	}
	*wtext++ = 0;
}

} // End of namespace Illusions
