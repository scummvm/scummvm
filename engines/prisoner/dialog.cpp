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
 * $URL$
 * $Id$
 *
 */

#include "prisoner/prisoner.h"
#include "prisoner/resource.h"
#include "prisoner/screen.h"

namespace Prisoner {

/* Dialog */

void PrisonerEngine::loadDialogKeywords(Common::String &pakName, int16 pakSlot, Dialog *dialog, bool init) {
	int16 keywordsResourceSlot = loadTextResource(pakName, pakSlot);
	TextResource *keywords = _res->get<TextResource>(keywordsResourceSlot);
	dialog->keywords.clear();
	for (uint i = 0; i < keywords->getCount(); i++) {
		const TextItem *textItem = keywords->getText(i);
		dialog->keywords.push_back(textItem->getChunkLineString(0, 0));
		if (init) {
			dialog->used = 1;
			dialog->keywords[i]._used = 0;
		}
	}
	dialog->pakName = pakName;
	dialog->pakSlot = pakSlot;
	_res->unload(keywordsResourceSlot);
}

int16 PrisonerEngine::loadDialog(Common::String &pakName, int16 pakSlot) {
	int16 dialogIndex = _dialogs.getFreeSlot();
	loadDialogKeywords(pakName, pakSlot, &_dialogs[dialogIndex], true);
	_currDialogIndex = dialogIndex;
	return dialogIndex;
}

void PrisonerEngine::unloadDialog(int16 dialogIndex) {
	_dialogs[dialogIndex].used = 0;
}

void PrisonerEngine::refreshDialogKeywords(int16 dialogIndex) {
	setActiveFont(_textFont);
	_dialogMaxKeywordTextWidth = 0;
	_dialogActiveKeywordsCount = 0;
	Dialog *dialog = &_dialogs[dialogIndex];
	for (uint keywordIndex = 0; keywordIndex < dialog->keywords.size(); keywordIndex++) {
		DialogKeyword &dialogKeyword = dialog->keywords[keywordIndex];
		if (dialogKeyword._used) {
			_currDialogKeywordIndices[_dialogActiveKeywordsCount++] = keywordIndex;
			int16 keywordTextWidth = getTextWidth(dialogKeyword._keyword);
			if (keywordTextWidth > _dialogMaxKeywordTextWidth)
				_dialogMaxKeywordTextWidth = keywordTextWidth;
		}
	}
	_dialogFontHeight = getActiveFontHeight();
}

void PrisonerEngine::resetDialogValues() {
	_dialogRunning = false;
	_selectedDialogKeywordIndex = -1;
	_dialogFlag = true;
}

void PrisonerEngine::enableDialogKeyword(int16 dialogIndex, int16 keywordIndex) {
	_dialogs[dialogIndex].keywords[keywordIndex]._used = 1;
	refreshDialogKeywords(dialogIndex);
}

void PrisonerEngine::disableDialogKeyword(int16 dialogIndex, int16 keywordIndex) {
	_dialogs[dialogIndex].keywords[keywordIndex]._used = 0;
	refreshDialogKeywords(dialogIndex);
	if (_dialogActiveKeywordsCount == 0) {
		_dialogRunning = false;
		_dialogFlag = _selectedDialogKeywordIndex != -1;
		_selectedDialogKeywordIndex = -1;
	}
}

void PrisonerEngine::startDialog(int16 dialogIndex) {

	ActorSprite *actorSprite = _actors[_mainActorIndex].actorSprite;
	int16 fontHeight;

	refreshDialogKeywords(dialogIndex);
	_currDialogIndex = dialogIndex;
	_dialogRunning = true;
	_selectedDialogIndex = 0;
	_dialogXAdd = 8;
	setFontColors(_textFont, _dialogFontColor.outlineColor, _dialogFontColor.inkColor);
	setActiveFont(_textFont);
	fontHeight = getActiveFontHeight();

	_dialogRectX1 = actorSprite->x + actorSprite->xsub - _cameraX;
	if (_dialogRectX1 + _dialogMaxKeywordTextWidth >= 630)
		_dialogRectX1 = 630 - _dialogMaxKeywordTextWidth;
	if (_dialogRectX1 < 10)
		_dialogRectX1 = 10;

	_dialogRectY1 = actorSprite->y - (_dialogActiveKeywordsCount * _dialogFontHeight + actorSprite->y) - _cameraY;
	if (_dialogRectY1 < fontHeight + 82)
		_dialogRectY1 = fontHeight + 82;

	_dialogRectX2 = _dialogRectX1 + _dialogMaxKeywordTextWidth + _dialogXAdd * 2;
	_dialogYAdd = _dialogFontHeight / 2;
	_dialogRectY2 = _dialogRectY1 + _dialogActiveKeywordsCount * _dialogFontHeight + _dialogYAdd * 2;

	if (_dialogPanelResourceCacheSlot == -1) {
		// TODO: Use loaded values
		// S_PANEL.PUK   13 89 87?
		Common::String pakName = "S_PANEL";
		int16 pakSlot = 13;
		_dialogPanelResourceCacheSlot = _res->load<AnimationResource>(pakName, pakSlot, 11);
	}

}

void PrisonerEngine::initDialog() {
	_dialogs.clear();
	if (_dialogPanelResourceCacheSlot == -1) {
		// TODO: Use loaded values
		// S_PANEL.PUK   13 89 87?
		Common::String pakName = "S_PANEL";
		int16 pakSlot = 13;
		_dialogPanelResourceCacheSlot = _res->load<AnimationResource>(pakName, pakSlot, 11);
	}
}

void PrisonerEngine::unloadDialogPanel() {
	if (_dialogPanelResourceCacheSlot != -1) {
		_res->unload(_dialogPanelResourceCacheSlot);
		_dialogPanelResourceCacheSlot = -1;
	}
}

void PrisonerEngine::updateDialog(int16 x, int16 y) {

	AnimationResource *dialogPanel;

	if (!_dialogRunning)
		return;

	if (_dialogActiveKeywordsCount <= 0) {
		_selectedDialogKeywordIndex = -2;
		_dialogRunning = false;
		_dialogFlag = true;
		return;
	}

	Dialog *dialog = &_dialogs[_currDialogIndex];

	x -= _cameraX;
	y -= _cameraY;

	setFontColors(_textFont, _dialogFontColor.outlineColor, _dialogFontColor.inkColor);
	setActiveFont(_textFont);

	debug("(%d, %d, %d, %d)", _dialogRectX1, _dialogRectY1, _dialogRectX2, _dialogRectY2);

	_screen->drawTransparentRect(_dialogRectX1, _dialogRectY1, _dialogRectX2, _dialogRectY2);

	// TODO: cseg02:00029654 - cseg02:000296E0

	_screen->frameRect(_dialogRectX1 - 2, _dialogRectY1 - 2, _dialogRectX2 + 2, _dialogRectY2 + 2, 87);
	_screen->frameRect(_dialogRectX1 - 1, _dialogRectY1 - 1, _dialogRectX2 + 1, _dialogRectY2 + 1, 89);

	dialogPanel = _res->get<AnimationResource>(_dialogPanelResourceCacheSlot);

	_screen->drawAnimationElement(dialogPanel, 0, _dialogRectX1, _dialogRectY1, 0);
	_screen->drawAnimationElement(dialogPanel, 1, _dialogRectX2, _dialogRectY1, 0);
	_screen->drawAnimationElement(dialogPanel, 2, _dialogRectX2, _dialogRectY2, 0);
	_screen->drawAnimationElement(dialogPanel, 3, _dialogRectX1, _dialogRectY2, 0);

	if (_dialogActiveKeywordsCount > 1) {
		int16 textX = _dialogRectX1 + _dialogXAdd;
		int16 textY = _dialogRectY1 + _dialogYAdd + _dialogFontHeight - getActiveFontUnk2();
		int16 currKeyword = -1, currKeywordY = -1;
		if (x > _dialogRectX1 && y > _dialogRectY1 && x < _dialogRectX2 && y < _dialogRectY2)
			currKeyword = CLIP((y - _dialogRectY1 - _dialogYAdd) / _dialogFontHeight, 0, _dialogActiveKeywordsCount - 1);
		for (int16 i = 0; i < _dialogActiveKeywordsCount; i++) {
			DialogKeyword *dialogKeyword = &dialog->keywords[_currDialogKeywordIndices[i]];
			if (i == currKeyword)
				currKeywordY = textY;
			drawText(textX, textY, dialogKeyword->_keyword);
			textY += _dialogFontHeight;
		}
		if (currKeyword != -1) {
			DialogKeyword *dialogKeyword = &dialog->keywords[_currDialogKeywordIndices[currKeyword]];
			setFontColors(_textFont, _dialogHoverFontColor.outlineColor, _dialogHoverFontColor.inkColor);
			setActiveFont(_textFont);
			drawText(textX, currKeywordY, dialogKeyword->_keyword);
			_selectedDialogKeywordIndex = _currDialogKeywordIndices[currKeyword];
		} else {
			_selectedDialogKeywordIndex = -1;
		}
		_selectedDialogIndex = currKeyword;
	} else {
		int16 currKeyword = 0;
		DialogKeyword *dialogKeyword = &dialog->keywords[_currDialogKeywordIndices[currKeyword]];
		setFontColors(_textFont, _dialogHoverFontColor.outlineColor, _dialogHoverFontColor.inkColor);
		setActiveFont(_textFont);
		drawTextEx(_dialogRectX1 + _dialogXAdd, -1, _dialogRectY1, _dialogRectY2, dialogKeyword->_keyword);//###
		_selectedDialogKeywordIndex = _currDialogKeywordIndices[currKeyword];
	}

}

bool PrisonerEngine::isPointInDialogRect(int16 x, int16 y) {
	return (x > _dialogRectX1) && (x < _dialogRectX2) && (y > _dialogRectY1) && (y < _dialogRectY2);
}

} // End of namespace Prisoner
