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
#include "mads/madsv2/nebular/rooms/room201.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {

Scene210::Scene210(RexNebularEngine *vm) : Scene2xx(vm) {
	_curDialogNode = -1;
	_nextHandsPlace = 0;
	_twinkleAnimationType = 0;
	_twinklesCurrentFrame = 0;
	_shouldTalk = false;
	_shouldFaceRex = false;
	_shouldMoveHead = false;
	_stopWalking = false;
	_twinklesTalking = false;
	_twinklesTalk2 = false;
	_doorway = 0;
	_subQuote2 = "";
}

void Scene210::synchronize(Common::Serializer &s) {
	Scene2xx::synchronize(s);

	s.syncAsSint32LE(_curDialogNode);
	s.syncAsSint32LE(_nextHandsPlace);
	s.syncAsSint32LE(_twinkleAnimationType);
	s.syncAsSint32LE(_twinklesCurrentFrame);

	s.syncAsByte(_shouldTalk);
	s.syncAsByte(_shouldFaceRex);
	s.syncAsByte(_shouldMoveHead);
	s.syncAsByte(_stopWalking);
	s.syncAsByte(_twinklesTalking);
	s.syncAsByte(_twinklesTalk2);

	s.syncAsSint32LE(_doorway);

	s.syncString(_subQuote2);
}

void Scene210::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(NOUN_DOORWAY);
	_scene->addActiveVocab(VERB_WALK_THROUGH);
}

void Scene210::handleConversations() {
	if (_game._trigger == 0) {
		_scene->_kernelMessages.reset();
		_game._player._stepEnabled = false;
		Common::String curQuote = _game.getQuote(_action._activeAction._verbId);
		if (_scene->_kernelMessages._talkFont->getWidth(curQuote, _scene->_textSpacing) > 200) {
			Common::String subQuote1;
			_game.splitQuote(curQuote, subQuote1, _subQuote2);
			_scene->_kernelMessages.add(Common::Point(0, -14), 0x1110, 34, 0, 240, subQuote1);
			_scene->_sequences.addTimer(60, 50);
		} else {
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 1, 120, curQuote);
		}
	} else if (_game._trigger == 50) {
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 240, _subQuote2);
		_scene->_sequences.addTimer(180, 1);
	} else {
		if (_game._trigger == 1)
			_scene->_kernelMessages.reset();

		switch (_curDialogNode) {
		case 1:
			handleConversation1();
			break;

		case 2:
			handleConversation2();
			break;

		case 3:
			handleConversation3();
			break;

		case 5:
			handleConversation5();
			break;

		case 6:
			handleConversation6();
			break;

		case 7:
			handleConversation7();
			break;

		case 8:
			handleConversation8();
			break;

		default:
			break;
		}
	}
}

void Scene210::handleConversation1() {
	switch (_action._activeAction._verbId) {
	case 180:
		setDialogNode(2);
		break;

	case 181:
		setDialogNode(6);
		break;

	case 182:
		setDialogNode(4);
		break;

	case 183:
		setDialogNode(9);
		break;

	case 184:
		setDialogNode(0);
		break;

	default:
		break;
	}
}

void Scene210::handleConversation2() {
	switch (_action._activeAction._verbId) {
	case 187:
		setDialogNode(3);
		break;

	case 188:
		setDialogNode(4);
		break;

	case 189:
		setDialogNode(0);
		break;

	default:
		break;
	}
}

void Scene210::handleConversation3() {
	switch (_action._activeAction._verbId) {
	case 193:
		setDialogNode(6);
		break;

	case 194:
		setDialogNode(5);
		break;

	case 195:
		setDialogNode(4);
		break;

	case 196:
		setDialogNode(0);
		break;

	default:
		break;
	}
}

void Scene210::handleConversation5() {
	switch (_action._activeAction._verbId) {
	case 204:
		setDialogNode(6);
		break;

	case 205:
	case 206:
		setDialogNode(4);
		break;

	case 207:
		setDialogNode(0);
		break;

	default:
		break;
	}
}

void Scene210::handleConversation6() {
	switch (_action._activeAction._verbId) {
	case 211:
		setDialogNode(7);
		break;

	case 212:
		setDialogNode(4);
		break;

	case 213:
		setDialogNode(0);
		break;

	default:
		break;
	}
}

void Scene210::handleConversation7() {
	switch (_action._activeAction._verbId) {
	case 216:
	case 217:
	case 219:
		setDialogNode(4);
		break;

	case 218:
		setDialogNode(8);
		break;

	case 220:
		setDialogNode(0);
		break;

	default:
		break;
	}
}

void Scene210::handleConversation8() {
	switch (_action._activeAction._verbId) {
	case 223:
	case 224:
		setDialogNode(4);
		break;

	case 225:
	case 226:
		setDialogNode(9);
		break;

	case 227:
		setDialogNode(0);
		break;

	default:
		break;
	}
}

void Scene210::setDialogNode(int node) {
	switch (node) {
	case 0:
		_scene->_userInterface.setup(kInputBuildingSentences);
		_shouldFaceRex = false;
		_shouldTalk = false;
		_game._player._stepEnabled = true;
		_curDialogNode = 0;
		break;

	case 2:
		switch (_game._trigger) {
		case 1:
			_nextHandsPlace = 1;
			_shouldTalk = true;
			_game._player._stepEnabled = false;
			if (_twinklesTalking) {
				handleTwinklesSpeech(0xB9, -1, 0);
				_scene->_sequences.addTimer(180, 2);
			} else {
				_scene->_sequences.addTimer(6, 1);
			}
			break;

		case 2:
			_nextHandsPlace = 0;
			handleTwinklesSpeech(186, 0, 0);
			_scene->_sequences.addTimer(180, 3);
			break;

		default:
			_nextHandsPlace = 0;
			_shouldTalk = false;
			_game._player._stepEnabled = true;
			newNode(2);
			break;
		}
		break;

	case 3:
		switch (_game._trigger) {
		case 1:
			_nextHandsPlace = 0;
			_shouldTalk = true;
			_game._player._stepEnabled = false;
			if (_twinklesTalking) {
				handleTwinklesSpeech(0xBE, -2, 0);
				_scene->_sequences.addTimer(180, 2);
			} else {
				_scene->_sequences.addTimer(6, 1);
			}
			break;
		case 2:
			_nextHandsPlace = 2;
			handleTwinklesSpeech(191, -1, 0);
			_scene->_sequences.addTimer(180, 3);
			break;
		case 3:
			_nextHandsPlace = 0;
			handleTwinklesSpeech(192, 0, 0);
			_scene->_sequences.addTimer(180, 4);
			break;
		default:
			_shouldTalk = false;
			_game._player._stepEnabled = true;
			newNode(3);
			break;
		}
		break;

	case 4:
		if (_game._trigger == 1) {
			_nextHandsPlace = 1;
			_shouldTalk = true;
			_game._player._stepEnabled = false;

			int quote;
			if (_game._storyMode == STORYMODE_NAUGHTY)
				quote = _vm->getRandomNumber(199, 201);
			else
				quote = _vm->getRandomNumber(197, 198);

			if (_twinklesTalking) {
				handleTwinklesSpeech(quote, 0, 360);
				_scene->_sequences.addTimer(120, 2);
			} else {
				_scene->_sequences.addTimer(6, 1);
			}
		} else {
			_scene->_userInterface.setup(kInputBuildingSentences);
			_shouldFaceRex = false;
			_shouldTalk = false;
			_game._player._stepEnabled = true;
		}
		break;

	case 5:
		switch (_game._trigger) {
		case 1:
			_nextHandsPlace = 2;
			_shouldTalk = true;
			_game._player._stepEnabled = false;
			if (_twinklesTalking) {
				handleTwinklesSpeech(0xCA, -1, 0);
				_scene->_sequences.addTimer(180, 2);
			} else {
				_scene->_sequences.addTimer(6, 1);
			}
			break;

		case 2:
			_nextHandsPlace = 1;
			handleTwinklesSpeech(0xCB, 0, 0);
			_scene->_sequences.addTimer(180, 3);
			break;

		default:
			_nextHandsPlace = 2;
			_shouldTalk = false;
			_game._player._stepEnabled = true;
			newNode(5);
			break;
		}
		break;

	case 6:
		switch (_game._trigger) {
		case 1:
			_nextHandsPlace = 1;
			_shouldTalk = true;
			_game._player._stepEnabled = false;
			if (_twinklesTalking) {
				handleTwinklesSpeech(0xD0, -2, 0);
				_scene->_sequences.addTimer(180, 2);
			} else {
				_scene->_sequences.addTimer(6, 1);
			}
			break;

		case 2:
			handleTwinklesSpeech(0xD1, -1, 0);
			_scene->_sequences.addTimer(180, 3);
			break;

		case 3:
			_nextHandsPlace = 1;
			handleTwinklesSpeech(0xD2, 0, 0);
			_scene->_sequences.addTimer(180, 4);
			break;

		default:
			_nextHandsPlace = 0;
			_shouldTalk = false;
			_game._player._stepEnabled = true;
			newNode(6);
			break;
		}
		break;

	case 7:
		switch (_game._trigger) {
		case 1:
			_nextHandsPlace = 2;
			_shouldTalk = true;
			_game._player._stepEnabled = false;
			if (_twinklesTalking) {
				handleTwinklesSpeech(0xD6, -1, 0);
				_scene->_sequences.addTimer(180, 2);
			} else {
				_scene->_sequences.addTimer(6, 1);
			}
			break;

		case 2:
			handleTwinklesSpeech(0xD7, 0, 0);
			_scene->_sequences.addTimer(180, 3);
			break;

		default:
			_shouldTalk = false;
			_game._player._stepEnabled = true;
			newNode(7);
			break;
		}
		break;

	case 8:
		switch (_game._trigger) {
		case 1:
			_nextHandsPlace = 2;
			_shouldTalk = true;
			_game._player._stepEnabled = false;
			if (_twinklesTalking) {
				handleTwinklesSpeech(0xDD, -1, 0);
				_scene->_sequences.addTimer(180, 2);
			} else {
				_scene->_sequences.addTimer(6, 1);
			}
			break;

		case 2:
			handleTwinklesSpeech(0xDE, 0, 0);
			_scene->_sequences.addTimer(180, 3);
			break;

		default:
			_shouldTalk = false;
			_game._player._stepEnabled = true;
			newNode(8);
			break;
		}
		break;


	case 9:
		switch (_game._trigger) {
		case 1:
			_nextHandsPlace = 0;
			_shouldTalk = true;
			_game._player._stepEnabled = false;
			if (_twinklesTalking) {
				_scene->_userInterface.emptyConversationList();
				_scene->_userInterface.setup(kInputConversation);
				handleTwinklesSpeech(0xE4, -1, 0);
				_scene->_sequences.addTimer(180, 2);
			} else {
				_scene->_sequences.addTimer(6, 1);
			}
			break;

		case 2:
			handleTwinklesSpeech(0xE5, 0, 0);
			_scene->_sequences.addTimer(180, 3);
			break;

		case 3:
			_twinkleAnimationType = 2;
			_globals[kCurtainOpen] = true;
			_game._player._visible = false;
			_vm->_palette->lock();
			_scene->_kernelMessages.reset();
			_scene->freeAnimation();
			_scene->_animation[0] = nullptr;
			_scene->resetScene();

			_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('c', -1));
			_game.loadQuoteSet(0xE6, 0xE9, 0xEA, 0xE7, 0xE8, 0);
			_scene->loadAnimation(formAnimName('B', -1), 4);
			break;

		case 4:
		{
			_globals._spriteIndexes[8] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 5);
			_scene->_sequences.setDepth(_globals._spriteIndexes[8], 1);

			int msgIndex = _scene->_kernelMessages.add(Common::Point(160, 20), 0x1110, 32, 5, 180, _game.getQuote(231));
			_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
		}
		break;

		case 5:
		{
			int msgIndex = _scene->_kernelMessages.add(Common::Point(160, 40), 0xFDFC, 32, 6, 180, _game.getQuote(233));
			_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
		}
		break;

		case 6:
		{
			int msgIndex = _scene->_kernelMessages.add(Common::Point(160, 60), 0x1110, 32, 7, 180, _game.getQuote(232));
			_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
		}
		break;

		case 7:
		{
			int msgIndex = _scene->_kernelMessages.add(Common::Point(160, 80), 0xFDFC, 32, 8, 180, _game.getQuote(234));
			_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
		}
		break;

		case 8:
			_globals[kTwinklesStatus] = TWINKLES_GONE;
			_scene->_nextSceneId = 216;
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}
}

void Scene210::handleTwinklesSpeech(int quoteId, int shiftX, uint32 delay) {
	_scene->_kernelMessages.add(Common::Point(10, 70 + (shiftX * 14)), 0xFDFC, 0, 0, (delay == 0) ? INDEFINITE_TIMEOUT : delay, _game.getQuote(quoteId));
}

void Scene210::newNode(int node) {
	_curDialogNode = node;

	switch (_curDialogNode) {
	case 1:
		_conv1.start();
		break;

	case 2:
		_conv2.start();
		break;

	case 3:
		_conv3.start();
		break;

	case 5:
		_conv5.start();
		break;

	case 6:
		_conv6.start();
		break;

	case 7:
		_conv7.start();
		break;

	case 8:
		_conv8.start();
		break;

	default:
		break;
	}
}

void Scene210::restoreDialogNode(int node, int msgId, int posY) {
	int curQuoteId = msgId;
	int curY = 1 - posY;
	for (int count = 0; count < posY; count++) {
		handleTwinklesSpeech(curQuoteId, curY, 0);
		curY++;
		curQuoteId++;
	}

	newNode(node);
}

void Scene210::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('c', -1));

	if (!_game._visitedScenes._sceneRevisited)
		_globals[kCurtainOpen] = 0;

	if (_scene->_priorSceneId == 205)
		_game._player._playerPos = Common::Point(277, 56);
	else if (_scene->_priorSceneId == 215) {
		_game._player._playerPos = Common::Point(168, 128);
		_game._player._facing = FACING_SOUTH;
		_globals[kCurtainOpen] = true;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(308, 132);

	if (!_globals[kCurtainOpen]) {
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 5);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 5);
	} else {
		int idx = _scene->_dynamicHotspots.add(112, 395, -1, Common::Rect(163, 87, 163 + 19, 87 + 36));
		_doorway = _scene->_dynamicHotspots.setPosition(idx, Common::Point(168, 127), FACING_NORTH);
		_scene->_dynamicHotspots.setCursor(_doorway, CURSOR_GO_UP);
	}

	_game.loadQuoteSet(0x5A, 0x73, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB8, 0xB7,
		0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA,
		0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC,
		0xDD, 0xDE, 0xDF, 0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0);

	_conv1.setup(0x2E, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0);

	if (!_game._visitedScenes._sceneRevisited) {
		_conv1.set(0x2E, 0xB4, 0xB5, 0xB6, 0xB8, 0);

		if (_game._widepipeCtr >= 2)
			_conv1.write(0xB7, true);
	}

	bool sceneRevisited = _game._visitedScenes._sceneRevisited;
	_conv2.setup(0x2F, 0xBC, 0xBB, 0xBD, sceneRevisited ? 0 : -1);
	_conv3.setup(0x30, 0xC3, 0xC2, 0xC1, 0xC4, sceneRevisited ? 0 : -1);
	_conv5.setup(0x31, 0xCD, 0xCC, 0xCE, 0xCF, sceneRevisited ? 0 : -1);
	_conv6.setup(0x32, 0xD3, 0xD4, 0xD5, sceneRevisited ? 0 : -1);
	_conv7.setup(0x33, 0xD8, 0xDA, 0xD9, 0xDB, 0xDC, sceneRevisited ? 0 : -1);
	_conv8.setup(0x34, 0xDF, 0xE0, 0xE1, 0xE2, 0xE3, sceneRevisited ? 0 : -1);

	_twinkleAnimationType = 0;
	_twinklesCurrentFrame = 0;

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_shouldMoveHead = false;
		_shouldFaceRex = false;
		_shouldTalk = false;
		_nextHandsPlace = 0;
		_twinklesTalking = false;
		_curDialogNode = 0;
		_stopWalking = false;
		_twinklesTalk2 = (_globals[kTwinklesApproached] > 0);
	}

	if (_globals[kTwinklesStatus] == 0) {
		_scene->loadAnimation(formAnimName('A', -1));
		_twinkleAnimationType = 1;
	} else
		_scene->_hotspots.activate(476, false);

	if (_curDialogNode) {
		int quote = 0;
		int number = 0;

		switch (_curDialogNode) {
		case 1:
			quote = 0xB3;
			number = 1;
			break;
		case 2:
			quote = 0xB9;
			number = 2;
			break;
		case 3:
			quote = 0xBE;
			number = 3;
			break;
		case 5:
			quote = 0xCA;
			number = 2;
			break;
		case 6:
			quote = 0xD0;
			number = 3;
			break;
		case 7:
			quote = 0xD6;
			number = 2;
			break;
		case 8:
			quote = 0xDD;
			number = 2;
			break;
		default:
			break;
		}

		restoreDialogNode(_curDialogNode, quote, number);
		if (_scene->_animation[0])
			_scene->_animation[0]->setCurrentFrame(131);
	}

	_vm->_palette->setEntry(252, 63, 63, 10);
	_vm->_palette->setEntry(253, 45, 45, 5);

	sceneEntrySound();
}

void Scene210::step() {
	if ((_twinkleAnimationType == 1) && _scene->_animation[0]) {
		if (_twinklesCurrentFrame != _scene->_animation[0]->getCurrentFrame()) {
			_twinklesCurrentFrame = _scene->_animation[0]->getCurrentFrame();
			int reset_frame = -1;
			int random = _vm->getRandomNumber(1, 1000);

			switch (_twinklesCurrentFrame) {
			case 31:
			case 58:
			case 74:
			case 108:
			case 190:
				if (_shouldFaceRex)
					reset_frame = 108;
				else if ((random <= 100) || _shouldMoveHead)
					reset_frame = 74;
				else if (random <= 300)
					reset_frame = 58;
				else if (random <= 500)
					reset_frame = 31;
				else
					reset_frame = 0;
				break;

			case 80:
			case 90:
				_twinklesTalking = false;
				if (_shouldMoveHead) {
					reset_frame = 90;
				} else if (_twinklesCurrentFrame == 90) {
					if ((random <= 400) || _shouldFaceRex)
						reset_frame = 99;
					else
						reset_frame = 79;
				}
				break;

			case 98:
				if (_shouldMoveHead)
					reset_frame = 91;
				break;

			case 99:
				if ((random <= 400) && !_shouldFaceRex)
					reset_frame = 79;
				break;

			case 124:
			case 136:
			case 174:
			case 143:
			case 152:
				if (_nextHandsPlace == 3)
					reset_frame = 174;
				else if (!_shouldFaceRex)
					reset_frame = 174;
				else if (_nextHandsPlace == 1)
					reset_frame = 136;
				else if (_nextHandsPlace == 2)
					reset_frame = 152;
				else if (_shouldTalk)
					reset_frame = 124;
				else
					reset_frame = (random <= 200) ? 131 : 143;
				break;

			case 131:
				if (_shouldTalk) {
					switch (_nextHandsPlace) {
					case 1:
						reset_frame = 136;
						break;
					case 2:
						reset_frame = 152;
						break;
					default:
						reset_frame = 124;
						break;
					}
				}
				break;

			case 138:
			case 141:
				if ((_nextHandsPlace == 0) || !_shouldFaceRex)
					reset_frame = 141;
				else if (_nextHandsPlace == 2)
					reset_frame = 152;
				else if (!_shouldTalk || (_twinklesCurrentFrame == 141))
					reset_frame = 137;
				break;

			case 153:
			case 169:
				if ((_nextHandsPlace == 0) || !_shouldFaceRex)
					reset_frame = 169;
				else if (_nextHandsPlace == 1)
					reset_frame = 136;
				else if (_shouldTalk)
					reset_frame = 153;
				else
					reset_frame = 162;
				break;

			case 162:
				if (_shouldTalk) {
					switch (_nextHandsPlace) {
					case 0:
						reset_frame = 124;
						break;
					case 1:
						reset_frame = 136;
						break;
					default:
						reset_frame = 153;
						break;
					}
				}
				break;

			default:
				break;
			}

			if (reset_frame >= 0) {
				if (reset_frame != _scene->_animation[0]->getCurrentFrame()) {
					_scene->_animation[0]->setCurrentFrame(reset_frame);
					_twinklesCurrentFrame = reset_frame;
				}

				if ((reset_frame == 90) || (reset_frame == 91) || (reset_frame == 124) || (reset_frame == 153)
					|| ((reset_frame == 137) && _shouldTalk)) {
					_twinklesTalking = true;
				} else {
					_twinklesTalking = false;
				}
			}
		}
	}

	if ((_twinkleAnimationType == 2) && _scene->_animation[0]) {
		if (_twinklesCurrentFrame != _scene->_animation[0]->getCurrentFrame()) {
			_twinklesCurrentFrame = _scene->_animation[0]->getCurrentFrame();
			int reset_frame = -1;

			if (_twinklesCurrentFrame == 53) {
				_scene->_kernelMessages.add(Common::Point(151, 61), 0xFDFC, 32, 70, 180, _game.getQuote(230));
				_shouldTalk = true;
			} else if ((_twinklesCurrentFrame == 75) && _shouldTalk)
				reset_frame = 60;

			if ((reset_frame >= 0) && (reset_frame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(reset_frame);
				_twinklesCurrentFrame = reset_frame;
			}
		}

		if (_game._trigger == 70)
			_shouldTalk = false;
	}

	if ((_twinkleAnimationType == 1) && (_scene->_rails.getNext() > 0)) {
		_game._player.walk(Common::Point(214, 150), FACING_NORTHWEST);
		_scene->_rails.resetNext();
		_stopWalking = true;
	}
}

void Scene210::preActions() {
	_stopWalking = false;

	if (_action.isAction(VERB_WALK_DOWN, NOUN_PATH_TO_EAST))
		_game._player._walkOffScreenSceneId = 211;
}

void Scene210::actions() {
	if (_action.isAction(VERB_LOOK, NOUN_BINOCULARS, NOUN_HUT_TO_NORTH)) {
		_vm->_dialogs->show(21017);
	} else if (_game._screenObjects._inputMode == kInputConversation) {
		handleConversations();
	} else if (_action.isAction(VERB_TALKTO, NOUN_NATIVE_WOMAN) ||
		((_game._player._playerPos == Common::Point(214, 150)) && (_game._player._facing == FACING_NORTHWEST) && (_twinkleAnimationType == 1) && _stopWalking)) {
		switch (_game._trigger) {
		case 0:
		{
			_game._player._stepEnabled = false;
			int quote;
			if (_globals[kTwinklesApproached] == 0)
				quote = 90;
			else if (_globals[kTwinklesApproached] == 1)
				quote = 115;
			else
				quote = 171;

			_shouldFaceRex = true;
			_nextHandsPlace = 0;
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.addQuote(quote, 1, 120);
		}
		break;

		case 1:
			_shouldTalk = true;
			if (!_twinklesTalking) {
				_scene->_sequences.addTimer(6, 1);
			} else {
				if (_globals[kTwinklesApproached] == 0) {
					handleTwinklesSpeech(0xAF, -1, 0);
					handleTwinklesSpeech(0xB0, 0, 0);
				} else if (_globals[kTwinklesApproached] == 1) {
					handleTwinklesSpeech(0xB1, 0, 0);
				} else {
					int quote = _twinklesTalk2 ? 0xB3 : 0xB2;
					_twinklesTalk2 = true;
					handleTwinklesSpeech(quote, 0, 0);
				}
				_scene->_sequences.addTimer(60, 3);
			}
			break;

		case 3:
			_game._player._stepEnabled = true;
			_shouldMoveHead = false;
			_shouldTalk = false;

			if (_globals[kTwinklesApproached] < 2)
				_globals[kTwinklesApproached]++;

			_conv1.start();
			_curDialogNode = 1;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_GIVE, NOUN_NATIVE_WOMAN) && _game._objects.isInInventory(_game._objects.getIdFromDesc(_action._activeAction._objectNameId))) {
		switch (_game._trigger) {
		case 0:
		{
			int quote = _vm->getRandomNumber(172, 174);
			_shouldMoveHead = true;
			_game._player._stepEnabled = false;
			handleTwinklesSpeech(quote, 0, 120);
			_scene->_sequences.addTimer(120, 1);
		}
		break;

		case 1:
			_game._player._stepEnabled = true;
			_shouldMoveHead = false;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_WALK_DOWN, NOUN_PATH_TO_NORTH) || _action.isAction(VERB_WALK_TOWARDS, NOUN_HUT_TO_NORTH)) {
		_scene->_nextSceneId = 205;
	} else if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOORWAY)) {
		_scene->_nextSceneId = 215;
	} else if ((_action.isAction(VERB_PULL, NOUN_CURTAIN) || _action.isAction(VERB_OPEN, NOUN_CURTAIN)) && !_globals[kCurtainOpen]) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 12, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 5);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
			_game._player._stepEnabled = true;
			_globals[kCurtainOpen] = true;
			_doorway = _scene->_dynamicHotspots.add(112, 395, -1, Common::Rect(163, 87, 163 + 19, 87 + 36));
			_scene->_dynamicHotspots.setPosition(_doorway, Common::Point(168, 127), FACING_NORTH);
			_scene->_dynamicHotspots.setCursor(_doorway, CURSOR_GO_UP);
			break;

		default:
			break;
		}
	} else if ((_action.isAction(VERB_PULL, NOUN_CURTAIN) || _action.isAction(VERB_CLOSE, NOUN_CURTAIN)) && _globals[kCurtainOpen]) {
		switch (_game._trigger) {
		case 0:
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_game._player._stepEnabled = false;
			_game._player._facing = FACING_NORTH;
			_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 12, 0, 0, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 5);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			_game._player._stepEnabled = false;
			break;
		case 1:
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 5);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 5);
			_scene->_sequences.addTimer(48, 2);
			break;
		case 2:
			_scene->_dynamicHotspots.remove(_doorway);
			_game._player._stepEnabled = true;
			_globals[kCurtainOpen] = false;
			break;
		default:
			break;
		}
	} else if (_action.isAction(VERB_LOOK, NOUN_HUT)) {
		if (_globals[kTwinklesStatus] == TWINKLES_GONE) {
			if (_game._storyMode == STORYMODE_NAUGHTY)
				_vm->_dialogs->show(21003);
			else
				_vm->_dialogs->show(21002);
		} else {
			_vm->_dialogs->show(21001);
		}
	} else if (_action.isAction(VERB_LOOK, NOUN_BRA)) {
		_vm->_dialogs->show(21004);
	} else if (_action.isAction(VERB_LOOK, NOUN_HOTPANTS)) {
		_vm->_dialogs->show(21005);
	} else if (_action.isAction(VERB_TAKE, NOUN_HOTPANTS) || _action.isAction(VERB_TAKE, NOUN_BRA)) {
		_vm->_dialogs->show(21006);
	} else if (_action.isAction(VERB_LOOK, NOUN_STREAM)) {
		_vm->_dialogs->show(21007);
	} else if (_action.isAction(VERB_LOOK, NOUN_BUSHY_FERN)) {
		_vm->_dialogs->show(21008);
	} else if (_action.isAction(VERB_LOOK, NOUN_VILLAGE_PATH)) {
		_vm->_dialogs->show(21009);
	} else if (_action.isAction(VERB_LOOK, NOUN_NATIVE_WOMAN)) {
		_vm->_dialogs->show(21010);
	} else if (_action.isAction(VERB_SHOOT, NOUN_NATIVE_WOMAN) || _action.isAction(VERB_HOSE_DOWN, NOUN_NATIVE_WOMAN)) {
		_vm->_dialogs->show(21011);
	} else if (_action.isAction(VERB_LOOK, NOUN_PATH_TO_NORTH)) {
		_vm->_dialogs->show(21012);
	} else if (_action.isAction(VERB_LOOK, NOUN_CURTAIN)) {
		_vm->_dialogs->show(21013);
	} else if (_action.isAction(VERB_LOOK, NOUN_CLOTHESLINE)) {
		_vm->_dialogs->show(21014);
	} else if (_action.isAction(VERB_TAKE, NOUN_CLOTHESLINE)) {
		_vm->_dialogs->show(21015);
	} else if (_action.isAction(VERB_LOOK, NOUN_HUT_TO_NORTH)) {
		_vm->_dialogs->show(21016);
	} else {
		// Not handled
		return;
	}

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
