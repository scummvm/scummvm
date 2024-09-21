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

int Room::_ripSketching;

void Room::preload() {
	_G(player).walker_type = 1;
	_G(player).shadow_type = 1;
	LoadWSAssets("OTHER SCRIPT");
}

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

void Room::triggerMachineByHashCallbackAlways(frac16 myMessage, machine *sender) {
	kernel_trigger_dispatchx(myMessage);
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

void Room::setGlobals3(int series, int val1, int val2) {
	_G(globals)[GLB_TEMP_1] = series << 24;
	_G(globals)[GLB_TEMP_2] = val1 << 16;
	_G(globals)[GLB_TEMP_3] = val2 << 16;
}

void Room::setGlobals4(int val1, int val2, int val3, int val4) {
	_G(globals)[GLB_TEMP_1] = val1;
	_G(globals)[GLB_TEMP_2] = val2;
	_G(globals)[GLB_TEMP_3] = val3;
	_G(globals)[GLB_TEMP_4] = val4;
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

void Room::sendWSMessage_10000(machine *recv, int val1, int val2, int val3,
		int trigger, int val4) {
	int val = ((val2 << 16) - _G(globals)[V002]) *_G(globals)[V006] +
		_G(globals)[V004];

	_G(globals)[GLB_TEMP_1] = val1 << 16;
	_G(globals)[GLB_TEMP_2] = val2 << 16;
	_G(globals)[GLB_TEMP_3] = val;
	_G(globals)[GLB_TEMP_4] = (val3 > 0) ? val3 : 0xffff0000;
	_G(globals)[GLB_TEMP_5] = kernel_trigger_create(trigger);
	_G(globals)[GLB_TEMP_6] = val4 << 16;

	sendWSMessage(0x10000, 0, recv, 0, nullptr, 1);
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

void Room::sendWSMessage_60000(machine *mach) {
	sendWSMessage(0x60000, 0, mach, 0, nullptr, 1);
}

void Room::sendWSMessage_80000(machine *mach) {
	sendWSMessage(0x80000, 0, mach, 0, nullptr, 1);
}

void Room::sendWSMessage_B0000(machine *mach, int trigger) {

}

void Room::sendWSMessage_B0000(int trigger) {
	sendWSMessage_B0000(_G(my_walker), trigger);
}

void Room::sendWSMessage_C0000(machine *mach, int trigger) {
	if (!trigger)
		trigger = -1;

	_G(globals)[GLB_TEMP_5] = kernel_trigger_create(trigger);
	sendWSMessage(0xC0000, 0, mach, 0, nullptr, 1);
}

void Room::sendWSMessage_C0000(int trigger) {
	sendWSMessage_C0000(_G(my_walker), trigger);
}

void Room::sendWSMessage_D0000(machine *mach) {
	sendWSMessage(0xd0000, 0, mach, 0, nullptr, 1);
}

void Room::sendWSMessage_D0000() {
	sendWSMessage_D0000(_G(my_walker));
}

void Room::sendWSMessage_E0000(machine *mach) {
	sendWSMessage(0xe0000, 0, mach, 0, nullptr, 1);
}

void Room::sendWSMessage_E0000() {
	sendWSMessage_E0000(_G(my_walker));
}

void Room::sendWSMessage_F0000(machine *mach, int trigger) {
	if (!trigger)
		trigger = -1;

	_G(globals)[GLB_TEMP_4] = kernel_trigger_create(trigger);
	sendWSMessage(0xf0000, 0, mach, 0, nullptr, 1);
}

void Room::sendWSMessage_F0000(int trigger) {
	sendWSMessage_F0000(_G(my_walker), trigger);
}

void Room::sendWSMessage_110000(machine *mach, int trigger) {
	if (!trigger)
		trigger = -1;

	_G(globals)[V023] = kernel_trigger_create(trigger);
	sendWSMessage(0x110000, 0, mach, 0, nullptr, 1);
}

void Room::sendWSMessage_110000(int trigger) {
	sendWSMessage_110000(_G(my_walker), trigger);
}

void Room::sendWSMessage_120000(machine *mach, int trigger) {
	if (!trigger)
		trigger = -1;

	_G(globals)[V023] = kernel_trigger_create(trigger);
	sendWSMessage(0x120000, 0, mach, 0, nullptr, 1);
}

void Room::sendWSMessage_120000(int trigger) {
	sendWSMessage_120000(_G(my_walker), trigger);
}

void Room::sendWSMessage_130000(machine *recv, int val1) {
	if (val1 == 0)
		val1 = -1;

	_G(globals)[V023] = kernel_trigger_create(val1);
	sendWSMessage(0x130000, 0, recv, 0, 0, 1);
}

void Room::sendWSMessage_130000(int val1) {
	sendWSMessage_130000(_G(my_walker), val1);
}

void Room::sendWSMessage_140000(machine *mach, int trigger) {
	if (!trigger)
		trigger = -1;

	_G(globals)[V023] = kernel_trigger_create(trigger);
	sendWSMessage(0x140000, 0, mach, 0, nullptr, 1);
}

void Room::sendWSMessage_140000(int trigger) {
	sendWSMessage_140000(_G(my_walker), trigger);
}

void Room::sendWSMessage_150000(machine *mach, int trigger) {
	_G(globals)[V023] = kernel_trigger_create(trigger);
	sendWSMessage(0x150000, 0, mach, 0, nullptr, 1);
}

void Room::sendWSMessage_150000(int trigger) {
	sendWSMessage_150000(_G(my_walker), trigger);
}

void Room::sendWSMessage_160000(machine *mach, int val1, int trigger) {
	if (!trigger)
		trigger = -1;

	_G(globals)[V023] = kernel_trigger_create(trigger);
	_G(globals)[V024] = val1 << 16;
	sendWSMessage(0x160000, 0, mach, 0, nullptr, 1);
}

void Room::sendWSMessage_160000(int val1, int trigger) {
	sendWSMessage_160000(_G(my_walker), val1, trigger);
}

void Room::sendWSMessage_180000(machine *recv, int trigger) {
	if (!trigger)
		trigger = -1;
	_G(globals)[V023] = kernel_trigger_create(trigger);

	sendWSMessage(0x180000, 0, recv, 0, nullptr, 1);
}

void Room::sendWSMessage_180000(int trigger) {
	sendWSMessage_180000(_G(my_walker), trigger);
}

void Room::sendWSMessage_190000(machine *recv, int trigger) {
	_G(globals)[V023] = trigger << 16;
	sendWSMessage(0x190000, 0, recv, 0, nullptr, 1);
}

void Room::sendWSMessage_190000(int trigger) {
	sendWSMessage_190000(_G(my_walker), trigger);
}

void Room::sendWSMessage_1a0000(machine *recv, int trigger) {
	_G(globals)[V024] = trigger << 16;
	sendWSMessage(0x1a0000, 0, recv, 0, nullptr, 1);
}

void Room::sendWSMessage_1e0000(machine *recv, int val1, int val2) {
	_G(globals)[V023] = val1 << 16;
	_G(globals)[V024] = val2 << 16;
	sendWSMessage(0x1e0000, 0, recv, 0, nullptr, 1);
}

void Room::sendWSMessage_1e0000(int val1, int val2) {
	sendWSMessage_1e0000(_G(my_walker), val1, val2);
}

void Room::sendWSMessage_1f0000(machine *recv, int val1, int val2) {
	_G(globals)[V023] = val1 << 16;
	_G(globals)[V024] = val2 << 16;
	sendWSMessage(0x1f0000, 0, recv, 0, nullptr, 1);
}

void Room::sendWSMessage_1f0000(int val1, int val2) {
	sendWSMessage_1f0000(_G(my_walker), val1, val2);
}

void Room::sendWSMessage_200000(machine *recv, int trigger) {
	_G(globals)[V023] = (trigger << 16) / 100;
	sendWSMessage(0x200000, 0, recv, 0, nullptr, 1);
}

void Room::sendWSMessage_210000(machine *recv, int trigger) {
	_G(globals)[V023] = (trigger << 16) / 100;
	sendWSMessage(0x210000, 0, recv, 0, nullptr, 1);
}

void Room::sendWSMessage_29a0000(machine *recv, int val1) {
	_G(globals)[GLB_TEMP_1] = val1 << 24;
	sendWSMessage(0x29a0000, 0, recv, 0, 0, 1);
}

void Room::sendWSMessage_29a0000(int val1) {
	sendWSMessage_29a0000(_G(my_walker), val1);
}

void Room::sendWSMessage_multi(const char *name) {
	int vSI = 0, vCX = 0;

	switch (_G(kernel).trigger) {
	case -1:
	case 8:
		player_update_info();

		switch (_G(player_info).facing) {
		case 1:
		case 2:
			ws_walk(_G(player_info).x, _G(player_info).y, nullptr, 1, 2);
			break;

		case 3:
			ws_walk(_G(player_info).x, _G(player_info).y, nullptr, 1, 3);
			break;

		case 4:
		case 5:
			ws_walk(_G(player_info).x, _G(player_info).y, nullptr, 1, 4);
			break;

		case 7:
		case 8:
			ws_walk(_G(player_info).x, _G(player_info).y, nullptr, 1, 8);
			break;

		case 9:
			ws_walk(_G(player_info).x, _G(player_info).y, nullptr, 1, 9);
			break;

		case 10:
		case 11:
			ws_walk(_G(player_info).x, _G(player_info).y, nullptr, 1, 10);
			break;

		default:
			break;
		}

		player_set_commands_allowed(false);
		break;

	case 1:
		player_update_info();
		digi_preload("950_s34");

		switch (_G(player_info).facing) {
		case 2:
		case 10:
			vCX = 39;
			vSI = 17;
			_ripSketching = series_load("RIP SKETCHING IN NOTEBOOK POS 2");
			break;

		case 3:
		case 9:
			vCX = 36;
			vSI = 22;
			_ripSketching = series_load("RIP SKETCHING IN NOTEBOOK POS 3");
			break;

		case 4:
		case 8:
			vCX = 45;
			vSI = 19;
			_ripSketching = series_load("RIP SKETCHING IN NOTEBOOK POS 4");
			break;

		default:
			break;
		}

		setGlobals1(_ripSketching, 1, vSI, vSI, vSI, 0, vCX + 1, vCX, vCX, vCX);
		sendWSMessage_110000(3);
		digi_play(name, 1);
		break;

	case 3:
		if (_G(player_info).facing == 0 || _G(player_info).facing == 9)
			sendWSMessage_190000(9);

		sendWSMessage_120000(4);
		digi_play("950_s34", 2, 200, 7);
		break;

	case 4:
		sendWSMessage_110000(5);
		break;

	case 5:
		sendWSMessage_140000(6);
		break;

	case 6:
		series_unload(_ripSketching);
		digi_unload("950_s34");
		player_set_commands_allowed(true);
		return;

	case 7:
		if (_G(player_info).facing == 3 || _G(player_info).facing == 9)
			sendWSMessage_190000(5);
		break;

	default:
		break;
	}
}

int Room::getNumKeyItemsPlaced() const {
	static const char *const ITEMS[11] = {
		"SHRUNKEN HEAD", "INCENSE BURNER", "CRYSTAL SKULL",
		"WHALE BONE HORN", "WHEELED TOY", "SILVER BUTTERFLY",
		"REBUS AMULET", "CHISEL", "GERMAN BANKNOTE",
		"POSTAGE STAMP", "STICK AND SHELL MAP"
	};

	int total = 0;
	for (int i = 0; i < 11; ++i) {
		if (inv_object_in_scene(ITEMS[i], 305))
			++total;
	}

	return total;
}

bool Room::setItemsPlacedFlags() {
	int count = getNumKeyItemsPlaced();

	switch (_G(flags)[V005]) {
	case 0:
		if (_G(flags)[V006] >= 2 && getNumKeyItemsPlaced() == 0) {
			_G(flags)[V005] += 1;
			return true;
		}
		break;

	case 1:
	case 2:
	case 3:
		if ((_G(flags)[V006] >= 15 && count < 9) ||
				(_G(flags)[V006] >= 10 && count < 6) ||
				(_G(flags)[V006] >= 5 && count < 3)) {
			_G(flags)[V005] += 1;
			return true;
		}
		break;

	case 4:
	case 5:
		if ((_G(flags)[V006] >= 15 && count < 9) ||
				(_G(flags)[V006] >= 10 && count < 6) ||
				(_G(flags)[V006] >= 5 && count < 3)) {
			if (_G(flags)[V005] == 4)
				_G(flags)[V005]++;
			return true;
		}
		break;

	default:
		break;
	}

	_G(flags)[GLB_TEMP_1] = 0;
	return false;
}

const char *Room::getItemsPlacedDigi() const {
	static const char *NAMES[5] = {
		"301r18", "301r19", "301r20", "301r21", "301r38"
	};
	int val = _G(flags)[V005];
	return (val >= 1 && val <= 5) ? NAMES[val - 1] : nullptr;
}

void Room::disableHotspots() {
	for (auto *hs = _G(currentSceneDef).hotspots; hs; hs = hs->next)
		hs->active = false;
}

void Room::enableHotspots() {
	for (auto *hs = _G(currentSceneDef).hotspots; hs; hs = hs->next)
		hs->active = true;
}

bool Room::checkStrings() const {
	return _G(string1).empty() && _G(string2).empty() && _G(string3).empty();
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
