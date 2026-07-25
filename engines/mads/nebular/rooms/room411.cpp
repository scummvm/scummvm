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
#include "mads/nebular/rooms/section4.h"
#include "mads/nebular/rooms/dialog.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int32 _curAnimationFrame;
	int32 _newIngredient;
	int32 _newQuantity;
	int32 _resetFrame;
	int32 _badThreshold;

	bool _killRox;
	bool _makeMushroomCloud;

	Dialog _dialog1;
	Dialog _dialog2;
	Dialog _dialog3;
	Dialog _dialog4;
};

static Scratch local;


static bool addIngredient() {
	bool retVal = false;

	switch (local._newIngredient) {
	case OBJ_LECITHIN:
		if (global[kIngredientList + global[kNextIngredient]] == 1)
			retVal = true;

		local._badThreshold = 1;
		break;

	case OBJ_ALIEN_LIQUOR:
		if (global[kIngredientList + global[kNextIngredient]] == 0)
			retVal = true;

		local._badThreshold = 0;
		break;

	case OBJ_FORMALDEHYDE:
		if (global[kIngredientList + global[kNextIngredient]] == 3)
			retVal = true;

		local._badThreshold = 3;
		break;

	case OBJ_PETROX:
		if (global[kIngredientList + global[kNextIngredient]] == 2)
			retVal = true;

		local._badThreshold = 2;
		break;

	default:
		break;
	}

	if (!retVal && (global[kNextIngredient] == 0))
		global[kBadFirstIngredient] = local._badThreshold;

	if (global[kNextIngredient] == 0)
		retVal = true;

	return(retVal);
}

static bool addQuantity() {
	bool retVal = false;

	if (global[kIngredientQuantity + global[kNextIngredient]] == local._newQuantity)
		retVal = true;

	if (!retVal && (global[kNextIngredient] == 0))
		global[kBadFirstIngredient] = local._badThreshold;

	if (global[kNextIngredient] == 0)
		retVal = true;

	return(retVal);
}

static int computeQuoteAndQuantity() {
	int quoteId;
	int quantity;

	switch (player2.words[0]) {
	case 0x252:
		quoteId = 0x26F;
		quantity = 0;
		break;

	case 0x253:
		quoteId = 0x271;
		quantity = 0;
		break;

	case 0x254:
		quoteId = 0x270;
		quantity = 0;
		break;

	case 0x255:
		quoteId = 0x272;
		quantity = 0;
		break;

	case 0x256:
		quoteId = 0x267;
		quantity = 2;
		break;

	case 0x257:
		quoteId = 0x269;
		quantity = 2;
		break;

	case 0x258:
		quoteId = 0x268;
		quantity = 2;
		break;

	case 0x259:
		quoteId = 0x26A;
		quantity = 2;
		break;

	case 0x25A:
		quoteId = 0x26B;
		quantity = 3;
		break;

	case 0x25B:
		quoteId = 0x26D;
		quantity = 3;
		break;

	case 0x25C:
		quoteId = 0x26C;
		quantity = 3;
		break;

	case 0x25D:
		quoteId = 0x26E;
		quantity = 3;
		break;

	case 0x25E:
		quoteId = 0x263;
		quantity = 1;
		break;

	case 0x25F:
		quoteId = 0x265;
		quantity = 1;
		break;

	case 0x260:
		quoteId = 0x264;
		quantity = 1;
		break;

	case 0x261:
		quoteId = 0x266;
		quantity = 1;
		break;

	default:
		quoteId = 0;
		quantity = 0;
		break;
	}

	_scene->_kernelMessages.add(Common::Point(202, 82), 0x1110, 32, 0, 120, quote_string(kernel.quotes, quoteId));
	return quantity;
}

static void handleKettleAction() {
	switch (global[kNextIngredient]) {
	case 1:
		g_sequence_ids[4] = _scene->_sequences.addSpriteCycle(g_sprite_ids[4],
			false, 15, 0, 0, 0);
		break;

	case 2:
		_scene->_sequences.remove(g_sequence_ids[4]);
		g_sequence_ids[4] = _scene->_sequences.addSpriteCycle(g_sprite_ids[4],
			false, 6, 0, 0, 0);
		break;

	case 3:
		local._makeMushroomCloud = true;
		break;

	default:
		break;
	}
}

static void handleDialog() {
	if ((player2.words[0] != 0x262) && (kernel.trigger == 0)) {
		if (player_has(local._newIngredient)) {
			switch (local._newIngredient) {
			case OBJ_FORMALDEHYDE:
				local._resetFrame = 17;
				break;

			case OBJ_PETROX:
				local._resetFrame = 55;
				break;

			case OBJ_LECITHIN:
				local._resetFrame = 36;
				break;

			default:
				local._resetFrame = 112;
				break;
			}

			player.clock = _scene->_frameStartTime + player.frame_delay;
			player.walker_visible = false;
			player.commands_allowed = false;
			_scene->_animation[0]->setCurrentFrame(local._resetFrame);
		}
		_scene->_kernelMessages.reset();
		local._newQuantity = computeQuoteAndQuantity();

		if ((global[kNextIngredient] == 1) && (global[kBadFirstIngredient] > -1))
			local._killRox = true;
		else if (addIngredient() && addQuantity()) {
			handleKettleAction();
			global[kNextIngredient]++;
		} else
			local._killRox = true;

		_scene->_userInterface.setup(kInputBuildingSentences);
	} else if (player2.words[0] == 0x262)
		_scene->_userInterface.setup(kInputBuildingSentences);
}

static void giveToRex(int object_id) {
	switch (object_id) {
	case 0:
		inter_give_to_player(OBJ_ALIEN_LIQUOR);
		break;

	case 1:
		inter_give_to_player(OBJ_LECITHIN);
		break;

	case 2:
		inter_give_to_player(OBJ_PETROX);
		break;

	case 3:
		inter_give_to_player(OBJ_FORMALDEHYDE);
		break;

	default:
		break;
	}
}

static void room_411_init() {
	if (_scene->_priorSceneId == 411) {
		if ((global[kNextIngredient] == 1) && (global[kBadFirstIngredient] > -1))
			giveToRex(global[kBadFirstIngredient]);
		else if (global[kNextIngredient] > 0) {
			for (int i = 0; i < global[kNextIngredient]; i++)
				giveToRex(global[kIngredientList + i]);
		}
		global[kNextIngredient] = 0;
		global[kBadFirstIngredient] = -1;
	}

	g_sprite_ids[1] = _scene->_sprites.addSprites(kernel_name('x', 0));
	g_sprite_ids[2] = _scene->_sprites.addSprites(kernel_name('x', 1));
	g_sprite_ids[4] = _scene->_sprites.addSprites(kernel_name('c', 0));
	g_sprite_ids[5] = _scene->_sprites.addSprites(kernel_name('f', 0));
	g_sprite_ids[6] = _scene->_sprites.addSprites(kernel_name('f', 1));
	g_sprite_ids[7] = _scene->_sprites.addSprites(kernel_name('f', 2));
	g_sprite_ids[9] = _scene->_sprites.addSprites(kernel_name('c', 1));
	g_sprite_ids[10] = _scene->_sprites.addSprites(kernel_name('a', 6));
	g_sprite_ids[11] = _scene->_sprites.addSprites(kernel_name('a', 1));
	g_sprite_ids[8] = _scene->_sprites.addSprites("*ROXRC_9");

	g_sequence_ids[1] = _scene->_sequences.addSpriteCycle(g_sprite_ids[1], false, 5, 0, 0, 0);
	g_sequence_ids[2] = _scene->_sequences.addSpriteCycle(g_sprite_ids[2], false, 50, 0, 0, 0);

	kernel.quotes = quote_load(0x252, 0x25E, 0x25A, 0x256, 0x253, 0x25F, 0x25B, 0x257, 0x254, 0x260, 0x25C, 0x258, 0x255,
		0x261, 0x25D, 0x259, 0x262, 0x267, 0x263, 0x26B, 0x26F, 0x268, 0x264, 0x26C, 0x270, 0x26A, 0x266, 0x26E,
		0x272, 0x269, 0x265, 0x26D, 0x271, 0);

	local._dialog1.setup(0x5B, 0x252, 0x25E, 0x25A, 0x256, 0x262, -1);
	local._dialog2.setup(0x5C, 0x253, 0x25F, 0x25B, 0x257, 0x262, -1);
	local._dialog3.setup(0x5D, 0x254, 0x260, 0x25C, 0x258, 0x262, -1);
	local._dialog4.setup(0x5E, 0x255, 0x261, 0x25D, 0x259, 0x262, -1);

	if (global[kNextIngredient] >= 4 && !object_check_quality(OBJ_CHARGE_CASES, 3)) {
		_scene->_hotspots.activate(words_kettle, false);
		_scene->_hotspots.activate(words_explosives, true);
	} else {
		_scene->_hotspots.activate(words_explosives, false);
		_scene->_hotspots.activate(words_kettle, true);
	}

	if (global[kNextIngredient] >= 4 && object_check_quality(OBJ_CHARGE_CASES, 3)) {
		g_sequence_ids[4] = _scene->_sequences.startCycle(g_sprite_ids[4], true, 6);
	} else if (!object_check_quality(OBJ_CHARGE_CASES, 3)) {
		switch (global[kNextIngredient]) {
		case 1:
			g_engine->_soundManager->command(53, 0);
			break;

		case 2:
			g_engine->_soundManager->command(53, 0);
			g_engine->_soundManager->command(54, 0);
			g_sequence_ids[4] = _scene->_sequences.addSpriteCycle(g_sprite_ids[4], false, 15, 0, 0, 0);
			break;

		case 3:
			g_engine->_soundManager->command(53, 0);
			g_engine->_soundManager->command(54, 0);
			g_engine->_soundManager->command(55, 0);
			g_sequence_ids[4] = _scene->_sequences.addSpriteCycle(g_sprite_ids[4], false, 6, 0, 0, 0);
			break;

		case 4:
			g_engine->_soundManager->command(53, 0);
			g_engine->_soundManager->command(54, 0);
			g_engine->_soundManager->command(55, 0);
			g_engine->_soundManager->command(56, 0);
			g_sequence_ids[4] = _scene->_sequences.addSpriteCycle(g_sprite_ids[4], false, 6, 0, 0, 0);
			break;

		default:
			g_engine->_soundManager->command(10, 0);
			break;
		}
	}

	if (global[kNextIngredient] >= 4 && object_check_quality(OBJ_CHARGE_CASES, 3)) {
		g_sequence_ids[4] = _scene->_sequences.startCycle(g_sprite_ids[4], true, 6);
		_scene->_sequences.setDepth(g_sequence_ids[4], 1);
	}

	if (object_is_here(OBJ_FORMALDEHYDE)) {
		g_sequence_ids[7] = _scene->_sequences.startCycle(g_sprite_ids[7], false, 1);
		_scene->_sequences.setDepth(g_sequence_ids[7], 1);
		int idx = _scene->_dynamicHotspots.add(words_formaldehyde, words_walkto, g_sequence_ids[7], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(206, 145), FACING_SOUTHEAST);
	}

	if (object_is_here(OBJ_PETROX)) {
		g_sequence_ids[5] = _scene->_sequences.startCycle(g_sprite_ids[5], false, 1);
		_scene->_sequences.setDepth(g_sequence_ids[5], 8);
		int idx = _scene->_dynamicHotspots.add(words_petrox, words_walkto, g_sequence_ids[5], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(186, 112), FACING_NORTHEAST);
	}

	if (object_is_here(OBJ_LECITHIN)) {
		g_sequence_ids[6] = _scene->_sequences.startCycle(g_sprite_ids[6], false, 1);
		_scene->_sequences.setDepth(g_sequence_ids[6], 8);
		int idx = _scene->_dynamicHotspots.add(words_lecithin, words_walkto, g_sequence_ids[6], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(220, 121), FACING_NORTHEAST);
	}

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		player.x = 60;
		player.y = 146;
		player.facing = FACING_NORTHEAST;
	}

	section_4_music();

	if (_scene->_roomChanged) {
		inter_give_to_player(OBJ_ALIEN_LIQUOR);
		inter_give_to_player(OBJ_CHARGE_CASES);
		inter_give_to_player(OBJ_TAPE_PLAYER);
		inter_give_to_player(OBJ_AUDIO_TAPE);
	}

	_scene->loadAnimation(kernel_name('a', -1));
	_scene->_animation[0]->setCurrentFrame(128);

	local._makeMushroomCloud = false;
	local._killRox = false;
}

static void room_411_daemon() {
	if (_scene->_animation[0] != nullptr) {
		if (local._curAnimationFrame != _scene->_animation[0]->getCurrentFrame()) {
			local._curAnimationFrame = _scene->_animation[0]->getCurrentFrame();
			local._resetFrame = -1;

			switch (local._curAnimationFrame) {
			case 16:
				player.commands_allowed = true;
				player.clock = _scene->_frameStartTime + player.frame_delay;
				player.walker_visible = true;
				local._resetFrame = 128;
				break;

			case 35:
			case 54:
			case 71:
			case 127:
				if (local._killRox) {
					local._resetFrame = 72;
				} else {
					local._resetFrame = 0;
					inter_take_from_player(local._newIngredient, NOWHERE);
					switch (global[kNextIngredient]) {
					case 1:
						g_engine->_soundManager->command(53, 0);
						break;

					case 2:
						g_engine->_soundManager->command(54, 0);
						break;

					case 3:
						g_engine->_soundManager->command(55, 0);
						break;

					case 4:
						g_engine->_soundManager->command(56, 0);
						break;

					default:
						break;
					}
				}
				break;

			case 22:
			case 41:
			case 59:
			case 115:
				if (local._makeMushroomCloud) {
					g_sequence_ids[9] = _scene->_sequences.addSpriteCycle(g_sprite_ids[9], false, 5, 1, 0, 0);
					local._makeMushroomCloud = false;
					_scene->_hotspots.activate(words_kettle, false);
					_scene->_hotspots.activate(words_explosives, true);
				}
				break;

			case 111:
				local._resetFrame = 111;
				_scene->_reloadSceneFlag = true;
				break;

			case 129:
				local._resetFrame = 128;
				break;

			default:
				break;
			}

			if ((local._resetFrame >= 0) && (local._resetFrame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(local._resetFrame);
				local._curAnimationFrame = local._resetFrame;
			}
		}
	}

	if (_scene->_animation[0]->getCurrentFrame() == 86)
		g_engine->_soundManager->command(59, 0);
}

static void room_411_pre_parser() {
	if (player_said_2(look, petrox) && (object_is_here(OBJ_PETROX)))
		player.need_to_walk = true;

	if (player_said_2(look, lecithin) && (object_is_here(OBJ_LECITHIN)))
		player.need_to_walk = true;

	if (player_said_2(look, formaldehyde) && (object_is_here(OBJ_FORMALDEHYDE)))
		player.need_to_walk = true;

	if (player_said_2(look, explosives) || player_said_2(look, kettle) || player_said_2(look, mishap) ||
		player_said_2(look, alcove) || player_said_2(look, sink) || player_said_2(put, sink) ||
		player_said_2(look, experiment) || player_said_2(look, drawing_board))
		player.need_to_walk = true;

	if (player_said_2(pull, knife_switch) || player_said_2(push, knife_switch))
		player.need_to_walk = false;
}

static void room_411_parser() {
	if (inter_input_mode == kInputConversation) {
		handleDialog();
		player.command_ready = false;
		return;
	}

	if (player_said_2(walk_into, corridor_to_south)) {
		_scene->_nextSceneId = 406;
		g_engine->_soundManager->command(10, 0);
		player.command_ready = false;
		return;
	}

	if ((global[kNextIngredient] >= 4) && (player_said_2(take, explosives) || player_said_3(put, charge_cases, explosives))
		&& !object_check_quality(OBJ_CHARGE_CASES, 3)
		&& player_has(OBJ_CHARGE_CASES)) {
		switch (kernel.trigger) {
		case 0:
			g_engine->_soundManager->command(10, 0);
			g_engine->_soundManager->command(57, 0);
			player.commands_allowed = false;
			player.walker_visible = false;
			g_sequence_ids[10] = _scene->_sequences.addSpriteCycle(g_sprite_ids[10], false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(g_sequence_ids[10], 1, 6);
			_scene->_sequences.setDepth(g_sequence_ids[10], 3);
			_scene->_sequences.addSubEntry(g_sequence_ids[10], SEQUENCE_TRIGGER_EXPIRE, 0, 110);
			break;

		case 110:
		{
			int idx = g_sequence_ids[10];
			g_sequence_ids[10] = _scene->_sequences.startCycle(g_sprite_ids[10], false, 6);
			_scene->_sequences.setDepth(g_sequence_ids[10], 3);
			_scene->_sequences.updateTimeout(g_sequence_ids[10], idx);
			_scene->_sequences.addTimer(180, 111);
		}
		break;

		case 111:
			_scene->_hotspots.activate(words_kettle, true);
			_scene->_hotspots.activate(words_explosives, false);
			_scene->_sequences.remove(g_sequence_ids[4]);
			_scene->_sequences.remove(g_sequence_ids[10]);

			g_sequence_ids[4] = _scene->_sequences.startCycle(g_sprite_ids[4], true, 6);
			_scene->_sequences.setDepth(g_sequence_ids[4], 1);

			g_sequence_ids[10] = _scene->_sequences.addReverseSpriteCycle(g_sprite_ids[10], false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(g_sequence_ids[10], 1, 6);
			_scene->_sequences.setDepth(g_sequence_ids[10], 3);
			_scene->_sequences.addSubEntry(g_sequence_ids[10], SEQUENCE_TRIGGER_EXPIRE, 0, 112);
			// fall through
		case 112:
			player.clock = _scene->_frameStartTime - player.frame_delay;
			player.walker_visible = true;
			player.commands_allowed = true;
			object_set_quality(OBJ_CHARGE_CASES, 3, 1);
			object_examine(OBJ_CHARGE_CASES, 41142, 0);
			break;

		default:
			break;
		}
		player.command_ready = false;
		return;
	} else if (!player_has(OBJ_CHARGE_CASES) && player_said_2(take, explosives)) {
		text_show(41143);
		player.command_ready = false;
		return;
	}

	if (player_said_2(take, petrox) && (object_is_here(OBJ_PETROX) || kernel.trigger)) {
		switch (kernel.trigger) {
		case 0:
			g_engine->_soundManager->command(57, 0);
			player.commands_allowed = false;
			player.walker_visible = false;
			g_sequence_ids[8] = _scene->_sequences.startPingPongCycle(g_sprite_ids[8], false, 7, 2, 0, 0);
			_scene->_sequences.setAnimRange(g_sequence_ids[8], 1, 2);
			_scene->_sequences.setMsgLayout(g_sequence_ids[8]);
			_scene->_sequences.addSubEntry(g_sequence_ids[8], SEQUENCE_TRIGGER_SPRITE, 2, 1);
			_scene->_sequences.addSubEntry(g_sequence_ids[8], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			_scene->_sequences.remove(g_sequence_ids[5]);
			inter_give_to_player(OBJ_PETROX);
			object_examine(OBJ_PETROX, 41120, 0);
			break;

		case 2:
			player.clock = _scene->_frameStartTime + player.frame_delay;
			player.clock = _scene->_frameStartTime + player.frame_delay;
			player.walker_visible = true;
			_scene->_sequences.addTimer(20, 3);
			break;

		case 3:
			player.commands_allowed = true;
			break;

		default:
			break;
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(take, lecithin) && (object_is_here(OBJ_LECITHIN) || kernel.trigger)) {
		switch (kernel.trigger) {
		case 0:
			g_engine->_soundManager->command(57, 0);
			player.commands_allowed = false;
			player.walker_visible = false;
			g_sequence_ids[8] = _scene->_sequences.startPingPongCycle(g_sprite_ids[8], false, 7, 2, 0, 0);
			_scene->_sequences.setAnimRange(g_sequence_ids[8], 1, 2);
			_scene->_sequences.setMsgLayout(g_sequence_ids[8]);
			_scene->_sequences.addSubEntry(g_sequence_ids[8], SEQUENCE_TRIGGER_SPRITE, 2, 1);
			_scene->_sequences.addSubEntry(g_sequence_ids[8], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			_scene->_sequences.remove(g_sequence_ids[6]);
			inter_give_to_player(OBJ_LECITHIN);
			object_examine(OBJ_LECITHIN, 41124, 0);
			break;

		case 2:
			player.clock = _scene->_frameStartTime + player.frame_delay;
			player.walker_visible = true;
			_scene->_sequences.addTimer(20, 3);
			break;

		case 3:
			player.commands_allowed = true;
			break;

		default:
			break;
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(take, formaldehyde) && object_is_here(OBJ_FORMALDEHYDE) && (kernel.trigger == 0)) {
		g_engine->_soundManager->command(57, 0);
		player.commands_allowed = false;
		player.walker_visible = false;
		g_sequence_ids[11] = _scene->_sequences.startCycle(g_sprite_ids[11], false, 2);
		_scene->_sequences.setDepth(g_sequence_ids[11], 1);
		_scene->_sequences.addTimer(20, 100);
		_scene->_sequences.remove(g_sequence_ids[7]);
		inter_give_to_player(OBJ_FORMALDEHYDE);
		player.command_ready = false;
		return;
	}

	if (kernel.trigger == 100) {
		_scene->_sequences.remove(g_sequence_ids[11]);
		player.clock = _scene->_frameStartTime - player.frame_delay;
		player.walker_visible = true;
		player.commands_allowed = true;
		_scene->_sequences.addTimer(20, 10);
	}

	if (kernel.trigger == 10)
		object_examine(OBJ_FORMALDEHYDE, 41124, 0);

	if (player_said_1(put) && player_said_1(kettle)) {
		if (player_said_1(petrox) ||
			player_said_1(formaldehyde) ||
			player_said_1(lecithin) ||
			player_said_1(alien_liquor)) {
			local._newIngredient = object_named(player2.words[1]);
			switch (local._newIngredient) {
			case OBJ_ALIEN_LIQUOR:
				local._dialog1.start();
				break;

			case OBJ_FORMALDEHYDE:
				local._dialog3.start();
				break;

			case OBJ_PETROX:
				local._dialog4.start();
				break;

			case OBJ_LECITHIN:
				local._dialog2.start();
				break;

			default:
				break;
			}
		}
	}


	if (player_said_2(look, monitor))
		text_show(41110);
	else if (player_said_2(look, air_purifier))
		text_show(41111);
	else if (player_said_2(look, lab_equipment))
		text_show(41112);
	else if (player_said_2(look, knife_switch))
		text_show(41113);
	else if (player_said_2(push, knife_switch) || player_said_2(pull, knife_switch))
		text_show(41114);
	else if (player_said_2(look, toxic_waste))
		text_show(41115);
	else if (player_said_2(take, toxic_waste))
		text_show(41116);
	else if (player_said_2(look, drawing_board))
		text_show(41117);
	else if (player_said_2(look, experiment))
		text_show(41118);
	else if (player_said_2(look, petrox) && object_is_here(OBJ_PETROX))
		text_show(41119);
	else if (player_said_2(look, alcove))
		text_show(41121);
	else if ((player_said_2(look, formaldehyde)) && (object_is_here(OBJ_FORMALDEHYDE)))
		text_show(41122);
	else if ((player_said_2(look, lecithin)) && (object_is_here(OBJ_LECITHIN)))
		text_show(41123);
	else if (player_said_2(look, kettle)) {
		if (global[kNextIngredient] > 0 && !object_check_quality(OBJ_CHARGE_CASES, 3)) {
			text_show(41126);
		} else if (global[kNextIngredient] == 0 || object_check_quality(OBJ_CHARGE_CASES, 3)) {
			text_show(41125);
		}
	} else if (player_said_2(look, explosives) && object_check_quality(OBJ_CHARGE_CASES, 3) == 0) {
		text_show(41127);
	} else if (player_said_2(take, kettle))
		text_show(41128);
	else if (player_said_2(look, control_panel))
		text_show(41129);
	else if (player_said_2(look, mishap))
		text_show(41130);
	else if (player_said_2(look, corridor_to_south))
		text_show(41131);
	else if (player.look_around)
		text_show(41132);
	else if (player_said_2(look, air_horn))
		text_show(41133);
	else if (player_said_2(look, debris))
		text_show(41134);
	else if (player_said_2(look, heater))
		text_show(41135);
	else if (player_said_2(look, pipe))
		text_show(41136);
	else if (player_said_2(look, sink))
		text_show(41137);
	else if (player_said_2(put, sink))
		text_show(41138);
	else if (player_said_2(take, experiment))
		text_show(41139);
	else if (player_said_2(look, electrodes))
		text_show(41140);
	else if (player_said_2(take, electrodes))
		text_show(41141);
	else
		return;

	player.command_ready = false;
}

void room_411_synchronize(Common::Serializer &s) {
	s.syncAsSint32LE(local._curAnimationFrame);
	s.syncAsSint32LE(local._newIngredient);
	s.syncAsSint32LE(local._newQuantity);
	s.syncAsSint32LE(local._resetFrame);
	s.syncAsSint32LE(local._badThreshold);

	s.syncAsByte(local._killRox);
	s.syncAsByte(local._makeMushroomCloud);
}

void room_411_preload() {
	room_init_code_pointer = room_411_init;
	room_pre_parser_code_pointer = room_411_pre_parser;
	room_parser_code_pointer = room_411_parser;
	room_daemon_code_pointer = room_411_daemon;

	section_4_walker();
	section_4_interface();
	_scene->addActiveVocab(words_walkto);
	_scene->addActiveVocab(words_alien_liquor);
	_scene->addActiveVocab(words_formaldehyde);
	_scene->addActiveVocab(words_petrox);
	_scene->addActiveVocab(words_lecithin);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
