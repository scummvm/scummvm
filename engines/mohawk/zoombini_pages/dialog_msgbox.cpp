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

#include "mohawk/console.h"
#include "mohawk/mohawk.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"

#include "mohawk/zoombini.h"
#include "mohawk/zoombini_graphics.h"
#include "mohawk/zoombini_pages/dialog_msgbox.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_sound.h"
#include "mohawk/zoombini_state.h"

namespace Mohawk {

ZoombiniDialogMsgBox::ZoombiniDialogMsgBox(MohawkEngine_Zoombini *vm, ZoombiniMsgBoxType type) : ZoombiniDialog(vm, ZoombiniPageType::kDialogMsgBox),
																								 _type(type),
																								 _msgKey(ZoombiniText::kNone) {
	ZoombiniText::Key yesKey = ZoombiniText::kDialogButtonYes;
	ZoombiniText::Key noKey = ZoombiniText::kDialogButtonNo;
	// Spanish and Portuguese v1.0 keep the Options OK label in a separate
	// executable string from the one-button alert label.
	ZoombiniText::Key alertAcceptKey;
	if (_vm->isGameVariant(MohawkGameFeatures::GF_ZMB_10_ESPT))
		alertAcceptKey = ZoombiniText::kDialogButtonAccept;
	else
		alertAcceptKey = ZoombiniText::kDialogButtonOkay;

	switch (_type) {
	case ZoombiniMsgBoxType::kAlertNoSavedGame:
		_msgKey = ZoombiniText::kDialogBodyNoSavedGames;
		yesKey = alertAcceptKey;
		noKey = ZoombiniText::kNone;
		break;
	case ZoombiniMsgBoxType::kAskCreateAndSaveNewGame:
		_msgKey = ZoombiniText::kDialogBodyCreateAndSaveNewGame;
		yesKey = ZoombiniText::kDialogButtonNewGame;
		noKey = ZoombiniText::kDialogButtonCancel;
		break;
	case ZoombiniMsgBoxType::kAskCreateNewGame:
		_msgKey = ZoombiniText::kDialogBodyCreateNewGame;
		yesKey = ZoombiniText::kDialogButtonNewGame;
		noKey = ZoombiniText::kDialogButtonCancel;
		break;
	case ZoombiniMsgBoxType::kAskReplaceSave:
		_msgKey = ZoombiniText::kDialogBodyReplaceGame;
		break;
	case ZoombiniMsgBoxType::kAskRemoveSave:
		_msgKey = ZoombiniText::kDialogBodyRemoveGame;
		break;
	case ZoombiniMsgBoxType::kAskSaveCurrentGame:
		_msgKey = ZoombiniText::kDialogBodySaveCurrentGame;
		break;
	case ZoombiniMsgBoxType::kAskSaveBeforeQuit:
		_msgKey = ZoombiniText::kDialogBodySaveBeforeQuit;
		break;
	case ZoombiniMsgBoxType::kAlertCannotSaveInPractice:
		_msgKey = ZoombiniText::kDialogBodyCannotSaveInPractice;
		yesKey = alertAcceptKey;
		noKey = ZoombiniText::kNone;
		break;
	case ZoombiniMsgBoxType::kAlertCannotSaveMoreGames:
		_msgKey = ZoombiniText::kDialogBodyCannotSaveMoreGame;
		yesKey = alertAcceptKey;
		noKey = ZoombiniText::kNone;
		break;
	case ZoombiniMsgBoxType::kAlertCannotLoadInPractice:
		_msgKey = ZoombiniText::kDialogBodyCannotLoadInPractice;
		yesKey = alertAcceptKey;
		noKey = ZoombiniText::kNone;
		break;
	case ZoombiniMsgBoxType::kAlertCannotCreateNewInPractice:
		_msgKey = ZoombiniText::kDialogBodyCannotCreateNewInPractice;
		yesKey = alertAcceptKey;
		noKey = ZoombiniText::kNone;
		break;
	case ZoombiniMsgBoxType::kAskReallyQuit:
		_msgKey = ZoombiniText::kDialogBodyReallyQuit;
		break;
	case ZoombiniMsgBoxType::kAskSaveDirtyGame:
		_msgKey = ZoombiniText::kDialogBodySaveDirtyGame;
		break;
	case ZoombiniMsgBoxType::kAskGoMapWillLost:
		_msgKey = ZoombiniText::kDialogBodyGoMapWillLost;
		yesKey = ZoombiniText::kDialogButtonLoseThem;
		noKey = ZoombiniText::kDialogButtonKeepThem;
		break;
	default:
		error("msgbox: invalid ZoombiniMsgBoxType: %u", static_cast<uint32>(_type));
		break;
	}

	initButtons(yesKey, noKey);
}

ZoombiniDialogMsgBox::ZoombiniDialogMsgBox(MohawkEngine_Zoombini *vm, const Common::U32String &message) : ZoombiniDialog(vm, ZoombiniPageType::kDialogMsgBox),
																										  _type(ZoombiniMsgBoxType::kNone),
																										  _msgKey(ZoombiniText::kNone),
																										  _msgText(message) {
	initButtons(ZoombiniText::kDialogButtonOkay, ZoombiniText::kNone);
}

ZoombiniDialogMsgBox::ZoombiniDialogMsgBox(MohawkEngine_Zoombini *vm, const Common::U32String &message, ZoombiniText::Key yesKey, ZoombiniText::Key noKey) : ZoombiniDialog(vm, ZoombiniPageType::kDialogMsgBox),
																																							 _type(ZoombiniMsgBoxType::kNone),
																																							 _msgKey(ZoombiniText::kNone),
																																							 _msgText(message) {
	initButtons(yesKey, noKey);
}

void ZoombiniDialogMsgBox::initButtons(ZoombiniText::Key yesKey, ZoombiniText::Key noKey) {
	ZmbResource soundResId(ZmbResource::kSystem, kSysResSound0999_ButtonSFX);
	_longButtonStateMap[kMsgBoxDialogButton01_Yes] = ButtonState(yesKey, soundResId, 0, 2, kSystemShape0001_12_LongGreenButtonNormal, kSystemShape0001_13_LongGreenButtonPressed);
	_longButtonStateMap[kMsgBoxDialogButton02_No] = ButtonState(noKey, soundResId, 1, 3, kSystemShape0001_14_LongRedButtonNormal, kSystemShape0001_15_LongRedButtonPressed);
	if (noKey == ZoombiniText::kNone) {
		_longButtonStateMap[kMsgBoxDialogButton02_No]._drawEnabled = false;
	}
}

Common::Rect ZoombiniDialogMsgBox::getBodyTextRect() const {
	if (_vm->isGameVariant(MohawkGameFeatures::GF_ZMB_10_EU) && _vm->getLanguage() == Common::IT_ITA)
		return Common::Rect(0x009A, 0x0082, 0x01E7, 0x00EB);

	return Common::Rect(0x009A, 0x0091, 0x01E7, 0x00DC);
}

ZoombiniDialogMsgBox::~ZoombiniDialogMsgBox() {
}

void ZoombiniDialogMsgBox::loadFeatures() {
	// Load SCRBs
	loadScrbFeature(ZmbResource(ZmbResource::kSystem, kSysResShapeBitmap0001_Dialog), kSysResScrb0010_DialogMsgBox, 0,
					ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00001000_TOPMOST);

	resetLongButtonStates();
	ZmbFeature::EventHooks hooksLongButtons;
	hooksLongButtons.setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniDialogMsgBox::longButtons_onPreRenderShape));
	hooksLongButtons.setPostRenderFunc(static_cast<ZmbFeature::OnPostRenderFunc>(&ZoombiniDialogMsgBox::longButtons_onPostRender));
	hooksLongButtons.setLButtonDownFunc(static_cast<ZmbFeature::OnLButtonDownFunc>(&ZoombiniDialogMsgBox::longButtons_onLButtonDown));
	hooksLongButtons.setKeyDownFunc(static_cast<ZmbFeature::OnKeyDownFunc>(&ZoombiniDialogMsgBox::longButtons_onKeyDown));
	loadScrbFeature(ZmbResource(ZmbResource::kSystem, kSysResShapeBitmap0001_Dialog), kSysResScrb0011_DialogMsgBox, 15,
					ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00001000_TOPMOST,
					hooksLongButtons);
}

void ZoombiniDialogMsgBox::longButtons_onPreRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)hsGroup;

	// Adjust Yes button position if No button is disabled (Alert MessageBox)
	if (!_longButtonStateMap[kMsgBoxDialogButton02_No]._drawEnabled) {
		uint16 hsYesNormalIdx = _longButtonStateMap[kMsgBoxDialogButton01_Yes]._hsNormalId;
		uint16 hsYesPressedIdx = _longButtonStateMap[kMsgBoxDialogButton01_Yes]._hsPressedId;
		uint16 hsNoNormalIdx = _longButtonStateMap[kMsgBoxDialogButton02_No]._hsNormalId;
		uint16 hsNoPressedIdx = _longButtonStateMap[kMsgBoxDialogButton02_No]._hsPressedId;
		if (hotspots.size() <= hsYesNormalIdx || hotspots.size() <= hsYesPressedIdx || hotspots.size() <= hsNoNormalIdx || hotspots.size() <= hsNoPressedIdx) {
			error("msgbox: long-button SCRB is missing required hotspots");
			return;
		}
		ZmbHotspot &hsYesNormal = hotspots[hsYesNormalIdx];
		ZmbHotspot &hsYesPressed = hotspots[hsYesPressedIdx];
		hsYesNormal._x -= 90;
		hsYesPressed._x -= 90;

		ZmbHotspot &hsNoNormal = hotspots[hsNoNormalIdx];
		ZmbHotspot &hsNoPressed = hotspots[hsNoPressedIdx];
		hsNoNormal._shapeIdx = ZmbHotspot::kShapeNone;
		hsNoPressed._shapeIdx = ZmbHotspot::kShapeNone;
	}

	genericButton_selectShapes(feature, hotspots, _longButtonStateMap);
}

void ZoombiniDialogMsgBox::longButtons_onPostRender(ZmbFeature *feature) {
	ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;

	// [Post-Animation Events]
	genericButton_action(feature, _longButtonStateMap, static_cast<ZoombiniPage::OnButtonActionFunc>(&ZoombiniDialogMsgBox::longButtons_onButtonAction));

	// [Text Render] Dialog Body
	ZoombiniGraphics::TextConf bodyConf;
	bodyConf._fontUsage = ZoombiniFontUsage::kFontTitle;
	bodyConf._hAlign = Graphics::kTextAlignCenter;
	bodyConf._vAlign = Graphics::kTextAlignCenter;
	bodyConf._outlineEffect = true;
	bodyConf._outlinePalette = ZoombiniGraphics::kColor0E_VeryLightGray;
	bodyConf._textPalette = ZoombiniGraphics::kBlackKey;
	bodyConf._wordWrap = true;
	const Common::Rect bodyTextRect = getBodyTextRect();
	if (_msgKey == ZoombiniText::kNone)
		_vm->_gfx->drawText(screenKind, _msgText, bodyTextRect, bodyConf);
	else
		_vm->_gfx->drawText(screenKind, _msgKey, bodyTextRect, bodyConf);

	// [Text Render] Yes/No Button Descriptions
	genericButton_textRender(feature, _longButtonStateMap, Graphics::kTextAlignCenter);
}

void ZoombiniDialogMsgBox::longButtons_onButtonAction(ZmbFeature *feature, uint32 bsIdx, ButtonState &bs) {
	(void)feature;
	(void)bs;

	switch (bsIdx) {
	case kMsgBoxDialogButton01_Yes:
		_dialogResult = ZoombiniDialogResult::kYes;
		close();
		break;
	case kMsgBoxDialogButton02_No:
		_dialogResult = ZoombiniDialogResult::kNo;
		close();
		break;
	default:
		error("msgbox: invalid msgbox dialog button event(%u)", bsIdx);
		break;
	}
}

ZmbEventHandleResult ZoombiniDialogMsgBox::longButtons_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	(void)relPos;

	return genericButton_onLButtonDown(feature, absPos, _longButtonStateMap);
}

ZmbEventHandleResult ZoombiniDialogMsgBox::longButtons_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat) {
	ZmbEventHandleResult result = ZmbEventHandleResult::kConsumed;
	switch (classifyDialogKey(kbd)) {
	case kDialogKeyAccept:
		if (!kbdRepeat && !_longButtonStateMap[kMsgBoxDialogButton01_Yes].isAnimating())
			_longButtonStateMap[kMsgBoxDialogButton01_Yes].press(_vm, feature, _currentFrameCounter);
		break;
	case kDialogKeyCancel:
		if (!kbdRepeat && _longButtonStateMap[kMsgBoxDialogButton02_No]._drawEnabled &&
			!_longButtonStateMap[kMsgBoxDialogButton02_No].isAnimating())
			_longButtonStateMap[kMsgBoxDialogButton02_No].press(_vm, feature, _currentFrameCounter);
		break;
	case kDialogKeyNone:
		result = ZmbEventHandleResult::kPassthrough;
		break;
	}
	return result;
}

} // End of namespace Mohawk
