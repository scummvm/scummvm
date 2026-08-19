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

#include "mads/mads.h"
#include "mads/core/mouse.h"
#include "mads/core/pal.h"
#include "mads/nebular/rooms/teleporter.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/global.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

static int _buttonTyped;
static int _curCode;
static int _digitCount;
static int _curMessageId;
static int _handSequenceId;
static int _finishedCodeCounter;
static int _meteorologistNextPlace;
static int _meteorologistCurPlace;
static int _teleporterSceneId;
static char _msgText1[256];
static char _msgText2[256];

void teleporter_init() {
	_buttonTyped = -1;
	_curMessageId = -1;
	_handSequenceId = -1;
	_meteorologistNextPlace = -1;
	_meteorologistCurPlace = -1;
	_teleporterSceneId = -1;

	player.walker_visible = false;
	player.commands_allowed = (global[kMeteorologistWatch] == METEOROLOGIST_NORMAL);
	kernel_message_font = font_tele;
	kernel_message_spacing = 0;
	_curCode = 0;
	_digitCount = 0;
	_finishedCodeCounter = 0;
	Common::strcpy_s(_msgText2, "_");

	if (previous_room == KERNEL_RESTORING_GAME)
		previous_room = global[kTeleporterDestination];

	if (previous_room < 101)
		previous_room = 201;

	global[kTeleporterDestination] = previous_room;
	pal_change_color(252, 63, 63, 0);
	pal_change_color(253, 0, 0, 0);
	_teleporterSceneId = previous_room;
	if (_teleporterSceneId == 202)
		_teleporterSceneId = 201;

	int codeVal = 0;
	for (int i = 0; i < 10; i++) {
		if (_teleporterSceneId == global[kTeleporterRoom + i])
			codeVal = global[kTeleporterCode + i];

		if (global[kTeleporterRoom + i] == 301)
			_meteorologistNextPlace = global[kTeleporterCode + i];
	}

	Common::sprintf_s(_msgText1, "#%.4d", codeVal);

	if (room_id != 711) {
		kernel_message_add(_msgText1, 133, 34, 0, INDEFINITE_TIMEOUT, 0, 32);
		kernel_message_add(_msgText2, 143, 61, 0xFDFC, INDEFINITE_TIMEOUT, 0, 16);
	}

	_meteorologistCurPlace = 0;

	if (global[kMeteorologistWatch] != METEOROLOGIST_NORMAL)
		kernel_timing_trigger(30, 230);

	g_engine->_soundManager->command(36, 0);
}

int teleporter_address(int code, bool working) {
	int limit = working ? 6 : 10;

	for (int i = 0; i < limit; i++) {
		if (code == global[kTeleporterCode + i])
			return global[kTeleporterRoom + i];
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
	switch (kernel.trigger) {
	case 0: {
		player.commands_allowed = false;
		Common::Point msgPos = teleporter_compute_location();
		_handSequenceId = kernel_seq_pingpong(g_sprite_ids[4], false, 4, 0, 0, 2);
		kernel_seq_loc(_handSequenceId, msgPos.x, msgPos.y);
		kernel_seq_depth(_handSequenceId, 2);
		kernel_seq_trigger(_handSequenceId, KERNEL_TRIGGER_LOOP, 0, 1);
		kernel_seq_trigger(_handSequenceId, KERNEL_TRIGGER_EXPIRE, 0, 2);

		if (global[kMeteorologistWatch] == METEOROLOGIST_NORMAL)
			mouse_hide();
		break;
	}

	case 1:
		kernel_seq_trigger(_handSequenceId, KERNEL_TRIGGER_SPRITE, 3, 3);
		if (_buttonTyped <= 9) {
			if (_digitCount < 4) {
				_curCode *= 10;
				_curCode += _buttonTyped;
				_digitCount++;

				Common::String format = "%01d";
				format.setChar('0' + _digitCount, 2);
				Common::strcpy_s(_msgText2, Common::String::format(format.c_str(), _curCode).c_str());
				if (_digitCount < 4)
					Common::strcat_s(_msgText2, "_");

				if (room_id != 711)
					g_engine->_soundManager->command(32, 0);
			}
		} else if (_buttonTyped == 11) {
			_digitCount = 0;
			_curCode = 0;
			Common::strcpy_s(_msgText2, "_");
			if (room_id != 711)
				g_engine->_soundManager->command(33, 0);
		} else if (_digitCount == 4) {
			if (room_id != 711)
				_finishedCodeCounter = 1;

			if (teleporter_address(_curCode, true) > 0) {
				pal_change_color(252, 0, 63, 0);
				if (room_id != 711)
					g_engine->_soundManager->command(34, 0);
			} else {
				pal_change_color(252, 63, 0, 0);
				if (room_id != 711)
					g_engine->_soundManager->command(35, 0);
			}
		}

		if (room_id != 711) {
			if (_curMessageId >= 0)
				kernel_message_delete(_curMessageId);
			_curMessageId = kernel_message_add(_msgText2, 143, 61, 0xFDFC, INDEFINITE_TIMEOUT, 0, 16);
		}
		break;

	case 2:
		if (_finishedCodeCounter == 1) {
			_finishedCodeCounter++;

			if (global[kMeteorologistWatch] != METEOROLOGIST_NORMAL)
				new_room = 202;
			else {
				mouse_show();
				int destination = teleporter_address(_curCode, true);

				if (destination > 0) {
					global[kTeleporterCommand] = 2;
					new_room = _teleporterSceneId;
					global[kTeleporterDestination] = destination;
				} else {
					global[kTeleporterCommand] = 4;
					new_room = _teleporterSceneId;
				}
			}
		} else if (global[kMeteorologistWatch] != METEOROLOGIST_NORMAL)
			kernel_timing_trigger(30, 230 + _meteorologistCurPlace);

		break;

	case 3:
		if (!_finishedCodeCounter) {
			if (global[kMeteorologistWatch] == METEOROLOGIST_NORMAL) {
				player.commands_allowed = true;
				mouse_show();
			}
		}
		break;

	default:
		break;
	}
}

bool teleporter_parser() {
	bool retVal = false;

	if (player_said_1(press) || player_said_1(push)) {
		static int _buttonList[12] = { words_0_key, words_1_key, words_2_key, words_3_key, words_4_key, words_5_key, words_6_key, words_7_key, words_8_key, words_9_key, words_smile_key, words_frown_key };
		for (int i = 0; i < 12; i++) {
			if (player2.words[1] == _buttonList[i])
				_buttonTyped = i;
		}
		teleporter_handle_key();
		retVal = true;
	}

	if (player_said_2(exit_from, device)) {
		global[kTeleporterCommand] = 3;
		new_room = _teleporterSceneId;
		retVal = true;
	}

	return retVal;
}

void teleporter_daemon() {
	if (global[kMeteorologistWatch] == METEOROLOGIST_NORMAL)
		return;

	if (kernel.trigger >= 230) {
		int place = kernel.trigger - 230;
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
		kernel.trigger = -1;
	}

	if (kernel.trigger) {
		if (kernel.trigger == -1)
			kernel.trigger = 0;
		teleporter_handle_key();
	}
}


} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
