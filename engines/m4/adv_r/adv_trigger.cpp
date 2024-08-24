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

#include "m4/adv_r/adv_trigger.h"
#include "m4/core/errors.h"
#include "m4/vars.h"
#include "m4/m4.h"

namespace M4 {

#define _GT(X) _G(triggers)._##X

int32 kernel_trigger_create(int32 trigger_num) {
	int32 new_trigger;

	if (trigger_num < 0)
		return (trigger_num);

	if (trigger_num > 0xffff) {		// If room changed, this is an invalid trigger 
		error_show(FL, 'BADT', "bad trigger. %d > 0xffff", trigger_num);
	}

	new_trigger = trigger_num + (_G(game).room_id << 16) + (_G(kernel).trigger_mode << 28);

	return new_trigger;
}

bool kernel_trigger_dispatch_now(int32 trigger_num) {
	if (g_engine->getGameType() == GType_Riddle)
		return kernel_trigger_dispatchx(trigger_num);
	else
		return kernel_trigger_dispatchx(kernel_trigger_create(trigger_num));
}

void cisco_dispatch_triggers() {
	for (int i = 0; i < _GT(q_end); ++i) {
		kernel_trigger_dispatchx(_GT(sound_trigger_q)[i]);
	}

	_GT(q_end) = 0;
}

void cisco_clear_triggers() {
	_GT(q_end) = _GT(q_start) = 0;
}

bool kernel_trigger_dispatchx(int32 trigger_num) {
	if (_G(between_rooms))
		return true;

	KernelTriggerType old_trigger_mode = _G(kernel).trigger_mode;
	int32 old_trigger = _G(kernel).trigger;
	bool result = false;

	if (trigger_num < 0)
		return false;

	if (((trigger_num >> 16) & 0xfff) != _G(game).room_id) {		// if room changed, this is an invalid trigger 
		term_message("orphan scene trigger:mode: %d, scene: %d, trigger: %d",
			trigger_num >> 28, (trigger_num >> 16) & 0xffff, trigger_num & 0xffff);
		return false;
	}

	_G(kernel).trigger = trigger_num & 0xffff;	// If no command in Q, must be here because of code

	switch (trigger_num >> 28) {
	case KT_PREPARSE:
		if (_G(kernel).trigger >= 32000)
			break;
		_G(kernel).trigger_mode = KT_PREPARSE;

		g_engine->room_pre_parser();
		result = true;
		break;

	case KT_PARSE:
		if (_G(kernel).trigger >= 32000)
			break;
		_G(kernel).trigger_mode = KT_PARSE;
		_G(player).command_ready = true;
		g_engine->room_parser();

		if (_G(player).command_ready) {
			g_engine->parser_code();
		}
		result = true;
		break;

	case KT_DAEMON:
		g_engine->game_daemon_code();
		result = true;
		break;

	default:
		term_message("orphan mode trigger: mode: %d, scene: %d, trigger: %d",
			trigger_num >> 28, (trigger_num >> 16) & 0xffff, trigger_num & 0xffff);
		result = false;
		break;
	}

	_G(kernel).trigger_mode = old_trigger_mode;
	_G(kernel).trigger = old_trigger;
	return result;
}

static void timer_callback(frac16 myMessage, struct machine *sender) {
	kernel_trigger_dispatchx(myMessage);
}

void kernel_timing_trigger(int32 ticks, int16 trigger, const char *name) {
	if (ticks <= 0) {
		// Trigger immediately
		kernel_trigger_dispatchx(kernel_trigger_create(trigger));
		return;
	}

	_G(globals)[GLB_TEMP_1] = ticks << 16;
	_G(globals)[GLB_TEMP_2] = kernel_trigger_create(trigger);

	if (name) {
		Common::String machName = Common::String::format("timer - %s", name);
		TriggerMachineByHash(2, nullptr, -1, -1, timer_callback, false, machName.c_str());

	} else {
		TriggerMachineByHash(2, nullptr, -1, -1, timer_callback, false, "timer trigger");
	}
}

void kernel_timing_trigger(int32 ticks, int16 trigger,
		KernelTriggerType preMode, KernelTriggerType postMode) {
	_G(kernel).trigger_mode = preMode;
	kernel_timing_trigger(ticks, trigger, nullptr);
	_G(kernel).trigger_mode = postMode;
}

void kernel_timing_trigger_daemon(int32 ticks, int16 trigger) {
	KernelTriggerType oldMode = _G(kernel).trigger_mode;
	_G(kernel).trigger_mode = KT_DAEMON;
	kernel_timing_trigger(ticks, trigger, nullptr);
	_G(kernel).trigger_mode = oldMode;
}

void kernel_service_timing_trigger_q() {
	// Dispatch pending timing triggers
	int32 iter = 0;
	int32 now = timer_read_60();

	while (iter < _GT(time_q_end) && _GT(time_q)[iter] <= now)
	{
		kernel_trigger_dispatchx(_GT(time_trigger_q)[iter]);
		++iter;
	}
	if (!iter)
		return;

	// Remove dispatched triggers from the q
	int32 total = iter;
	int32 dispatched = iter;
	iter = 0;
	while (dispatched < _GT(time_q_end)) {
		_GT(time_q)[iter] = _GT(time_q)[dispatched];
		_GT(time_trigger_q)[iter] = _GT(time_trigger_q)[dispatched];
		++iter;
		++dispatched;
	}

	_GT(time_q_end) -= total;
}

} // End of namespace M4
