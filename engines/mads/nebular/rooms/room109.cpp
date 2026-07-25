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
#include "mads/core/himem.h"
#include "mads/core/pal.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section1.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	byte _rexThrowingObject;
	byte _hoovicDifficultFl;
	byte _beforeEatingRex;
	byte _eatingRex;
	byte _hungryFl;
	byte _eatingFirstFish;

	int16 _throwingObjectId;
	int16 _hoovicTrigger;
};

static Scratch local;


static void room_109_init() {
	_globals[kFishIn105] = true;

	_globals._spriteIndexes[0] = _scene->_sprites.addSprites("*RXSWRC_6");
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(kernel_name('O', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(kernel_name('O', 2));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(kernel_name('O', 0));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(kernel_name('H', 4));

	local._rexThrowingObject = false;
	local._throwingObjectId = 0;
	local._beforeEatingRex = false;
	local._eatingRex = false;
	local._hungryFl = false;

	if (_scene->_priorSceneId == 110) {
		player.x = 248;
		player.y = 38;
		_globals[kHoovicSated] = 2;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		player.x = 20;
		player.y = 68;
		player.facing = FACING_EAST;
	}

	if (!_globals[kHoovicAlive]) {
		_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 6, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 4);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], -2, -2);

		int idx = _scene->_dynamicHotspots.add(words_dead_purple_monster, words_swim_to, -1, Common::Rect(256, 57, 267, 87));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(241, 91), FACING_NORTHEAST);
		idx = _scene->_dynamicHotspots.add(words_dead_purple_monster, words_swim_to, -1, Common::Rect(242, 79, 265, 90));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(241, 91), FACING_NORTHEAST);
		idx = _scene->_dynamicHotspots.add(words_monster_sludge, words_swim_to, -1, Common::Rect(231, 88, 253, 94));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(241, 91), FACING_NORTHEAST);
	}

	if (!_globals[kHoovicAlive] || _globals[kHoovicSated])
		_scene->changeVariant(1);

	if (object_is_here(OBJ_BURGER)) {
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], -2, -2);
		int idx = _scene->_dynamicHotspots.add(words_burger, words_swim_to, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(-3, 0), FACING_NORTHEAST);
	} else if (_scene->_roomChanged)
		inter_give_to_player(OBJ_BURGER);

	if (_scene->_roomChanged) {
		inter_give_to_player(OBJ_DEAD_FISH);
		inter_give_to_player(OBJ_STUFFED_FISH);
	}

	pal_change_color(252, 50, 50, 63);
	pal_change_color(253, 30, 30, 50);

	kernel.quotes = quote_load(0x53, 0x52, 0x54, 0x55, 0x56, 0x57, 0x58, 0);
	local._eatingFirstFish = (!player.been_here_before) && (_scene->_priorSceneId < 110);

	if (local._eatingFirstFish) {
		_globals._spriteIndexes[10] = _scene->_sprites.addSprites(kernel_full_name(105, 'F', 1, "", EXT_SS));
		_globals._spriteIndexes[9] = _scene->_sprites.addSprites(kernel_name('H', 1));

		_globals._sequenceIndexes[10] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[10], true, 4, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 5);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[10], Common::Point(126, 39));
		_scene->_sequences.setMotion(_globals._sequenceIndexes[10], 0, 200, 0);
		_scene->_sequences.setScale(_globals._sequenceIndexes[10], 80);
		player.commands_allowed = false;
	}

	section_1_music();
}

static void room_109_daemon() {
	if (local._beforeEatingRex) {
		if (!local._eatingRex) {
			if (player.x > 205) {
				_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 6, 1, 0, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 4);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_SPRITE, 6, 70);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 71);

				local._eatingRex = true;
				g_engine->_soundManager->command(34, 0);
			}
		} else {
			switch (kernel.trigger) {
			case 70:
				player.walker_visible = false;
				break;

			case 71:
				_scene->_reloadSceneFlag = true;
				break;

			default:
				break;
			}
		}
	}

	if (local._hungryFl && (Common::Point(player.x, player.y) == Common::Point(160, 32)) && (player.facing == FACING_EAST)) {
		player_walk(226, 24, FACING_EAST);
		player.commands_allowed = false;
		local._hungryFl = false;
		local._beforeEatingRex = true;
		_scene->_sprites.remove(_globals._spriteIndexes[6]);
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites(kernel_name('H', 0));
		kernel_new_palette();
	}

	if (player.walking && (_scene->_rails.getNext() > 0) && _globals[kHoovicAlive] && !_globals[kHoovicSated] && !local._hungryFl && !local._beforeEatingRex) {
		player_cancel_command();
		player_start_walking(160, 32, FACING_EAST);
		_scene->_rails.resetNext();
		local._hungryFl = true;
	}

	if (local._eatingFirstFish && (_scene->_sequences[_globals._sequenceIndexes[10]]._position.x >= 178)) {
		_globals._sequenceIndexes[9] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 4, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 4);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_SPRITE, 29, 72);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_EXPIRE, 29, 73);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[10], _globals._sequenceIndexes[9]);
		local._eatingFirstFish = false;
		player.commands_allowed = true;
		g_engine->_soundManager->command(34, 0);
	}

	if (kernel.trigger == 72)
		_scene->_sequences.remove(_globals._sequenceIndexes[10]);

	if (kernel.trigger == 73) {
		_scene->_sequences.remove(_globals._sequenceIndexes[9]);
		_scene->_sprites.remove(_globals._spriteIndexes[9]);
		_scene->_sprites.remove(_globals._spriteIndexes[10]);

		_scene->_spriteSlots.clear();
		_scene->_spriteSlots.fullRefresh();

		int randVal = g_engine->getRandomNumber(85, 88);
		int idx = _scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, quote_string(kernel.quotes, randVal));
		_scene->_kernelMessages.setQuoted(idx, 4, true);
		_scene->_kernelMessages._entries[idx]._frameTimer = _scene->_frameStartTime + 4;
	}
}

static void room_109_pre_parser() {
	if (player_said_2(swim_under, overhang_to_west))
		player.walk_off_edge_to_room = 108;

	if ((player_said_1(throw) || player_said_1(give) || player_said_1(put))
		&& (player_said_1(small_hole) || player_said_1(tunnel))
		&& (player_said_1(dead_fish) || player_said_1(stuffed_fish) || player_said_1(burger))) {
		int idx = object_named(player2.words[1]);
		if ((idx >= 0) && player_has(idx)) {
			player.prepare_walk_x = 106;
			player.prepare_walk_y = 38;
			player.prepare_walk_facing = FACING_EAST;
			player.need_to_walk = true;
			player.ready_to_walk = true;
		}
	}

	if ((player_said_2(swim_into, tunnel) || player_said_2(swim_to, small_hole))
		&& (!_globals[kHoovicAlive] || _globals[kHoovicSated]) && (player_said_1(tunnel)))
		player.walk_off_edge_to_room = 110;

	local._hungryFl = false;
}

static void room_109_parser() {
	if (player.look_around) {
		text_show(10912);
		player.command_ready = false;
		return;
	}

	if ((player_said_1(throw) || player_said_1(give)) && (player_said_1(small_hole) || player_said_1(tunnel))) {
		if (player_said_1(dead_fish) || player_said_1(stuffed_fish) || player_said_1(burger)) {
			local._throwingObjectId = object_named(player2.words[1]);
			if (local._throwingObjectId >= 0) {
				if ((player_has(local._throwingObjectId) && _globals[kHoovicAlive]) || local._rexThrowingObject) {
					switch (kernel.trigger) {
					case 0:
						local._rexThrowingObject = true;
						local._hoovicDifficultFl = false;
						inter_move_object(local._throwingObjectId, NOWHERE);
						_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 4, 1, 0, 0);
						_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[0]);
						_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
						player.walker_visible = false;
						player.commands_allowed = false;

						switch (local._throwingObjectId) {
						case OBJ_DEAD_FISH:
						case OBJ_STUFFED_FISH:
							_globals._spriteIndexes[8] = _scene->_sprites.addSprites(kernel_name('H', 1));
							break;

						case OBJ_BURGER:
							local._hoovicDifficultFl = (game.difficulty == DIFFICULTY_HARD);
							_globals._spriteIndexes[8] = _scene->_sprites.addSprites(kernel_name('H', (local._hoovicDifficultFl ? 3 : 1)));
							break;

						default:
							break;
						}

						kernel_new_palette();
						break;

					case 1:
						player.walker_visible = true;
						local._hoovicTrigger = 4;
						switch (local._throwingObjectId) {
						case OBJ_BURGER:
							_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, (local._hoovicDifficultFl ? 4 : 6), 1, 0, 0);
							_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_SPRITE, 2, 2);
							if (local._hoovicDifficultFl) {
								_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 30);
								_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 5);
							} else {
								_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 4);
								_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 8);
								local._hoovicTrigger = 3;
							}
							break;
						case OBJ_DEAD_FISH:
							_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 4, 1, 0, 0);
							_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_SPRITE, 2, 2);
							break;
						case OBJ_STUFFED_FISH:
							_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 4, 1, 0, 0);
							_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 2, 2);
							local._hoovicTrigger = 3;
							break;
						default:
							break;
						}
						break;

					case 2:
						if (local._hoovicDifficultFl)
							_globals._sequenceIndexes[8] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[8], false, 4, 2, 0, 0);
						else
							_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 4, 1, 0, 0);

						_scene->_sequences.setDepth(_globals._sequenceIndexes[8], 4);
						_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, local._hoovicTrigger);
						g_engine->_soundManager->command(34, 0);
						break;

					case 3:
						_scene->loadAnimation(kernel_full_name(109, 'H', 2, "", EXT_AA), 4);
						g_engine->_soundManager->command(35, 0);
						_globals[kHoovicAlive] = false;
						break;

					case 4:
						if (!_globals[kHoovicAlive]) {
							_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 6, 1, 0, 0);
							_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 4);
							_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], -2, -2);
							int idx = _scene->_dynamicHotspots.add(words_dead_purple_monster, words_swim_to, -1, Common::Rect(256, 57, 256 + 12, 57 + 31));
							_scene->_dynamicHotspots.setPosition(idx, Common::Point(241, 91), FACING_NORTHEAST);
							idx = _scene->_dynamicHotspots.add(words_dead_purple_monster, words_swim_to, -1, Common::Rect(242, 79, 242 + 24, 79 + 12));
							_scene->_dynamicHotspots.setPosition(idx, Common::Point(241, 91), FACING_NORTHEAST);
							idx = _scene->_dynamicHotspots.add(words_monster_sludge, words_swim_to, -1, Common::Rect(231, 88, 231 + 23, 88 + 7));
							_scene->_dynamicHotspots.setPosition(idx, Common::Point(241, 91), FACING_NORTHEAST);
							_scene->changeVariant(1);
						} else {
							if (local._throwingObjectId == OBJ_DEAD_FISH) {
								++_globals[kHoovicFishEaten];
								int threshold;
								switch (game.difficulty) {
								case DIFFICULTY_HARD:
									threshold = 1;
									break;
								case DIFFICULTY_MEDIUM:
									threshold = 3;
									break;
								default:
									threshold = 50;
									break;
								}

								if (_globals[kHoovicFishEaten] >= threshold) {
									int randVal = g_engine->getRandomNumber(83, 84);
									_scene->_kernelMessages.add(Common::Point(230, 24), 0xFDFC, 0, 0, 120, quote_string(kernel.quotes, randVal));
									_globals[kHoovicFishEaten] = 0;
									_globals[kHoovicSated] = 1;
									_scene->changeVariant(1);
								}
							}
						}
						_scene->freeAnimation();
						_scene->_sequences.remove(_globals._sequenceIndexes[8]);
						_scene->_sprites.remove(_globals._spriteIndexes[8]);
						_scene->_spriteSlots.clear();
						_scene->_spriteSlots.fullRefresh();
						_scene->_sequences.scan();
						if (player.walker_visible) {
							player.sprite_changed = true;
							player_set_image();
						}

						player.commands_allowed = true;
						local._rexThrowingObject = false;
						break;

					case 5:
					{
						inter_move_object(OBJ_BURGER, _scene->_currentSceneId);
						_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 0, 0, 0);
						_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 30, 30);
						int idx = _scene->_dynamicHotspots.add(words_burger, words_swim_to, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
						_scene->_dynamicHotspots.setPosition(idx, Common::Point(-3, 0), FACING_NORTHEAST);
						_scene->_sequences.addTimer(65, 6);
					}
					break;

					case 6:
					{
						_scene->_sequences.remove(_globals._sequenceIndexes[3]);
						_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 1, 0, 0);
						_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 31, 46);
						int idx = _scene->_dynamicHotspots.add(words_burger, words_swim_to, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
						_scene->_dynamicHotspots.setPosition(idx, Common::Point(-3, 0), FACING_NORTHEAST);
						_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 7);
					}
					break;

					case 7:
					{
						_scene->_sequences.remove(_globals._sequenceIndexes[3]);
						_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
						int idx = _scene->_dynamicHotspots.add(words_burger, words_swim_to, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
						_scene->_dynamicHotspots.setPosition(idx, Common::Point(-3, 0), FACING_NORTHEAST);
						text_show(10915);
					}
					break;

					case 8:
						_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 7, 1, 0, 0);
						_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 5, 16);
						break;

					default:
						break;
					}
					player.command_ready = false;
					return;
				} else if (player_has(local._throwingObjectId)) {
					// Nothing.
				}
			}
		}
	}

	if (player_said_2(take, burger) && object_is_here(OBJ_BURGER)) {
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		inter_give_to_player(OBJ_BURGER);
	} else if (player_said_2(look, ocean_floor))
		text_show(10901);
	else if (player_said_2(look, coral))
		text_show(10902);
	else if ((player_said_1(take) || player_said_1(pull)) && player_said_1(coral))
		text_show(10903);
	else if (player_said_2(look, rocks))
		text_show(10904);
	else if (player_said_2(take, rocks))
		text_show(10905);
	else if (player_said_2(look, cave_wall))
		text_show(10906);
	else if (player_said_2(look, tunnel)) {
		if (_globals[kHoovicAlive])
			text_show(10907);
		else
			text_show(10913);
	} else if (player_said_2(look, small_hole))
		text_show(10908);
	else if (player_said_2(look, overhang_to_west))
		text_show(10911);
	else if (player_said_2(put, small_hole))
		text_show(10910);
	else if (player_said_2(look, dead_purple_monster))
		text_show(10914);
	else
		return;

	player.command_ready = false;
}

void room_109_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._rexThrowingObject);
	s.syncAsByte(local._hoovicDifficultFl);
	s.syncAsByte(local._beforeEatingRex);
	s.syncAsByte(local._eatingRex);
	s.syncAsByte(local._hungryFl);
	s.syncAsByte(local._eatingFirstFish);
	s.syncAsSint32LE(local._throwingObjectId);
	s.syncAsSint32LE(local._hoovicTrigger);
}

void room_109_preload() {
	room_init_code_pointer = room_109_init;
	room_pre_parser_code_pointer = room_109_pre_parser;
	room_parser_code_pointer = room_109_parser;
	room_daemon_code_pointer = room_109_daemon;

	for (int count = 0; count < 4; ++count) {
		himem_preload_series(kernel_full_name(109, 'H', count, nullptr, 0), 3);
	}

	_scene->addActiveVocab(words_dead_purple_monster);
	_scene->addActiveVocab(words_monster_sludge);

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
