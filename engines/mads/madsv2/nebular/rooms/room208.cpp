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

#include "mads/madsv2/core/game.h"
#include "mads/madsv2/nebular/global.h"
#include "mads/madsv2/nebular/nebular.h"
#include "mads/madsv2/nebular/mads/inventory.h"
#include "mads/madsv2/nebular/mads/words.h"
#include "mads/madsv2/nebular/rooms/section2.h"
#include "mads/madsv2/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _rhotundaTurnFl;
	bool _boundingFl;
	long _rhotundaTime;
};

static Scratch local;


static void updateTrap() {
	if (_globals[kRhotundaStatus] == 1) {
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 8, 0, 0, 24);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 5);
		int idx = _scene->_dynamicHotspots.add(words_huge_legs, words_walkto, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(100, 146), FACING_NORTH);
		_scene->_hotspots.activate(414, false);
		return;
	}

	switch (_globals[kLeavesStatus]) {
	case LEAVES_ON_GROUND:
	{
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 15);
		int idx = _scene->_dynamicHotspots.add(words_pile_of_leaves, words_walkto, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(60, 152), FACING_NORTH);
	}
	break;
	case LEAVES_ON_TRAP:
	{
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 15);
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
		_scene->_hotspots.activate(words_deep_pit, false);
		int idx = _scene->_dynamicHotspots.add(words_leaf_covered_pit, words_walkto, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(100, 146), FACING_NORTH);
		_scene->_dynamicHotspots[idx]._articleNumber = PREP_ON;
	}
	break;
	default:
		break;
	}
}

static void room_208_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('a', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('x', 2));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites("*RXMBD_8");

	updateTrap();

	local._rhotundaTurnFl = false;
	local._boundingFl = false;
	_scene->_kernelMessages._talkFont = font_inter;
	_scene->_textSpacing = 0;

	if (_scene->_priorSceneId == 207) {
		_game._player._playerPos = Common::Point(8, 122);
		_game._player._facing = FACING_EAST;
	} else if (_scene->_priorSceneId == 203) {
		_game._player._playerPos = Common::Point(142, 108);
		_game._player._facing = FACING_SOUTH;
	} else if (_scene->_priorSceneId == 209) {
		_game._player._playerPos = Common::Point(307, 123);
		_game._player._facing = FACING_WEST;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(162, 149);
		_game._player._facing = FACING_NORTH;
	}

	_game.loadQuoteSet(0x81, 0x46, 0);

	if ((_scene->_priorSceneId == 207) && (_globals[kMonkeyStatus] == MONKEY_HAS_BINOCULARS)) {
		int msgIndex = _scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(129));
		_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
	}

	_vm->_palette->setEntry(16, 0, 0, 63);
	_vm->_palette->setEntry(17, 0, 0, 45);
	section_2_music();
}

static void room_208_daemon() {
	if (local._boundingFl && _scene->_animation[0] &&
		(local._rhotundaTime <= _scene->_animation[0]->getCurrentFrame())) {
		local._rhotundaTime = _scene->_animation[0]->getCurrentFrame();

		if (local._rhotundaTime == 125)
			_scene->_sequences.remove(_globals._sequenceIndexes[4]);
	}

	if (!local._rhotundaTurnFl)
		return;

	if ((_game._player._playerPos != Common::Point(20, 148)) || (_game._player._facing != FACING_EAST))
		return;

	if ((_game._trigger == 0) && local._boundingFl)
		return;

	local._boundingFl = true;

	switch (_game._trigger) {
	case 0:
		_scene->loadAnimation(formAnimName('A', -1), 81);
		local._rhotundaTime = 0;
		break;
	case 81:
		_scene->_sequences.remove(_globals._spriteIndexes[15]);
		_globals[kRhotundaStatus] = 1;
		updateTrap();
		_scene->_sequences.addTimer(90, 82);
		break;
	case 82:
		_game._player._stepEnabled = true;
		break;
	default:
		break;
	}
}

static void room_208_pre_parser() {
	auto &gplayer = _vm->_game->_player;

	if (_action.isAction(words_look) && gplayer._readyToWalk)
		gplayer._needToWalk = true;

	if (_action.isAction(words_walk_towards, words_grassland_to_east))
		gplayer._walkOffScreenSceneId = 209;

	if (_action.isAction(words_walk_towards, words_open_area_to_west))
		gplayer._walkOffScreenSceneId = 207;
}

static void subAction(int mode) {
	switch (_game._trigger) {
	case 0:
	{
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 6, 1, 0, 0);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);

		int endTrigger;
		if ((mode == 1) || (mode == 2))
			endTrigger = 1;
		else
			endTrigger = 2;

		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, endTrigger);
	}
	break;
	case 1:
	{
		int oldSeq = _globals._sequenceIndexes[5];
		_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 12, 3, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 3, 4);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[5], oldSeq);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
		_vm->_sound->command(20);
	}
	break;

	case 2:
	{
		switch (mode) {
		case 1:
			_game._objects.addToInventory(OBJ_BIG_LEAVES);
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals[kLeavesStatus] = 1;
			break;

		case 2:
			_game._objects.setRoom(OBJ_BIG_LEAVES, 1);
			_globals[kLeavesStatus] = 2;
			updateTrap();
			break;

		case 3:
			_scene->_sequences.remove(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
			_game._objects.removeFromInventory(OBJ_TWINKIFRUIT, 1);
			_vm->_sound->command(34);
			break;

		case 4:
			_game._objects.removeFromInventory(OBJ_BURGER, 1);
			_vm->_sound->command(33);
			break;

		case 5:
			_game._objects.removeFromInventory(OBJ_DEAD_FISH, 1);
			_vm->_sound->command(33);
			break;

		default:
			break;
		}

		int oldVal = _globals._sequenceIndexes[5];
		_globals._sequenceIndexes[5] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[5], false, 6, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 1, 3);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[5], oldVal);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
	}
	break;

	case 3:
		_game._player._visible = true;
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}
}

static void room_208_parser() {
	if (_action.isAction(words_walk_towards, words_lowlands_to_north)) {
		if (_globals[kRhotundaStatus])
			_scene->_nextSceneId = 203;
		else if (_game._trigger == 0) {
			_game._player._stepEnabled = false;
			int msgIndex = _scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 1, 120, _game.getQuote(70));
			_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
		} else if (_game._trigger == 1)
			_scene->_nextSceneId = 203;
	} else if (_action.isAction(words_walk_towards, words_field_to_south))
		_scene->_nextSceneId = 212;
	else if (_action.isAction(words_take, words_pile_of_leaves) && (!_globals[kLeavesStatus] || _game._trigger)) {
		subAction(1);
		if (_game._player._stepEnabled)
			_vm->_dialogs->showItem(OBJ_BIG_LEAVES, 0x326, 0);
	} else if (_action.isAction(words_put, words_big_leaves, words_deep_pit) && (_globals[kLeavesStatus] == 1 || _game._trigger))
		subAction(2);
	else if (_action.isAction(words_put, words_twinkifruit, words_leaf_covered_pit)) {
		subAction(3);
		if (_game._player._stepEnabled) {
			_game._player._stepEnabled = false;
			local._rhotundaTurnFl = true;
			_game._player.walk(Common::Point(20, 148), FACING_EAST);
		}
	} else if (_action.isAction(words_put, words_burger, words_leaf_covered_pit)) {
		subAction(4);
		if (_game._player._stepEnabled)
			_vm->_dialogs->show(20812);
	} else if (_action.isAction(words_put, words_dead_fish, words_leaf_covered_pit)) {
		subAction(5);
		if (_game._player._stepEnabled)
			_vm->_dialogs->show(20812);
	} else if (_action.isAction(words_look, words_cumulous_cloud))
		_vm->_dialogs->show(20801);
	else if (_action.isAction(words_look, words_open_area_to_west))
		_vm->_dialogs->show(20802);
	else if (_action.isAction(words_look, words_thorny_bush))
		_vm->_dialogs->show(20803);
	else if (_action.isAction(words_look, words_rocks))
		_vm->_dialogs->show(20804);
	else if (_action.isAction(words_look, words_small_cactus))
		_vm->_dialogs->show(20805);
	else if (_action.isAction(words_take, words_small_cactus))
		_vm->_dialogs->show(20806);
	else if (_action.isAction(words_look, words_grassland_to_east))
		_vm->_dialogs->show(20807);
	else if (_action.isAction(words_look, words_deep_pit))
		_vm->_dialogs->show(20808);
	else if (_action.isAction(words_look, words_pile_of_leaves))
		_vm->_dialogs->show(20809);
	else if (_action.isAction(words_look, words_leaf_covered_pit)) {
		if (_game._difficulty == DIFFICULTY_EASY)
			_vm->_dialogs->show(20810);
		else
			_vm->_dialogs->show(20811);
	} else if (_action.isAction(words_look, words_tree) || _action.isAction(words_look, words_trees))
		_vm->_dialogs->show(20813);
	else if (_action.isAction(words_take, words_leaf_covered_pit))
		_vm->_dialogs->show(20814);
	else if (_action.isAction(words_look, words_huge_legs))
		_vm->_dialogs->show(20815);
	else if (_action.isAction(words_take, words_huge_legs) || _action.isAction(words_pull, words_huge_legs))
		_vm->_dialogs->show(20816);
	else if (_action._savedFields._lookFlag && (_globals[kRhotundaStatus] == 1))
		_vm->_dialogs->show(20819);
	else if (_action._savedFields._lookFlag && (_globals[kLeavesStatus] == 2))
		_vm->_dialogs->show(20818);
	else if (_action._savedFields._lookFlag)
		_vm->_dialogs->show(20817);
	else
		return;

	_action._inProgress = false;
}

void room_208_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._rhotundaTurnFl);
	s.syncAsByte(local._boundingFl);
	s.syncAsSint32LE(local._rhotundaTime);
}

void room_208_preload() {
	room_init_code_pointer = room_208_init;
	room_pre_parser_code_pointer = room_208_pre_parser;
	room_parser_code_pointer = room_208_parser;
	room_daemon_code_pointer = room_208_daemon;

	section_2_walker();
	section_2_interface();
	_scene->addActiveVocab(words_huge_legs);
	_scene->addActiveVocab(words_leaf_covered_pit);
	_scene->addActiveVocab(words_pile_of_leaves);
	_scene->addActiveVocab(words_walkto);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
