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
#include "mads/nebular/rooms/dialog.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section2.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int32 _curDialogNode;
	int32 _nextHandsPlace;
	int32 _twinkleAnimationType;
	int32 _twinklesCurrentFrame;
	bool _shouldTalk;
	bool _shouldFaceRex;
	bool _shouldMoveHead;
	bool _stopWalking;
	bool _twinklesTalking;
	bool _twinklesTalk2;
	int32 _doorway;
	char _subQuote2[256];
	Dialog _conv1;
	Dialog _conv2;
	Dialog _conv3;
	Dialog _conv5;
	Dialog _conv6;
	Dialog _conv7;
	Dialog _conv8;
};

static Scratch local;


static void handleTwinklesSpeech(int quoteId, int shiftX, uint32 delay) {
	_scene->_kernelMessages.add(Common::Point(10, 70 + (shiftX * 14)), 0xFDFC, 0, 0, (delay == 0) ? INDEFINITE_TIMEOUT : delay, _game.getQuote(quoteId));
}

static void newNode(int node) {
	local._curDialogNode = node;

	switch (local._curDialogNode) {
	case 1:
		local._conv1.start();
		break;

	case 2:
		local._conv2.start();
		break;

	case 3:
		local._conv3.start();
		break;

	case 5:
		local._conv5.start();
		break;

	case 6:
		local._conv6.start();
		break;

	case 7:
		local._conv7.start();
		break;

	case 8:
		local._conv8.start();
		break;

	default:
		break;
	}
}

static void setDialogNode(int node) {
	switch (node) {
	case 0:
		_scene->_userInterface.setup(kInputBuildingSentences);
		local._shouldFaceRex = false;
		local._shouldTalk = false;
		_game._player._stepEnabled = true;
		local._curDialogNode = 0;
		break;

	case 2:
		switch (_game._trigger) {
		case 1:
			local._nextHandsPlace = 1;
			local._shouldTalk = true;
			_game._player._stepEnabled = false;
			if (local._twinklesTalking) {
				handleTwinklesSpeech(0xB9, -1, 0);
				_scene->_sequences.addTimer(180, 2);
			} else {
				_scene->_sequences.addTimer(6, 1);
			}
			break;

		case 2:
			local._nextHandsPlace = 0;
			handleTwinklesSpeech(186, 0, 0);
			_scene->_sequences.addTimer(180, 3);
			break;

		default:
			local._nextHandsPlace = 0;
			local._shouldTalk = false;
			_game._player._stepEnabled = true;
			newNode(2);
			break;
		}
		break;

	case 3:
		switch (_game._trigger) {
		case 1:
			local._nextHandsPlace = 0;
			local._shouldTalk = true;
			_game._player._stepEnabled = false;
			if (local._twinklesTalking) {
				handleTwinklesSpeech(0xBE, -2, 0);
				_scene->_sequences.addTimer(180, 2);
			} else {
				_scene->_sequences.addTimer(6, 1);
			}
			break;
		case 2:
			local._nextHandsPlace = 2;
			handleTwinklesSpeech(191, -1, 0);
			_scene->_sequences.addTimer(180, 3);
			break;
		case 3:
			local._nextHandsPlace = 0;
			handleTwinklesSpeech(192, 0, 0);
			_scene->_sequences.addTimer(180, 4);
			break;
		default:
			local._shouldTalk = false;
			_game._player._stepEnabled = true;
			newNode(3);
			break;
		}
		break;

	case 4:
		if (_game._trigger == 1) {
			local._nextHandsPlace = 1;
			local._shouldTalk = true;
			_game._player._stepEnabled = false;

			int quote;
			if (_game._storyMode == STORYMODE_NAUGHTY)
				quote = _vm->getRandomNumber(199, 201);
			else
				quote = _vm->getRandomNumber(197, 198);

			if (local._twinklesTalking) {
				handleTwinklesSpeech(quote, 0, 360);
				_scene->_sequences.addTimer(120, 2);
			} else {
				_scene->_sequences.addTimer(6, 1);
			}
		} else {
			_scene->_userInterface.setup(kInputBuildingSentences);
			local._shouldFaceRex = false;
			local._shouldTalk = false;
			_game._player._stepEnabled = true;
		}
		break;

	case 5:
		switch (_game._trigger) {
		case 1:
			local._nextHandsPlace = 2;
			local._shouldTalk = true;
			_game._player._stepEnabled = false;
			if (local._twinklesTalking) {
				handleTwinklesSpeech(0xCA, -1, 0);
				_scene->_sequences.addTimer(180, 2);
			} else {
				_scene->_sequences.addTimer(6, 1);
			}
			break;

		case 2:
			local._nextHandsPlace = 1;
			handleTwinklesSpeech(0xCB, 0, 0);
			_scene->_sequences.addTimer(180, 3);
			break;

		default:
			local._nextHandsPlace = 2;
			local._shouldTalk = false;
			_game._player._stepEnabled = true;
			newNode(5);
			break;
		}
		break;

	case 6:
		switch (_game._trigger) {
		case 1:
			local._nextHandsPlace = 1;
			local._shouldTalk = true;
			_game._player._stepEnabled = false;
			if (local._twinklesTalking) {
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
			local._nextHandsPlace = 1;
			handleTwinklesSpeech(0xD2, 0, 0);
			_scene->_sequences.addTimer(180, 4);
			break;

		default:
			local._nextHandsPlace = 0;
			local._shouldTalk = false;
			_game._player._stepEnabled = true;
			newNode(6);
			break;
		}
		break;

	case 7:
		switch (_game._trigger) {
		case 1:
			local._nextHandsPlace = 2;
			local._shouldTalk = true;
			_game._player._stepEnabled = false;
			if (local._twinklesTalking) {
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
			local._shouldTalk = false;
			_game._player._stepEnabled = true;
			newNode(7);
			break;
		}
		break;

	case 8:
		switch (_game._trigger) {
		case 1:
			local._nextHandsPlace = 2;
			local._shouldTalk = true;
			_game._player._stepEnabled = false;
			if (local._twinklesTalking) {
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
			local._shouldTalk = false;
			_game._player._stepEnabled = true;
			newNode(8);
			break;
		}
		break;


	case 9:
		switch (_game._trigger) {
		case 1:
			local._nextHandsPlace = 0;
			local._shouldTalk = true;
			_game._player._stepEnabled = false;
			if (local._twinklesTalking) {
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
			local._twinkleAnimationType = 2;
			_globals[kCurtainOpen] = true;
			_game._player._visible = false;
			_vm->_palette->lock();
			_scene->_kernelMessages.reset();
			_scene->freeAnimation();
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

static void handleConversation1() {
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

static void handleConversation2() {
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

static void handleConversation3() {
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

static void handleConversation5() {
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

static void handleConversation6() {
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

static void handleConversation7() {
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

static void handleConversation8() {
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

static void handleConversations() {
	if (_game._trigger == 0) {
		_scene->_kernelMessages.reset();
		_game._player._stepEnabled = false;
		const char *curQuote = _game.getQuote(_action._activeAction._verbId);
		if (_scene->_kernelMessages._talkFont->getWidth(curQuote, _scene->_textSpacing) > 200) {
			static char line1[40], line2[40];
			_game.splitQuote(curQuote, line1, line2);
			Common::strcpy_s(local._subQuote2, line2);
			_scene->_kernelMessages.add(Common::Point(0, -14), 0x1110, 34, 0, 240, line1);
			_scene->_sequences.addTimer(60, 50);
		} else {
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 1, 120, curQuote);
		}
	} else if (_game._trigger == 50) {
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 240, local._subQuote2);
		_scene->_sequences.addTimer(180, 1);
	} else {
		if (_game._trigger == 1)
			_scene->_kernelMessages.reset();

		switch (local._curDialogNode) {
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

static void restoreDialogNode(int node, int msgId, int posY) {
	int curQuoteId = msgId;
	int curY = 1 - posY;
	for (int count = 0; count < posY; count++) {
		handleTwinklesSpeech(curQuoteId, curY, 0);
		curY++;
		curQuoteId++;
	}

	newNode(node);
}

static void room_210_init() {
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
		int idx = _scene->_dynamicHotspots.add(words_doorway, words_walk_through, -1, Common::Rect(163, 87, 163 + 19, 87 + 36));
		local._doorway = _scene->_dynamicHotspots.setPosition(idx, Common::Point(168, 127), FACING_NORTH);
		_scene->_dynamicHotspots.setCursor(local._doorway, CURSOR_GO_UP);
	}

	_game.loadQuoteSet(0x5A, 0x73, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB8, 0xB7,
		0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA,
		0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC,
		0xDD, 0xDE, 0xDF, 0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0);

	local._conv1.setup(0x2E, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0);

	if (!_game._visitedScenes._sceneRevisited) {
		local._conv1.set(0x2E, 0xB4, 0xB5, 0xB6, 0xB8, 0);

		if (_game._widepipeCtr >= 2)
			local._conv1.write(0xB7, true);
	}

	bool sceneRevisited = _game._visitedScenes._sceneRevisited;
	local._conv2.setup(0x2F, 0xBC, 0xBB, 0xBD, sceneRevisited ? 0 : -1);
	local._conv3.setup(0x30, 0xC3, 0xC2, 0xC1, 0xC4, sceneRevisited ? 0 : -1);
	local._conv5.setup(0x31, 0xCD, 0xCC, 0xCE, 0xCF, sceneRevisited ? 0 : -1);
	local._conv6.setup(0x32, 0xD3, 0xD4, 0xD5, sceneRevisited ? 0 : -1);
	local._conv7.setup(0x33, 0xD8, 0xDA, 0xD9, 0xDB, 0xDC, sceneRevisited ? 0 : -1);
	local._conv8.setup(0x34, 0xDF, 0xE0, 0xE1, 0xE2, 0xE3, sceneRevisited ? 0 : -1);

	local._twinkleAnimationType = 0;
	local._twinklesCurrentFrame = 0;

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		local._shouldMoveHead = false;
		local._shouldFaceRex = false;
		local._shouldTalk = false;
		local._nextHandsPlace = 0;
		local._twinklesTalking = false;
		local._curDialogNode = 0;
		local._stopWalking = false;
		local._twinklesTalk2 = (_globals[kTwinklesApproached] > 0);
	}

	if (_globals[kTwinklesStatus] == 0) {
		_scene->loadAnimation(formAnimName('A', -1));
		local._twinkleAnimationType = 1;
	} else
		_scene->_hotspots.activate(476, false);

	if (local._curDialogNode) {
		int quote = 0;
		int number = 0;

		switch (local._curDialogNode) {
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

		restoreDialogNode(local._curDialogNode, quote, number);
		if (_scene->_animation[0])
			_scene->_animation[0]->setCurrentFrame(131);
	}

	_vm->_palette->setEntry(252, 63, 63, 10);
	_vm->_palette->setEntry(253, 45, 45, 5);

	section_2_music();
}

static void room_210_daemon() {
	if ((local._twinkleAnimationType == 1) && _scene->_animation[0]) {
		if (local._twinklesCurrentFrame != _scene->_animation[0]->getCurrentFrame()) {
			local._twinklesCurrentFrame = _scene->_animation[0]->getCurrentFrame();
			int reset_frame = -1;
			int random = _vm->getRandomNumber(1, 1000);

			switch (local._twinklesCurrentFrame) {
			case 31:
			case 58:
			case 74:
			case 108:
			case 190:
				if (local._shouldFaceRex)
					reset_frame = 108;
				else if ((random <= 100) || local._shouldMoveHead)
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
				local._twinklesTalking = false;
				if (local._shouldMoveHead) {
					reset_frame = 90;
				} else if (local._twinklesCurrentFrame == 90) {
					if ((random <= 400) || local._shouldFaceRex)
						reset_frame = 99;
					else
						reset_frame = 79;
				}
				break;

			case 98:
				if (local._shouldMoveHead)
					reset_frame = 91;
				break;

			case 99:
				if ((random <= 400) && !local._shouldFaceRex)
					reset_frame = 79;
				break;

			case 124:
			case 136:
			case 174:
			case 143:
			case 152:
				if (local._nextHandsPlace == 3)
					reset_frame = 174;
				else if (!local._shouldFaceRex)
					reset_frame = 174;
				else if (local._nextHandsPlace == 1)
					reset_frame = 136;
				else if (local._nextHandsPlace == 2)
					reset_frame = 152;
				else if (local._shouldTalk)
					reset_frame = 124;
				else
					reset_frame = (random <= 200) ? 131 : 143;
				break;

			case 131:
				if (local._shouldTalk) {
					switch (local._nextHandsPlace) {
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
				if ((local._nextHandsPlace == 0) || !local._shouldFaceRex)
					reset_frame = 141;
				else if (local._nextHandsPlace == 2)
					reset_frame = 152;
				else if (!local._shouldTalk || (local._twinklesCurrentFrame == 141))
					reset_frame = 137;
				break;

			case 153:
			case 169:
				if ((local._nextHandsPlace == 0) || !local._shouldFaceRex)
					reset_frame = 169;
				else if (local._nextHandsPlace == 1)
					reset_frame = 136;
				else if (local._shouldTalk)
					reset_frame = 153;
				else
					reset_frame = 162;
				break;

			case 162:
				if (local._shouldTalk) {
					switch (local._nextHandsPlace) {
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
					local._twinklesCurrentFrame = reset_frame;
				}

				if ((reset_frame == 90) || (reset_frame == 91) || (reset_frame == 124) || (reset_frame == 153)
					|| ((reset_frame == 137) && local._shouldTalk)) {
					local._twinklesTalking = true;
				} else {
					local._twinklesTalking = false;
				}
			}
		}
	}

	if ((local._twinkleAnimationType == 2) && _scene->_animation[0]) {
		if (local._twinklesCurrentFrame != _scene->_animation[0]->getCurrentFrame()) {
			local._twinklesCurrentFrame = _scene->_animation[0]->getCurrentFrame();
			int reset_frame = -1;

			if (local._twinklesCurrentFrame == 53) {
				_scene->_kernelMessages.add(Common::Point(151, 61), 0xFDFC, 32, 70, 180, _game.getQuote(230));
				local._shouldTalk = true;
			} else if ((local._twinklesCurrentFrame == 75) && local._shouldTalk)
				reset_frame = 60;

			if ((reset_frame >= 0) && (reset_frame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(reset_frame);
				local._twinklesCurrentFrame = reset_frame;
			}
		}

		if (_game._trigger == 70)
			local._shouldTalk = false;
	}

	if ((local._twinkleAnimationType == 1) && (_scene->_rails.getNext() > 0)) {
		_game._player.walk(Common::Point(214, 150), FACING_NORTHWEST);
		_scene->_rails.resetNext();
		local._stopWalking = true;
	}
}

static void room_210_pre_parser() {
	local._stopWalking = false;

	if (player_said_2(walk_down, path_to_east))
		_game._player._walkOffScreenSceneId = 211;
}

static void room_210_parser() {
	if (player_said_3(look, binoculars, hut_to_north)) {
		_vm->_dialogs->show(21017);
	} else if (_game._screenObjects._inputMode == kInputConversation) {
		handleConversations();
	} else if (player_said_2(talkto, native_woman) ||
		((_game._player._playerPos == Common::Point(214, 150)) && (_game._player._facing == FACING_NORTHWEST) && (local._twinkleAnimationType == 1) && local._stopWalking)) {
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

			local._shouldFaceRex = true;
			local._nextHandsPlace = 0;
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.addQuote(quote, 1, 120);
		}
		break;

		case 1:
			local._shouldTalk = true;
			if (!local._twinklesTalking) {
				_scene->_sequences.addTimer(6, 1);
			} else {
				if (_globals[kTwinklesApproached] == 0) {
					handleTwinklesSpeech(0xAF, -1, 0);
					handleTwinklesSpeech(0xB0, 0, 0);
				} else if (_globals[kTwinklesApproached] == 1) {
					handleTwinklesSpeech(0xB1, 0, 0);
				} else {
					int quote = local._twinklesTalk2 ? 0xB3 : 0xB2;
					local._twinklesTalk2 = true;
					handleTwinklesSpeech(quote, 0, 0);
				}
				_scene->_sequences.addTimer(60, 3);
			}
			break;

		case 3:
			_game._player._stepEnabled = true;
			local._shouldMoveHead = false;
			local._shouldTalk = false;

			if (_globals[kTwinklesApproached] < 2)
				_globals[kTwinklesApproached]++;

			local._conv1.start();
			local._curDialogNode = 1;
			break;

		default:
			break;
		}
	} else if (player_said_2(give, native_woman) && _game._objects.isInInventory(_game._objects.getIdFromDesc(_action._activeAction._objectNameId))) {
		switch (_game._trigger) {
		case 0:
		{
			int quote = _vm->getRandomNumber(172, 174);
			local._shouldMoveHead = true;
			_game._player._stepEnabled = false;
			handleTwinklesSpeech(quote, 0, 120);
			_scene->_sequences.addTimer(120, 1);
		}
		break;

		case 1:
			_game._player._stepEnabled = true;
			local._shouldMoveHead = false;
			break;

		default:
			break;
		}
	} else if (player_said_2(walk_down, path_to_north) || player_said_2(walk_towards, hut_to_north)) {
		_scene->_nextSceneId = 205;
	} else if (player_said_2(walk_through, doorway)) {
		_scene->_nextSceneId = 215;
	} else if ((player_said_2(pull, curtain) || player_said_2(open, curtain)) && !_globals[kCurtainOpen]) {
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
			local._doorway = _scene->_dynamicHotspots.add(words_doorway, words_walk_through, -1, Common::Rect(163, 87, 163 + 19, 87 + 36));
			_scene->_dynamicHotspots.setPosition(local._doorway, Common::Point(168, 127), FACING_NORTH);
			_scene->_dynamicHotspots.setCursor(local._doorway, CURSOR_GO_UP);
			break;

		default:
			break;
		}
	} else if ((player_said_2(pull, curtain) || player_said_2(close, curtain)) && _globals[kCurtainOpen]) {
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
			_scene->_dynamicHotspots.remove(local._doorway);
			_game._player._stepEnabled = true;
			_globals[kCurtainOpen] = false;
			break;
		default:
			break;
		}
	} else if (player_said_2(look, hut)) {
		if (_globals[kTwinklesStatus] == TWINKLES_GONE) {
			if (_game._storyMode == STORYMODE_NAUGHTY)
				_vm->_dialogs->show(21003);
			else
				_vm->_dialogs->show(21002);
		} else {
			_vm->_dialogs->show(21001);
		}
	} else if (player_said_2(look, bra)) {
		_vm->_dialogs->show(21004);
	} else if (player_said_2(look, hotpants)) {
		_vm->_dialogs->show(21005);
	} else if (player_said_2(take, hotpants) || player_said_2(take, bra)) {
		_vm->_dialogs->show(21006);
	} else if (player_said_2(look, stream)) {
		_vm->_dialogs->show(21007);
	} else if (player_said_2(look, bushy_fern)) {
		_vm->_dialogs->show(21008);
	} else if (player_said_2(look, village_path)) {
		_vm->_dialogs->show(21009);
	} else if (player_said_2(look, native_woman)) {
		_vm->_dialogs->show(21010);
	} else if (player_said_2(shoot, native_woman) || player_said_2(hose_down, native_woman)) {
		_vm->_dialogs->show(21011);
	} else if (player_said_2(look, path_to_north)) {
		_vm->_dialogs->show(21012);
	} else if (player_said_2(look, curtain)) {
		_vm->_dialogs->show(21013);
	} else if (player_said_2(look, clothesline)) {
		_vm->_dialogs->show(21014);
	} else if (player_said_2(take, clothesline)) {
		_vm->_dialogs->show(21015);
	} else if (player_said_2(look, hut_to_north)) {
		_vm->_dialogs->show(21016);
	} else {
		// Not handled
		return;
	}

	_action._inProgress = false;
}

void room_210_synchronize(Common::Serializer &s) {
	s.syncAsSint32LE(local._curDialogNode);
	s.syncAsSint32LE(local._nextHandsPlace);
	s.syncAsSint32LE(local._twinkleAnimationType);
	s.syncAsSint32LE(local._twinklesCurrentFrame);

	s.syncAsByte(local._shouldTalk);
	s.syncAsByte(local._shouldFaceRex);
	s.syncAsByte(local._shouldMoveHead);
	s.syncAsByte(local._stopWalking);
	s.syncAsByte(local._twinklesTalking);
	s.syncAsByte(local._twinklesTalk2);

	s.syncAsSint32LE(local._doorway);

	Common::String subQuote2 = local._subQuote2;
	s.syncString(subQuote2);
	if (s.isLoading())
		Common::strcpy_s(local._subQuote2, subQuote2.c_str());
}

void room_210_preload() {
	room_init_code_pointer = room_210_init;
	room_pre_parser_code_pointer = room_210_pre_parser;
	room_parser_code_pointer = room_210_parser;
	room_daemon_code_pointer = room_210_daemon;

	section_2_walker();
	section_2_interface();
	_scene->addActiveVocab(words_doorway);
	_scene->addActiveVocab(words_walk_through);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
