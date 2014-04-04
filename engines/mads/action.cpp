/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "mads/mads.h"
#include "mads/action.h"
#include "mads/inventory.h"
#include "mads/scene.h"
#include "mads/staticres.h"

namespace MADS {

MADSAction::MADSAction(MADSEngine *vm) : _vm(vm) {
	clear();
	_statusTextIndex = -1;
	_selectedAction = 0;
	_inProgress = false;

	_savedFields._actionMode = KERNEL_TRIGGER_PARSER;
	_savedFields._actionMode2 = KERNEL_TRIGGER_PARSER;
	_savedFields._selectedRow = -1;
	_savedFields._hotspotId = 0;
	_savedFields._v86F3A = 0;
	_savedFields._v86F42 = 0;
	_savedFields._articleNumber = 0;
	_savedFields._lookFlag = false;
}

void MADSAction::clear() {
	_interAwaiting = AWAITING_COMMAND;
	_actionMode = ACTIONMODE_NONE;
	_actionMode2 = ACTIONMODE2_0;
	_v86F42 = 0;
	_recentCommandSource = 0;
	_articleNumber = 0;
	_lookFlag = false;
	_v86F4A = 0;
	_selectedRow = -1;
	_hotspotId = -1;
	_v86F3A = -1;
	_recentCommand = -1;
	_action._verbId = VERB_NONE;
	_action._objectNameId = -1;
	_action._indirectObjectId = -1;
	_textChanged = true;
	_pickedWord = 0;
}

void MADSAction::appendVocab(int vocabId, bool capitalise) {
	Common::String vocabStr = _vm->_game->_scene.getVocab(vocabId);
	if (capitalise)
		vocabStr.setChar(toupper(vocabStr[0]), 0);

	_statusText += vocabStr;
	_statusText += " ";
}

void MADSAction::checkCustomDest(int v) {
	Scene &scene = _vm->_game->_scene;
	Player &player = _vm->_game->_player;

	if (_v86F4A && (v == -3 || _savedFields._selectedRow < 0)) {
		player._needToWalk = true;
		player._prepareWalkPos = scene._customDest;
	}
}

void MADSAction::set() {
	Scene &scene = _vm->_game->_scene;
	UserInterface &userInterface = scene._userInterface;
	bool flag = false;
	_statusText = "";

	_action._verbId = VERB_NONE;
	_action._objectNameId = -1;
	_action._indirectObjectId = -1;

	if (_actionMode == ACTIONMODE_TALK) {
		// Handle showing the conversation selection. Rex at least doesn't actually seem to use this
		if (_selectedRow >= 0) {
			Common::String desc = userInterface._talkStrings[userInterface._talkIds[_selectedRow]];
			if (!desc.empty())
				_statusText = desc;
		}
	} else if (_lookFlag && (_selectedRow == 0)) {
		// Two 'look' actions in succession, so the action becomes 'Look around'
		_statusText = kLookAroundStr;
	} else {
		if ((_actionMode == ACTIONMODE_OBJECT) && (_selectedRow >= 0) && (_flags1 == 2) && (_flags2 == 0)) {
			// Use/to action
			int invIndex = userInterface._selectedInvIndex;
			InventoryObject &objEntry = _vm->_game->_objects.getItem(invIndex);

			_action._objectNameId = objEntry._descId;
			_action._verbId = objEntry._vocabList[_selectedRow]._vocabId;

			// Set up the status text stirng
			_statusText = kUseStr;
			appendVocab(_action._objectNameId);
			_statusText += kToStr;
			appendVocab(_action._verbId);
		} else {
			// Handling for if an action has been selected
			if (_selectedRow >= 0) {
				if (_actionMode == ACTIONMODE_VERB) {
					// Standard verb action
					_action._verbId = scene._verbList[_selectedRow]._id;
				} else {
					// Selected action on an inventory object
					int invIndex = userInterface._selectedInvIndex;
					InventoryObject &objEntry = _vm->_game->_objects.getItem(invIndex);

					_action._verbId = objEntry._vocabList[_selectedRow]._vocabId;
				}

				appendVocab(_action._verbId, true);

				if (_action._verbId == VERB_LOOK) {
					// Add in the word 'add'
					_statusText += kAtStr;
					_statusText += " ";
				}
			}

			// Handling for if a hotspot has been selected/highlighted
			if ((_hotspotId >= 0) && (_selectedRow >= 0) && (_articleNumber > 0) && (_flags1 == 2)) {
				flag = true;

				_statusText += kArticleList[_articleNumber];
				_statusText += " ";
			}

			if (_hotspotId >= 0) {
				if (_selectedRow < 0) {
					int verbId;

					if (_hotspotId < (int)scene._hotspots.size()) {
						// Get the verb Id from the hotspot
						verbId = scene._hotspots[_hotspotId]._verbId;
					} else {
						// Get the verb Id from the scene object
						verbId = scene._dynamicHotspots[_hotspotId - scene._hotspots.size()]._vocabId;
					}

					if (verbId > 0) {
						// Set the specified action
						_action._verbId = verbId;
						appendVocab(_action._verbId, true);
					} else {
						// Default to a standard 'walk to'
						_action._verbId = VERB_WALKTO;
						_statusText += kWalkToStr;
					}
				}

				if ((_actionMode2 == ACTIONMODE2_2) || (_actionMode2 == ACTIONMODE2_5)) {
					// Get name from given inventory object
					InventoryObject &invObject = _vm->_game->_objects.getItem(_hotspotId);
					_action._objectNameId = invObject._descId;
				} else if (_hotspotId < (int)scene._hotspots.size()) {
					// Get name from scene hotspot
					_action._objectNameId = scene._hotspots[_hotspotId]._vocabId;
				} else {
					// Get name from temporary scene hotspot
					_action._objectNameId = scene._dynamicHotspots[_hotspotId - scene._hotspots.size()]._vocabId;
				}
				appendVocab(_action._objectNameId);
			}
		}

		if ((_hotspotId >= 0) && (_articleNumber > 0) && !flag) {
			if (_articleNumber == -1) {
				if (_v86F3A >= 0) {
					int articleNum = 0;

					if ((_v86F42 == 2) || (_v86F42 == 5)) {
						InventoryObject &invObject = _vm->_game->_objects.getItem(_hotspotId);
						articleNum = invObject._article;
					} else if (_v86F3A < (int)scene._hotspots.size()) {
						articleNum = scene._hotspots[_hotspotId]._articleNumber;
					} else {
						articleNum = scene._hotspots[_hotspotId - scene._hotspots.size()]._articleNumber;
					}

					_statusText += kArticleList[articleNum];
					_statusText += " ";
				}
			} else if ((_articleNumber == VERB_LOOK) || (_vm->getGameID() != GType_RexNebular) ||
				(scene._vocabStrings[_action._indirectObjectId] != kFenceStr)) {
				// Write out the article
				_statusText += kArticleList[_articleNumber];
			} else {
				// Special case for a 'fence' entry in Rex Nebular
				_statusText += kOverStr;
			}

			_statusText += " ";
		}

		// Append object description if necessary
		if (_v86F3A >= 0)
			appendVocab(_action._indirectObjectId);

		// Remove any trailing space character
		if (_statusText.hasSuffix(" "))
			_statusText.deleteLastChar();
	}

	_textChanged = true;
}

void MADSAction::refresh() {
	Scene &scene = _vm->_game->_scene;

	// Exit immediately if nothing has changed
	if (!_textChanged)
		return;

	// Remove any old copy of the status text
	if (_statusTextIndex >= 0) {
		scene._textDisplay.expire(_statusTextIndex);
		_statusTextIndex = -1;
	}

	if (!_statusText.empty()) {
		if ((_vm->_game->_screenObjects._v832EC == 0) || (_vm->_game->_screenObjects._v832EC == 2)) {
			Font *font = _vm->_font->getFont(FONT_MAIN);
			int textSpacing = -1;

			int strWidth = font->getWidth(_statusText);
			if (strWidth > MADS_SCREEN_WIDTH) {
				// Too large to fit, so fall back on interface font
				font = _vm->_font->getFont(FONT_INTERFACE);
				strWidth = font->getWidth(_statusText, 0);
				textSpacing = 0;
			}

			// Add a new text display entry to display the status text at the bottom of the screen area
			_statusTextIndex = scene._textDisplay.add(160 - (strWidth / 2),
				MADS_SCENE_HEIGHT + scene._posAdjust.y - 13, 3, textSpacing, _statusText, font);
		}
	}

	_textChanged = false;
}

void MADSAction::startAction() {
	Game &game = *_vm->_game;
	Player &player = game._player;
	Scene &scene = _vm->_game->_scene;
	DynamicHotspots &dynHotspots = scene._dynamicHotspots;
	Hotspots &hotspots = scene._hotspots;

	player.cancelCommand();

	_inProgress = true;
	_v8453A = KERNEL_TRIGGER_PARSER;
	_savedFields._selectedRow = _selectedRow;
	_savedFields._hotspotId = _hotspotId;
	_savedFields._v86F3A = _v86F3A;
	_savedFields._articleNumber = _articleNumber;
	_savedFields._actionMode = _actionMode;
	_savedFields._actionMode2 = _actionMode2;
	_savedFields._v86F42 = _v86F42;
	_savedFields._lookFlag = _lookFlag;
	_activeAction = _action;

	// Copy the action to be active
	_activeAction = _action;
	_sentence = _statusText;

	if ((_actionMode2 == ACTIONMODE2_4) && (_v86F42 == 0))
		_v8453A = -1;

	player._needToWalk = false;
	int hotspotId = -1;

	if (!_savedFields._lookFlag && (_vm->_game->_screenObjects._v832EC != 1)) {
		if (_savedFields._actionMode2 == ACTIONMODE2_4)
			hotspotId = _savedFields._hotspotId;
		else if (_v86F42 == 4)
			hotspotId = _savedFields._v86F3A;

		if (hotspotId >= (int)hotspots.size()) {
			DynamicHotspot &hs = dynHotspots[hotspotId - hotspots.size()];
			if ((hs._feetPos.x == -1) || (hs._feetPos.x == -3)) {
				checkCustomDest(hs._feetPos.x);
			} else if (hs._feetPos.x == 0) {
				player._prepareWalkFacing = hs._facing;
			} else if (_savedFields._actionMode == ACTIONMODE_NONE || hs._cursor >= CURSOR_WAIT) {
				player._needToWalk = true;
				player._prepareWalkPos = hs._feetPos;
			}

			player._prepareWalkFacing = hs._facing;
			hotspotId = -1;
		}
	}

	if (hotspotId >= 0 && hotspotId < (int)hotspots.size()) {
		Hotspot &hs = hotspots[hotspotId];

		if (hs._feetPos.x == -1 || hs._feetPos.x != -3) {
			checkCustomDest(hs._feetPos.x);
		} else if (hs._feetPos.x >= 0) {
			if (_savedFields._actionMode == ACTIONMODE_NONE || hs._cursor < CURSOR_WAIT) {
				player._needToWalk = true;
				player._prepareWalkPos = hs._feetPos;
			}
		}

		player._prepareWalkFacing = hs._facing;
	}

	player._readyToWalk = player._needToWalk;
}

void MADSAction::checkAction() {
	if (isAction(VERB_LOOK) || isAction(VERB_THROW))
		_vm->_game->_player._needToWalk = false;
}

bool MADSAction::isAction(int verbId, int objectNameId, int indirectObjectId) {
	if (_activeAction._verbId != verbId)
		return false;
	if ((objectNameId != 0) && (_activeAction._objectNameId != objectNameId))
		return false;
	if ((indirectObjectId != 0) && (_activeAction._indirectObjectId != indirectObjectId))
		return false;

	return true;
}

void MADSAction::checkActionAtMousePos() {
	Scene &scene = _vm->_game->_scene;
	UserInterface &userInterface = scene._userInterface;
	ScreenObjects &screenObjects = _vm->_game->_screenObjects;

	if ((userInterface._category == CAT_COMMAND || userInterface._category == CAT_INV_VOCAB) &&
			_interAwaiting != AWAITING_COMMAND && _pickedWord >= 0) {
		if (_recentCommandSource == userInterface._category || _recentCommand != _pickedWord ||
			(_interAwaiting != AWAITING_THIS && _interAwaiting != 3))
			clear();
		else if (_selectedRow != 0 || userInterface._category != CAT_COMMAND)
			scene._lookFlag = false;
		else
			scene._lookFlag = true;
	}

	if (_vm->_events->_anyStroke && _vm->_events->_mouseButtons) {
		switch (userInterface._category) {
		case CAT_COMMAND:
		case CAT_INV_VOCAB:
			return;

		case CAT_INV_LIST:
		case CAT_HOTSPOT:
		case CAT_INV_ANIM:
			if (_interAwaiting != AWAITING_THAT) {
				if (userInterface._selectedActionIndex >= 0) {
					_actionMode = ACTIONMODE_VERB;
					_selectedRow = userInterface._selectedActionIndex;
					_flags1 = scene._verbList[_selectedRow]._action1;
					_flags2 = scene._verbList[_selectedRow]._action2;
					_interAwaiting = AWAITING_THIS;
				} else if (userInterface._selectedItemVocabIdx >= 0) {
					_actionMode = ACTIONMODE_OBJECT;
					_selectedRow = userInterface._selectedItemVocabIdx;
					int objectId = _vm->_game->_objects._inventoryList[_selectedRow];
					InventoryObject &invObject = _vm->_game->_objects[objectId];

					_flags1 = invObject._vocabList[_selectedRow - 1]._actionFlags1;
					_flags2 = invObject._vocabList[_selectedRow - 1]._actionFlags2;
					_actionMode2 = ACTIONMODE2_2;
					_hotspotId = userInterface._selectedInvIndex;
					_articleNumber = _flags2;

					if ((_flags1 == 1 && _flags2 == 0) || (_flags1 == 2 && _flags2 != 0))
						_interAwaiting = AWAITING_RIGHT_MOUSE;
					else
						_interAwaiting = AWAITING_THAT;
				}
			}
			break;
		}
	}

	switch (_interAwaiting) {
	case AWAITING_COMMAND:
		_articleNumber = 0;
		switch (userInterface._category) {
		case CAT_COMMAND:
			_actionMode = ACTIONMODE_VERB;
			_selectedRow = _pickedWord;
			if (_selectedRow >= 0) {
				_flags1 = scene._verbList[_selectedRow]._action1;
				_flags2 = scene._verbList[_selectedRow]._action2;
			}
			break;

		case CAT_INV_VOCAB:
			_actionMode = ACTIONMODE_OBJECT;
			_selectedRow = _pickedWord;
			if (_selectedRow < 0) {
				_hotspotId = -1;
				_actionMode2 = ACTIONMODE2_0;
			} else {
				int objectId = _vm->_game->_objects._inventoryList[_selectedRow];
				InventoryObject &invObject = _vm->_game->_objects[objectId];

				_flags1 = invObject._vocabList[_selectedRow - 2]._actionFlags1;
				_flags2 = invObject._vocabList[_selectedRow - 2]._actionFlags2;
				_hotspotId = userInterface._selectedInvIndex;
				_actionMode2 = ACTIONMODE2_2;

				if (_flags1 == 2)
					_articleNumber = _flags2;
			}
			break;

		case CAT_HOTSPOT:
			_selectedRow = -1;
			_actionMode = ACTIONMODE_NONE;
			_actionMode2 = ACTIONMODE2_4;
			_hotspotId = _pickedWord;
			break;

		case CAT_TALK_ENTRY:
			_actionMode = ACTIONMODE_TALK;
			_selectedRow = _pickedWord;
			break;

		default:
			break;
		}
		break;

	case AWAITING_THIS:
		_articleNumber = 0;
		switch (userInterface._category) {
		case CAT_INV_LIST:
		case CAT_HOTSPOT:
		case CAT_INV_ANIM:
			// TODO: We may not need a separate ActionMode2 enum
			_actionMode2 = (ActionMode2)userInterface._category;
			_hotspotId = _pickedWord;
			break;
		default:
			break;
		}
		break;

	case AWAITING_THAT:
		switch (userInterface._category) {
		case CAT_INV_LIST:
		case CAT_HOTSPOT:
		case CAT_INV_ANIM:
			_v86F42 = userInterface._category;
			_v86F3A = _pickedWord;
			break;
		default:
			break;
		}
		break;

	default:
		break;
	}
}

void MADSAction::leftClick() {
	Scene &scene = _vm->_game->_scene;
	UserInterface &userInterface = scene._userInterface;
	ScreenObjects &screenObjects = _vm->_game->_screenObjects;
	bool abortFlag = false;

	if ((userInterface._category == CAT_COMMAND || userInterface._category == CAT_INV_VOCAB) &&
		_interAwaiting != 1 && _pickedWord >= 0 &&
			_recentCommandSource == userInterface._category && _recentCommand == _pickedWord &&
			(_interAwaiting == 2 || userInterface._category == CAT_INV_VOCAB)) {
		abortFlag = true;
		if (_selectedRow == 0 && userInterface._category == CAT_COMMAND) {
			_selectedAction = CAT_COMMAND;
			scene._lookFlag = true;
		} else {
			_selectedAction = CAT_NONE;
			scene._lookFlag = false;
			clear();
		}
	}

	if (abortFlag || (_vm->_events->_anyStroke && (userInterface._category == CAT_COMMAND ||
			userInterface._category == CAT_INV_VOCAB)))
		return;

	switch (_interAwaiting) {
	case AWAITING_COMMAND:
		switch (userInterface._category) {
		case CAT_COMMAND:
			if (_selectedRow >= 0) {
				if (!_flags1) {
					_selectedAction = -1;
				}
				else {
					_recentCommand = _selectedRow;
					_recentCommandSource = _actionMode;
					_interAwaiting = AWAITING_THIS;
				}
			}
			break;

		case CAT_INV_LIST:
			if (_pickedWord >= 0) {
				userInterface.selectObject(_pickedWord);
			}
			break;

		case CAT_INV_VOCAB:
			if (_selectedRow >= 0) {
				if (_flags1 != 1 || _flags2 != 0) {
					if (_flags1 != 2 || _flags2 == 0) {
						_interAwaiting = AWAITING_THAT;
						_articleNumber = _flags2;
					}
					else {
						_articleNumber = _flags2;
						_selectedAction = -1;
					}
				}
				else {
					_selectedAction = -1;
				}

				_recentCommand = _selectedRow;
				_recentCommandSource = _actionMode;
			}
			break;

		case CAT_HOTSPOT:
			_recentCommand = -1;
			_recentCommandSource = 0;

			if (_vm->_events->currentPos().y < MADS_SCENE_HEIGHT) {
				scene._customDest = _vm->_events->currentPos() + scene._posAdjust;
				_selectedAction = -1;
				_v86F4A = true;
			}
			break;

		case CAT_TALK_ENTRY:
			if (_selectedRow >= 0)
				_selectedAction = -1;
			break;

		default:
			break;
		}
		break;

	case AWAITING_THIS:
		switch (userInterface._category) {
		case CAT_INV_LIST:
		case CAT_HOTSPOT:
		case CAT_INV_ANIM:
			if (_hotspotId >= 0) {
				if (_flags2) {
					_articleNumber = _flags2;
					_interAwaiting = AWAITING_THAT;
				}
				else {
					_selectedAction = -1;
				}

				if (userInterface._category == CAT_HOTSPOT) {
					scene._customDest = _vm->_events->mousePos() + scene._posAdjust;
					_v86F4A = true;
				}
			}
			break;
		default:
			break;
		}
		break;

	case AWAITING_THAT:
		switch (userInterface._category) {
		case CAT_INV_LIST:
		case CAT_HOTSPOT:
		case CAT_INV_ANIM:
			if (_v86F3A >= 0) {
				_selectedAction = -1;

				if (userInterface._category == CAT_HOTSPOT) {
					if (!_v86F4A) {
						scene._customDest = _vm->_events->mousePos() + scene._posAdjust;
						_v86F4A = true;
					}
				}
			}
			break;
		default:
			break;
		}
		break;
	}
}

} // End of namespace MADS
