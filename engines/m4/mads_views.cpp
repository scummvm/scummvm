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

#include "m4/m4_views.h"
#include "m4/animation.h"
#include "m4/dialogs.h"
#include "m4/events.h"
#include "m4/font.h"
#include "m4/globals.h"
#include "m4/mads_menus.h"
#include "m4/m4.h"
#include "m4/staticres.h"

#include "common/algorithm.h"
#include "common/textconsole.h"

namespace M4 {

MadsAction::MadsAction(MadsView &owner): _owner(owner) {
	clear();
	_currentAction = kVerbNone;
	_startWalkFlag = false;
	_statusTextIndex = -1;
	_selectedAction = 0;
	_inProgress = false;
}

void MadsAction::clear() {
	_v83338 = 1;
	_actionMode = ACTMODE_NONE;
	_actionMode2 = ACTMODE2_0;
	_v86F42 = 0;
	_v86F4E = 0;
	_articleNumber = 0;
	_lookFlag = false;
	_v86F4A = 0;
	_statusText[0] = '\0';
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

void MadsAction::appendVocab(int vocabId, bool capitalise) {
	char *s = _statusText + strlen(_statusText);
	const char *vocabStr = _madsVm->globals()->getVocab(vocabId);
	strcpy(s, vocabStr);
	if (capitalise)
		*s = toupper(*s);

	strcat(s, " ");
}

void MadsAction::set() {
	int hotspotCount = _madsVm->scene()->getSceneResources().hotspots->size();
	bool flag = false;
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
}

void MadsAction::refresh() {
	// Exit immediately if nothing has changed
	if (!_textChanged)
		return;

	// Remove any old copy of the status text
	if (_statusTextIndex >= 0) {
		_owner._textDisplay.expire(_statusTextIndex);
		_statusTextIndex = -1;
	}

	if (strlen(_statusText) != 0) {
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
}

void MadsAction::startAction() {
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
}

void MadsAction::checkAction() {
	if (isAction(kVerbLookAt) || isAction(kVerbThrow))
		_startWalkFlag = 0;
}

bool MadsAction::isAction(int verbId, int objectNameId, int indirectObjectId) {
	if (_activeAction.verbId != verbId)
		return false;
	if ((objectNameId != 0) && (_activeAction.objectNameId != objectNameId))
		return false;
	if ((indirectObjectId != 0) && (_activeAction.indirectObjectId != indirectObjectId))
		return false;
	return true;
}

//--------------------------------------------------------------------------

bool MadsSpriteSlot::operator==(const SpriteSlotSubset &other) const {
	return (spriteListIndex == other.spriteListIndex) && (frameNumber == other.frameNumber) &&
		(xp == other.xp) && (yp == other.yp) && (depth == other.depth) && (scale == other.scale);
}

void MadsSpriteSlot::copy(const SpriteSlotSubset &other) {
	spriteListIndex = other.spriteListIndex;
	frameNumber = other.frameNumber;
	xp = other.xp;
	yp = other.yp;
	depth = other.depth;
	scale = other.scale;
}

//--------------------------------------------------------------------------

MadsSpriteSlots::MadsSpriteSlots(MadsView &owner): _owner(owner) {
	for (int i = 0; i < SPRITE_SLOTS_SIZE; ++i) {
		MadsSpriteSlot rec;
		_entries.push_back(rec);
	}

	startIndex = 0;
}

MadsSpriteSlots::~MadsSpriteSlots() {
	for (uint i = 0; i < _sprites.size(); ++i)
		delete _sprites[i];
}

void MadsSpriteSlots::clear() {
	_owner._textDisplay.clear();
	for (uint i = 0; i < _sprites.size(); ++i)
		delete _sprites[i];
	_sprites.clear();

	// Reset the sprite slots list back to a single entry for a full screen refresh
	startIndex = 1;
	_entries[0].spriteType = FULL_SCREEN_REFRESH;
	_entries[0].seqIndex = -1;
}

int MadsSpriteSlots::getIndex() {
	if (startIndex == SPRITE_SLOTS_SIZE)
		error("Run out of sprite slots");

	return startIndex++;
}

int MadsSpriteSlots::addSprites(const char *resName, bool suppressErrors, int flags) {
	// If errors are suppressed, first check if the resource exists
	if (suppressErrors) {
		if (!_vm->res()->resourceExists(resName))
			return -1;
	}

	// Append on a '.SS' suffix if the resource doesn't already have an extension
	char buffer[100];
	strncpy(buffer, resName, 95);
	buffer[95] = '\0';
	if (!strchr(buffer, '.'))
		strcat(buffer, ".SS");

	// Get the sprite set
	Common::SeekableReadStream *data = _vm->res()->get(buffer);
	SpriteAsset *spriteSet = new SpriteAsset(_vm, data, data->size(), buffer, false, flags);
	spriteSet->translate(_madsVm->_palette);
	assert(spriteSet != NULL);

	_sprites.push_back(spriteSet);
	_vm->res()->toss(buffer);

	return _sprites.size() - 1;
}

int MadsSpriteSlots::addSprites(SpriteAsset *spriteSet) {
	_sprites.push_back(spriteSet);

	return _sprites.size() - 1;
}

void MadsSpriteSlots::deleteSprites(int listIndex) {
	if (listIndex < 0)
		return;

	delete _sprites[listIndex];
	_sprites[listIndex] = NULL;
	if (listIndex == ((int)_sprites.size() - 1))
		_sprites.remove_at(listIndex);
}

/*
 * Deletes the sprite slot with the given timer entry
 */
void MadsSpriteSlots::deleteTimer(int seqIndex) {
	for (int idx = 0; idx < startIndex; ++idx) {
		if (_entries[idx].seqIndex == seqIndex)
			_entries[idx].spriteType = EXPIRED_SPRITE;
	}
}

class DepthEntry {
public:
	int depth;
	int index;

	DepthEntry(int depthAmt, int indexVal) { depth = depthAmt; index = indexVal; }
};

bool sortHelper(const DepthEntry &entry1, const DepthEntry &entry2) {
	return entry1.depth < entry2.depth;
}

typedef Common::List<DepthEntry> DepthList;

void MadsSpriteSlots::drawBackground() {
	// Draw all active sprites onto the background surface
	for (int i = 0; i < startIndex; ++i) {
		MadsSpriteSlot &slot = _entries[i];

		if (slot.spriteType >= 0) {
			_owner._dirtyAreas[i].active = false;
		} else {
			_owner._dirtyAreas[i].textActive = true;
			_owner._dirtyAreas.setSpriteSlot(i, slot);

			if (slot.spriteType == BACKGROUND_SPRITE) {
				SpriteAsset &spriteSet = getSprite(slot.spriteListIndex);
				M4Sprite *frame = spriteSet.getFrame((slot.frameNumber & 0x7fff) - 1);
				int xp = slot.xp;
				int yp = slot.yp;

				if (_entries[i].scale != -1) {
					// Adjust position based on frame size
					xp -= frame->width() / 2;
					yp -= frame->height() / 2;
				}

				if (slot.depth > 1) {
					// Draw the frame with depth processing
					_owner._bgSurface->copyFrom(frame, xp, yp, slot.depth, _owner._depthSurface, 100,
						frame->getTransparencyIndex());
				} else {
					// No depth, so simply draw the image
					frame->copyTo(_owner._bgSurface, xp, yp, frame->getTransparencyIndex());
				}
			}
		}
	}

	// Flag any remaining dirty areas as inactive
	for (uint i = startIndex; i < DIRTY_AREAS_TEXT_DISPLAY_IDX; ++i)
		_owner._dirtyAreas[i].active = false;
}

void MadsSpriteSlots::drawForeground(M4Surface *viewport) {
	DepthList depthList;

	// Get a list of sprite object depths for active objects
	for (int i = 0; i < startIndex; ++i) {
		if (_entries[i].spriteType >= SPRITE_ZERO) {
			DepthEntry rec(16 - _entries[i].depth, i);
			depthList.push_back(rec);
		}
	}

	// Sort the list in order of the depth
	Common::sort(depthList.begin(), depthList.end(), sortHelper);

	// Loop through each of the objects
	DepthList::iterator i;
	for (i = depthList.begin(); i != depthList.end(); ++i) {
		DepthEntry &de = *i;
		MadsSpriteSlot &slot = _entries[de.index];
		assert(slot.spriteListIndex < (int)_sprites.size());
		SpriteAsset &spriteSet = *_sprites[slot.spriteListIndex];

		// Get the sprite frame
		int frameNumber = slot.frameNumber & 0x7fff;
		bool flipped = (slot.frameNumber & 0x8000) != 0;
		M4Sprite *sprite = spriteSet.getFrame(frameNumber - 1);

		M4Surface *spr = sprite;
		if (flipped) {
			// Create a flipped copy of the sprite temporarily
			spr = sprite->flipHorizontal();
		}

		if ((slot.scale < 100) && (slot.scale != -1)) {
			// Minimalised drawing
			viewport->copyFrom(spr, slot.xp, slot.yp, slot.depth, _owner._depthSurface, slot.scale,
				sprite->getTransparencyIndex());
		} else {
			int xp, yp;

			if (slot.scale == -1) {
				xp = slot.xp - _owner._posAdjust.x;
				yp = slot.yp - _owner._posAdjust.y;
			} else {
				xp = slot.xp - (spr->width() / 2) - _owner._posAdjust.x;
				yp = slot.yp - spr->height() - _owner._posAdjust.y + 1;
			}

			if (slot.depth > 1) {
				// Draw the frame with depth processing
				viewport->copyFrom(spr, xp, yp, slot.depth, _owner._depthSurface, 100, sprite->getTransparencyIndex());
			} else {
				// No depth, so simply draw the image
				spr->copyTo(viewport, xp, yp, sprite->getTransparencyIndex());
			}
		}

		// Free sprite if it was a flipped one
		if (flipped)
			delete spr;
	}
}

void MadsSpriteSlots::setDirtyAreas() {
	for (int i = 0; i < startIndex; ++i) {
		if (_entries[i].spriteType >= 0) {
			_owner._dirtyAreas.setSpriteSlot(i, _entries[i]);

			_owner._dirtyAreas[i].textActive = (_entries[i].spriteType <= 0) ? 0 : 1;
			_entries[i].spriteType = 0;
		}
	}
}

/**
 * Flags the entire screen to be redrawn during the next drawing cycle
 */
void MadsSpriteSlots::fullRefresh() {
	int idx = getIndex();

	_entries[idx].spriteType = FULL_SCREEN_REFRESH;
	_entries[idx].seqIndex = -1;
}

/**
 * Removes any sprite slots that are no longer needed
 */
void MadsSpriteSlots::cleanUp() {
	// Delete any entries that aren't needed
	int idx = 0;
	while (idx < startIndex) {
		if (_entries[idx].spriteType < 0) {
			_entries.remove_at(idx);
			--startIndex;
		} else {
			++idx;
		}
	}

	// Original engine sprite slot list was a fixed array, so to keep the engine similiar, for
	// now I'm adding in new entries to make up the original fixed total again
	while (_entries.size() < SPRITE_SLOTS_SIZE) {
		MadsSpriteSlot rec;
		_entries.push_back(rec);
	}
}

//--------------------------------------------------------------------------

MadsTextDisplay::MadsTextDisplay(MadsView &owner): _owner(owner) {
	for (int i = 0; i < TEXT_DISPLAY_SIZE; ++i) {
		MadsTextDisplayEntry rec;
		rec.active = false;
		rec.expire = 0;
		_entries.push_back(rec);
	}
}

void MadsTextDisplay::clear() {
	for (int i = 0; i < TEXT_DISPLAY_SIZE; ++i)
		_entries[i].active = false;
}

int MadsTextDisplay::add(int xp, int yp, uint fontColor, int charSpacing, const char *msg, Font *font) {
	int usedSlot = -1;

	for (int idx = 0; idx < TEXT_DISPLAY_SIZE; ++idx) {
		if (!_entries[idx].active) {
			usedSlot = idx;

			_entries[idx].bounds.left = xp;
			_entries[idx].bounds.top = yp;
			_entries[idx].font = font;
			_entries[idx].msg = msg;
			_entries[idx].bounds.setWidth(font->getWidth(msg, charSpacing));
			_entries[idx].bounds.setHeight(font->getHeight());
			_entries[idx].color1 = fontColor & 0xff;
			_entries[idx].color2 = fontColor >> 8;
			_entries[idx].spacing = charSpacing;
			_entries[idx].expire = 1;
			_entries[idx].active = true;
			break;
		}
	}

	return usedSlot;
}

void MadsTextDisplay::setDirtyAreas() {
	// Determine dirty areas for active text areas
	for (uint idx = 0, dirtyIdx = DIRTY_AREAS_TEXT_DISPLAY_IDX; dirtyIdx < DIRTY_AREAS_SIZE; ++idx, ++dirtyIdx) {
		if ((_entries[idx].expire >= 0) || !_entries[idx].active)
			_owner._dirtyAreas[dirtyIdx].active = false;
		else {
			_owner._dirtyAreas[dirtyIdx].textActive = true;
			_owner._dirtyAreas.setTextDisplay(dirtyIdx, _entries[idx]);
		}
	}
}

void MadsTextDisplay::setDirtyAreas2() {
	// Determine dirty areas for active text areas
	for (uint idx = 0, dirtyIdx = DIRTY_AREAS_TEXT_DISPLAY_IDX; dirtyIdx < DIRTY_AREAS_SIZE; ++idx, ++dirtyIdx) {
		if (_entries[idx].active && (_entries[idx].expire >= 0)) {
			_owner._dirtyAreas.setTextDisplay(dirtyIdx, _entries[idx]);
			_owner._dirtyAreas[dirtyIdx].textActive = (_entries[idx].expire <= 0) ? 0 : 1;
		}
	}
}

void MadsTextDisplay::draw(M4Surface *view) {
	for (uint idx = 0; idx < _entries.size(); ++idx) {
		if (_entries[idx].active && (_entries[idx].expire >= 0)) {
			_entries[idx].font->setColors(_entries[idx].color1, _entries[idx].color2, 0);
			_entries[idx].font->writeString(view, _entries[idx].msg,
				_entries[idx].bounds.left, _entries[idx].bounds.top, _entries[idx].bounds.width(),
				_entries[idx].spacing);
		}
	}
}

/**
 * Deactivates any text display entries that are finished
 */
void MadsTextDisplay::cleanUp() {
	for (uint idx = 0; idx < _entries.size(); ++idx) {
		if (_entries[idx].expire < 0) {
			_entries[idx].active = false;
			_entries[idx].expire = 0;
		}
	}
}

//--------------------------------------------------------------------------

MadsKernelMessageList::MadsKernelMessageList(MadsView &owner): _owner(owner) {
	for (int i = 0; i < TIMED_TEXT_SIZE; ++i) {
		MadsKernelMessageEntry rec;
		_entries.push_back(rec);
	}

	_owner._textSpacing = -1;
	_talkFont = _vm->_font->getFont(FONT_CONVERSATION_MADS);
	word_8469E = 0;
}

void MadsKernelMessageList::clear() {
	for (uint i = 0; i < _entries.size(); ++i)
		_entries[i].flags = 0;

	_owner._textSpacing = -1;
	_talkFont = _vm->_font->getFont(FONT_CONVERSATION_MADS);
}

int MadsKernelMessageList::add(const Common::Point &pt, uint fontColor, uint8 flags, uint8 abortTimers, uint32 timeout, const char *msg) {
	// Find a free slot
	uint idx = 0;
	while ((idx < _entries.size()) && ((_entries[idx].flags & KMSG_ACTIVE) != 0))
		++idx;
	if (idx == _entries.size()) {
		if (abortTimers == 0)
			return -1;

		error("MadsKernelList overflow");
	}

	MadsKernelMessageEntry &rec = _entries[idx];
	strcpy(rec.msg, msg);
	rec.flags = flags | KMSG_ACTIVE;
	rec.color1 = fontColor & 0xff;
	rec.color2 = fontColor >> 8;
	rec.position = pt;
	rec.textDisplayIndex = -1;
	rec.timeout = timeout;
	rec.frameTimer = _madsVm->_currentTimer;
	rec.abortTimers = abortTimers;
	rec.abortMode = _owner._abortTimersMode2;

	for (int i = 0; i < 3; ++i)
		rec.actionNouns[i] = _madsVm->globals()->actionNouns[i];

	if (flags & KMSG_PLAYER_TIMEOUT)
		rec.frameTimer = _madsVm->_player._ticksAmount + _madsVm->_player._priorTimer;

	return idx;
}

int MadsKernelMessageList::addQuote(int quoteId, int abortTimers, uint32 timeout) {
	const char *quoteStr = _madsVm->globals()->getQuote(quoteId);
	return add(Common::Point(0, 0), 0x1110, KMSG_PLAYER_TIMEOUT | KMSG_CENTER_ALIGN, abortTimers, timeout, quoteStr);
}

void MadsKernelMessageList::scrollMessage(int msgIndex, int numTicks, bool quoted) {
	if (msgIndex < 0)
		return;

	_entries[msgIndex].flags |= quoted ? (KMSG_SCROLL | KMSG_QUOTED) : KMSG_SCROLL;
	_entries[msgIndex].msgOffset = 0;
	_entries[msgIndex].numTicks = numTicks;
	_entries[msgIndex].frameTimer2 = _madsVm->_currentTimer;

	const char *msgP = _entries[msgIndex].msg;
	_entries[msgIndex].asciiChar = *msgP;
	_entries[msgIndex].asciiChar2 = *(msgP + 1);

	if (_entries[msgIndex].flags & KMSG_PLAYER_TIMEOUT)
		_entries[msgIndex].frameTimer2 = _madsVm->_player._ticksAmount + _madsVm->_player._priorTimer;

	_entries[msgIndex].frameTimer = _entries[msgIndex].frameTimer2;
}

void MadsKernelMessageList::setSeqIndex(int msgIndex, int seqIndex) {
	if (msgIndex >= 0) {
		_entries[msgIndex].flags |= KMSG_SEQ_ENTRY;
		_entries[msgIndex].sequenceIndex = seqIndex;
	}
}

void MadsKernelMessageList::remove(int msgIndex) {
	MadsKernelMessageEntry &rec = _entries[msgIndex];

	if (rec.flags & KMSG_ACTIVE) {
		if (rec.flags & KMSG_SCROLL) {
			*(rec.msg + rec.msgOffset) = rec.asciiChar;
			*(rec.msg + rec.msgOffset + 1) = rec.asciiChar2;
		}

		if (rec.textDisplayIndex >= 0)
			_owner._textDisplay.expire(rec.textDisplayIndex);

		rec.flags &= ~KMSG_ACTIVE;
	}
}

void MadsKernelMessageList::reset() {
	for (uint i = 0; i < _entries.size(); ++i)
		remove(i);

	// sub_20454
}

void MadsKernelMessageList::update() {
	uint32 currentTimer = _madsVm->_currentTimer;

	for (uint i = 0; i < _entries.size(); ++i) {
		if (((_entries[i].flags & KMSG_ACTIVE) != 0) && (currentTimer >= _entries[i].frameTimer))
			processText(i);
	}
}

void MadsKernelMessageList::processText(int msgIndex) {
	MadsKernelMessageEntry &msg = _entries[msgIndex];
	uint32 currentTimer = _madsVm->_currentTimer;
	bool flag = false;

	if ((msg.flags & KMSG_EXPIRE) != 0) {
		_owner._textDisplay.expire(msg.textDisplayIndex);
		msg.flags &= !KMSG_ACTIVE;
		return;
	}

	if ((msg.flags & KMSG_SCROLL) == 0) {
		msg.timeout -= 3;
	}

	if (msg.flags & KMSG_SEQ_ENTRY) {
		MadsSequenceEntry &seqEntry = _owner._sequenceList[msg.sequenceIndex];
		if (seqEntry.doneFlag || !seqEntry.active)
			msg.timeout = 0;
	}

	if ((msg.timeout <= 0) && (_owner._abortTimers == 0)) {
		msg.flags |= KMSG_EXPIRE;
		if (msg.abortTimers != 0) {
			_owner._abortTimers = msg.abortTimers;
			_owner._abortTimersMode = msg.abortMode;

			if (_owner._abortTimersMode != ABORTMODE_1) {
				for (int i = 0; i < 3; ++i)
					_madsVm->globals()->actionNouns[i] = msg.actionNouns[i];
			}
		}
	}

	msg.frameTimer = currentTimer + 3;
	int x1 = 0, y1 = 0;

	if (msg.flags & KMSG_SEQ_ENTRY) {
		MadsSequenceEntry &seqEntry = _owner._sequenceList[msg.sequenceIndex];
		if (!seqEntry.nonFixed) {
			SpriteAsset &spriteSet = _owner._spriteSlots.getSprite(seqEntry.spriteListIndex);
			M4Sprite *frame = spriteSet.getFrame(seqEntry.frameIndex - 1);
			x1 = frame->bounds().left;
			y1 = frame->bounds().top;
		} else {
			x1 = seqEntry.msgPos.x;
			y1 = seqEntry.msgPos.y;
		}
	}

	if (msg.flags & KMSG_PLAYER_TIMEOUT) {
		if (word_8469E != 0) {
			// TODO: Figure out various flags
		} else {
			x1 = 160;
			y1 = 78;
		}
	}

	x1 += msg.position.x;
	y1 += msg.position.y;

	if ((msg.flags & KMSG_SCROLL) && (msg.frameTimer >= currentTimer)) {
		msg.msg[msg.msgOffset] = msg.asciiChar;
		char *msgP = &msg.msg[++msg.msgOffset];
		*msgP = msg.asciiChar2;

		msg.asciiChar = *msgP;
		msg.asciiChar2 = *(msgP + 1);

		if (!msg.asciiChar) {
			// End of message
			*msgP = '\0';
			msg.flags &= ~KMSG_SCROLL;
		} else if (msg.flags & KMSG_QUOTED) {
			*msgP = '"';
			*(msgP + 1) = '\0';
		}

		msg.frameTimer = msg.frameTimer2 = currentTimer + msg.numTicks;
		flag = true;
	}

	int strWidth = _talkFont->getWidth(msg.msg, _owner._textSpacing);

	if (msg.flags & (KMSG_RIGHT_ALIGN | KMSG_CENTER_ALIGN)) {
		x1 -= (msg.flags & KMSG_CENTER_ALIGN) ? strWidth / 2 : strWidth;
	}

	// Make sure text appears entirely on-screen
	int x2 = x1 + strWidth;
	if (x2 > MADS_SURFACE_WIDTH)
		x1 -= x2 - MADS_SURFACE_WIDTH;
	if (x1 > (MADS_SURFACE_WIDTH - 1))
		x1 = MADS_SURFACE_WIDTH - 1;
	if (x1 < 0)
		x1 = 0;

	if (y1 > (MADS_SURFACE_HEIGHT - 1))
		y1 = MADS_SURFACE_HEIGHT - 1;
	if (y1 < 0)
		y1 = 0;

	if (msg.textDisplayIndex >= 0) {
		MadsTextDisplayEntry textEntry = _owner._textDisplay[msg.textDisplayIndex];

		if (flag || (textEntry.bounds.left != x1) || (textEntry.bounds.top != y1)) {
			// Mark the associated text entry as deleted, so it can be re-created
			_owner._textDisplay.expire(msg.textDisplayIndex);
			msg.textDisplayIndex = -1;
		}
	}

	if (msg.textDisplayIndex < 0) {
		// Need to create a new text display entry for this message
		int idx = _owner._textDisplay.add(x1, y1, msg.color1 | (msg.color2 << 8), _owner._textSpacing, msg.msg, _talkFont);
		if (idx >= 0)
			msg.textDisplayIndex = idx;
	}
}

//--------------------------------------------------------------------------

ScreenObjects::ScreenObjects(MadsView &owner): _owner(owner) {
	_v832EC = 0;
	_v7FECA = 0;
	_v7FED6 = 0;
	_v8332A = 0;
	_yp = 0;
	_v8333C = 0;
	_selectedObject = 0;
	_category = 0;
	_objectIndex = 0;
}

/**
 * Clears the entries list
 */
void ScreenObjects::clear() {
	_entries.clear();
}

/**
 * Adds a new entry to the list of screen objects
 */
void ScreenObjects::add(const Common::Rect &bounds, int layer, int idx, int category) {
	ScreenObjectEntry rec;
	rec.bounds = bounds;
	rec.layer = layer;
	rec.index = idx;
	rec.category = category;
	rec.active = true;

	_entries.push_back(rec);
}

/**
 * Scans the list for an element that contains the given mode. The result will be 1 based for a match,
 * with 0 indicating no entry was found
 */
int ScreenObjects::scan(int xp, int yp, int layer) {
	for (uint i = 0; i < _entries.size(); ++i) {
		if (_entries[i].active && _entries[i].bounds.contains(xp, yp) && (_entries[i].layer == layer))
			return i + 1;
	}

	// Entry not found
	return 0;
}

int ScreenObjects::scanBackwards(int xp, int yp, int layer) {
	for (int i = (int)_entries.size() - 1; i >= 0; --i) {
		if (_entries[i].active && _entries[i].bounds.contains(xp, yp) && (_entries[i].layer == layer))
			return i + 1;
	}

	// Entry not found
	return 0;
}

void ScreenObjects::setActive(int category, int idx, bool active) {
	for (uint i = 0; i < _entries.size(); ++i) {
		if (_entries[i].active && (_entries[i].category == category) && (_entries[i].index == idx))
			_entries[i].active = active;
	}
}

void ScreenObjects::check(bool scanFlag, bool mouseClick) {
	if (!mouseClick || _v832EC)
		_v7FECA = 0;

	if (!_v7FED6 && !_v8332A && !_yp && (_v8333C != 0)) {
		if (scanFlag) {
			_category = CAT_NONE;
			_selectedObject = scanBackwards(_madsVm->_mouse->currentPos().x, _madsVm->_mouse->currentPos().y,
				LAYER_GUI);

			if (_selectedObject > 0) {
				ScreenObjectEntry &obj = _entries[_selectedObject];
				_category = obj.category & 7;
				_objectIndex = obj.index;
			}

			// TODO: Other stuff related to the user interface
		}
	}

	_owner._action.refresh();
}

/*--------------------------------------------------------------------------*/

MadsDynamicHotspots::MadsDynamicHotspots(MadsView &owner): _owner(owner) {
	for (int i = 0; i < DYNAMIC_HOTSPOTS_SIZE; ++i) {
		DynamicHotspot rec;
		rec.active = false;
		_entries.push_back(rec);
	}
	_changed = true;
	_count = 0;
}

int MadsDynamicHotspots::add(int descId, int field14, int seqIndex, const Common::Rect &bounds) {
	// Find a free slot
	uint idx = 0;
	while ((idx < _entries.size()) && _entries[idx].active)
		++idx;
	if (idx == _entries.size())
		error("MadsDynamicHotspots overflow");

	_entries[idx].active = true;
	_entries[idx].descId = descId;
	_entries[idx].seqIndex = seqIndex;
	_entries[idx].bounds = bounds;
	_entries[idx].pos.x = -3;
	_entries[idx].pos.y = 0;
	_entries[idx].facing = 5;
	_entries[idx].field_14 = field14;
	_entries[idx].articleNumber = 6;
	_entries[idx].field_17 = 0;

	++_count;
	_changed = true;

	if (seqIndex >= 0)
		_owner._sequenceList[seqIndex].dynamicHotspotIndex = idx;

	return idx;
}

int MadsDynamicHotspots::setPosition(int index, int xp, int yp, int facing) {
	if (index >= 0) {
		_entries[index].pos.x = xp;
		_entries[index].pos.y = yp;
		_entries[index].facing = facing;
	}

	return index;
}

int MadsDynamicHotspots::set17(int index, int v) {
	if (index >= 0)
		_entries[index].field_17 = v;

	return index;
}

void MadsDynamicHotspots::remove(int index) {
	if (_entries[index].active) {
		if (_entries[index].seqIndex >= 0)
			_owner._sequenceList[_entries[index].seqIndex].dynamicHotspotIndex = -1;
		_entries[index].active = false;

		--_count;
		_changed = true;
	}
}

void MadsDynamicHotspots::reset() {
	for (uint i = 0; i < _entries.size(); ++i)
		_entries[i].active = false;

	_count = 0;
	_changed = false;
}

/*--------------------------------------------------------------------------*/

void MadsDirtyArea::setArea(int width, int height, int maxWidth, int maxHeight) {
	if (bounds.left % 2) {
		--bounds.left;
		++width;
	}

	if (bounds.left < 0)
		bounds.left = 0;
	else if (bounds.left > maxWidth)
		bounds.left = maxWidth;
	int right = bounds.left + width;
	if (right < 0)
		right = 0;
	if (right > maxWidth)
		right = maxWidth;

	bounds.right = right;
	bounds2.left = bounds.width() / 2;
	bounds2.right = bounds.left + (bounds.width() + 1) / 2 - 1;

	if (bounds.top < 0)
		bounds.top = 0;
	else if (bounds.top > maxHeight)
		bounds.top = maxHeight;
	int bottom = bounds.top + height;
	if (bottom < 0)
		bottom = 0;
	if (bottom > maxHeight)
		bottom = maxHeight;

	bounds.bottom = bottom;
	bounds2.top = bounds.height() / 2;
	bounds2.bottom = bounds.top + (bounds.height() + 1) / 2 - 1;

	active = true;
}

/*--------------------------------------------------------------------------*/

MadsDirtyAreas::MadsDirtyAreas(MadsView &owner): _owner(owner) {
	for (int i = 0; i < DIRTY_AREAS_SIZE; ++i) {
		MadsDirtyArea rec;
		rec.active = false;
		_entries.push_back(rec);
	}
}

void MadsDirtyAreas::setSpriteSlot(int dirtyIdx, const MadsSpriteSlot &spriteSlot) {
	int width, height;
	MadsDirtyArea &dirtyArea = _entries[dirtyIdx];

	if (spriteSlot.spriteType == FULL_SCREEN_REFRESH) {
		// Special entry to refresh the entire screen
		dirtyArea.bounds.left = 0;
		dirtyArea.bounds.top = 0;
		width = MADS_SURFACE_WIDTH;
		height = MADS_SURFACE_HEIGHT;
	} else {
		// Standard sprite slots
		dirtyArea.bounds.left = spriteSlot.xp - _owner._posAdjust.x;
		dirtyArea.bounds.top = spriteSlot.yp - _owner._posAdjust.y;

		SpriteAsset &spriteSet = _owner._spriteSlots.getSprite(spriteSlot.spriteListIndex);
		M4Sprite *frame = spriteSet.getFrame(((spriteSlot.frameNumber & 0x7fff) - 1) & 0x7f);

		if (spriteSlot.scale == -1) {
			width = frame->width();
			height = frame->height();
		} else {
			width = frame->width() * spriteSlot.scale / 100;
			height = frame->height() * spriteSlot.scale / 100;

			dirtyArea.bounds.left -= width / 2;
			dirtyArea.bounds.top += -(height - 1);
		}
	}

	dirtyArea.setArea(width, height, MADS_SURFACE_WIDTH, MADS_SURFACE_HEIGHT);
}

void MadsDirtyAreas::setTextDisplay(int dirtyIdx, const MadsTextDisplayEntry &textDisplay) {
	MadsDirtyArea &dirtyArea = _entries[dirtyIdx];
	dirtyArea.bounds.left = textDisplay.bounds.left;
	dirtyArea.bounds.top = textDisplay.bounds.top;

	dirtyArea.setArea(textDisplay.bounds.width(), textDisplay.bounds.height(), MADS_SURFACE_WIDTH, MADS_SURFACE_HEIGHT);
}

/**
 * Merge together any designated dirty areas that overlap
 * @param startIndex	1-based starting dirty area starting index
 * @param count			Number of entries to process
 */
void MadsDirtyAreas::merge(int startIndex, int count) {
return;//***DEBUG***
	if (startIndex >= count)
		return;

	for (int outerCtr = startIndex - 1, idx = 0; idx < count; ++outerCtr, ++idx) {
		if (!_entries[outerCtr].active)
			continue;

		for (int innerCtr = outerCtr + 1; innerCtr < count; ++innerCtr) {
			if (!_entries[innerCtr].active || !intersects(outerCtr, innerCtr))
				continue;

			if (_entries[outerCtr].textActive && _entries[innerCtr].textActive)
				mergeAreas(outerCtr, innerCtr);
		}
	}
}

/**
 * Returns true if two dirty areas intersect
 */
bool MadsDirtyAreas::intersects(int idx1, int idx2) {
	return _entries[idx1].bounds2.intersects(_entries[idx2].bounds2);
}

void MadsDirtyAreas::mergeAreas(int idx1, int idx2) {
	MadsDirtyArea &da1 = _entries[idx1];
	MadsDirtyArea &da2 = _entries[idx2];

	da1.bounds.extend(da2.bounds);

	da1.bounds2.left = da1.bounds.width() / 2;
	da1.bounds2.right = da1.bounds.left + (da1.bounds.width() + 1) / 2 - 1;
	da1.bounds2.top = da1.bounds.height() / 2;
	da1.bounds2.bottom = da1.bounds.top + (da1.bounds.height() + 1) / 2 - 1;

	da2.active = false;
	da1.textActive = true;
}

void MadsDirtyAreas::copy(M4Surface *dest, M4Surface *src, const Common::Point &posAdjust) {
	for (uint i = 0; i < _entries.size(); ++i) {
		const Common::Rect &srcBounds = _entries[i].bounds;

		Common::Rect bounds(srcBounds.left + posAdjust.x, srcBounds.top + posAdjust.y,
			srcBounds.right + posAdjust.x, srcBounds.bottom + posAdjust.y);

		if (_entries[i].active && _entries[i].bounds.isValidRect())
			src->copyTo(dest, bounds, _entries[i].bounds.left, _entries[i].bounds.top);
	}
}

void MadsDirtyAreas::clear() {
	for (uint i = 0; i < _entries.size(); ++i)
		_entries[i].active = false;
}

/*--------------------------------------------------------------------------*/

MadsSequenceList::MadsSequenceList(MadsView &owner): _owner(owner) {
	for (int i = 0; i < TIMER_LIST_SIZE; ++i) {
		MadsSequenceEntry rec;
		rec.active = 0;
		rec.dynamicHotspotIndex = -1;
		_entries.push_back(rec);
	}
}

void MadsSequenceList::clear() {
	for (uint i = 0; i < _entries.size(); ++i) {
		_entries[i].active = 0;
		_entries[i].dynamicHotspotIndex = -1;
	}
}

bool MadsSequenceList::addSubEntry(int index, SequenceSubEntryMode mode, int frameIndex, int abortVal) {
	if (_entries[index].entries.count >= TIMER_ENTRY_SUBSET_MAX)
		return true;

	int subIndex = _entries[index].entries.count++;
	_entries[index].entries.mode[subIndex] = mode;
	_entries[index].entries.frameIndex[subIndex] = frameIndex;
	_entries[index].entries.abortVal[subIndex] = abortVal;

	return false;
}

int MadsSequenceList::add(int spriteListIndex, bool flipped, int frameIndex, int triggerCountdown, int delayTicks, int extraTicks, int numTicks,
		int msgX, int msgY, bool nonFixed, char scale, uint8 depth, int frameInc, SpriteAnimType animType, int numSprites,
		int frameStart) {

	// Find a free slot
	uint seqIndex = 0;
	while ((seqIndex < _entries.size()) && (_entries[seqIndex].active))
		++seqIndex;
	if (seqIndex == _entries.size())
		error("TimerList full");

	if (frameStart <= 0)
		frameStart = 1;
	if (numSprites == 0)
		numSprites = _madsVm->scene()->_spriteSlots.getSprite(spriteListIndex).getCount();
	if (frameStart == numSprites)
		frameInc = 0;

	// Set the list entry fields
	_entries[seqIndex].active = true;
	_entries[seqIndex].spriteListIndex = spriteListIndex;
	_entries[seqIndex].flipped = flipped;
	_entries[seqIndex].frameIndex = frameIndex;
	_entries[seqIndex].frameStart = frameStart;
	_entries[seqIndex].numSprites = numSprites;
	_entries[seqIndex].animType = animType;
	_entries[seqIndex].frameInc = frameInc;
	_entries[seqIndex].depth = depth;
	_entries[seqIndex].scale = scale;
	_entries[seqIndex].nonFixed = nonFixed;
	_entries[seqIndex].msgPos.x = msgX;
	_entries[seqIndex].msgPos.y = msgY;
	_entries[seqIndex].numTicks = numTicks;
	_entries[seqIndex].extraTicks = extraTicks;

	_entries[seqIndex].timeout = _madsVm->_currentTimer + delayTicks;

	_entries[seqIndex].triggerCountdown = triggerCountdown;
	_entries[seqIndex].doneFlag = false;
	_entries[seqIndex].field_13 = 0;
	_entries[seqIndex].dynamicHotspotIndex = -1;
	_entries[seqIndex].entries.count = 0;
	_entries[seqIndex].abortMode = _owner._abortTimersMode2;

	for (int i = 0; i < 3; ++i)
		_entries[seqIndex].actionNouns[i] = _madsVm->globals()->actionNouns[i];

	return seqIndex;
}

void MadsSequenceList::remove(int seqIndex) {
	if (_entries[seqIndex].active) {
		if (_entries[seqIndex].dynamicHotspotIndex >= 0)
			_owner._dynamicHotspots.remove(_entries[seqIndex].dynamicHotspotIndex);
	}

	_entries[seqIndex].active = false;
	_owner._spriteSlots.deleteTimer(seqIndex);
}

void MadsSequenceList::setSpriteSlot(int seqIndex, MadsSpriteSlot &spriteSlot) {
	MadsSequenceEntry &timerEntry = _entries[seqIndex];
	SpriteAsset &spriteSet = _owner._spriteSlots.getSprite(timerEntry.spriteListIndex);

	spriteSlot.spriteType = spriteSet.isBackground() ? BACKGROUND_SPRITE : FOREGROUND_SPRITE;
	spriteSlot.seqIndex = seqIndex;
	spriteSlot.spriteListIndex = timerEntry.spriteListIndex;
	spriteSlot.frameNumber = (timerEntry.flipped ? 0x8000 : 0) | timerEntry.frameIndex;
	spriteSlot.depth = timerEntry.depth;
	spriteSlot.scale = timerEntry.scale;

	if (!timerEntry.nonFixed) {
		spriteSlot.xp = timerEntry.msgPos.x;
		spriteSlot.yp = timerEntry.msgPos.y;
	} else {
		spriteSlot.xp = spriteSet.getFrame(timerEntry.frameIndex - 1)->x;
		spriteSlot.yp = spriteSet.getFrame(timerEntry.frameIndex - 1)->y;
	}
}

bool MadsSequenceList::loadSprites(int seqIndex) {
	MadsSequenceEntry &seqEntry = _entries[seqIndex];
	int slotIndex;
	bool result = false;
	int idx = -1;

	_owner._spriteSlots.deleteTimer(seqIndex);
	if (seqEntry.doneFlag) {
		remove(seqIndex);
		return false;
	}

	if (seqEntry.spriteListIndex == -1) {
		// Doesn't have an associated sprite anymore, so mark as done
		seqEntry.doneFlag = true;
	} else if ((slotIndex = _owner._spriteSlots.getIndex()) >= 0) {
		MadsSpriteSlot &spriteSlot = _owner._spriteSlots[slotIndex];
		setSpriteSlot(seqIndex, spriteSlot);

		int x2 = 0, y2 = 0;

		if ((seqEntry.field_13 != 0) || (seqEntry.dynamicHotspotIndex >= 0)) {
			SpriteAsset &spriteSet = _owner._spriteSlots.getSprite(seqEntry.spriteListIndex);
			M4Sprite *frame = spriteSet.getFrame(seqEntry.frameIndex - 1);
			int width = frame->width() * seqEntry.scale / 200;
			int height = frame->height() * seqEntry.scale / 100;

			warning("frame size %d x %d", width, height);

			// TODO: Missing stuff here, and I'm not certain about the dynamic hotspot stuff below

			if (seqEntry.dynamicHotspotIndex >= 0) {
				DynamicHotspot &dynHotspot = _owner._dynamicHotspots[seqEntry.dynamicHotspotIndex];

				dynHotspot.bounds.left = MAX(x2 - width, 0);
				dynHotspot.bounds.right = MAX(x2 - width, 319) - dynHotspot.bounds.left + 1;
				dynHotspot.bounds.top = MAX(y2 - height, 0);
				dynHotspot.bounds.bottom = MIN(y2, 155) - dynHotspot.bounds.top;

				_owner._dynamicHotspots._changed = true;
			}
		}

		// Frame adjustments
		if (seqEntry.frameStart != seqEntry.numSprites)
			seqEntry.frameIndex += seqEntry.frameInc;

		if (seqEntry.frameIndex >= seqEntry.frameStart) {
			if (seqEntry.frameIndex > seqEntry.numSprites) {
				result = true;
				if (seqEntry.animType == ANIMTYPE_CYCLED) {
					// Reset back to the starting frame (cyclic)
					seqEntry.frameIndex = seqEntry.frameStart;
				} else {
					// Switch into reverse mode
					seqEntry.frameIndex = seqEntry.numSprites - 1;
					seqEntry.frameInc = -1;
				}
			}
		} else {
			// Currently in reverse mode and moved past starting frame
			result = true;

			if (seqEntry.animType == ANIMTYPE_CYCLED)
			{
				// Switch back to forward direction again
				seqEntry.frameIndex = seqEntry.frameStart + 1;
				seqEntry.frameInc = 1;
			} else {
				// Otherwise reset back to last sprite for further reverse animating
				seqEntry.frameIndex = seqEntry.numSprites;
			}
		}

		if (result && (seqEntry.triggerCountdown != 0)) {
			if (--seqEntry.triggerCountdown == 0)
				seqEntry.doneFlag = true;
		}
	} else {
		// Out of sprite display slots, so mark entry as done
		seqEntry.doneFlag = true;
	}

	if (seqEntry.entries.count > 0) {
		for (int i = 0; i <= seqEntry.entries.count; ++i) {
			switch (seqEntry.entries.mode[i]) {
			case SM_0:
			case SM_1:
				if (((seqEntry.entries.mode[i] == SM_0) && seqEntry.doneFlag) ||
					((seqEntry.entries.mode[i] == SM_1) && result))
				idx = i;
				break;

			case SM_FRAME_INDEX: {
				int v = seqEntry.entries.frameIndex[i];
				if ((v == seqEntry.frameIndex) || (v == 0))
					idx = i;
			}

			default:
				break;
			}
		}
	}

	if (idx >= 0) {
		_owner._abortTimers = seqEntry.entries.abortVal[idx];
		_owner._abortTimersMode = seqEntry.abortMode;
	}

	return result;
}

/**
 * Handles counting down entries in the timer list for action
 */
void MadsSequenceList::tick() {
	for (uint idx = 0; idx < _entries.size(); ++idx) {
		if ((_owner._abortTimers2 == 0) && (_owner._abortTimers != 0))
			break;

		MadsSequenceEntry &seqEntry = _entries[idx];
		uint32 currentTimer = _madsVm->_currentTimer;

		if (!seqEntry.active || (currentTimer < seqEntry.timeout))
			continue;

		// Set the next timeout for the timer entry
		seqEntry.timeout = currentTimer + seqEntry.numTicks;

		// Action the sprite
		if (loadSprites(idx)) {
			seqEntry.timeout += seqEntry.extraTicks;
		}
	}
}

void MadsSequenceList::delay(uint32 v1, uint32 v2) {
	for (uint idx = 0; idx < _entries.size(); ++idx) {
		if (_entries[idx].active) {
			_entries[idx].timeout += v1 - v2;
		}
	}
}

void MadsSequenceList::setAnimRange(int seqIndex, int startVal, int endVal) {
	MadsSequenceEntry &seqEntry = _entries[seqIndex];
	SpriteAsset &spriteSet = _owner._spriteSlots.getSprite(seqEntry.spriteListIndex);
	int numSprites = spriteSet.getCount();
	int tempStart = startVal, tempEnd = endVal;

	switch (startVal) {
	case -2:
		tempStart = numSprites;
		break;
	case -1:
		tempStart = 1;
		break;
	}

	switch (endVal) {
	case -2:
	case 0:
		tempEnd = numSprites;
		break;
	case -1:
		tempEnd = 1;
		break;
	default:
		tempEnd = numSprites;
		break;
	}

	seqEntry.frameStart = tempStart;
	seqEntry.numSprites = tempEnd;

	seqEntry.frameIndex = (seqEntry.frameInc < 0) ? tempStart : tempEnd;
}

void MadsSequenceList::scan() {
	for (uint i = 0; i < _entries.size(); ++i) {
		if (!_entries[i].active && (_entries[i].spriteListIndex != -1)) {
			int idx = _owner._spriteSlots.getIndex();
			setSpriteSlot(i, _owner._spriteSlots[idx]);
		}
	}
}

/**
 * Sets the depth of the specified entry in the sequence list
 */
void MadsSequenceList::setDepth(int seqIndex, int depth) {
	_entries[seqIndex].depth = depth;
}

//--------------------------------------------------------------------------

Animation::Animation(MadsM4Engine *vm): _vm(vm) {
}

Animation::~Animation() {
}

//--------------------------------------------------------------------------

MadsView::MadsView(View *view): _view(view), _dynamicHotspots(*this), _sequenceList(*this),
		_kernelMessages(*this), _spriteSlots(*this), _dirtyAreas(*this), _textDisplay(*this),
		_screenObjects(*this), _action(*this) {

	_textSpacing = -1;
	_newTimeout = 0;
	_abortTimers = 0;
	_abortTimers2 = 0;
	_abortTimersMode = ABORTMODE_0;
	_abortTimersMode2 = ABORTMODE_0;

	_depthSurface = NULL;
	_bgSurface = NULL;
	_viewport = NULL;
	_sceneAnimation = new MadsAnimation(_vm, this);
}

MadsView::~MadsView() {
	delete _sceneAnimation;
	delete _viewport;
}

void MadsView::refresh() {
	if (!_viewport)
		setViewport(_view->bounds());

	// Draw any sprites
	_dirtyAreas.clear();
	_spriteSlots.drawBackground();

	// Process dirty areas
	_textDisplay.setDirtyAreas();

	// Merge any identified dirty areas
	_dirtyAreas.merge(1, DIRTY_AREAS_SIZE);

	// Copy dirty areas to the main display surface
	_dirtyAreas.copy(_viewport, _bgSurface, _posAdjust);

	// Handle dirty areas for foreground objects
	_spriteSlots.setDirtyAreas();
	_textDisplay.setDirtyAreas2();
	_dirtyAreas.merge(1, DIRTY_AREAS_SIZE);

	// Draw foreground sprites
	_spriteSlots.drawForeground(_viewport);

	// Draw text elements onto the view
	_textDisplay.draw(_viewport);

	// Remove any sprite slots that are no longer needed
	_spriteSlots.cleanUp();

	// Deactivate any text display entries that are no longer needed
	_textDisplay.cleanUp();
}

void MadsView::update() {
	_sequenceList.tick();
	_kernelMessages.update();
}

void MadsView::clearLists() {
	_textDisplay.clear();
	_kernelMessages.clear();
	_spriteSlots.clear();
}

void MadsView::setViewport(const Common::Rect &bounds) {
	delete _viewport;
	_viewport = new M4Surface(bounds.width(), bounds.height(), _view->getBasePtr(bounds.left, bounds.top),
		_view->getPitch());
}

} // End of namespace M4
