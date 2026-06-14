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
#include "mads/madsv2/nebular/rooms/section3.h"
#include "mads/madsv2/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int32 _monsterTime;
	int32 _circularQuoteId;

};

static Scratch local;


static void room_389_init() {
	_scene->_userInterface.setup(kInputLimitedSentences);
	local._monsterTime = 0;
	local._circularQuoteId = 0x159;

	if (_globals[kAfterHavoc])
		_scene->_hotspots.activate(NOUN_MONSTER, false);
	else {
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('m', -1));
		_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 6, 0, 0, 0);
		_scene->_kernelMessages.initRandomMessages(1,
			Common::Rect(88, 19, 177, 77), 13, 2, 0xFDFC, 60,
			247, 248, 249, 0);
	}

	_vm->_palette->setEntry(252, 63, 37, 26);
	_vm->_palette->setEntry(253, 45, 24, 17);
	_game._player._visible = false;
	_game.loadQuoteSet(0xF7, 0xF8, 0xF9, 0x159, 0x15A, 0x15B, 0);

	section_3_music();
}

static void room_389_daemon() {
	_scene->_kernelMessages.randomServer();
	if (_scene->_frameStartTime >= local._monsterTime) {
		int chanceMinor = _scene->_kernelMessages.checkRandom() * 4 + 1;
		_scene->_kernelMessages.generateRandom(20, chanceMinor);
		local._monsterTime = _scene->_frameStartTime + 2;
	}
}

static void room_389_parser() {
	if (_action.isAction(VERB_RETURN_TO, NOUN_AIR_SHAFT))
		_scene->_nextSceneId = 313;
	else if (_action.isAction(VERB_TALKTO, NOUN_MONSTER)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_kernelMessages.add(Common::Point(160, 136), 0x1110, 32, 1, 120, _game.getQuote(local._circularQuoteId));
			local._circularQuoteId++;
			if (local._circularQuoteId > 0x15B)
				local._circularQuoteId = 0x159;

			break;

		case 1:
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_LOOK_THROUGH, NOUN_GRATE)) {
		if (_globals[kAfterHavoc]) {
			if ((_game._difficulty != DIFFICULTY_HARD) && (_game._objects[OBJ_SECURITY_CARD]._roomNumber == 359))
				_vm->_dialogs->show(38911);
			else
				_vm->_dialogs->show(38912);
		} else
			_vm->_dialogs->show(38910);
	} else if (_action.isAction(VERB_OPEN, NOUN_GRATE)) {
		if (_globals[kAfterHavoc])
			_vm->_dialogs->show(38914);
		else
			_vm->_dialogs->show(38913);
	} else
		return;

	_action._inProgress = false;
}

void room_389_synchronize(Common::Serializer &s) {
	s.syncAsUint32LE(local._monsterTime);
	s.syncAsSint32LE(local._circularQuoteId);
}

void room_389_preload() {
	room_init_code_pointer = room_389_init;
	room_pre_parser_code_pointer = section_3_pre_parser;
	room_parser_code_pointer = room_389_parser;
	room_daemon_code_pointer = room_389_daemon;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
