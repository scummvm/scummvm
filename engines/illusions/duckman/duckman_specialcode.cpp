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
#include "illusions/duckman/duckman_credits.h"
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
	_credits = new DuckmanCredits(_vm);

	_wasCursorHoldingElvisPoster = false;
	_counter = 0;
	_savedTempMasterSfxVolume = 16;
	_lastRandomSoundIndex = 6;
}

DuckmanSpecialCode::~DuckmanSpecialCode() {
	delete _propertyTimers;
	delete _inventory;
	delete _credits;

	for (SpecialCodeMap::iterator it = _specialCodeMap.begin(); it != _specialCodeMap.end(); ++it) {
		delete (*it)._value;
	}
}

typedef Common::Functor1Mem<OpCall&, void, DuckmanSpecialCode> SpecialCodeFunctionDM;
#define SPECIAL(id, func) _specialCodeMap[id] = new SpecialCodeFunctionDM(this, &DuckmanSpecialCode::func);

void DuckmanSpecialCode::init() {
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
		error("DuckmanSpecialCode::run() Unimplemented special code");
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
	default:
		break;
	}
	_vm->notifyThreadId(opCall._threadId);
}

void DuckmanSpecialCode::spcStartCredits(OpCall &opCall) {
	ARG_BYTE(mode);
	if (mode == 0)
		_credits->start();
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

} // End of namespace Illusions
