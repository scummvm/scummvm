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

#include "mads/core/game.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section8.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _messWithThrottle;
	bool _movingThrottle;
	bool _throttleGone;
	bool _dontPullThrottleAgain;
	bool _pullThrottleReally;
	bool _alreadyOrgan;
	bool _alreadyPop;
	int16 _throttleCounter;
	int16 _resetFrame;
};

static Scratch local;


static void room_804_init() {
	local._messWithThrottle = false;
	local._throttleCounter = 0;
	local._movingThrottle = false;
	local._throttleGone = false;
	local._dontPullThrottleAgain = false;
	local._resetFrame = -1;
	local._pullThrottleReally = false;
	local._alreadyOrgan = false;
	local._alreadyPop = false;


	if (global[kCopyProtectFailed]) {
		// Copy protection failed
		global[kInSpace] = true;
		global[kWindowFixed] = 0;
	}

	g_sprite_ids[4] = kernel_load_series(kernel_name('x', 0), 0);
	g_sprite_ids[5] = kernel_load_series(kernel_name('x', 1), 0);
	g_sprite_ids[6] = kernel_load_series(kernel_name('x', 2), 0);
	g_sprite_ids[1] = kernel_load_series(kernel_name('x', 3), 0);
	g_sprite_ids[7] = kernel_load_series(kernel_name('x', 4), 0);
	g_sprite_ids[8] = kernel_load_series(kernel_name('f', 1), 0);

	kernel.quotes = quote_load(791, 0);

	if (global[kInSpace]) {
		if (global[kWindowFixed]) {
			g_sequence_ids[5] = _scene->_sequences.startCycle(g_sprite_ids[5], 0, 1);
			_scene->_sequences.addTimer(60, 100);
		} else {
			g_sequence_ids[6] = _scene->_sequences.startCycle(g_sprite_ids[6], false, 1);
			g_sequence_ids[7] = _scene->_sequences.startPingPongCycle(g_sprite_ids[7], false, 4, 0, 0, 0);
			_scene->_sequences.addTimer(160, 70);
			player.commands_allowed = false;
		}
	} else {
		if (global[kBeamIsUp]) {
			g_sequence_ids[8] = _scene->_sequences.startCycle(g_sprite_ids[8], false, 1);
			_scene->_sequences.setDepth(g_sequence_ids[8], 7);
		}

		if (global[kWindowFixed])
			g_sequence_ids[4] = _scene->_sequences.startCycle(g_sprite_ids[4], false, 1);

		g_sequence_ids[1] = _scene->_sequences.startCycle(g_sprite_ids[1], false, 1);
		_scene->_sequences.setPosition(g_sequence_ids[1], Common::Point(133, 139));
		_scene->_sequences.setDepth(g_sequence_ids[1], 8);
	}

	kernel_run_animation(kernel_full_name(804, 'r', 1, "", EXT_AA), 0);

	section_8_music();

	if (global[kInSpace] && !global[kWindowFixed]) {
		_scene->_userInterface.setup(kInputLimitedSentences);
		g_engine->_soundManager->command(19, 0);
	}
}

static void room_804_daemon() {
	if (!local._messWithThrottle) {

		if ((local._throttleGone) && (local._movingThrottle) && (_scene->_animation[0]->getCurrentFrame() == 39)) {
			g_sequence_ids[1] = _scene->_sequences.startCycle
			(g_sprite_ids[1], false, 1);
			_scene->_sequences.setPosition(g_sequence_ids[1], Common::Point(133, 139));
			_scene->_sequences.setDepth(g_sequence_ids[1], 8);
			local._throttleGone = false;
		}

		if ((local._movingThrottle) && (_scene->_animation[0]->getCurrentFrame() == 42)) {
			local._resetFrame = 0;
			local._movingThrottle = false;
		}

		if (kernel.trigger == 70) {
			local._resetFrame = 42;
		}

		if (_scene->_animation[0]->getCurrentFrame() == 65)
			_scene->_sequences.remove(g_sequence_ids[7]);

		switch (config_file.naughtiness) {
		case STORYMODE_NAUGHTY:
		default:
			if (_scene->_animation[0]->getCurrentFrame() == 81) {
				local._resetFrame = 80;
				global[kInSpace] = false;
				global[kBeamIsUp] = true;

				//assert(!global[kCopyProtectFailed]);
				win_status = 4;
				return;
			}
			break;

		case STORYMODE_NICE:
			if (_scene->_animation[0]->getCurrentFrame() == 68) {
				local._resetFrame = 66;
				global[kInSpace] = false;
				global[kBeamIsUp] = true;

				assert(!global[kCopyProtectFailed]);
				win_status = 4;
				return;
			}
		}

		if (_scene->_animation[0]->getCurrentFrame() == 34) {
			local._resetFrame = 36;
			_scene->_sequences.remove(g_sequence_ids[1]);
		}

		if (_scene->_animation[0]->getCurrentFrame() == 37) {
			local._resetFrame = 36;
			if (!local._dontPullThrottleAgain) {
				local._dontPullThrottleAgain = true;
				_scene->_sequences.addTimer(60, 80);
			}
		}

		if (kernel.trigger == 80) {
			new_room = 803;
		}

		if ((_scene->_animation[0]->getCurrentFrame() == 7) && (!global[kWindowFixed])) {
			g_sequence_ids[4] = _scene->_sequences.startCycle(g_sprite_ids[4], false, 1);
			_scene->_sequences.addTimer(20, 110);
			global[kWindowFixed] = true;
		}

		if (_scene->_animation[0]->getCurrentFrame() == 10) {
			local._resetFrame = 0;
			player.commands_allowed = true;
			inter_move_object(OBJ_POLYCEMENT, NOWHERE);
		}

		// FIXME: Original doesn't have resetFrame check. Check why this has been needed
		if (local._resetFrame == -1 && _scene->_animation[0]->getCurrentFrame() == 1) {
			int randomVal = g_engine->getRandomNumber(29) + 1;
			switch (randomVal) {
			case 1:
				local._resetFrame = 25;
				break;
			case 2:
				local._resetFrame = 27;
				break;
			case 3:
				local._resetFrame = 29;
				break;
			default:
				local._resetFrame = 0;
				break;
			}
		}

		switch (_scene->_animation[0]->getCurrentFrame()) {
		case 26:
		case 28:
		case 31:
			local._resetFrame = 0;
			break;
		default:
			break;
		}
	} else {
		if ((_scene->_animation[0]->getCurrentFrame() == 36) && (!local._throttleGone)) {
			_scene->_sequences.remove(g_sequence_ids[1]);
			local._throttleGone = true;
		}

		if (_scene->_animation[0]->getCurrentFrame() == 39) {
			local._movingThrottle = false;
			switch (local._throttleCounter) {
			case 1:
				break;
			case 3:
				_scene->_sequences.addTimer(130, 120);
				break;
			default:
				break;
			}
		}

		if (!local._movingThrottle) {
			++local._throttleCounter;
			local._movingThrottle = true;
			if (local._throttleCounter < 4) {
				local._resetFrame = 34;
			} else {
				local._messWithThrottle = false;
				local._throttleCounter = 0;
				player.commands_allowed = true;
			}
		}
	}

	if (kernel.trigger == 120) {
		text_show(80422);
	}

	if (kernel.trigger == 110) {
		text_show(80426);
	}

	if (local._pullThrottleReally) {
		local._resetFrame = 32;
		local._pullThrottleReally = false;
	}

	if (local._resetFrame >= 0) {
		if (local._resetFrame != _scene->_animation[0]->getCurrentFrame()) {
			_scene->_animation[0]->setCurrentFrame(local._resetFrame);
			local._resetFrame = -1;
		}
	}

	if (kernel.trigger == 90) {
		new_room = 803;
	}

	if ((_scene->_animation[0]->getCurrentFrame() == 72) && !local._alreadyPop) {
		g_engine->_soundManager->command(21, 0);
		local._alreadyPop = true;
	}

	if ((_scene->_animation[0]->getCurrentFrame() == 80) && !local._alreadyOrgan) {
		g_engine->_soundManager->command(22, 0);
		local._alreadyOrgan = true;
	}
}

static void room_804_parser() {
	if (player_said_2(look, service_panel) ||
		player_said_2(open, service_panel)) {
		new_room = 805;
	} else if ((player_said_2(activate, remote)) && global[kTopButtonPushed]) {
		if (!global[kInSpace]) {
			// Top button pressed on panel in hanger control
			if (!global[kBeamIsUp]) {
				global[kFromCockpit] = true;
				global[kUpBecauseOfRemote] = true;
				new_room = 803;
			} else {
				// Player turning off remote
				global[kBeamIsUp] = false;
				global[kUpBecauseOfRemote] = false;
				_scene->_sequences.remove(g_sequence_ids[8]);
				g_engine->_soundManager->command(15, 0);
			}
		}
	} else if (player_said_2(pull, throttle)) {
		player.commands_allowed = false;
		if (global[kBeamIsUp]) {
			if (!player_has(OBJ_VASE) && global[kWindowFixed]) {
				text_show(80423);
				player.commands_allowed = true;
			} else {
				player.command_ready = false;

				text_show(80424);
				local._pullThrottleReally = true;
				_scene->_kernelMessages.add(Common::Point(78, 75), 0x1110, 0, 0,
					120, quote_string(kernel.quotes, 791));
			}
		} else {
			local._messWithThrottle = true;
		}
	} else if (player_said_3(apply, polycement, crack) ||
		player_said_3(put, polycement, crack)) {
		if (!global[kWindowFixed]) {
			local._resetFrame = 2;
			player.commands_allowed = false;
		}
	} else if (player_said_2(exit, ship)) {
		global[kExitShip] = true;
		global[kFromCockpit] = true;
		if (global[kBeamIsUp]) {
			text_show(80425);
			_scene->_sequences.remove(g_sequence_ids[8]);
			g_engine->_soundManager->command(15, 0);
			global[kBeamIsUp] = false;
		}
		kernel.trigger_setup_mode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.addTimer(2, 90);
	} else  if (player.look_around) {
		text_show(80410);
	} else if ((player_said_2(look, window)) ||
		(player_said_2(look_out, window))) {
		if (global[kBeamIsUp]) {
			text_show(80412);
		} else {
			text_show(80411);
		}
	} else if (player_said_2(look, crack)) {
		if (global[kWindowFixed]) {
			text_show(80414);
		} else {
			text_show(80413);
		}
	} else if (player_said_2(look, controls)) {
		text_show(80415);
	} else if (player_said_2(look, status_panel)) {
		if (global[kBeamIsUp]) {
			text_show(80417);
		} else {
			text_show(80416);
		}
	} else if (player_said_2(look, tp)) {
		text_show(80418);
	} else if (player_said_2(take, tp)) {
		text_show(80419);
	} else if (player_said_2(look, instrumentation)) {
		text_show(80420);
	} else  if (player_said_2(look, seat)) {
		text_show(80421);
	} else
		return;

	player.command_ready = false;
}

void room_804_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._messWithThrottle);
	s.syncAsByte(local._movingThrottle);
	s.syncAsByte(local._throttleGone);
	s.syncAsByte(local._dontPullThrottleAgain);
	s.syncAsByte(local._pullThrottleReally);
	s.syncAsByte(local._alreadyOrgan);
	s.syncAsByte(local._alreadyPop);

	s.syncAsSint16LE(local._resetFrame);
	s.syncAsUint32LE(local._throttleCounter);
}

void room_804_preload() {
	room_init_code_pointer = room_804_init;
	room_daemon_code_pointer = room_804_daemon;
	room_parser_code_pointer = room_804_parser;

	section_8_walker();
	section_8_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
