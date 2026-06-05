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

#include "common/scummsys.h"
#include "math/utils.h"
#include "mads/madsv2/nebular/nebular.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {

void Scene361::setup() {
	if (_scene->_currentSceneId == 391)
		_globals[kSexOfRex] = REX_MALE;

	setPlayerSpritesPrefix();
	setAAName();
}

void Scene361::handleRexAction() {
	switch (_game._trigger) {
	case 0:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 50, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 3, -2);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 15);

		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 15, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -1, 4);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
		break;

	case 1:
	{
		int seqIdx = _globals._sequenceIndexes[1];
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 4);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 15);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[1], seqIdx);
	}
	break;

	case 2:
	{
		int seqIdx = _globals._sequenceIndexes[2];
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 4, 10);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], seqIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
	}
	break;

	case 3:
	{
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 3);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);

		int seqIdx = _globals._sequenceIndexes[2];
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 11);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(167, 100));
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], seqIdx);
		_scene->_sequences.addTimer(15, 4);
	}
	break;

	case 4:
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 2);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);

		_scene->_sequences.setDone(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 12, 14);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(167, 100));
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 5);
		break;

	case 5:
	{
		int seqIdx = _globals._sequenceIndexes[2];
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 15);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(167, 100));
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], seqIdx);
		_scene->_sequences.addTimer(15, 6);
	}
	break;

	case 6:
		_scene->_sequences.setDone(_globals._sequenceIndexes[2]);
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		_scene->_sequences.addTimer(48, 7);
		break;

	case 7:
		_scene->_nextSceneId = 313;
		break;

	default:
		break;
	}
}

void Scene361::handleRoxAction() {
	switch (_game._trigger) {
	case 0:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 18, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 2, 4);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 15);

		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 18, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], -1, 3);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
		break;

	case 1:
	{
		int tmpIdx = _globals._sequenceIndexes[1];
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 4);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 15);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[1], tmpIdx);
	}
	break;

	case 2:
	{
		int tmpIdx = _globals._sequenceIndexes[4];
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 4, 8);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], tmpIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
	}
	break;

	case 3:
	{
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 3);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);

		int tmpIdx = _globals._sequenceIndexes[4];
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 9, 10);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(167, 100));
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], tmpIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 4);
	}
	break;

	case 4:
	{
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 2);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);

		int tmpIdx = _globals._sequenceIndexes[4];
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 11, 15);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(167, 100));
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], tmpIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 5);
	}
	break;

	case 5:
	{
		int tmpIdx = _globals._sequenceIndexes[4];
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 16);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(167, 100));
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], tmpIdx);
		_scene->_sequences.addTimer(48, 6);
	}
	break;

	case 6:
		_scene->_sequences.setDone(_globals._sequenceIndexes[4]);

		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		_scene->_sequences.addTimer(48, 7);
		break;

	case 7:
		_scene->_nextSceneId = 313;
		break;

	default:
		break;
	}
}

void Scene361::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(Resources::formatName(307, 'X', 0, EXT_SS, ""));

	if (_globals[kSexOfRex] == REX_MALE) {
		_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*RXCL_8");
		_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RXCL_2");
	} else
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*ROXCL_8");

	_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
	_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 15);

	if (_scene->_priorSceneId == 391) {
		_globals[kSexOfRex] = REX_MALE;
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_game._player._facing = FACING_SOUTH;
		_game._player._playerPos = Common::Point(166, 101);
		_scene->_sequences.addTimer(120, 70);
	} else if (_scene->_priorSceneId == 360)
		_game._player._playerPos = Common::Point(302, 145);
	else if (_scene->_priorSceneId == 320) {
		_game._player._playerPos = Common::Point(129, 113);
		_game._player._facing = FACING_SOUTH;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(13, 145);

	_game.loadQuoteSet(0xFB, 0xFC, 0);

	if (_scene->_priorSceneId == 320)
		_scene->_kernelMessages.setQuoted(_scene->_kernelMessages.addQuote(0xFB, 0, 0x78), 4, true);

	sceneEntrySound();
}

void Scene361::step() {
	if (_game._trigger >= 70) {
		switch (_game._trigger) {
		case 70:
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 2);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);

			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addTimer(15, 71);
			break;

		case 71:
			_scene->_sequences.setDone(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addTimer(15, 72);
			break;

		case 72:
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 3);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);

			_scene->_sequences.setDone(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 3);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addTimer(15, 73);
			break;

		case 73:
			_scene->_sequences.setDone(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 12, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 4, 5);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 74);
			break;

		case 74:
		{
			int seqIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 6);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], seqIdx);
			_scene->_sequences.addTimer(15, 75);
		}
		break;

		case 75:
			_scene->_sequences.setDone(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 7);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addTimer(15, 76);
			break;

		case 76:
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 2);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);

			_scene->_sequences.setDone(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 8);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addTimer(15, 77);
			break;

		case 77:
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 15);

			_scene->_sequences.setDone(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 9);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addTimer(15, 78);
			break;

		case 78:
			_scene->_sequences.setDone(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 12, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 10, -2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 79);
			break;

		case 79:
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[3]);
			_game._player._stepEnabled = true;
			_game._player._visible = true;
			break;

		default:
			break;
		}
	}
}

void Scene361::preActions() {
	if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_EAST))
		_game._player._walkOffScreenSceneId = 360;

	if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_WEST))
		_game._player._walkOffScreenSceneId = 354;
}

void Scene361::actions() {
	if (_action._lookFlag)
		_vm->_dialogs->show(36119);
	else if (_action.isAction(VERB_SIT_AT, NOUN_DESK)) {
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.addQuote(252, 0, 120);
	} else if (_action.isAction(VERB_CLIMB_INTO, NOUN_AIR_VENT)) {
		if (_globals[kSexOfRex] == REX_FEMALE)
			handleRoxAction();
		else
			handleRexAction();
	} else if (_action.isAction(VERB_LOOK, NOUN_DESK))
		_vm->_dialogs->show(36110);
	else if (_action.isAction(VERB_LOOK, NOUN_WALL))
		_vm->_dialogs->show(36111);
	else if (_action.isAction(VERB_LOOK, NOUN_LIGHTING_FIXTURE) || _action.isAction(VERB_STARE_AT, NOUN_LIGHTING_FIXTURE))
		_vm->_dialogs->show(36112);
	else if (_action.isAction(VERB_LOOK, NOUN_LIGHTS) || _action.isAction(VERB_STARE_AT, NOUN_LIGHTS))
		_vm->_dialogs->show(36113);
	else if (_action.isAction(VERB_TAKE, NOUN_LIGHTS))
		_vm->_dialogs->show(36114);
	else if (_action.isAction(VERB_LOOK, NOUN_LIGHT_BULB) || _action.isAction(VERB_STARE_AT, NOUN_LIGHT_BULB))
		_vm->_dialogs->show(36115);
	else if (_action.isAction(VERB_TAKE, NOUN_LIGHT_BULB))
		_vm->_dialogs->show(36116);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_WEST))
		_vm->_dialogs->show(36117);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_EAST))
		_vm->_dialogs->show(36118);
	else if (_action.isAction(VERB_LOOK, NOUN_AIR_VENT))
		_vm->_dialogs->show(36120);
	else
		return;

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
