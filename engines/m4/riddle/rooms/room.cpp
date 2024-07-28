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

#include "m4/riddle/rooms/room.h"
#include "m4/riddle/vars.h"
#include "m4/riddle/riddle.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room::restoreAutosave() {
	if (g_engine->autosaveExists()) {
		_G(kernel).restore_slot = 0;
		auto oldMode = _G(kernel).trigger_mode;
		_G(kernel).trigger_mode = KT_DAEMON;
		disable_player_commands_and_fade_init(255);

		_G(kernel).trigger_mode = oldMode;
		player_set_commands_allowed(false);
	}
}

void Room::triggerMachineByHashCallback(frac16 myMessage, machine *) {
	int32 hi = myMessage >> 16;

	if (hi >= 0)
		kernel_trigger_dispatch_now(hi);
}

void Room::triggerMachineByHashCallbackNegative(frac16 myMessage, machine *) {
	int32 hi = myMessage >> 16;

	if (hi < 0)
		kernel_trigger_dispatchx(hi);
}

void Room::triggerMachineByHashCallback3000(frac16 myMessage, machine *sender) {
	int triggerType = _G(globals)[GLB_TEMP_1] >> 16;
	int param = _G(globals)[GLB_TEMP_2] >> 16;
	int msg = myMessage >> 16;

	switch (triggerType) {
	case 0:
		break;

	case 1:
	case 3:
		if (msg >= 0)
			kernel_trigger_dispatchx(myMessage);
		break;

	case 2:
		if (param)
			sendWSMessage(0x30000, triggerType, sender, 0, nullptr, 1);
		else if(msg >= 0)
			kernel_trigger_dispatchx(myMessage);
		break;

	default:
		error("spawn walker callback with triggerType = %d", triggerType);
		break;
	}
}

int Room::checkFlags(bool flag) {
	int count = 0;

	if (_G(flags)[V364] == 1) {
		++count;
		switch (_G(flags)[V005]) {
		case 1:
			_G(flags)[V351] = 1;
			break;
		case 2:
			_G(flags)[V352] = 1;
			break;
		case 3:
			_G(flags)[V353] = 1;
			break;
		case 4:
			_G(flags)[V354] = 1;
			break;
		default:
			break;
		}
	}

	if (_G(flags)[V365] == 1) {
		_G(flags)[V355] = 1;
		++count;
	}
	if (_G(flags)[V366] == 1) {
		_G(flags)[V356] = 1;
		++count;
	}

	if (flag) {
		if (_G(flags)[V367] == 1) {
			_G(flags)[V357] = 1;
			++count;
		}
		if (_G(flags)[V368] == 1) {
			_G(flags)[V358] = 1;
			++count;
		}
		if (_G(flags)[V369] == 1) {
			_G(flags)[V359] = 1;
			++count;
		}
	}

	if (_G(game).room_id == 501 && _G(flags)[V370] == 1) {
		_G(flags)[V360] = 1;
		++count;
	}

	if (_G(game).room_id != 401 && _G(flags)[V373] == 1) {
		_G(flags)[V363] = 1;
		++count;
	}

	if (_G(game).room_id != 501 && _G(flags)[V371] == 1) {
		_G(flags)[V361] = 1;
		++count;
	}

	if (_G(flags)[V372] == 1) {
		_G(flags)[V362] = 1;
		++count;
	}

	return count;
}

void Room::setFlag45() {
	_G(flags)[V045] = 1;
}

void Room::setGlobals1(int val1, int val2, int val3, int val4, int val5,
		int val6, int val7, int val8, int val9, int val10,
		int val11, int val12, int val13, int val14, int val15,
		int val16, int val17, int val18, int val19, int val20,
		int val21) {
	_G(globals)[GLB_TEMP_1] = val1 << 24;
	_G(globals)[GLB_TEMP_2] = val2 << 16;
	_G(globals)[GLB_TEMP_3] = val3 << 16;
	_G(globals)[GLB_TEMP_4] = val4 << 16;
	_G(globals)[GLB_TEMP_5] = val5 << 16;
	_G(globals)[GLB_TEMP_6] = val6 << 16;
	_G(globals)[GLB_TEMP_7] = val7 << 16;
	_G(globals)[GLB_TEMP_8] = val8 << 16;
	_G(globals)[GLB_TEMP_9] = val9 << 16;
	_G(globals)[GLB_TEMP_10] = val10 << 16;
	_G(globals)[GLB_TEMP_11] = val11 << 16;
	_G(globals)[GLB_TEMP_12] = val12 << 16;
	_G(globals)[GLB_TEMP_13] = val13 << 16;
	_G(globals)[GLB_TEMP_14] = val14 << 16;
	_G(globals)[V021] = val15 << 16;
	_G(globals)[V022] = val16 << 16;
	_G(globals)[V024] = val17 << 16;
	_G(globals)[V025] = val18 << 16;
	_G(globals)[V026] = val19 << 16;
	_G(globals)[V027] = val20 << 16;
	_G(globals)[V028] = val21 << 16;
}

void Room::sendWSMessage_10000(int val1, machine *recv, int val2, int val3,
		int val4, int trigger, int val9, int val6, int val7, int val8) {
	if (!trigger)
		trigger = -1;

	_G(globals)[GLB_TEMP_1] = val1 << 16;
	_G(globals)[GLB_TEMP_2] = val2 << 16;
	_G(globals)[GLB_TEMP_3] = val3 << 16;
	_G(globals)[GLB_TEMP_4] = val4 << 16;
	_G(globals)[GLB_TEMP_5] = kernel_trigger_create(trigger);
	_G(globals)[GLB_TEMP_6] = val6 << 16;
	_G(globals)[GLB_TEMP_7] = val7 << 16;
	_G(globals)[GLB_TEMP_8] = val8 << 16;
	_G(globals)[GLB_TEMP_9] = val9 << 24;

	sendWSMessage(0x10000, 0, nullptr, 0, nullptr, 1);
}

machine *Room::triggerMachineByHash_3000(int val1, int val2, const int16 * normalDirs,
		const int16 * shadowDirs, int val3, int val4, int val5,
		MessageCB intrMsg, const char *machName) {
#if 0
	static const byte NUMS[14] = { 0, 0, 1, 2, 3, 4, 4, 5, 6, 7, 8, 9, 9, 0 };
	byte  nums[14];
	Common::copy(NUMS, NUMS + 14, nums);

	_G(globals)[GLB_TEMP_1] = val2 << 16;
	_G(globals)[GLB_TEMP_2] = val1 << 24;
	_G(globals)[GLB_TEMP_3] = val3
#else
	error("sendWSMessage_3000");
#endif
}

void Room::sendWSMessage_80000(machine *walker) {
	sendWSMessage(0x80000, 0, walker, 0, nullptr, 1);
}

void Room::sendWSMessage_C0000(machine *walker, int trigger) {
	if (!trigger)
		trigger = -1;

	_G(globals)[GLB_TEMP_5] = kernel_trigger_create(trigger);
	sendWSMessage(0xC0000, 0, walker, 0, nullptr, 1);
}

void Room::sendWSMessage_C0000(int trigger) {
	sendWSMessage_C0000(_G(my_walker), trigger);
}

void Room::sendWSMessage_110000(machine *walker, int trigger) {
	if (!trigger)
		trigger = -1;

	_G(globals)[V023] = kernel_trigger_create(trigger);
	sendWSMessage(0x110000, 0, walker, 0, nullptr, 1);
}

void Room::sendWSMessage_110000(int trigger) {
	sendWSMessage_110000(_G(my_walker), trigger);
}

void Room::sendWSMessage_140000(machine *walker, int trigger) {
	if (!trigger)
		trigger = -1;

	_G(globals)[V023] = kernel_trigger_create(trigger);
	sendWSMessage(0x140000, 0, walker, 0, nullptr, 1);
}

void Room::sendWSMessage_140000(int trigger) {
	sendWSMessage_140000(_G(my_walker), trigger);
}

void Room::sendWSMessage_150000(machine *walker, int trigger) {
	_G(globals)[V023] = kernel_trigger_create(trigger);
	sendWSMessage(0x150000, 0, walker, 0, nullptr, 1);
}

void Room::sendWSMessage_150000(int trigger) {
	sendWSMessage_150000(_G(my_walker), trigger);
}

void Room::sendWSMessage_190000(machine *recv, int trigger) {
	_G(globals)[V023] = trigger << 16;
	sendWSMessage(0x190000, 0, recv, 0, nullptr, 1);
}

void Room::sendWSMessage_1a0000(machine *recv, int trigger) {
	_G(globals)[V024] = trigger << 16;
	sendWSMessage(0x1a0000, 0, recv, 0, nullptr, 1);
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
