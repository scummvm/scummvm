/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "mads/madsv2/nebular/rooms/teleporter_room.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

static int _buttonTyped;
static int _curCode;
static int _digitCount;
static int _curMessageId;
static int _handSpriteId;
static int _handSequenceId;
static int _finishedCodeCounter;
static int _meteorologistNextPlace;
static int _meteorologistCurPlace;
static int _teleporterSceneId;
static char _msgText[256];


void teleporter_init() {
	_buttonTyped = -1;
	_curCode = -1;
	_digitCount = -1;
	_curMessageId = -1;
	_handSpriteId = -1;
	_handSequenceId = -1;
	_finishedCodeCounter = -1;
	_meteorologistNextPlace = -1;
	_meteorologistCurPlace = -1;
	_teleporterSceneId = -1;

	_game._player._visible = false;
	_game._player._stepEnabled = (_globals[kMeteorologistWatch] == METEOROLOGIST_NORMAL);
	_scene->_kernelMessages._talkFont = _vm->_font->getFont(FONT_TELE);
	_scene->_textSpacing = 0;
	_curCode = 0;
	_digitCount = 0;
	_finishedCodeCounter = 0;
	_curMessageId = -1;
	_msgText = "_";

	if (_scene->_priorSceneId == RETURNING_FROM_DIALOG)
		_scene->_priorSceneId = _globals[kTeleporterDestination];

	if (_scene->_priorSceneId < 101)
		_scene->_priorSceneId = 201;

	_globals[kTeleporterDestination] = _scene->_priorSceneId;
	_vm->_palette->setEntry(252, 63, 63, 0);
	_vm->_palette->setEntry(253, 0, 0, 0);
	_teleporterSceneId = _scene->_priorSceneId;
	if (_teleporterSceneId == 202)
		_teleporterSceneId = 201;

	int codeVal = 0;
	for (int i = 0; i < 10; i++) {
		if (_teleporterSceneId == _globals[kTeleporterRoom + i])
			codeVal = _globals[kTeleporterCode + i];

		if (_globals[kTeleporterRoom + i] == 301)
			_meteorologistNextPlace = _globals[kTeleporterCode + i];
	}

	Common::String msgText2 = Common::String::format("#%.4d", codeVal);

	if (_scene->_currentSceneId != 711) {
		_scene->_kernelMessages.add(Common::Point(133, 34), 0, 32, 0, INDEFINITE_TIMEOUT, msgText2);
		_scene->_kernelMessages.add(Common::Point(143, 61), 0xFDFC, 16, 0, INDEFINITE_TIMEOUT, _msgText);
	}

	_meteorologistCurPlace = 0;

	if (_globals[kMeteorologistWatch] != METEOROLOGIST_NORMAL)
		_scene->_sequences.addTimer(30, 230);

	_vm->_sound->command(36);
}

int teleporter_address(int code, bool working) {
	int limit = working ? 6 : 10;

	for (int i = 0; i < limit; i++) {
		if (code == _globals[kTeleporterCode + i])
			return _globals[kTeleporterRoom + i];
	}

	return -1;
}

Common::Point teleporter_compute_location() {
	Common::Point result;

	switch (_buttonTyped) {
	case 0:
		result = Common::Point(179, 200);
		break;

	case 1:
		result = Common::Point(166, 170);
		break;

	case 2:
		result = Common::Point(179, 170);
		break;

	case 3:
		result = Common::Point(192, 170);
		break;

	case 4:
		result = Common::Point(166, 180);
		break;

	case 5:
		result = Common::Point(179, 180);
		break;

	case 6:
		result = Common::Point(192, 180);
		break;

	case 7:
		result = Common::Point(166, 190);
		break;

	case 8:
		result = Common::Point(179, 190);
		break;

	case 9:
		result = Common::Point(192, 190);
		break;

	case 10:
		result = Common::Point(194, 200);
		break;

	case 11:
		result = Common::Point(164, 200);
		break;

	default:
		error("teleporterComputeLocation() - Unexpected button pressed");
	}

	return result;
}

void teleporter_handle_key() {
	switch (_game._trigger) {
	case 0:
	{
		_game._player._stepEnabled = false;
		Common::Point msgPos = teleporterComputeLocation();
		_handSequenceId = _scene->_sequences.startPingPongCycle(_handSpriteId, false, 4, 2, 0, 0);
		_scene->_sequences.setPosition(_handSequenceId, msgPos);
		_scene->_sequences.setDepth(_handSequenceId, 2);
		_scene->_sequences.addSubEntry(_handSequenceId, SEQUENCE_TRIGGER_LOOP, 0, 1);
		_scene->_sequences.addSubEntry(_handSequenceId, SEQUENCE_TRIGGER_EXPIRE, 0, 2);

		if (_globals[kMeteorologistWatch] == METEOROLOGIST_NORMAL)
			_vm->_events->hideCursor();

	}
	break;

	case 1:
		_scene->_sequences.addSubEntry(_handSequenceId, SEQUENCE_TRIGGER_SPRITE, 3, 3);
		if (_buttonTyped <= 9) {
			if (_digitCount < 4) {
				_curCode *= 10;
				_curCode += _buttonTyped;
				_digitCount++;

				Common::String format = "%01d";
				format.setChar('0' + _digitCount, 2);
				_msgText = Common::String::format(format.c_str(), _curCode);
				if (_digitCount < 4)
					_msgText += "_";

				if (_scene->_currentSceneId != 711)
					_vm->_sound->command(32);
			}
		} else if (_buttonTyped == 11) {
			_digitCount = 0;
			_curCode = 0;
			_msgText = "_";
			if (_scene->_currentSceneId != 711)
				_vm->_sound->command(33);
		} else if (_digitCount == 4) {
			if (_scene->_currentSceneId != 711)
				_finishedCodeCounter = 1;

			if (teleporterAddress(_curCode, true) > 0) {
				_vm->_palette->setEntry(252, 0, 63, 0);
				if (_scene->_currentSceneId != 711)
					_vm->_sound->command(34);
			} else {
				_vm->_palette->setEntry(252, 63, 0, 0);
				if (_scene->_currentSceneId != 711)
					_vm->_sound->command(35);
			}
		}

		if (_scene->_currentSceneId != 711) {
			if (_curMessageId >= 0)
				_scene->_kernelMessages.remove(_curMessageId);
			_curMessageId = _scene->_kernelMessages.add(Common::Point(143, 61), 0xFDFC, 16, 0, INDEFINITE_TIMEOUT, _msgText);
		}
		break;

	case 2:
		if (_finishedCodeCounter == 1) {
			_finishedCodeCounter++;

			if (_globals[kMeteorologistWatch] != METEOROLOGIST_NORMAL)
				_scene->_nextSceneId = 202;
			else {
				_vm->_events->showCursor();
				int destination = teleporterAddress(_curCode, true);

				if (destination > 0) {
					_globals[kTeleporterCommand] = 2;
					_scene->_nextSceneId = _teleporterSceneId;
					_globals[kTeleporterDestination] = destination;
				} else {
					_globals[kTeleporterCommand] = 4;
					_scene->_nextSceneId = _teleporterSceneId;
				}
			}
		} else if (_globals[kMeteorologistWatch] != METEOROLOGIST_NORMAL)
			_scene->_sequences.addTimer(30, 230 + _meteorologistCurPlace);

		break;

	case 3:
		if (!_finishedCodeCounter) {
			if (_globals[kMeteorologistWatch] == METEOROLOGIST_NORMAL) {
				_game._player._stepEnabled = true;
				_vm->_events->showCursor();
			}
		}
		break;

	default:
		break;
	}
}

bool teleeporter_parser() {
	bool retVal = false;

	if (_action.isAction(VERB_PRESS) || _action.isAction(VERB_PUSH)) {
		static int _buttonList[12] = { NOUN_0_KEY, NOUN_1_KEY, NOUN_2_KEY, NOUN_3_KEY, NOUN_4_KEY, NOUN_5_KEY, NOUN_6_KEY, NOUN_7_KEY, NOUN_8_KEY, NOUN_9_KEY, NOUN_SMILE_KEY, NOUN_FROWN_KEY };
		for (int i = 0; i < 12; i++) {
			if (_action._activeAction._objectNameId == _buttonList[i])
				_buttonTyped = i;
		}
		teleporterHandleKey();
		retVal = true;
	}

	if (_action.isAction(VERB_EXIT_FROM, NOUN_DEVICE)) {
		_globals[kTeleporterCommand] = 3;
		_scene->_nextSceneId = _teleporterSceneId;
		retVal = true;
	}

	return (retVal);
}

void teleporter_daemon() {
	if (_globals[kMeteorologistWatch] == METEOROLOGIST_NORMAL)
		return;

	if (_game._trigger >= 230) {
		int place = _game._trigger - 230;
		int digit;

		if (place < 4) {
			digit = _meteorologistNextPlace;
			for (int i = 0; i < (3 - place); i++)
				digit = digit / 10;

			digit = digit % 10;
		} else {
			digit = 10;
		}
		_buttonTyped = digit;
		_meteorologistCurPlace = place + 1;
		_game._trigger = -1;
	}

	if (_game._trigger) {
		if (_game._trigger == -1)
			_game._trigger = 0;
		teleporter_handle_key();
	}
}


} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
