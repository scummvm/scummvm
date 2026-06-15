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
#include "mads/madsv2/nebular/rooms/section8.h"
#include "mads/madsv2/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

static void room_805_init() {
	_game._player._visible = false;
	_scene->_userInterface.setup(kInputLimitedSentences);

	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('a', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('a', 2));

	if (_globals[kShieldModInstalled]) {
		_scene->_hotspots.activate(OBJ_SHIELD_MODULATOR, false);
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 25);
		int idx = _scene->_dynamicHotspots.add(NOUN_SHIELD_MODULATOR, VERB_REMOVE, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(0, 0), 0);
	}

	if (_globals[kTargetModInstalled]) {
		_scene->_hotspots.activate(OBJ_TARGET_MODULE, false);
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 12);
		int idx = _scene->_dynamicHotspots.add(NOUN_TARGET_MODULE, VERB_REMOVE, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(0, 0), 0);
	}

	section_8_music();
}

static void room_805_daemon() {
	auto &userInterface = _vm->_game->_scene._userInterface;

	if (_game._trigger == 70) {
		_scene->_hotspots.activate(OBJ_SHIELD_MODULATOR, false);
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 25);
		int idx = _scene->_dynamicHotspots.add(NOUN_SHIELD_MODULATOR, VERB_REMOVE, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(0, 0), 0);
		_globals[kShieldModInstalled] = true;
		_game._objects.setRoom(OBJ_SHIELD_MODULATOR, NOWHERE);
		userInterface._selectedInvIndex = -1;
		_game._player._stepEnabled = true;
		_vm->_sound->command(24);
	}

	if (_game._trigger == 80) {
		_scene->_hotspots.activate(OBJ_TARGET_MODULE, false);
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 12);
		int idx = _scene->_dynamicHotspots.add(NOUN_TARGET_MODULE, VERB_REMOVE, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(0, 0), 0);
		_globals[kTargetModInstalled] = true;
		_game._objects.setRoom(OBJ_TARGET_MODULE, NOWHERE);
		userInterface._selectedInvIndex = -1;
		_game._player._stepEnabled = true;
		_vm->_sound->command(24);
	}

	if (_game._trigger == 71) {
		_scene->_hotspots.activate(OBJ_SHIELD_MODULATOR, true);
		_globals[kShieldModInstalled] = false;
		_game._objects.addToInventory(OBJ_SHIELD_MODULATOR);
		_game._player._stepEnabled = true;
	}

	if (_game._trigger == 81) {
		_scene->_hotspots.activate(OBJ_TARGET_MODULE, true);
		_globals[kTargetModInstalled] = false;
		_game._objects.addToInventory(OBJ_TARGET_MODULE);
		_game._player._stepEnabled = true;
	}
}

static void room_805_pre_parser() {
	_game._player._needToWalk = false;
}

static void room_805_parser() {
	if (_action.isAction(VERB_EXIT, NOUN_SERVICE_PANEL))
		_scene->_nextSceneId = 804;
	else if (_action.isAction(VERB_INSTALL, NOUN_SHIELD_MODULATOR) && _game._objects.isInInventory(OBJ_SHIELD_MODULATOR)) {
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 7, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], -1, -2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 70);
		_game._player._stepEnabled = false;
	} else if (_action.isAction(VERB_INSTALL, NOUN_TARGET_MODULE) && _game._objects.isInInventory(OBJ_TARGET_MODULE)) {
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 7, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -1, -2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 80);
		_game._player._stepEnabled = false;
	} else if (_action.isAction(VERB_REMOVE, NOUN_SHIELD_MODULATOR) && _globals[kShieldModInstalled]) {
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 7, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], -1, -2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
		_game._player._stepEnabled = false;
	} else if (_action.isAction(VERB_REMOVE, NOUN_TARGET_MODULE) && _globals[kTargetModInstalled]) {
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 7, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -1, -2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 81);
		_game._player._stepEnabled = false;
	} else if (_action.isAction(VERB_INSTALL, NOUN_SHIELD_MODULATOR) && !_game._objects.isInInventory(OBJ_SHIELD_MODULATOR))
		_vm->_dialogs->show(80511);
	else if (_action.isAction(VERB_INSTALL, NOUN_TARGET_MODULE) && !_game._objects.isInInventory(OBJ_TARGET_MODULE))
		_vm->_dialogs->show(80510);
	else if (_action.isAction(VERB_REMOVE, NOUN_LIFE_SUPPORT_MODULE))
		_vm->_dialogs->show(80512);
	else
		return;

	_action._inProgress = false;
}

void room_805_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_805_preload() {
	room_init_code_pointer = room_805_init;
	room_daemon_code_pointer = room_805_daemon;
	room_pre_parser_code_pointer = room_805_pre_parser;
	room_parser_code_pointer = room_805_parser;

	section_8_walker();
	section_8_interface();
	_scene->addActiveVocab(VERB_REMOVE);
	_scene->addActiveVocab(NOUN_TARGET_MODULE);
	_scene->addActiveVocab(NOUN_SHIELD_MODULATOR);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
