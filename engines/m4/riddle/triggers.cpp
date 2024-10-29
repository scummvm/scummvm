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

#include "m4/riddle/triggers.h"
#include "m4/riddle/vars.h"
#include "m4/adv_r/adv_control.h"
#include "m4/core/imath.h"
#include "m4/graphics/gr_series.h"
#include "m4/detection.h"

namespace M4 {
namespace Riddle {

void setGlobals1(int series, int val2, int val3, int val4, int val5,
	int val6, int val7, int val8, int val9, int val10,
	int val11, int val12, int val13, int val14, int val15,
	int val16, int val17, int val18, int val19, int val20,
	int val21) {
	_G(globals)[GLB_TEMP_1] = series << 24;
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

void setGlobals3(int series, int val1, int val2) {
	_G(globals)[GLB_TEMP_1] = series << 24;
	_G(globals)[GLB_TEMP_2] = val1 << 16;
	_G(globals)[GLB_TEMP_3] = val2 << 16;
}

void setGlobals4(int series, int val2, int val3, int val4) {
	_G(globals)[GLB_TEMP_1] = series << 24;
	_G(globals)[GLB_TEMP_2] = val2;
	_G(globals)[GLB_TEMP_3] = val3;
	_G(globals)[GLB_TEMP_4] = val4;
}

machine *triggerMachineByHash_3000(int myHash, int dataHash, int normalDir, int shadowDir,
	int param1, int param2, int index, MessageCB intrMsg, const char *machName) {
	static const byte NUMS[14] = { 0, 0, 1, 2, 3, 4, 4, 5, 6, 7, 8, 9, 9, 0 };

	_G(globals)[GLB_TEMP_1] = dataHash << 16;
	_G(globals)[GLB_TEMP_2] = normalDir << 24;
	_G(globals)[GLB_TEMP_3] = shadowDir << 24;
	_G(globals)[GLB_TEMP_4] = param1 << 16;
	_G(globals)[GLB_TEMP_5] = param2 << 16;
	_G(globals)[GLB_TEMP_6] = MulSF16((param2 << 16) - _G(globals)[V002],
		_G(globals)[V006]) + _G(globals)[V004];
	_G(globals)[GLB_TEMP_7] = NUMS[index] << 16;

	machine *result = TriggerMachineByHash(myHash, nullptr, dataHash + 8, 0,
		intrMsg ? intrMsg : &triggerMachineByHashCallback3000,
		false, machName);
	_G(inverse_pal)->release();

	return result;
}

void sendWSMessage_10000(int val1, machine *mach, int series1, int val3,
		int val4, int trigger, int series2, int val6, int val7, int val8) {
	if (!trigger)
		trigger = -1;

	_G(globals)[GLB_TEMP_1] = val1 << 16;
	_G(globals)[GLB_TEMP_2] = series1 << 24;
	_G(globals)[GLB_TEMP_3] = val3 << 16;
	_G(globals)[GLB_TEMP_4] = val4 << 16;
	_G(globals)[GLB_TEMP_5] = kernel_trigger_create(trigger);
	_G(globals)[GLB_TEMP_6] = val6 << 16;
	_G(globals)[GLB_TEMP_7] = val7 << 16;
	_G(globals)[GLB_TEMP_8] = val8 << 16;
	_G(globals)[GLB_TEMP_9] = series2 << 24;

	sendWSMessage(STARTWALK << 16, 0, mach, 0, nullptr, 1);
}

void sendWSMessage_10000(machine *mach, int destX, int destY, int facing, int trigger, int complete_walk) {
	static const byte FACINGS[14] = { 0, 0, 1, 2, 3, 4, 4, 5, 6, 7, 8, 9, 9, 0 };

	_G(globals)[GLB_TEMP_1] = destX << 16;
	_G(globals)[GLB_TEMP_2] = destY << 16;
	_G(globals)[GLB_TEMP_3] = MulSF16((destY << 16) - _G(globals)[V002], _G(globals)[V006])
		+ _G(globals)[V004];
	_G(globals)[GLB_TEMP_4] = (facing > 0 ? FACINGS[facing] : -1) << 16;
	_G(globals)[GLB_TEMP_5] = kernel_trigger_create(trigger);
	_G(globals)[GLB_TEMP_6] = complete_walk << 16;

	debugC(1, kDebugMessages, "STARTWALK dest=(%d,%d), facing=%d, trigger=%d, complete_walk=%d",
		destX, destY, facing, trigger, complete_walk);
	sendWSMessage(STARTWALK << 16, 0, mach, 0, nullptr, 1);
}

void sendWSMessage_60000(machine *mach) {
	sendWSMessage(TERMINATE << 16, 0, mach, 0, nullptr, 1);
}

void sendWSMessage_80000(machine *mach) {
	sendWSMessage(PLAYER_UNHIDE << 16, 0, mach, 0, nullptr, 1);
}

void sendWSMessage_B0000(machine *mach, int trigger) {
	if (!trigger)
		trigger = -1;

	_G(globals)[GLB_TEMP_5] = kernel_trigger_create(trigger);
	sendWSMessage(ACTION_11 << 16, 0, mach, 0, nullptr, 1);
}

void sendWSMessage_B0000(int trigger) {
	sendWSMessage_B0000(_G(my_walker), trigger);
}

void sendWSMessage_C0000(machine *mach, int trigger) {
	if (!trigger)
		trigger = -1;

	_G(globals)[GLB_TEMP_5] = kernel_trigger_create(trigger);
	sendWSMessage(ACTION_12 << 16, 0, mach, 0, nullptr, 1);
}

void sendWSMessage_C0000(int trigger) {
	sendWSMessage_C0000(_G(my_walker), trigger);
}

void sendWSMessage_D0000(machine *mach) {
	sendWSMessage(ACTION_13 << 16, 0, mach, 0, nullptr, 1);
}

void sendWSMessage_D0000() {
	sendWSMessage_D0000(_G(my_walker));
}

void sendWSMessage_E0000(machine *mach) {
	sendWSMessage(ACTION_14 << 16, 0, mach, 0, nullptr, 1);
}

void sendWSMessage_E0000() {
	sendWSMessage_E0000(_G(my_walker));
}

void sendWSMessage_F0000(machine *mach, int trigger) {
	if (!trigger)
		trigger = -1;

	_G(globals)[GLB_TEMP_4] = kernel_trigger_create(trigger);
	sendWSMessage(ACTION_15 << 16, 0, mach, 0, nullptr, 1);
}

void sendWSMessage_F0000(int trigger) {
	sendWSMessage_F0000(_G(my_walker), trigger);
}

void sendWSMessage_110000(machine *mach, int trigger) {
	if (!trigger)
		trigger = -1;

	_G(globals)[V023] = kernel_trigger_create(trigger);
	sendWSMessage(ACTION_17 << 16, 0, mach, 0, nullptr, 1);
}

void sendWSMessage_110000(int trigger) {
	sendWSMessage_110000(_G(my_walker), trigger);
}

void sendWSMessage_120000(machine *mach, int trigger) {
	if (!trigger)
		trigger = -1;

	_G(globals)[V023] = kernel_trigger_create(trigger);
	sendWSMessage(ACTION_18 << 16, 0, mach, 0, nullptr, 1);
}

void sendWSMessage_120000(int trigger) {
	sendWSMessage_120000(_G(my_walker), trigger);
}

void sendWSMessage_130000(machine *recv, int val1) {
	if (val1 == 0)
		val1 = -1;

	_G(globals)[V023] = kernel_trigger_create(val1);
	sendWSMessage(ACTION_19 << 16, 0, recv, 0, 0, 1);
}

void sendWSMessage_130000(int val1) {
	sendWSMessage_130000(_G(my_walker), val1);
}

void sendWSMessage_140000(machine *mach, int trigger) {
	if (!trigger)
		trigger = -1;

	_G(globals)[V023] = kernel_trigger_create(trigger);
	sendWSMessage(ACTION_20 << 16, 0, mach, 0, nullptr, 1);
}

void sendWSMessage_140000(int trigger) {
	sendWSMessage_140000(_G(my_walker), trigger);
}

void sendWSMessage_150000(machine *mach, int trigger) {
	_G(globals)[V023] = kernel_trigger_create(trigger);
	sendWSMessage(ACTION_21 << 16, 0, mach, 0, nullptr, 1);
}

void sendWSMessage_150000(int trigger) {
	sendWSMessage_150000(_G(my_walker), trigger);
}

void sendWSMessage_160000(machine *mach, int val1, int trigger) {
	if (!trigger)
		trigger = -1;

	_G(globals)[V023] = kernel_trigger_create(trigger);
	_G(globals)[V024] = val1 << 16;
	sendWSMessage(ACTION_22 << 16, 0, mach, 0, nullptr, 1);
}

void sendWSMessage_160000(int val1, int trigger) {
	sendWSMessage_160000(_G(my_walker), val1, trigger);
}

void sendWSMessage_180000(machine *recv, int trigger) {
	if (!trigger)
		trigger = -1;
	_G(globals)[V023] = kernel_trigger_create(trigger);

	sendWSMessage(ACTION_24 << 16, 0, recv, 0, nullptr, 1);
}

void sendWSMessage_180000(int trigger) {
	sendWSMessage_180000(_G(my_walker), trigger);
}

void sendWSMessage_190000(machine *recv, int trigger) {
	_G(globals)[V023] = trigger << 16;
	sendWSMessage(ACTION_25 << 16, 0, recv, 0, nullptr, 1);
}

void sendWSMessage_190000(int trigger) {
	sendWSMessage_190000(_G(my_walker), trigger);
}

void sendWSMessage_1a0000(machine *recv, int trigger) {
	_G(globals)[V024] = trigger << 16;
	sendWSMessage(ACTION_26 << 16, 0, recv, 0, nullptr, 1);
}

void sendWSMessage_1e0000(machine *recv, int val1, int val2) {
	_G(globals)[V023] = val1 << 16;
	_G(globals)[V024] = val2 << 16;
	sendWSMessage(ACTION_30 << 16, 0, recv, 0, nullptr, 1);
}

void sendWSMessage_1e0000(int val1, int val2) {
	sendWSMessage_1e0000(_G(my_walker), val1, val2);
}

void sendWSMessage_1f0000(machine *recv, int val1, int val2) {
	_G(globals)[V023] = val1 << 16;
	_G(globals)[V024] = val2 << 16;
	sendWSMessage(ACTION_31 << 16, 0, recv, 0, nullptr, 1);
}

void sendWSMessage_1f0000(int val1, int val2) {
	sendWSMessage_1f0000(_G(my_walker), val1, val2);
}

void sendWSMessage_200000(machine *recv, int trigger) {
	_G(globals)[V023] = (trigger << 16) / 100;
	sendWSMessage(ACTION_32 << 16, 0, recv, 0, nullptr, 1);
}

void sendWSMessage_210000(machine *recv, int trigger) {
	_G(globals)[V023] = (trigger << 16) / 100;
	sendWSMessage(ACTION_33 << 16, 0, recv, 0, nullptr, 1);
}

void sendWSMessage_29a0000(machine *recv, int val1) {
	_G(globals)[GLB_TEMP_1] = val1 << 24;
	sendWSMessage(ACTION_666 << 16, 0, recv, 0, 0, 1);
}

void sendWSMessage_29a0000(int val1) {
	sendWSMessage_29a0000(_G(my_walker), val1);
}

void sendWSMessage_multi(const char *name) {
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
			_G(ripSketching) = series_load("RIP SKETCHING IN NOTEBOOK POS 2");
			break;

		case 3:
		case 9:
			vCX = 36;
			vSI = 22;
			_G(ripSketching) = series_load("RIP SKETCHING IN NOTEBOOK POS 3");
			break;

		case 4:
		case 8:
			vCX = 45;
			vSI = 19;
			_G(ripSketching) = series_load("RIP SKETCHING IN NOTEBOOK POS 4");
			break;

		default:
			break;
		}

		setGlobals1(_G(ripSketching), 1, vSI, vSI, vSI, 0, vCX + 1, vCX, vCX, vCX);
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
		series_unload(_G(ripSketching));
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

void intrMsgNull(frac16 myMessage, machine *sender) {
}

void triggerMachineByHashCallback(frac16 myMessage, machine *) {
	if ((myMessage >> 16) >= 0)
		kernel_trigger_dispatch_now(myMessage);
}

void triggerMachineByHashCallbackAlways(frac16 myMessage, machine *sender) {
	kernel_trigger_dispatchx(myMessage);
}

void triggerMachineByHashCallback3000(frac16 myMessage, machine *sender) {
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
			sendWSMessage(0x30000, 0, sender, 0, nullptr, 1);
		else if (msg >= 0)
			kernel_trigger_dispatchx(myMessage);
		break;

	default:
		error("spawn walker callback with triggerType = %d", triggerType);
		break;
	}
}

} // namespace Riddle
} // namespace M4

