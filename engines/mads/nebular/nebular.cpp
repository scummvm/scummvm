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

#include "engines/util.h"
#include "mads/core/mps_installer.h"
#include "mads/core/attr.h"
#include "mads/core/config.h"
#include "mads/core/conv.h"
#include "mads/core/env.h"
#include "mads/core/game.h"
#include "mads/core/imath.h"
#include "mads/core/inter.h"
#include "mads/core/kernel.h"
#include "mads/core/matte.h"
#include "mads/core/object.h"
#include "mads/core/pal.h"
#include "mads/core/rail.h"
#include "mads/core/screen.h"
#include "mads/core/sound.h"
#include "mads/core/text.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/console.h"
#include "mads/nebular/copy.h"
#include "mads/nebular/global.h"
#include "mads/nebular/main.h"
#include "mads/nebular/popup.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/sound_nebular.h"
#include "mads/nebular/rooms/section1.h"
#include "mads/nebular/rooms/section2.h"
#include "mads/nebular/rooms/section3.h"
#include "mads/nebular/rooms/section4.h"
#include "mads/nebular/rooms/section5.h"
#include "mads/nebular/rooms/section6.h"
#include "mads/nebular/rooms/section7.h"
#include "mads/nebular/rooms/section8.h"

namespace MADS {
namespace RexNebular {

RexNebularEngine::RexNebularEngine(OSystem *syst, const MADSGameDescription *gameDesc) :
		MADSEngine(syst, gameDesc) {
	// Initialize globals
	RexNebular::popup_init();
}

Common::Error RexNebularEngine::run() {
	initGraphics(320, 200);
	_screen = new Graphics::Screen();
	scr_live.data = (byte *)_screen->getPixels();

	// Create a debugger console
	setDebugger(new Console());

	// Set up to read mpslabs installer archive if needed
	if (_gameDescription->desc.flags & GF_INSTALLER) {
		Common::Archive *arch = MpsInstaller::open("MPSLABS");
		if (arch)
			SearchMan.add("mpslabs", arch);
	}

	// Set up sound manager
	_soundManager = new RexSoundManager(_mixer, _soundFlag);
	_soundManager->validate();

	// Run the game
	RexNebular::nebular_main();

	return Common::kNoError;
}

int RexNebularEngine::main_copy_verify() {
	return global_copy_verify();
}

void RexNebularEngine::global_init_code() {
	RexNebular::global_init_code();
}

void RexNebularEngine::section_music(int section_num) {
	switch (section_num) {
	case 1: Rooms::section_1_music(); break;
	case 2: Rooms::section_2_music(); break;
	case 3: Rooms::section_3_music(); break;
	case 4: Rooms::section_4_music(); break;
	case 5: Rooms::section_5_music(); break;
	case 6: Rooms::section_6_music(); break;
	case 7: Rooms::section_7_music(); break;
	case 8: Rooms::section_8_music(); break;
	}
}

void RexNebularEngine::global_section_constructor() {
	RexNebular::global_section_constructor();
}

void RexNebularEngine::syncRoom(Common::Serializer &s) {
	RexNebular::sync_room(s);
}

void RexNebularEngine::global_daemon_code() {
	if (player.walker_visible && player.commands_allowed && !player.walking &&
		(player.facing == player.turn_to_facing)) {


		if (kernel.clock >= READGLOBAL32(kWalkerTiming)) {
			if (player.stop_walker_pointer == 0) {
				int randomVal = getRandomNumber(29999);
				if (global[kSexOfRex] == REX_MALE) {
					switch (player.facing) {
					case FACING_SOUTHWEST:
					case FACING_SOUTHEAST:
					case FACING_NORTHWEST:
					case FACING_NORTHEAST:
						if (randomVal < 200) {
							player_add_stop_walker(-1, 0);
							player_add_stop_walker(1, 0);
						}
						break;

					case FACING_WEST:
					case FACING_EAST:
						if (randomVal < 500) {
							for (int count = 0; count < 10; ++count) {
								player_add_stop_walker(1, 0);
							}
						}
						break;

					case FACING_SOUTH:
						if (randomVal < 500) {
							for (int count = 0; count < 10; ++count) {
								player_add_stop_walker((randomVal < 250) ? 1 : 2, 0);
							}
						} else if (randomVal < 750) {
							for (int count = 0; count < 5; ++count) {
								player_add_stop_walker(1, 0);
							}

							player_add_stop_walker(0, 0);
							player_add_stop_walker(0, 0);

							for (int count = 0; count < 5; ++count) {
								player_add_stop_walker(2, 0);
							}
						}
						break;

					default:
						break;
					}
				}
			}

			WRITEGLOBAL32(kWalkerTiming, READGLOBAL32(kWalkerTiming) + 6);
		}
	}

	// Below is countdown to set the timebomb off in room 604
	if (global[kTimebombStatus] == TIMEBOMB_ACTIVATED) {
		int diff = kernel.clock - READGLOBAL32(kTimebombClock);
		if ((diff >= 0) && (diff <= 60)) {
			WRITEGLOBAL32(kTimebombTimer, READGLOBAL32(kTimebombTimer) + diff);
		} else {
			WRITEGLOBAL32(kTimebombTimer, READGLOBAL32(kTimebombTimer) + 1);
		}

		WRITEGLOBAL32(kTimebombClock, kernel.clock);
	}
}

void RexNebularEngine::global_pre_parser_code() {
	if (player_said_1(look) || player_said_1(throw))
		player.need_to_walk = false;
}

void RexNebularEngine::global_verb_filter() {
	// Switches the 'look at' used with the binoculars to a standard look verb,
	// as expected by the room scripts in at least room 202
	if (player_verb == words_look_at)
		player_verb = words_look;
}

void RexNebularEngine::showRecipe() {
	int count;

	for (count = 0; count < 4; count++) {
		switch (global[kIngredientQuantity + count]) {
		case 0:
			text_index[count] = words_drop;
			break;
		case 1:
			text_index[count] = words_dollop;
			break;
		case 2:
			text_index[count] = words_dash;
			break;
		case 3:
			text_index[count] = words_splash;
			break;
		default:
			break;
		}
	}

	for (count = 0; count < 4; count++) {
		switch (global[kIngredientList + count]) {
		case 0:
			text_index[count + 4] = words_alcohol;
			break;
		case 1:
			text_index[count + 4] = words_lecithin;
			break;
		case 2:
			text_index[count + 4] = words_petrox;
			break;
		case 3:
			text_index[count + 4] = words_formaldehyde;
			break;
		default:
			break;
		}
	}

	text_show(401);
}

void RexNebularEngine::global_parser_code() {
	int id;

	if (player_said_1(smell) && room_id > 103 && room_id < 111) {
		text_show(440);
	} else if (player_said_1(eat) && room_id > 103 && room_id < 111) {
		text_show(441);
	} else if (player_said_2(smell, burger)) {
		text_show(442);
	} else if (player_said_2(eat, burger)) {
		text_show(443);
	} else if (player_said_2(smell, stuffed_fish)) {
		text_show(444);
	} else if (player_said_2(eat, stuffed_fish)) {
		text_show(445);
	} else if (player_said_2(wear, rebreather)) {
		text_show(room_id > 103 && room_id < 111 ? 446 : 447);
	} else if (player_said_2(set, timer_module)) {
		text_show(448);
	} else if (player_said_2(nibble_on, big_leaves)) {
		text_show(449);
	} else if (player_said_2(lick, poison_darts)) {
		text_show(450);
	} else if (player_said_2(eat, twinkifruit)) {
		inter_move_object(OBJ_TWINKIFRUIT, NOWHERE);
		text_show(451);
	} else if (player_said_2(gorge_on, twinkifruit)) {
		inter_move_object(OBJ_TWINKIFRUIT, NOWHERE);
		text_show(452);
	} else if (player_said_1(gnaw_on)) {
		text_show(453);
	} else if (player_said_2(massage, audio_tape)) {
		text_show(454);
	} else if (player_said_2(mangle, credit_chip)) {
		text_show(455);
	} else if (player_said_2(fondle, charge_cases)) {
		text_show(456);
	} else if (player_said_2(rub, bomb)) {
		text_show(457);
	} else if (player_said_2(set, timebomb)) {
		text_show(458);
		// Original also had a second unreachable block for: text_show(427);
	} else if (player_said_2(guzzle, alien_liquor)) {
		text_show(459);
	} else if (player_said_2(smash, target_module)) {
		text_show(460);
	} else if (player_said_1(juggle)) {
		text_show(461);
	} else if (player_said_2(apply, polycement)) {
		text_show(462);
	} else if (player_said_2(sniff, polycement)) {
		text_show(465);
	} else if (player_said_2(tie, fishing_line)) {
		text_show(463);
	} else if (player_said_2(attach, fishing_line)) {
		text_show(463);
	} else if (player_said_1(unlock)) {
		text_show(464);
	} else if (player_said_1(reflect)) {
		text_show(466);
	} else if (player_said_2(gaze_into, rearview_mirror)) {
		text_show(467);
	} else if (player_said_2(eat, chicken_bomb)) {
		text_show(469);
	} else if (player_said_2(break, vase)) {
		text_show(471);
	} else if (player_said_2(shake_hands, guards_arm2)) {
		text_show(472);
	} else if (player_said_2(read, log)) {
		text_show(473);
	} else if (player_said_2(rub, bombs)) {
		text_show(474);
	} else if (player_said_2(drink, formaldehyde)) {
		text_show(475);
	} else if (player_said_2(drink, petrox)) {
		text_show(476);
	} else if (player_said_2(drink, lecithin)) {
		text_show(477);
	} else if (player_said_3(put, poison_darts, plant_stalk) && player_has(OBJ_POISON_DARTS)
		&& player_has(OBJ_PLANT_STALK)) {
		inter_give_to_player(OBJ_BLOWGUN);
		inter_move_object(OBJ_PLANT_STALK, NOWHERE);
		global[kBlowgunStatus] = 0;
		object_examine(OBJ_BLOWGUN, 809);
	} else if (player_said_3(put, poison_darts, blowgun) && player_has(OBJ_POISON_DARTS)
		&& player_has(OBJ_BLOWGUN)) {
		text_show(433);
	} else if (player_said_1(deface) && player_said_1(fold) && player_said_1(mutilate)) {
		text_show(434);
	} else if (player_said_1(spindle)) {
		text_show(479);
	} else if ((player_said_1(read) || player_said_1(look_at) || player_said_1(look)) &&
		player_said_1(note) && player_has(OBJ_NOTE)) {
		inter_move_object(OBJ_NOTE, NOWHERE);
		inter_give_to_player(OBJ_COMBINATION);
		object_examine(OBJ_COMBINATION, 851);
	} else if ((player_said_1(look) || player_said_1(read)) &&
		((id = object_named(inter_main_noun)) > 0 ||
			(inter_second_noun > 0 &&
				(id = object_named(inter_second_noun)))) &&
		player_has(id)) {
		if (id == OBJ_REPAIR_LIST) {
			text_index[0] = global[kTeleporterCode + 7];
			text_index[1] = global[kTeleporterCode + 8];
			text_index[2] = global[kTeleporterCode + 6];
			text_index[3] = global[kTeleporterCode + 9];
			text_index[4] = global[kTeleporterCode + 0];
			text_index[5] = global[kTeleporterCode + 1];
			text_index[6] = global[kTeleporterCode + 4];
			text_index[7] = global[kTeleporterCode + 5];
			text_index[8] = global[kTeleporterCode + 2];

			object_examine(id, 402);
		} else {
			int messageId = 800 + id;
			if ((id == OBJ_CHARGE_CASES) && object_check_quality(OBJ_CHARGE_CASES, 3) != 0) {
				messageId = 860;
			}

			if (id == OBJ_TAPE_PLAYER && object[OBJ_AUDIO_TAPE].location == OBJ_TAPE_PLAYER)
				messageId = 867;

			if (id == 32 && object[OBJ_FISHING_LINE].location == 3)
				messageId = 862;

			if (id == OBJ_BOTTLE && global[kBottleStatus] != 0)
				messageId = 862 + global[kBottleStatus];

			if (id == OBJ_PHONE_HANDSET && global[kHandsetCellStatus])
				messageId = 861;

			object_examine(id, messageId);
		}
	} else if (player_said_3(put, burger, dead_fish)) {
		if (player_has(OBJ_BURGER) || player_has(OBJ_DEAD_FISH)) {
			inter_take_from_player(OBJ_DEAD_FISH, NOWHERE);
			inter_take_from_player(OBJ_BURGER, NOWHERE);
			inter_give_to_player(OBJ_STUFFED_FISH);
			object_examine(OBJ_STUFFED_FISH, 803);
		}
	} else if (player_said_3(put, audio_tape, tape_player) && player_has(OBJ_AUDIO_TAPE) &&
		player_has(OBJ_TAPE_PLAYER)) {
		inter_move_object(OBJ_AUDIO_TAPE, OBJ_TAPE_PLAYER);
	} else if (player_said_2(activate, tape_player) && player_has(OBJ_TAPE_PLAYER)) {
		if (object[OBJ_AUDIO_TAPE].location == OBJ_TAPE_PLAYER) {
			showRecipe();
		} else {
			text_show(406);
		}
	} else if (player_said_2(eject, tape_player) && player_has(OBJ_TAPE_PLAYER)) {
		if (object[OBJ_AUDIO_TAPE].location == OBJ_TAPE_PLAYER) {
			inter_give_to_player(OBJ_AUDIO_TAPE);
		} else {
			text_show(407);
		}
	} else if (player_said_2(disassemble, tape_player)) {
		text_show(408);
	} else if (player_said_2(activate, remote)) {
		text_show(global[kTopButtonPushed] ? 502 : 501);
	} else if ((player_said_3(attach, detonators, charge_cases) || player_said_3(put, detonators, charge_cases)) &&
		player_has(OBJ_DETONATORS) && player_has(OBJ_CHARGE_CASES)) {
		if (object_check_quality(OBJ_CHARGE_CASES, 3)) {
			inter_move_object(OBJ_CHARGE_CASES, NOWHERE);
			inter_move_object(OBJ_DETONATORS, NOWHERE);
			inter_give_to_player(OBJ_BOMBS);
			object_examine(OBJ_BOMBS, 403);
		} else {
			text_show(405);
		}
	} else if (player_said_2(attach, detonators)) {
		text_show(470);
	} else if ((player_said_3(attach, timer_module, bombs) || player_said_3(put, timer_module, bombs) || player_said_3(attach, timer_module, bomb)
		|| player_said_3(put, timer_module, bomb)) && player_has(OBJ_TIMER_MODULE) && (
			player_has(OBJ_BOMBS) || player_has(OBJ_BOMB))) {
		if (player_has(OBJ_BOMBS)) {
			inter_move_object(OBJ_BOMBS, NOWHERE);
			inter_give_to_player(OBJ_BOMB);
		} else {
			inter_move_object(OBJ_BOMB, NOWHERE);
		}

		inter_move_object(OBJ_TIMER_MODULE, NOWHERE);
		inter_give_to_player(OBJ_TIMEBOMB);
		object_examine(OBJ_TIMEBOMB, 404);
	} else if (player_said_2(fondle, plant_stalk)) {
		text_show(410);
	} else if (player_said_2(empty, bottle)) {
		global[kBottleStatus] = 0;
		text_show(432);
	} else if (player_said_2(disassemble, fishing_rod)) {
		if (object[OBJ_FISHING_LINE].location == 3) {
			inter_give_to_player(OBJ_FISHING_LINE);
			object_examine(OBJ_FISHING_LINE, 409);
		} else {
			text_show(428);
		}
	} else if (player_said_2(disassemble, penlight)) {
		switch (global[kPenlightCellStatus]) {
		case 1:
		case 2:
			inter_give_to_player(OBJ_DURAFAIL_CELLS);
			object_examine(OBJ_DURAFAIL_CELLS, 412);
			break;
		case 3:
			inter_give_to_player(OBJ_PHONE_CELLS);
			object_examine(OBJ_DURAFAIL_CELLS, 413);
			break;
		case 5:
			inter_give_to_player(OBJ_DURAFAIL_CELLS);
			object_examine(OBJ_DURAFAIL_CELLS, 411);
			break;
		case 6:
			inter_give_to_player(OBJ_DURAFAIL_CELLS);
			object_examine(OBJ_DURAFAIL_CELLS, 429);
			break;
		default:
			text_show(478);
			break;
		}

		global[kPenlightCellStatus] = 0;

	} else if (player_said_2(disassemble, phone_handset)) {
		switch (global[kHandsetCellStatus]) {
		case 1:
			inter_give_to_player(OBJ_DURAFAIL_CELLS);
			object_examine(OBJ_DURAFAIL_CELLS,
				game.difficulty != DIFFICULTY_HARD || global[kDurafailRecharged] ? 415 : 414);
			global[kDurafailRecharged] = true;
			break;
		case 2:
			inter_give_to_player(OBJ_DURAFAIL_CELLS);
			if (game.difficulty == DIFFICULTY_HARD) {
				object_examine(OBJ_DURAFAIL_CELLS, 416);
			}
			break;
		case 3:
			inter_give_to_player(OBJ_PHONE_CELLS);
			object_examine(OBJ_PHONE_CELLS, 418);
			break;
		case 4:
			inter_give_to_player(OBJ_PHONE_CELLS);
			object_examine(OBJ_PHONE_CELLS, 417);
			break;
		default:
			text_show(478);
			break;
		}

		global[kHandsetCellStatus] = 0;

	} else if (player_said_3(put, phone_cells, penlight)) {
		if (global[kPenlightCellStatus] == 0) {
			global[kPenlightCellStatus] = 3;
			inter_move_object(OBJ_PHONE_CELLS, NOWHERE);
			text_show(419);
		} else {
			text_show(420);
		}
	} else if (player_said_3(put, phone_cells, phone_handset)) {
		if (global[kHandsetCellStatus] == 0) {
			global[kHandsetCellStatus] = 3;
			inter_move_object(OBJ_PHONE_CELLS, NOWHERE);
			text_show(421);
		} else {
			text_show(422);
		}
	} else if (player_said_3(put, durafail_cells, penlight)) {
		if (global[kPenlightCellStatus]) {
			text_show(424);
		} else {
			inter_move_object(OBJ_DURAFAIL_CELLS, NOWHERE);
			global[kPenlightCellStatus] = game.difficulty != DIFFICULTY_HARD || global[kDurafailRecharged] ? 1 : 2;
			text_show(423);
		}
	} else if (player_said_3(put, durafail_cells, phone_handset)) {
		if (global[kHandsetCellStatus]) {
			text_show(426);
		} else {
			inter_move_object(OBJ_DURAFAIL_CELLS, NOWHERE);
			global[kHandsetCellStatus] = game.difficulty != DIFFICULTY_HARD || global[kDurafailRecharged] ? 1 : 2;
			text_show(425);
		}
	} else if (player_said_3(put, bomb, chicken) || player_said_3(put, bombs, chicken)) {
		inter_move_object(OBJ_CHICKEN, NOWHERE);
		if (player_has(OBJ_BOMBS)) {
			inter_move_object(OBJ_BOMBS, NOWHERE);
			inter_give_to_player(OBJ_BOMB);
		} else {
			inter_move_object(OBJ_BOMB, NOWHERE);
		}
		inter_give_to_player(OBJ_CHICKEN_BOMB);
		object_examine(OBJ_CHICKEN_BOMB, 430);
	} else {
		goto done;
	}

	player.command_ready = false;

done:
	;
}

void RexNebularEngine::global_error_code() {
	int randVal = getRandomNumber(1, 1000);

	if (player_said_2(throw, bomb) || player_said_2(throw, bombs)
		|| player_said_2(throw, timebomb) || player_said_2(throw, chicken_bomb))
		text_show(42);
	else if (player_said_1(disassemble))
		text_show(435);
	else if ((player_said_2(eat, dead_fish) || player_said_2(eat, stuffed_fish)) && player_has(object_named(inter_main_noun)))
		text_show(12);
	else if ((player_said_2(smell, dead_fish) || player_said_2(smell, stuffed_fish)) && player_has(object_named(inter_main_noun)))
		text_show(13);
	else if (player_said_2(eat, chicken) && player_has(OBJ_CHICKEN))
		text_show(912);
	else if ((player_said_1(shoot) || player_said_1(hose_down)) && player_said_1(blowgun)) {
		if ((room_id >= 104) && (room_id <= 111))
			text_show(38);
		else if (player_said_1(piranha))
			text_show(41);
		else if (player_said_1(chicken) || player_said_1(vulture) || player_said_1(spider)
			|| player_said_1(yellow_bird) || player_said_1(swooping_creature) || player_said_1(captive_creature)) {
			text_show(40);
		} else
			text_show(39);
	} else if (player_said_1(talkto)) {
		global[kTalkInanimateCount] = (global[kTalkInanimateCount] + 1) % 16;
		if (!global[kTalkInanimateCount]) {
			text_show(2);
		} else {
			Common::String tmpMsg = "\"Greetings, ";
			tmpMsg += vocab_string(inter_main_noun);
			tmpMsg += "!\"";
			kernel_message_purge();
			kernel_message_add(const_cast<char *>(tmpMsg.c_str()), 0, 0, 0x1110, 120, 34, 0);
		}
	} else if (player_said_3(give, door, ceiling) || player_said_2(close, chair))
		text_show(3);
	else if (player_said_1(throw)) {
		int objId = object_named(inter_main_noun);
		if (objId < 0)
			text_show(4);
		else if (object[objId].location != 2)
			text_show(5);
		else
			text_show(6);
	} else if (player_said_1(look)) {
		if (player_said_1(binoculars) && (inter_second_noun > 0))
			text_show(10);
		else if (randVal < 600)
			text_show(7);
		else
			text_show(21);
	} else if (player_said_1(take)) {
		int objId = object_named(inter_main_noun);
		if (player_has(objId))
			text_show(16);
		else if (randVal <= 333)
			text_show(8);
		else if (randVal <= 666)
			text_show(22);
		else
			text_show(23);
	} else if (player_said_1(close)) {
		if (randVal <= 333)
			text_show(9);
		else
			text_show(33);
	} else if (player_said_1(open)) {
		if (randVal <= 500)
			text_show(30);
		else if (randVal <= 750)
			text_show(31);
		else
			text_show(32);
	} else if (player_said_1(pull))
		text_show(18);
	else if (player_said_1(push)) {
		if (randVal < 750)
			text_show(19);
		else
			text_show(20);
	} else if (player_said_1(put)) {
		int objId = object_named(inter_main_noun);
		if (player_has(objId))
			text_show(25);
		else
			text_show(24);
	} else if (player_said_1(give)) {
		int objId = object_named(inter_main_noun);
		if (!player_has(objId))
			text_show(26);
		else if (randVal <= 500)
			text_show(28);
		else
			text_show(29);
	} else if (!player_said_1(walkto) && !player_said_1(walk_across) && !player_said_1(walk_towards) && !player_said_1(walk_down)
		&& !player_said_1(swim_to) && !player_said_1(swim_across) && !player_said_1(swim_into) && !player_said_1(swim_through)
		&& !player_said_1(swim_under)) {
		if (randVal <= 100)
			text_show(36);
		else if (randVal <= 200)
			text_show(1);
		else if (randVal <= 475)
			text_show(34);
		else if (randVal <= 750)
			text_show(35);
		else
			text_show(37);
	}
}

void RexNebularEngine::global_sound_driver() {
	Common::strcpy_s(kernel.sound_driver, "/");
	env_catint(kernel.sound_driver, new_section, 1);
}

} // namespace RexNebular
} // namespace MADS
