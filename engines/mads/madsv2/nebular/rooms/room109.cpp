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
#include "mads/madsv2/nebular/rooms/room102.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {

Room109::Room109(RexNebularEngine *vm) : Room1xx(vm) {
	_rexThrowingObject = false;
	_hoovicDifficultFl = false;
	_beforeEatingRex = false;
	_eatingRex = false;
	_hungryFl = false;
	_eatingFirstFish = false;

	_throwingObjectId = -1;
	_hoovicTrigger = 0;
}

void Room109::synchronize(Common::Serializer &s) {
	Room1xx::synchronize(s);

	s.syncAsByte(_rexThrowingObject);
	s.syncAsByte(_hoovicDifficultFl);
	s.syncAsByte(_beforeEatingRex);
	s.syncAsByte(_eatingRex);
	s.syncAsByte(_hungryFl);
	s.syncAsByte(_eatingFirstFish);
	s.syncAsSint32LE(_throwingObjectId);
	s.syncAsSint32LE(_hoovicTrigger);
}

void Room109::setup() {
	_scene->addActiveVocab(NOUN_DEAD_PURPLE_MONSTER);
	_scene->addActiveVocab(NOUN_MONSTER_SLUDGE);

	setPlayerSpritesPrefix();
	setAAName();
}

void Room109::enter() {
	_globals[kFishIn105] = true;

	_globals._spriteIndexes[0] = _scene->_sprites.addSprites("*RXSWRC_6");
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('O', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('O', 2));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('O', 0));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('H', 4));

	_rexThrowingObject = false;
	_throwingObjectId = 0;
	_beforeEatingRex = false;
	_eatingRex = false;
	_hungryFl = false;

	if (_scene->_priorSceneId == 110) {
		_game._player._playerPos = Common::Point(248, 38);
		_globals[kHoovicSated] = 2;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(20, 68);
		_game._player._facing = FACING_EAST;
	}

	if (!_globals[kHoovicAlive]) {
		_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 6, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 4);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], -2, -2);

		int idx = _scene->_dynamicHotspots.add(102, 348, -1, Common::Rect(256, 57, 267, 87));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(241, 91), FACING_NORTHEAST);
		idx = _scene->_dynamicHotspots.add(102, 348, -1, Common::Rect(242, 79, 265, 90));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(241, 91), FACING_NORTHEAST);
		idx = _scene->_dynamicHotspots.add(229, 348, -1, Common::Rect(231, 88, 253, 94));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(241, 91), FACING_NORTHEAST);
	}

	if (!_globals[kHoovicAlive] || _globals[kHoovicSated])
		_scene->changeVariant(1);

	if (_game._objects.isInRoom(OBJ_BURGER)) {
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], -2, -2);
		int idx = _scene->_dynamicHotspots.add(53, 348, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(-3, 0), FACING_NORTHEAST);
	} else if (_scene->_roomChanged)
		_game._objects.addToInventory(OBJ_BURGER);

	if (_scene->_roomChanged) {
		_game._objects.addToInventory(OBJ_DEAD_FISH);
		_game._objects.addToInventory(OBJ_STUFFED_FISH);
	}

	_vm->_palette->setEntry(252, 50, 50, 63);
	_vm->_palette->setEntry(253, 30, 30, 50);

	_game.loadQuoteSet(0x53, 0x52, 0x54, 0x55, 0x56, 0x57, 0x58, 0);
	_eatingFirstFish = (!_game._visitedScenes._sceneRevisited) && (_scene->_priorSceneId < 110);

	if (_eatingFirstFish) {
		_globals._spriteIndexes[10] = _scene->_sprites.addSprites(Resources::formatName(105, 'F', 1, EXT_SS, ""));
		_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('H', 1));

		_globals._sequenceIndexes[10] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[10], true, 4, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 5);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[10], Common::Point(126, 39));
		_scene->_sequences.setMotion(_globals._sequenceIndexes[10], 0, 200, 0);
		_scene->_sequences.setScale(_globals._sequenceIndexes[10], 80);
		_game._player._stepEnabled = false;
	}

	sceneEntrySound();
}

void Room109::step() {
	if (_beforeEatingRex) {
		if (!_eatingRex) {
			if (_game._player._playerPos.x > 205) {
				_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 6, 1, 0, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 4);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_SPRITE, 6, 70);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 71);

				_eatingRex = true;
				_vm->_sound->command(34);
			}
		} else {
			switch (_game._trigger) {
			case 70:
				_game._player._visible = false;
				break;

			case 71:
				_scene->_reloadSceneFlag = true;
				break;

			default:
				break;
			}
		}
	}

	if (_hungryFl && (_game._player._playerPos == Common::Point(160, 32)) && (_game._player._facing == FACING_EAST)) {
		_game._player.walk(Common::Point(226, 24), FACING_EAST);
		_game._player._stepEnabled = false;
		_hungryFl = false;
		_beforeEatingRex = true;
		_scene->_sprites.remove(_globals._spriteIndexes[6]);
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('H', 0));
		_vm->_palette->refreshSceneColors();
	}

	if (_game._player._moving && (_scene->_rails.getNext() > 0) && _globals[kHoovicAlive] && !_globals[kHoovicSated] && !_hungryFl && !_beforeEatingRex) {
		_game._player.cancelCommand();
		_game._player.startWalking(Common::Point(160, 32), FACING_EAST);
		_scene->_rails.resetNext();
		_hungryFl = true;
	}

	if (_eatingFirstFish && (_scene->_sequences[_globals._sequenceIndexes[10]]._position.x >= 178)) {
		_globals._sequenceIndexes[9] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 4, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 4);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_SPRITE, 29, 72);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_EXPIRE, 29, 73);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[10], _globals._sequenceIndexes[9]);
		_eatingFirstFish = false;
		_game._player._stepEnabled = true;
		_vm->_sound->command(34);
	}

	if (_game._trigger == 72)
		_scene->_sequences.remove(_globals._sequenceIndexes[10]);

	if (_game._trigger == 73) {
		_scene->_sequences.remove(_globals._sequenceIndexes[9]);
		_scene->_sprites.remove(_globals._spriteIndexes[9]);
		_scene->_sprites.remove(_globals._spriteIndexes[10]);

		_scene->_spriteSlots.clear();
		_scene->_spriteSlots.fullRefresh();

		int randVal = _vm->getRandomNumber(85, 88);
		int idx = _scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(randVal));
		_scene->_kernelMessages.setQuoted(idx, 4, true);
		_scene->_kernelMessages._entries[idx]._frameTimer = _scene->_frameStartTime + 4;
	}
}

void Room109::preActions() {
	if (_action.isAction(VERB_SWIM_UNDER, NOUN_OVERHANG_TO_WEST))
		_game._player._walkOffScreenSceneId = 108;

	if ((_action.isAction(VERB_THROW) || _action.isAction(VERB_GIVE) || _action.isAction(VERB_PUT))
		&& (_action.isTarget(NOUN_SMALL_HOLE) || _action.isTarget(NOUN_TUNNEL))
		&& (_action.isObject(NOUN_DEAD_FISH) || _action.isObject(NOUN_STUFFED_FISH) || _action.isObject(NOUN_BURGER))) {
		int idx = _game._objects.getIdFromDesc(_action._activeAction._objectNameId);
		if ((idx >= 0) && _game._objects.isInInventory(idx)) {
			_game._player._prepareWalkPos = Common::Point(106, 38);
			_game._player._prepareWalkFacing = FACING_EAST;
			_game._player._needToWalk = true;
			_game._player._readyToWalk = true;
		}
	}

	if ((_action.isAction(VERB_SWIM_INTO, NOUN_TUNNEL) || _action.isAction(VERB_SWIM_TO, NOUN_SMALL_HOLE))
		&& (!_globals[kHoovicAlive] || _globals[kHoovicSated]) && (_action.isObject(NOUN_TUNNEL)))
		_game._player._walkOffScreenSceneId = 110;

	_hungryFl = false;
}

void Room109::actions() {
	if (_action._lookFlag) {
		_vm->_dialogs->show(10912);
		_action._inProgress = false;
		return;
	}

	if ((_action.isAction(VERB_THROW) || _action.isAction(VERB_GIVE)) && (_action.isTarget(NOUN_SMALL_HOLE) || _action.isTarget(NOUN_TUNNEL))) {
		if (_action.isObject(NOUN_DEAD_FISH) || _action.isObject(NOUN_STUFFED_FISH) || _action.isObject(NOUN_BURGER)) {
			_throwingObjectId = _game._objects.getIdFromDesc(_action._activeAction._objectNameId);
			if (_throwingObjectId >= 0) {
				if ((_game._objects.isInInventory(_throwingObjectId) && _globals[kHoovicAlive]) || _rexThrowingObject) {
					switch (_game._trigger) {
					case 0:
						_rexThrowingObject = true;
						_hoovicDifficultFl = false;
						_game._objects.setRoom(_throwingObjectId, NOWHERE);
						_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 4, 1, 0, 0);
						_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[0]);
						_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
						_game._player._visible = false;
						_game._player._stepEnabled = false;

						switch (_throwingObjectId) {
						case OBJ_DEAD_FISH:
						case OBJ_STUFFED_FISH:
							_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('H', 1));
							break;

						case OBJ_BURGER:
							_hoovicDifficultFl = (_game._difficulty == DIFFICULTY_HARD);
							_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('H', (_hoovicDifficultFl ? 3 : 1)));
							break;

						default:
							break;
						}

						_vm->_palette->refreshSceneColors();
						break;

					case 1:
						_game._player._visible = true;
						_hoovicTrigger = 4;
						switch (_throwingObjectId) {
						case OBJ_BURGER:
							_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, (_hoovicDifficultFl ? 4 : 6), 1, 0, 0);
							_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_SPRITE, 2, 2);
							if (_hoovicDifficultFl) {
								_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 30);
								_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 5);
							} else {
								_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 4);
								_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 8);
								_hoovicTrigger = 3;
							}
							break;
						case OBJ_DEAD_FISH:
							_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 4, 1, 0, 0);
							_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_SPRITE, 2, 2);
							break;
						case OBJ_STUFFED_FISH:
							_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 4, 1, 0, 0);
							_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 2, 2);
							_hoovicTrigger = 3;
							break;
						default:
							break;
						}
						break;

					case 2:
						if (_hoovicDifficultFl)
							_globals._sequenceIndexes[8] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[8], false, 4, 2, 0, 0);
						else
							_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 4, 1, 0, 0);

						_scene->_sequences.setDepth(_globals._sequenceIndexes[8], 4);
						_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, _hoovicTrigger);
						_vm->_sound->command(34);
						break;

					case 3:
						_scene->loadAnimation(Resources::formatName(109, 'H', 2, EXT_AA, ""), 4);
						_vm->_sound->command(35);
						_globals[kHoovicAlive] = false;
						break;

					case 4:
						if (!_globals[kHoovicAlive]) {
							_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 6, 1, 0, 0);
							_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 4);
							_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], -2, -2);
							int idx = _scene->_dynamicHotspots.add(102, 348, -1, Common::Rect(256, 57, 256 + 12, 57 + 31));
							_scene->_dynamicHotspots.setPosition(idx, Common::Point(241, 91), FACING_NORTHEAST);
							idx = _scene->_dynamicHotspots.add(102, 348, -1, Common::Rect(242, 79, 242 + 24, 79 + 12));
							_scene->_dynamicHotspots.setPosition(idx, Common::Point(241, 91), FACING_NORTHEAST);
							idx = _scene->_dynamicHotspots.add(229, 348, -1, Common::Rect(231, 88, 231 + 23, 88 + 7));
							_scene->_dynamicHotspots.setPosition(idx, Common::Point(241, 91), FACING_NORTHEAST);
							_scene->changeVariant(1);
						} else {
							if (_throwingObjectId == OBJ_DEAD_FISH) {
								++_globals[kHoovicFishEaten];
								int threshold;
								switch (_game._difficulty) {
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
									int randVal = _vm->getRandomNumber(83, 84);
									_scene->_kernelMessages.add(Common::Point(230, 24), 0xFDFC, 0, 0, 120, _game.getQuote(randVal));
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
						if (_game._player._visible) {
							_game._player._forceRefresh = true;
							_game._player.update();
						}

						_game._player._stepEnabled = true;
						_rexThrowingObject = false;
						break;

					case 5:
					{
						_game._objects.setRoom(OBJ_BURGER, _scene->_currentSceneId);
						_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 0, 0, 0);
						_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 30, 30);
						int idx = _scene->_dynamicHotspots.add(53, 348, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
						_scene->_dynamicHotspots.setPosition(idx, Common::Point(-3, 0), FACING_NORTHEAST);
						_scene->_sequences.addTimer(65, 6);
					}
					break;

					case 6:
					{
						_scene->_sequences.remove(_globals._sequenceIndexes[3]);
						_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 1, 0, 0);
						_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 31, 46);
						int idx = _scene->_dynamicHotspots.add(53, 348, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
						_scene->_dynamicHotspots.setPosition(idx, Common::Point(-3, 0), FACING_NORTHEAST);
						_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 7);
					}
					break;

					case 7:
					{
						_scene->_sequences.remove(_globals._sequenceIndexes[3]);
						_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
						int idx = _scene->_dynamicHotspots.add(53, 348, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
						_scene->_dynamicHotspots.setPosition(idx, Common::Point(-3, 0), FACING_NORTHEAST);
						_vm->_dialogs->show(10915);
					}
					break;

					case 8:
						_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 7, 1, 0, 0);
						_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 5, 16);
						break;

					default:
						break;
					}
					_action._inProgress = false;
					return;
				} else if (_game._objects.isInInventory(_throwingObjectId)) {
					// Nothing.
				}
			}
		}
	}

	if (_action.isAction(VERB_TAKE, NOUN_BURGER) && _game._objects.isInRoom(OBJ_BURGER)) {
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_game._objects.addToInventory(OBJ_BURGER);
	} else if (_action.isAction(VERB_LOOK, NOUN_OCEAN_FLOOR))
		_vm->_dialogs->show(10901);
	else if (_action.isAction(VERB_LOOK, NOUN_CORAL))
		_vm->_dialogs->show(10902);
	else if ((_action.isAction(VERB_TAKE) || _action.isAction(VERB_PULL)) && _action.isObject(NOUN_CORAL))
		_vm->_dialogs->show(10903);
	else if (_action.isAction(VERB_LOOK, NOUN_ROCKS))
		_vm->_dialogs->show(10904);
	else if (_action.isAction(VERB_TAKE, NOUN_ROCKS))
		_vm->_dialogs->show(10905);
	else if (_action.isAction(VERB_LOOK, NOUN_CAVE_WALL))
		_vm->_dialogs->show(10906);
	else if (_action.isAction(VERB_LOOK, NOUN_TUNNEL)) {
		if (_globals[kHoovicAlive])
			_vm->_dialogs->show(10907);
		else
			_vm->_dialogs->show(10913);
	} else if (_action.isAction(VERB_LOOK, NOUN_SMALL_HOLE))
		_vm->_dialogs->show(10908);
	else if (_action.isAction(VERB_LOOK, NOUN_OVERHANG_TO_WEST))
		_vm->_dialogs->show(10911);
	else if (_action.isAction(VERB_PUT, NOUN_SMALL_HOLE))
		_vm->_dialogs->show(10910);
	else if (_action.isAction(VERB_LOOK, NOUN_DEAD_PURPLE_MONSTER))
		_vm->_dialogs->show(10914);
	else
		return;

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
