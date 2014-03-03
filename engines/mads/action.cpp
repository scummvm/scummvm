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
#include "mads/scene.h"

namespace MADS {

MADSAction::MADSAction(MADSEngine *vm) : _vm(vm) {
	clear();
	_currentAction = VERB_NONE;
	_startWalkFlag = false;
	_statusTextIndex = -1;
	_selectedAction = 0;
	_inProgress = false;
}

void MADSAction::clear() {
	_v83338 = 1;
	_actionMode = ACTMODE_NONE;
	_actionMode2 = ACTMODE2_0;
	_v86F42 = 0;
	_v86F4E = 0;
	_articleNumber = 0;
	_lookFlag = false;
	_v86F4A = 0;
	_selectedRow = -1;
	_hotspotId = -1;
	_v86F3A = -1;
	_v86F4C = -1;
	_action.verbId = -1;
	_action.objectNameId = -1;
	_action.indirectObjectId = -1;
	_textChanged = true;
	_walkFlag = false;
}

void MADSAction::appendVocab(int vocabId, bool capitalise) {
	/*
	char *s = _statusText + strlen(_statusText);
	vocabStr = _madsVm->globals()->getVocab(vocabId);
	strcpy(s, vocabStr);
	if (capitalise)
		*s = toupper(*s);

	strcat(s, " ");
	*/
}

void MADSAction::set() {
	/*
	int hotspotCount = _madsVm->scene()->getSceneResources().hotspots->size();
	bool flag = false; // FIXME: unused
	strcpy(_statusText, "");

	_currentAction = -1;
	_action.objectNameId = -1;
	_action.indirectObjectId = -1;

	if (_actionMode == ACTMODE_TALK) {
		// Handle showing the conversation selection. Rex at least doesn't actually seem to use this
		if (_selectedRow >= 0) {
			const char *desc = _madsVm->_converse[_selectedRow].desc;
			if (desc)
				strcpy(_statusText, desc);
		}
	} else if (_lookFlag && (_selectedRow == 0)) {
		// Two 'look' actions in succession, so the action becomes 'Look around'
		strcpy(_statusText, lookAroundStr);
	} else {
		if ((_actionMode == ACTMODE_OBJECT) && (_selectedRow >= 0) && (_flags1 == 2) && (_flags2 == 0)) {
			// Use/to action
			int selectedObject = _madsVm->scene()->getInterface()->getSelectedObject();
			MadsObject *objEntry = _madsVm->globals()->getObject(selectedObject);

			_action.objectNameId = objEntry->_descId;
			_currentAction = objEntry->_vocabList[_selectedRow].vocabId;

			// Set up the status text stirng
			strcpy(_statusText, useStr);
			appendVocab(_action.objectNameId);
			strcpy(_statusText, toStr);
			appendVocab(_currentAction);
		} else {
			// Handling for if an action has been selected
			if (_selectedRow >= 0) {
				if (_actionMode == ACTMODE_VERB) {
					// Standard verb action
					_currentAction = verbList[_selectedRow].verb;
				} else {
					// Selected action on an inventory object
					int selectedObject = _madsVm->scene()->getInterface()->getSelectedObject();
					MadsObject *objEntry = _madsVm->globals()->getObject(selectedObject);

					_currentAction = objEntry->_vocabList[_selectedRow].vocabId;
				}

				appendVocab(_currentAction, true);

				if (_currentAction == kVerbLook) {
					// Add in the word 'add'
					strcat(_statusText, atStr);
					strcat(_statusText, " ");
				}
			}

			// Handling for if a hotspot has been selected/highlighted
			if ((_hotspotId >= 0) && (_selectedRow >= 0) && (_articleNumber > 0) && (_flags1 == 2)) {
				flag = true;

				strcat(_statusText, englishMADSArticleList[_articleNumber]);
				strcat(_statusText, " ");
			}

			if (_hotspotId >= 0) {
				if (_selectedRow < 0) {
					int verbId;

					if (_hotspotId < hotspotCount) {
						// Get the verb Id from the hotspot
						verbId = (*_madsVm->scene()->getSceneResources().hotspots)[_hotspotId].getVerbID();
					} else {
						// Get the verb Id from the scene object
						verbId = (*_madsVm->scene()->getSceneResources().dynamicHotspots)[_hotspotId - hotspotCount].getVerbID();
					}

					if (verbId > 0) {
						// Set the specified action
						_currentAction = verbId;
						appendVocab(_currentAction, true);
					} else {
						// Default to a standard 'walk to'
						_currentAction = kVerbWalkTo;
						strcat(_statusText, walkToStr);
					}
				}

				if ((_actionMode2 == ACTMODE2_2) || (_actionMode2 == ACTMODE2_5)) {
					// Get name from given inventory object
					int objectId = _madsVm->scene()->getInterface()->getInventoryObject(_hotspotId);
					_action.objectNameId = _madsVm->globals()->getObject(objectId)->_descId;
				} else if (_hotspotId < hotspotCount) {
					// Get name from scene hotspot
					_action.objectNameId = (*_madsVm->scene()->getSceneResources().hotspots)[_hotspotId].getVocabID();
				} else {
					// Get name from temporary scene hotspot
					_action.objectNameId = (*_madsVm->scene()->getSceneResources().dynamicHotspots)[_hotspotId].getVocabID();
				}
				appendVocab(_action.objectNameId);
			}
		}

		if ((_hotspotId >= 0) && (_articleNumber > 0) && !flag) {
			if (_articleNumber == -1) {
				if (_v86F3A >= 0) {
					int articleNum = 0;

					if ((_v86F42 == 2) || (_v86F42 == 5)) {
						int objectId = _madsVm->scene()->getInterface()->getInventoryObject(_hotspotId);
						articleNum = _madsVm->globals()->getObject(objectId)->_article;
					} else if (_v86F3A < hotspotCount) {
						articleNum = (*_madsVm->scene()->getSceneResources().hotspots)[_hotspotId].getArticle();
					} else {

					}
				}

			} else if ((_articleNumber == kVerbLook) || (_vm->getGameType() != GType_RexNebular) ||
				(strcmp(_madsVm->globals()->getVocab(_action.indirectObjectId), fenceStr) != 0)) {
				// Write out the article
				strcat(_statusText, englishMADSArticleList[_articleNumber]);
			} else {
				// Special case for a 'fence' entry in Rex Nebular
				strcat(_statusText, overStr);
			}

			strcat(_statusText, " ");
		}

		// Append object description if necessary
		if (_v86F3A >= 0)
			appendVocab(_action.indirectObjectId);

		// Remove any trailing space character
		int statusLen = strlen(_statusText);
		if ((statusLen > 0) && (_statusText[statusLen - 1] == ' '))
			_statusText[statusLen - 1] = '\0';
	}

	_textChanged = true;
	*/
}

void MADSAction::refresh() {
	/*
	// Exit immediately if nothing has changed
	if (!_textChanged)
		return;

	// Remove any old copy of the status text
	if (_statusTextIndex >= 0) {
		_owner._textDisplay.expire(_statusTextIndex);
		_statusTextIndex = -1;
	}

	if (_statusText[0] != '\0') {
		if ((_owner._screenObjects._v832EC == 0) || (_owner._screenObjects._v832EC == 2)) {
			Font *font = _madsVm->_font->getFont(FONT_MAIN_MADS);
			int textSpacing = -1;

			int strWidth = font->getWidth(_statusText);
			if (strWidth > 320) {
				// Too large to fit, so fall back on interface font
				font = _madsVm->_font->getFont(FONT_INTERFACE_MADS);
				strWidth = font->getWidth(_statusText, 0);
				textSpacing = 0;
			}

			// Add a new text display entry to display the status text at the bottom of the screen area
			uint colors = (_vm->getGameType() == GType_DragonSphere) ? 0x0300 : 0x0003;

			_statusTextIndex = _owner._textDisplay.add(160 - (strWidth / 2),
				MADS_SURFACE_HEIGHT + _owner._posAdjust.y - 13, colors, textSpacing, _statusText, font);
		}
	}

	_textChanged = false;
	*/
}

void MADSAction::startAction() {
	/*
	_madsVm->_player.moveComplete();

	_inProgress = true;
	_v8453A = ABORTMODE_0;
	_savedFields.selectedRow = _selectedRow;
	_savedFields.articleNumber = _articleNumber;
	_savedFields.actionMode = _actionMode;
	_savedFields.actionMode2 = _actionMode2;
	_savedFields.lookFlag = _lookFlag;
	int savedHotspotId = _hotspotId;
	int savedV86F3A = _v86F3A;
	int savedV86F42 = _v86F42;

	// Copy the action to be active
	_activeAction = _action;
	strcpy(_dialogTitle, _statusText);

	if ((_savedFields.actionMode2 == ACTMODE2_4) && (savedV86F42 == 0))
		_v8453A = ABORTMODE_1;

	_startWalkFlag = false;
	int hotspotId = -1;
	HotSpotList &dynHotspots = *_madsVm->scene()->getSceneResources().dynamicHotspots;
	HotSpotList &hotspots = *_madsVm->scene()->getSceneResources().hotspots;

	if (!_savedFields.lookFlag && (_madsVm->scene()->_screenObjects._v832EC != 1)) {
		if (_savedFields.actionMode2 == ACTMODE2_4)
			hotspotId = savedHotspotId;
		else if (savedV86F42 == 4)
			hotspotId = savedV86F3A;

		if (hotspotId >= hotspots.size()) {
			HotSpot &hs = dynHotspots[hotspotId - hotspots.size()];
			if ((hs.getFeetX() == -1) || (hs.getFeetX() == -3)) {
				if (_v86F4A && ((hs.getFeetX() == -3) || (_savedFields.selectedRow < 0))) {
					_startWalkFlag = true;
					_madsVm->scene()->_destPos = _madsVm->scene()->_customDest;
				}
			} else if ((hs.getFeetX() >= 0) && ((_savedFields.actionMode == ACTMODE_NONE) || (hs.getCursor() < 2))) {
				_startWalkFlag = true;
				_madsVm->scene()->_destPos.x = hs.getFeetX();
				_madsVm->scene()->_destPos.y = hs.getFeetY();
			}
			_madsVm->scene()->_destFacing = hs.getFacing();
			hotspotId = -1;
		}
	}

	if (hotspotId >= 0) {
		HotSpot &hs = hotspots[hotspotId];
		if ((hs.getFeetX() == -1) || (hs.getFeetX() == -3)) {
			if (_v86F4A && ((hs.getFeetX() == -3) || (_savedFields.selectedRow < 0))) {
				_startWalkFlag = true;
				_madsVm->scene()->_destPos = _madsVm->scene()->_customDest;
			}
		} else if ((hs.getFeetX() >= 0) && ((_savedFields.actionMode == ACTMODE_NONE) || (hs.getCursor() < 2))) {
			_startWalkFlag = true;
			_madsVm->scene()->_destPos.x = hs.getFeetX();
			_madsVm->scene()->_destPos.y = hs.getFeetY();
		}
		_madsVm->scene()->_destFacing = hs.getFacing();
	}

	_walkFlag = _startWalkFlag;
	*/
}

void MADSAction::checkAction() {
	/*
	if (isAction(kVerbLookAt) || isAction(kVerbThrow))
		_startWalkFlag = 0;
	*/
}

bool MADSAction::isAction(int verbId, int objectNameId, int indirectObjectId) {
	/*
	if (_activeAction.verbId != verbId)
		return false;
	if ((objectNameId != 0) && (_activeAction.objectNameId != objectNameId))
		return false;
	if ((indirectObjectId != 0) && (_activeAction.indirectObjectId != indirectObjectId))
		return false;
	*/
	return true;
}

} // End of namespace MADS
