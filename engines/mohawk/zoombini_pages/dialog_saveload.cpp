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

#include "mohawk/zoombini_pages/dialog_saveload.h"
#include "mohawk/zoombini.h"
#include "mohawk/zoombini_graphics.h"
#include "mohawk/zoombini_state.h"

namespace Mohawk {

ZoombiniDialogSaveLoad::ZoombiniDialogSaveLoad(MohawkEngine_Zoombini *vm, ZoombiniPageType pageType, ZoombiniText::Key titleKey,
											   const Common::Rect &titleRect, ZoombiniText::Key yesKey, ZoombiniText::Key noKey, uint16 saveEntryTop)
	: ZoombiniDialog(vm, pageType), _titleRect(titleRect), _titleKey(titleKey), _saveEntryTop(saveEntryTop) {
	_titleRect = _vm->getRelInfo().getLayoutAdjustment(ZmbReleaseInfo::LayoutKind::kSaveLoadTitle).apply(_titleRect);

	ZmbResource soundResId = ZmbResource(ZmbResource::kSystem, kSysResSound0999_ButtonSFX);
	_scrollButtonStateMap[kSaveLoadDialogButton01_ScrollUp] = ButtonState(soundResId, 0, 2, kSystemShape0001_16_SaveLoadScrollUpButtonNormal, kSystemShape0001_17_SaveLoadScrollUpButtonPressed);
	_scrollButtonStateMap[kSaveLoadDialogButton02_ScrollDown] = ButtonState(soundResId, 1, 3, kSystemShape0001_18_SaveLoadScrollDownButtonNormal, kSystemShape0001_19_SaveLoadScrollDownButtonPressed);
	_longButtonStateMap[kSaveLoadDialogButton03_Okay] = ButtonState(yesKey, soundResId, 0, 2, kSystemShape0001_12_LongGreenButtonNormal, kSystemShape0001_13_LongGreenButtonPressed);
	_longButtonStateMap[kSaveLoadDialogButton04_Cancel] = ButtonState(noKey, soundResId, 1, 3, kSystemShape0001_14_LongRedButtonNormal, kSystemShape0001_15_LongRedButtonPressed);
}

ZoombiniDialogSaveLoad::~ZoombiniDialogSaveLoad() {
}

void ZoombiniDialogSaveLoad::loadSaveLoadFeatures(int16 frameScrb, int16 scrollButtonsScrb, int16 longButtonsScrb) {
	ZmbFeature::EventHooks hooksDialogFrame;
	hooksDialogFrame.setPostRenderFunc(&ZoombiniDialogSaveLoad::dialogFrame_onPostRender);
	hooksDialogFrame.setLButtonDownFunc(&ZoombiniDialogSaveLoad::dialogFrame_onLButtonDown);
	hooksDialogFrame.setKeyDownFunc(&ZoombiniDialogSaveLoad::dialogFrame_onKeyDown);
	_dialogFrameFeature = loadScrbFeature(ZmbResource(ZmbResource::kSystem, kSysResShapeBitmap0001_Dialog), frameScrb, 0,
										  ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00001000_TOPMOST,
										  hooksDialogFrame);

	for (Common::StableMap<uint32, ButtonState>::iterator it = _scrollButtonStateMap.begin(); it != _scrollButtonStateMap.end(); it++)
		it->second.reset();
	ZmbFeature::EventHooks hooksScrollButtons;
	hooksScrollButtons.setPreRenderShapeFunc(&ZoombiniDialogSaveLoad::scrollButtons_onPreRenderShape);
	hooksScrollButtons.setPostRenderFunc(&ZoombiniDialogSaveLoad::scrollButtons_onPostRender);
	hooksScrollButtons.setWheelUpFunc(&ZoombiniDialogSaveLoad::scrollButtons_onWheelUp);
	hooksScrollButtons.setWheelDownFunc(&ZoombiniDialogSaveLoad::scrollButtons_onWheelDown);
	hooksScrollButtons.setLButtonDownFunc(&ZoombiniDialogSaveLoad::scrollButtons_onLButtonDown);
	hooksScrollButtons.setKeyDownFunc(&ZoombiniDialogSaveLoad::scrollButtons_onKeyDown);
	loadScrbFeature(ZmbResource(ZmbResource::kSystem, kSysResShapeBitmap0001_Dialog), scrollButtonsScrb, 11, ZmbFeature::FLAG_04000000_OVERLAY, hooksScrollButtons);

	resetLongButtonStates();
	ZmbFeature::EventHooks hooksLongButtons;
	hooksLongButtons.setPreRenderShapeFunc(&ZoombiniDialogSaveLoad::longButtons_onPreRenderShape);
	hooksLongButtons.setPostRenderFunc(&ZoombiniDialogSaveLoad::longButtons_onPostRender);
	hooksLongButtons.setLButtonDownFunc(&ZoombiniDialogSaveLoad::longButtons_onLButtonDown);
	hooksLongButtons.setKeyDownFunc(&ZoombiniDialogSaveLoad::longButtons_onKeyDown);
	_longButtonsFeature = loadScrbFeature(ZmbResource(ZmbResource::kSystem, kSysResShapeBitmap0001_Dialog), longButtonsScrb, 13,
										  ZmbFeature::FLAG_04000000_OVERLAY,
										  hooksLongButtons);
}

Common::Rect ZoombiniDialogSaveLoad::getSaveEntryBaseRect() {
	return Common::Rect(_saveEntryLeft, _saveEntryTop, _saveEntryRight, _saveEntryTop + _saveEntryHeight);
}

void ZoombiniDialogSaveLoad::markDialogFrameDirty() {
	if (_dialogFrameFeature)
		_dialogFrameFeature->setNeedsRedraw(true);
}

void ZoombiniDialogSaveLoad::drawDialogTitle() {
	ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;

	ZoombiniGraphics::TextConf titleConf;
	titleConf._fontUsage = ZoombiniFontUsage::kFontTitle;
	titleConf._hAlign = Graphics::kTextAlignCenter;
	titleConf._vAlign = Graphics::kTextAlignCenter;
	titleConf._outlineEffect = true;
	titleConf._outlinePalette = ZoombiniGraphics::kColor0E_VeryLightGray;
	titleConf._textPalette = ZoombiniGraphics::kColor2D_Black;
	titleConf._wordWrap = false;
	_vm->_gfx->drawText(screenKind, _titleKey, _titleRect, titleConf);
}

void ZoombiniDialogSaveLoad::drawSaveEntries(uint32 textPalette, int32 selectedIdx) {
	ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;
	Common::Rect saveEntryRect = getSaveEntryBaseRect();

	// [Text Render] SaveEntry List (Up to 8 in one screen)
	// Render the visible names from the in-memory save roster in roster order.
	// Compare selectedIdx with the absolute roster index even after scrolling.
	for (int32 slotIdx = _saveEntryBaseIdx; slotIdx < _saveEntryBaseIdx + SAVESLOTS_PER_SCREEN && slotIdx < _vm->_state->_r.getEntryCount(); slotIdx++) {
		const ZmbRosterEntry &roster = _vm->_state->_r.getEntry(slotIdx);
		const Common::U32String &saveName = roster.getSaveName(_vm);

		ZoombiniGraphics::TextConf entryConf;
		entryConf._fontUsage = ZoombiniFontUsage::kFontTitle;
		entryConf._textPalette = textPalette;
		entryConf._wordWrap = false;
		if (selectedIdx == slotIdx) {
			entryConf._outlineEffect = true;
			entryConf._outlinePalette = ZoombiniGraphics::kColor2D_Black;
			entryConf._textPalette = ZoombiniGraphics::kColor22_LimeGreen;
		}

		_vm->_gfx->drawText(screenKind, saveName, saveEntryRect, entryConf);

		saveEntryRect.top += 20;
		saveEntryRect.bottom += 20;
	}
}

ZmbEventHandleResult ZoombiniDialogSaveLoad::dialogFrame_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	(void)feature;
	(void)absPos;
	(void)relPos;
	return ZmbEventHandleResult::kPassthrough;
}

void ZoombiniDialogSaveLoad::scrollButtons_onPreRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)hsGroup;

	genericButton_selectShapes(feature, hotspots, _scrollButtonStateMap);
}

void ZoombiniDialogSaveLoad::scrollButtons_onPostRender(ZmbFeature *feature) {
	// [Post-Animation Events]
	genericButton_action(feature, _scrollButtonStateMap, static_cast<ZoombiniPage::OnButtonActionFunc>(&ZoombiniDialogSaveLoad::scrollButtons_onButtonAction));
}

void ZoombiniDialogSaveLoad::scrollButtons_onButtonAction(ZmbFeature *feature, uint32 bsIdx, ButtonState &bs) {
	(void)feature;
	(void)bs;

	int32 oldSaveEntryBaseIdx = _saveEntryBaseIdx;

	switch (bsIdx) {
	case kSaveLoadDialogButton01_ScrollUp:
		_saveEntryBaseIdx -= SAVESLOTS_PER_SCREEN;
		break;
	case kSaveLoadDialogButton02_ScrollDown:
		_saveEntryBaseIdx += SAVESLOTS_PER_SCREEN;
		break;
	default:
		error("saveload: invalid saveload dialog button event(%u)", bsIdx);
		break;
	}

	// Keep @ref ZoombiniDialogSaveLoad::_saveEntryBaseIdx within the visible save-entry range.
	// @ref ZmbRosterFile::getEntryCount() defines the upper bound.
	_saveEntryBaseIdx = CLIP<int32>(_saveEntryBaseIdx, 0, MAX<int32>(_vm->_state->_r.getEntryCount() - SAVESLOTS_PER_SCREEN, 0));
	if (oldSaveEntryBaseIdx != _saveEntryBaseIdx)
		markDialogFrameDirty();
}

ZmbEventHandleResult ZoombiniDialogSaveLoad::scrollButtons_onWheelUp(ZmbFeature *feature, const Common::Point &absPos) {
	(void)absPos;

	scrollButtons_onButtonAction(feature, kSaveLoadDialogButton01_ScrollUp, _scrollButtonStateMap[kSaveLoadDialogButton01_ScrollUp]);
	return ZmbEventHandleResult::kConsumed;
}

ZmbEventHandleResult ZoombiniDialogSaveLoad::scrollButtons_onWheelDown(ZmbFeature *feature, const Common::Point &absPos) {
	(void)absPos;

	scrollButtons_onButtonAction(feature, kSaveLoadDialogButton02_ScrollDown, _scrollButtonStateMap[kSaveLoadDialogButton02_ScrollDown]);
	return ZmbEventHandleResult::kConsumed;
}

ZmbEventHandleResult ZoombiniDialogSaveLoad::scrollButtons_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	(void)relPos;

	return genericButton_onLButtonDown(feature, absPos, _scrollButtonStateMap);
}

ZmbEventHandleResult ZoombiniDialogSaveLoad::scrollButtons_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat) {
	(void)kbdRepeat;

	ZmbEventHandleResult result = ZmbEventHandleResult::kPassthrough;
	if (kbd.hasFlags(0)) {
		switch (getKeyboardNavDirection(kbd)) {
		case KBD_NAV_PAGEUP: // PgUp - enhanced shortcut: scroll one page up
			if (!_vm->useEnhancedKbdShortcuts())
				return result;
			if (!_scrollButtonStateMap[kSaveLoadDialogButton01_ScrollUp].isAnimating())
				_scrollButtonStateMap[kSaveLoadDialogButton01_ScrollUp].press(_vm, feature, _currentFrameCounter);
			result = ZmbEventHandleResult::kConsumed;
			break;
		case KBD_NAV_UP: // Up Arrow
			// Enhanced shortcut mode enabled: Select previous save entry
			// Enhanced shortcut mode disabled: Scroll one page up
			if (getPageType() == ZoombiniPageType::kDialogLoad && _vm->useEnhancedKbdShortcuts())
				return result;
			if (!_scrollButtonStateMap[kSaveLoadDialogButton01_ScrollUp].isAnimating())
				_scrollButtonStateMap[kSaveLoadDialogButton01_ScrollUp].press(_vm, feature, _currentFrameCounter);
			result = ZmbEventHandleResult::kConsumed;
			break;
		case KBD_NAV_PAGEDOWN: // PgDn - enhanced shortcut: scroll one page down
			if (!_vm->useEnhancedKbdShortcuts())
				return result;
			if (!_scrollButtonStateMap[kSaveLoadDialogButton02_ScrollDown].isAnimating())
				_scrollButtonStateMap[kSaveLoadDialogButton02_ScrollDown].press(_vm, feature, _currentFrameCounter);
			result = ZmbEventHandleResult::kConsumed;
			break;
		case KBD_NAV_DOWN: // Down Arrow
			// Enhanced shortcut mode enabled: Select next save entry
			// Enhanced shortcut mode disabled: Scroll one page down
			if (getPageType() == ZoombiniPageType::kDialogLoad && _vm->useEnhancedKbdShortcuts())
				return result;
			if (!_scrollButtonStateMap[kSaveLoadDialogButton02_ScrollDown].isAnimating())
				_scrollButtonStateMap[kSaveLoadDialogButton02_ScrollDown].press(_vm, feature, _currentFrameCounter);
			result = ZmbEventHandleResult::kConsumed;
			break;
		default:
			break;
		}
	}
	return result;
}

void ZoombiniDialogSaveLoad::longButtons_onPreRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)hsGroup;

	genericButton_selectShapes(feature, hotspots, _longButtonStateMap);
}

void ZoombiniDialogSaveLoad::longButtons_onPostRender(ZmbFeature *feature) {
	// [Post-Animation Events]
	genericButton_action(feature, _longButtonStateMap, static_cast<ZoombiniPage::OnButtonActionFunc>(&ZoombiniDialogSaveLoad::longButtons_onButtonAction));

	// [Text Render] Long Button Descriptions
	genericButton_textRender(feature, _longButtonStateMap, Graphics::kTextAlignCenter);
}

void ZoombiniDialogSaveLoad::longButtons_onButtonAction(ZmbFeature *feature, uint32 bsIdx, ButtonState &bs) {
	(void)feature;
	(void)bs;

	switch (bsIdx) {
	case kSaveLoadDialogButton03_Okay:
		confirm();
		break;
	case kSaveLoadDialogButton04_Cancel:
		_dialogResult = ZoombiniDialogResult::kNo;
		close();
		break;
	default:
		error("saveload: invalid saveload dialog button event(%u)", bsIdx);
		break;
	}
}

ZmbEventHandleResult ZoombiniDialogSaveLoad::longButtons_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	(void)relPos;

	return genericButton_onLButtonDown(feature, absPos, _longButtonStateMap);
}

ZmbEventHandleResult ZoombiniDialogSaveLoad::longButtons_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat) {
	(void)kbdRepeat;

	ZmbEventHandleResult result = ZmbEventHandleResult::kPassthrough;
	switch (classifyDialogKey(kbd)) {
	case kDialogKeyCancel:
		if (!_longButtonStateMap[kSaveLoadDialogButton04_Cancel].isAnimating())
			_longButtonStateMap[kSaveLoadDialogButton04_Cancel].press(_vm, feature, _currentFrameCounter);
		result = ZmbEventHandleResult::kConsumed;
		break;
	case kDialogKeyAccept:
		if (!_longButtonStateMap[kSaveLoadDialogButton03_Okay].isAnimating())
			_longButtonStateMap[kSaveLoadDialogButton03_Okay].press(_vm, feature, _currentFrameCounter);
		result = ZmbEventHandleResult::kConsumed;
		break;
	case kDialogKeyNone:
		break;
	}
	return result;
}

} // End of namespace Mohawk
