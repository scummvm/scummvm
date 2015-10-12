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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "mads/mads.h"
#include "mads/action.h"
#include "mads/inventory.h"
#include "mads/resources.h"
#include "mads/scene.h"
#include "mads/staticres.h"

namespace MADS {

void ActionDetails::synchronize(Common::Serializer &s) {
	s.syncAsUint16LE(_verbId);
	s.syncAsUint16LE(_objectNameId);
	s.syncAsUint16LE(_indirectObjectId);
}

void ActionSavedFields::synchronize(Common::Serializer &s) {
	s.syncAsByte(_commandError);
	s.syncAsSint16LE(_commandSource);
	s.syncAsSint16LE(_command);
	s.syncAsSint16LE(_mainObject);
	s.syncAsSint16LE(_secondObject);
	s.syncAsSint16LE(_mainObjectSource);
	s.syncAsSint16LE(_secondObjectSource);
	s.syncAsSint16LE(_articleNumber);
	s.syncAsSint16LE(_lookFlag);
}

/*------------------------------------------------------------------------*/

MADSAction::MADSAction(MADSEngine *vm) : _vm(vm) {
	clear();
	_statusTextIndex = -1;
	_selectedAction = 0;
	_inProgress = false;
	_pickedWord = -1;

	_savedFields._commandSource = CAT_NONE;
	_savedFields._mainObjectSource = CAT_NONE;
	_savedFields._command = -1;
	_savedFields._mainObject = 0;
	_savedFields._secondObject = 0;
	_savedFields._secondObjectSource = CAT_NONE;
	_savedFields._articleNumber = PREP_NONE;
	_savedFields._lookFlag = false;

	_activeAction._verbId = VERB_NONE;
	_activeAction._objectNameId = -1;
	_activeAction._indirectObjectId = -1;
	_savedFields._commandError = false;
	_verbType = VERB_INIT;
	_prepType = PREP_NONE;
}

void MADSAction::clear() {
	_interAwaiting = AWAITING_COMMAND;
	_commandSource = CAT_NONE;
	_mainObjectSource = CAT_NONE;
	_secondObjectSource = CAT_NONE;
	_recentCommandSource = CAT_NONE;
	_articleNumber = 0;
	_lookFlag = false;
	_pointEstablished = 0;
	_statusText.clear();
	_selectedRow = -1;
	_hotspotId = -1;
	_secondObject = -1;
	_recentCommand = -1;
	_action._verbId = VERB_NONE;
	_action._objectNameId = -1;
	_action._indirectObjectId = -1;
	_textChanged = true;
}

void MADSAction::appendVocab(int vocabId, bool capitalize) {
	Common::String vocabStr = _vm->_game->_scene.getVocab(vocabId);
	if (capitalize)
		vocabStr.setChar(toupper(vocabStr[0]), 0);

	_statusText += vocabStr;
	_statusText += " ";
}

void MADSAction::startWalkingDirectly(int walkType) {
	Scene &scene = _vm->_game->_scene;
	Player &player = _vm->_game->_player;

	if (_pointEstablished && (walkType == -3 || _savedFields._command < 0)) {
		player._needToWalk = true;
		player._prepareWalkPos = scene._customDest;
	}
}

void MADSAction::set() {
	Scene &scene = _vm->_game->_scene;
	UserInterface &userInterface = scene._userInterface;
	_statusText = "";

	_action._verbId = VERB_NONE;
	_action._objectNameId = -1;
	_action._indirectObjectId = -1;

	if (_commandSource == CAT_TALK_ENTRY) {
		// Handle showing the conversation selection. Rex at least doesn't actually seem to use this
		if (_selectedRow >= 0) {
			_action._verbId = userInterface._talkIds[_selectedRow];
			Common::String desc = userInterface._talkStrings[_selectedRow];
			if (!desc.empty())
				_statusText = desc;
		}
	} else if (_lookFlag && (_selectedRow == 0)) {
		// Two 'look' actions in succession, so the action becomes 'Look around'
		_statusText = kLookAroundStr;
	} else {
		bool flag = false;
		if ((_commandSource == CAT_INV_VOCAB) && (_selectedRow >= 0)
				&& (_verbType == VERB_THAT) && (_prepType == PREP_NONE)) {
			// Use/to action
			int invIndex = userInterface._selectedInvIndex;
			InventoryObject &objEntry = _vm->_game->_objects.getItem(invIndex);

			_action._objectNameId = objEntry._descId;
			_action._verbId = objEntry._vocabList[_selectedRow]._vocabId;

			// Set up the status text string
			_statusText = kUseStr;
			appendVocab(_action._objectNameId);
			_statusText += kToStr;
			appendVocab(_action._verbId);
		} else {
			// Handling for if an action has been selected
			if (_selectedRow >= 0) {
				if (_commandSource == CAT_COMMAND) {
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
					_statusText += kArticleList[PREP_AT];
					_statusText += " ";
				}
			}

			// Add in any necessary article if necessary
			if ((_hotspotId >= 0) && (_selectedRow >= 0) && (_articleNumber > 0) && (_verbType == VERB_THAT)) {
				flag = true;

				_statusText += kArticleList[_articleNumber];
				_statusText += " ";
			}

			// Handling for hotspot
			if (_hotspotId >= 0) {
				if (_selectedRow < 0) {
					int verbId;

					if (_hotspotId < (int)scene._hotspots.size()) {
						// Get the verb Id from the hotspot
						verbId = scene._hotspots[_hotspotId]._verbId;
					} else {
						// Get the verb Id from the scene object
						verbId = scene._dynamicHotspots.get(_hotspotId - scene._hotspots.size())._verbId;
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

				if ((_mainObjectSource == CAT_INV_LIST) || (_mainObjectSource == CAT_INV_ANIM)) {
					// Get name from given inventory object
					InventoryObject &invObject = _vm->_game->_objects.getItem(_hotspotId);
					_action._objectNameId = invObject._descId;
				} else if (_hotspotId < (int)scene._hotspots.size()) {
					// Get name from scene hotspot
					_action._objectNameId = scene._hotspots[_hotspotId]._vocabId;
				} else {
					// Get name from temporary scene hotspot
					_action._objectNameId = scene._dynamicHotspots.get(_hotspotId - scene._hotspots.size())._descId;
				}
				appendVocab(_action._objectNameId);
			}
		}

		if (_secondObject >= 0) {
			if (_secondObjectSource == CAT_INV_LIST || _secondObjectSource == CAT_INV_ANIM) {
				InventoryObject &invObject = _vm->_game->_objects.getItem(_secondObject);
				_action._indirectObjectId = invObject._descId;
			} else if (_secondObject < (int)scene._hotspots.size()) {
				_action._indirectObjectId = scene._hotspots[_secondObject]._vocabId;
			} else {
				_action._indirectObjectId = scene._dynamicHotspots.get(_secondObject - scene._hotspots.size())._descId;
			}
		}

		if ((_hotspotId >= 0) && (_articleNumber > 0) && !flag) {
			if (_articleNumber == PREP_RELATIONAL) {
				if (_secondObject >= 0) {
					int articleNum = 0;

					if ((_secondObjectSource == 2) || (_secondObjectSource == 5)) {
						InventoryObject &invObject = _vm->_game->_objects.getItem(_secondObject);
						articleNum = invObject._article;
					} else if (_secondObject < (int)scene._hotspots.size()) {
						articleNum = scene._hotspots[_secondObject]._articleNumber;
					} else {
						articleNum = scene._dynamicHotspots.get(_secondObject - scene._hotspots.size())._articleNumber;
					}

					_statusText += kArticleList[articleNum];
				}
			} else if ((_articleNumber != VERB_LOOK) || (_vm->getGameID() != GType_RexNebular) ||
				(_action._indirectObjectId >= 0 && scene.getVocab(_action._indirectObjectId) != kFenceStr)) {
				// Write out the article
				_statusText += kArticleList[_articleNumber];
			} else {
				// Special case for a 'fence' entry in Rex Nebular
				_statusText += kOverStr;
			}

			_statusText += " ";
		}

		// Append object description if necessary
		if (_secondObject >= 0)
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
		if ((_vm->_game->_screenObjects._inputMode == kInputBuildingSentences) ||
				(_vm->_game->_screenObjects._inputMode == kInputLimitedSentences)) {
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
	_savedFields._commandError = false;
	_savedFields._command = _selectedRow;
	_savedFields._mainObject = _hotspotId;
	_savedFields._secondObject = _secondObject;
	_savedFields._articleNumber = _articleNumber;
	_savedFields._commandSource = _commandSource;
	_savedFields._mainObjectSource = _mainObjectSource;
	_savedFields._secondObjectSource = _secondObjectSource;
	_savedFields._lookFlag = _lookFlag;
	_activeAction = _action;

	// Copy the action to be active
	_activeAction = _action;
	_sentence = _statusText;

	if ((_mainObjectSource == CAT_HOTSPOT) && (_secondObjectSource == 4))
		_savedFields._commandError = true;

	player._needToWalk = false;
	int hotspotId = -1;

	if (!_savedFields._lookFlag && (_vm->_game->_screenObjects._inputMode != kInputConversation)) {
		if (_savedFields._mainObjectSource == CAT_HOTSPOT)
			hotspotId = _savedFields._mainObject;
		else if (_secondObjectSource == 4)
			hotspotId = _savedFields._secondObject;

		if (hotspotId >= (int)hotspots.size()) {
			DynamicHotspot &hs = dynHotspots.get(hotspotId - hotspots.size());
			if ((hs._feetPos.x == -1) || (hs._feetPos.x == -3)) {
				startWalkingDirectly(hs._feetPos.x);
			} else if (hs._feetPos.x < 0) {
				player._prepareWalkFacing = hs._facing;
			} else if (_savedFields._commandSource == CAT_NONE || hs._cursor < CURSOR_WAIT) {
				player._needToWalk = true;
				player._prepareWalkPos = hs._feetPos;
			}

			player._prepareWalkFacing = hs._facing;
			hotspotId = -1;
		}
	}

	if (hotspotId >= 0) {
		Hotspot &hs = hotspots[hotspotId];

		if (hs._feetPos.x == -1 || hs._feetPos.x == -3) {
			startWalkingDirectly(hs._feetPos.x);
		} else if (hs._feetPos.x >= 0) {
			if (_savedFields._commandSource == CAT_NONE || hs._cursor < CURSOR_WAIT) {
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

bool MADSAction::isObject(int objectNameId) {
	return _activeAction._objectNameId == objectNameId;
}

bool MADSAction::isTarget(int objectNameId) {
	return _activeAction._indirectObjectId == objectNameId;
}

void MADSAction::checkActionAtMousePos() {
	Scene &scene = _vm->_game->_scene;
	UserInterface &userInterface = scene._userInterface;

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

	if (_vm->_events->_rightMousePressed && _vm->_events->_mouseButtons) {
		switch (userInterface._category) {
		case CAT_COMMAND:
		case CAT_INV_VOCAB:
			return;

		case CAT_INV_LIST:
		case CAT_HOTSPOT:
		case CAT_INV_ANIM:
			if (_interAwaiting != AWAITING_THAT) {
				if (userInterface._selectedActionIndex >= 0) {
					_commandSource = CAT_COMMAND;
					_selectedRow = userInterface._selectedActionIndex;
					_verbType = scene._verbList[_selectedRow]._verbType;
					_prepType = scene._verbList[_selectedRow]._prepType;
					_interAwaiting = AWAITING_THIS;
				} else if (userInterface._selectedItemVocabIdx >= 0) {
					_commandSource = CAT_INV_VOCAB;
					_selectedRow = userInterface._selectedItemVocabIdx;
					int objectId = _vm->_game->_objects._inventoryList[_selectedRow];
					InventoryObject &invObject = _vm->_game->_objects[objectId];

					_verbType = invObject._vocabList[_selectedRow - 1]._verbType;
					_prepType = invObject._vocabList[_selectedRow - 1]._prepType;
					_mainObjectSource = CAT_INV_LIST;
					_hotspotId = userInterface._selectedInvIndex;
					_articleNumber = _prepType;

					if ((_verbType == VERB_THIS && _prepType == PREP_NONE) ||
							(_verbType == VERB_THAT && _prepType != PREP_NONE))
						_interAwaiting = AWAITING_RIGHT_MOUSE;
					else
						_interAwaiting = AWAITING_THAT;
				}
			}
			break;

		default:
			break;
		}
	}

	switch (_interAwaiting) {
	case AWAITING_COMMAND:
		_articleNumber = 0;
		switch (userInterface._category) {
		case CAT_COMMAND:
			_commandSource = CAT_COMMAND;
			_selectedRow = _pickedWord;
			if (_selectedRow >= 0) {
				_verbType = scene._verbList[_selectedRow]._verbType;
				_prepType = scene._verbList[_selectedRow]._prepType;
			}
			break;

		case CAT_INV_VOCAB:
			_commandSource = CAT_INV_VOCAB;
			_selectedRow = _pickedWord;
			if (_selectedRow < 0) {
				_hotspotId = -1;
				_mainObjectSource = CAT_NONE;
			} else {
				InventoryObject &invObject = _vm->_game->_objects.getItem(userInterface._selectedInvIndex);
				_verbType = invObject._vocabList[_selectedRow]._verbType;
				_prepType = invObject._vocabList[_selectedRow]._prepType;
				_hotspotId = userInterface._selectedInvIndex;
				_mainObjectSource = CAT_INV_LIST;

				if (_verbType == VERB_THAT)
					_articleNumber = _prepType;
			}
			break;

		case CAT_HOTSPOT:
			_selectedRow = -1;
			_commandSource = CAT_NONE;
			_mainObjectSource = CAT_HOTSPOT;
			_hotspotId = _pickedWord;
			break;

		case CAT_TALK_ENTRY:
			_commandSource = CAT_TALK_ENTRY;
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
			_mainObjectSource = userInterface._category;
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
			_secondObjectSource = userInterface._category;
			_secondObject = _pickedWord;
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

	if (abortFlag || (_vm->_events->_rightMousePressed && (userInterface._category == CAT_COMMAND ||
			userInterface._category == CAT_INV_VOCAB)))
		return;

	switch (_interAwaiting) {
	case AWAITING_COMMAND:
		switch (userInterface._category) {
		case CAT_COMMAND:
			if (_selectedRow >= 0) {
				if (_verbType == VERB_ONLY)
					_selectedAction = -1;
				else {
					_recentCommand = _selectedRow;
					_recentCommandSource = _commandSource;
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
				if (_verbType != VERB_THIS || _prepType != PREP_NONE) {
					if (_verbType != VERB_THAT || _prepType == PREP_NONE) {
						_interAwaiting = AWAITING_THAT;
						_articleNumber = _prepType;
					} else {
						_articleNumber = _prepType;
						_selectedAction = -1;
					}
				} else {
					_selectedAction = -1;
				}

				_recentCommand = _selectedRow;
				_recentCommandSource = _commandSource;
			}
			break;

		case CAT_HOTSPOT:
			_recentCommand = -1;
			_recentCommandSource = CAT_NONE;

			if (_vm->_events->currentPos().y < MADS_SCENE_HEIGHT) {
				scene._customDest = _vm->_events->currentPos() + scene._posAdjust;
				_selectedAction = -1;
				_pointEstablished = true;
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
				if (_prepType) {
					_articleNumber = _prepType;
					_interAwaiting = AWAITING_THAT;
				} else {
					_selectedAction = -1;
				}

				if (userInterface._category == CAT_HOTSPOT) {
					scene._customDest = _vm->_events->mousePos() + scene._posAdjust;
					_pointEstablished = true;
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
			if (_secondObject >= 0) {
				_selectedAction = -1;

				if (userInterface._category == CAT_HOTSPOT) {
					if (!_pointEstablished) {
						scene._customDest = _vm->_events->mousePos() + scene._posAdjust;
						_pointEstablished = true;
					}
				}
			}
			break;
		default:
			break;
		}
		break;

	default:
		break;
	}
}

void MADSAction::synchronize(Common::Serializer &s) {
	_action.synchronize(s);
	_activeAction.synchronize(s);
	s.syncAsSint16LE(_articleNumber);
	s.syncAsByte(_lookFlag);
	s.syncAsByte(_textChanged);
	s.syncAsSint16LE(_selectedRow);
	s.syncAsSint16LE(_selectedAction);
	s.syncAsSint16LE(_statusTextIndex);
	s.syncAsSint16LE(_hotspotId);
	_savedFields.synchronize(s);

	// TODO: When saving in Rex Village Hut, _senetence size() doesn't match
	// string length. Find out why not
	_sentence = Common::String(_sentence.c_str());
	s.syncString(_sentence);

	s.syncAsSint16LE(_verbType);
	s.syncAsSint16LE(_prepType);
	s.syncAsSint16LE(_commandSource);
	s.syncAsSint16LE(_mainObjectSource);
	s.syncAsSint16LE(_secondObject);
	s.syncAsSint16LE(_secondObjectSource);
	s.syncAsSint16LE(_recentCommandSource);
	s.syncAsSint16LE(_recentCommand);
	s.syncAsSint16LE(_interAwaiting);
	s.syncAsSint16LE(_pickedWord);
	s.syncAsByte(_pointEstablished);
	s.syncAsByte(_inProgress);
}

} // End of namespace MADS
