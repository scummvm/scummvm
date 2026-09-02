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
#include "mohawk/zoombini_pages/dialog_help.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_sound.h"
#include "mohawk/zoombini_state.h"

namespace Mohawk {

ZoombiniDialogHelp::ZoombiniDialogHelp(MohawkEngine_Zoombini *vm, ZoombiniPageType forPage) : ZoombiniDialog(vm, ZoombiniPageType::kDialogHelp),
																							  _forPageType(forPage),
																							  _showPickerHeaderEasterEgg(vm->consumePickerUpdateHelpRequest()) {

	_helpStrlMap[ZoombiniPageType::kPicker] = kSysResStrl1300_HelpPicker;
	_helpStrlMap[ZoombiniPageType::kBasecamp1] = kSysResStrl1400_HelpBasecamp1;
	_helpStrlMap[ZoombiniPageType::kBasecamp2] = kSysResStrl1500_HelpBasecamp2;
	_helpStrlMap[ZoombiniPageType::kTown] = kSysResStrl1600_HelpTown;
	_helpStrlMap[ZoombiniPageType::kBridge] = kSysResStrl1700_HelpBridge;
	_helpStrlMap[ZoombiniPageType::kCaves] = kSysResStrl2600_HelpCaves;
	_helpStrlMap[ZoombiniPageType::kPizza] = kSysResStrl1900_HelpPizza;
	_helpStrlMap[ZoombiniPageType::kFerry] = kSysResStrl2000_HelpFerry;
	_helpStrlMap[ZoombiniPageType::kLilly] = kSysResStrl2100_HelpLilly;
	_helpStrlMap[ZoombiniPageType::kSlides] = kSysResStrl2200_HelpSlides;
	_helpStrlMap[ZoombiniPageType::kFleens] = kSysResStrl2300_HelpFleens;
	_helpStrlMap[ZoombiniPageType::kHotel] = kSysResStrl2400_HelpHotel;
	_helpStrlMap[ZoombiniPageType::kNet] = kSysResStrl2500_HelpNet;
	_helpStrlMap[ZoombiniPageType::kTunnels] = kSysResStrl1800_HelpTunnels;
	_helpStrlMap[ZoombiniPageType::kSmoke] = kSysResStrl2700_HelpSmoke;
	_helpStrlMap[ZoombiniPageType::kMaze] = kSysResStrl2800_HelpMaze;
	_helpStrlMap[ZoombiniPageType::kLogo] = kSysResStrl2900_HelpPickerUpdate;
}

ZoombiniDialogHelp::~ZoombiniDialogHelp() {
	if (_helpSoundQueue != ZoombiniSound::kInvalidSoundQueueHandle)
		_vm->_sound->deleteSoundQueue(_helpSoundQueue);
}

void ZoombiniDialogHelp::loadFeatures() {
	loadHelpStrings();
	if (_vm->isVersionFamilyTlcV2())
		_helpSoundQueue = _vm->_sound->createSoundQueue();

	// Initialize button states
	ZmbResource soundResId = ZmbResource(ZmbResource::kSystem, kSysResSound0999_ButtonSFX);
	_helpDialogButtonStateMap[kHelpDialogButton01_Prev] = ButtonState(ZoombiniText::kDialogButtonPrev, soundResId, 2, 5, kSystemShape0001_26_HelpDialogPrevButtonNormal, kSystemShape0001_27_HelpDialogPrevButtonPressed);
	_helpDialogButtonStateMap[kHelpDialogButton02_Next] = ButtonState(ZoombiniText::kDialogButtonNext, soundResId, 3, 6, kSystemShape0001_28_HelpDialogNextButtonNormal, kSystemShape0001_29_HelpDialogNextButtonPressed);
	_helpDialogButtonStateMap[kHelpDialogButton03_Okay] = ButtonState(ZoombiniText::kDialogButtonOkay, soundResId, 4, 7, kSystemShape0001_09_ShortGreenButtonNormal, kSystemShape0001_10_ShortGreenButtonPressed);
	updateNavigationButtons();

	// Load SCRBs
	for (Common::StableMap<uint32, ButtonState>::iterator it = _helpDialogButtonStateMap.begin(); it != _helpDialogButtonStateMap.end(); it++)
		it->second.reset();
	ZmbFeature::EventHooks hooks0017;
	hooks0017.setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniDialogHelp::helpDialog_onPreRenderShape));
	hooks0017.setPostRenderFunc(static_cast<ZmbFeature::OnPostRenderFunc>(&ZoombiniDialogHelp::helpDialog_onPostRender));
	hooks0017.setLButtonDownFunc(static_cast<ZmbFeature::OnLButtonDownFunc>(&ZoombiniDialogHelp::helpDialog_onMouseLButtonDown));
	hooks0017.setKeyDownFunc(static_cast<ZmbFeature::OnKeyDownFunc>(&ZoombiniDialogHelp::helpDialog_onKeyDown));
	loadScrbFeature(ZmbResource(ZmbResource::kSystem, kSysResShapeBitmap0001_Dialog), kSysResScrb0017_DialogHelp, 0,
					ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00001000_TOPMOST,
					hooks0017);

	playHelpVoice();
}

void ZoombiniDialogHelp::loadHelpStrings() {
	loadRegularHelpStrings();
	_normalHelpBodyCount = _pageHelpBodyStrs.size();

	if (_showPickerHeaderEasterEgg && _forPageType == ZoombiniPageType::kPicker) {
		_showPickerUpdateHelpBody = true;
		_helpStrlResId = kSysResStrl2900_HelpPickerUpdate;
		Common::Array<Common::U32String> updateHelpBodyStrs;
		if (_vm->_text->getStrl(updateHelpBodyStrs, ZmbResource(ZmbResource::kSystem, _helpStrlResId)))
			_pageHelpBodyStrs = updateHelpBodyStrs;
		else {
			error("help: malformed required picker-update STRL %u", _helpStrlResId);
			return;
		}
	}
}

void ZoombiniDialogHelp::loadRegularHelpStrings() {
	uint16 routeLevel = 0;
	switch (_forPageType) {
	case ZoombiniPageType::kBasecamp1:
	case ZoombiniPageType::kPicker:
	case ZoombiniPageType::kTown:
	case ZoombiniPageType::kBasecamp2:
		break;
	default:
		routeLevel = _vm->_state->readPageRouteLevel(_forPageType);
		break;
	}

	Common::HashMap<ZoombiniPageType, SystemResourceId>::const_iterator it = _helpStrlMap.find(_forPageType);
	if (it == _helpStrlMap.end()) {
		error("help: cannot find help strings for pageType(%u)", static_cast<uint16>(_forPageType));
		return;
	}

	_helpStrlResId = static_cast<uint16>(it->_value) + 20 * routeLevel;
	Common::Array<Common::U32String> parsedHelpBodyStrs;
	if (_vm->_text->getStrl(parsedHelpBodyStrs, ZmbResource(ZmbResource::kSystem, _helpStrlResId))) {
		_pageHelpBodyStrs = parsedHelpBodyStrs;
	} else {
		error("help: malformed required help STRL %u", _helpStrlResId);
		return;
	}
}

void ZoombiniDialogHelp::helpDialog_onPreRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)hsGroup;

	for (uint32 buttonIdx = kHelpDialogButton01_Prev; buttonIdx <= kHelpDialogButton02_Next; buttonIdx++) {
		ButtonState &buttonState = _helpDialogButtonStateMap[buttonIdx];
		if (buttonState._drawEnabled)
			continue;

		if (hotspots.size() <= buttonState._hsNormalId || hotspots.size() <= buttonState._hsPressedId) {
			error("help: required SCRB %d is missing button hotspots", feature->getScrbId());
			return;
		}
		hotspots[buttonState._hsNormalId]._shapeIdx = ZmbHotspot::kShapeNone;
		hotspots[buttonState._hsPressedId]._shapeIdx = ZmbHotspot::kShapeNone;
	}

	genericButton_selectShapes(feature, hotspots, _helpDialogButtonStateMap);
}

void ZoombiniDialogHelp::helpDialog_onPostRender(ZmbFeature *feature) {
	ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;

	// [Post-Animation Events]
	genericButton_action(feature, _helpDialogButtonStateMap, static_cast<ZoombiniPage::OnButtonActionFunc>(&ZoombiniDialogHelp::helpDialog_onPostAnimation));

	{ // [Text Render] Dialog Title
		ZoombiniGraphics::TextConf titleConf;
		titleConf._fontUsage = ZoombiniFontUsage::kFontTitle;
		titleConf._hAlign = Graphics::kTextAlignCenter;
		titleConf._vAlign = Graphics::kTextAlignCenter;
		titleConf._outlineEffect = true;
		titleConf._outlinePalette = 0x0E;
		titleConf._textPalette = ZoombiniGraphics::kColor2D_Black;
		titleConf._wordWrap = false;
		_vm->_gfx->drawText(screenKind, ZoombiniText::kDialogHelpTitle, helpDialog_getTitleRect(), titleConf);
	}

	// [Text Render] Prev/Next/Okay Button Descriptions
	ZoombiniGraphics::TextConf tc;
	tc._hAlign = Graphics::kTextAlignCenter;
	tc._vAlign = Graphics::kTextAlignCenter;
	genericButton_textRender(feature, _helpDialogButtonStateMap,
							 static_cast<ZoombiniPage::ButtonGetRectFunc>(&ZoombiniDialogHelp::helpDialog_getButtonTextRect), tc);

	// [Text Render] String Header
	Common::U32String helpHead;
	if (_showPickerHeaderEasterEgg)
		helpHead = _vm->_text->getLocalizedString(ZoombiniText::kDialogHelpPickerUpdateVersion);
	else {
		helpHead = _vm->_text->getPageName(_forPageType);
		const int16 routeLevel = _vm->_state->readPageRouteLevel(_forPageType);
		if (0 < routeLevel) {
			helpHead += Common::U32String::format(" %d ", routeLevel + 1);
			helpHead += _vm->_text->getLocalizedString(ZoombiniText::kDialogHelpLevel);
		}
	}

	ZoombiniGraphics::TextConf headConf;
	headConf._textPalette = 0x23;
	headConf._wordWrap = false;
	_vm->_gfx->drawText(screenKind, helpHead, helpDialog_getHeadRect(), headConf);

	// [Text Render] String Body
	ZoombiniGraphics::TextConf bodyConf;
	bodyConf._wordWrap = true;
	_vm->_gfx->drawText(screenKind, _pageHelpBodyStrs[_pageHelpBodyIdx], helpDialog_getBodyRect(), bodyConf);
}

void ZoombiniDialogHelp::helpDialog_onPostAnimation(ZmbFeature *feature, uint32 bsIdx, ButtonState &bs) {
	(void)feature;
	(void)bs;

	uint32 previousHelpBodyIdx = _pageHelpBodyIdx;
	bool helpBodyChanged = false;
	if (bsIdx == kHelpDialogButton01_Prev || bsIdx == kHelpDialogButton02_Next)
		_showPickerHeaderEasterEgg = false;

	if (_showPickerUpdateHelpBody &&
		(bsIdx == kHelpDialogButton01_Prev || bsIdx == kHelpDialogButton02_Next)) {
		_showPickerUpdateHelpBody = false;
		loadRegularHelpStrings();
		if (bsIdx == kHelpDialogButton02_Next && 1 < _pageHelpBodyStrs.size())
			_pageHelpBodyIdx = 1;
		else
			_pageHelpBodyIdx = 0;
		helpBodyChanged = true;
	}

	switch (bsIdx) {
	case kHelpDialogButton01_Prev:
		if (!helpBodyChanged && 0 < _pageHelpBodyIdx)
			_pageHelpBodyIdx -= 1;
		break;
	case kHelpDialogButton02_Next:
		if (!helpBodyChanged && _pageHelpBodyIdx + 1 < _pageHelpBodyStrs.size())
			_pageHelpBodyIdx += 1;
		break;
	case kHelpDialogButton03_Okay:
		close();
		break;
	default:
		error("help: invalid help dialog button event(%u)", bsIdx);
		break;
	}

	if (helpBodyChanged || _pageHelpBodyIdx != previousHelpBodyIdx) {
		updateNavigationButtons();
		playHelpVoice();
	}
}

Common::Rect ZoombiniDialogHelp::helpDialog_getButtonTextRect(ZmbFeature *feature, uint32 bsIdx, ButtonState &bs, const Common::Rect &buttonRect) {
	(void)feature;
	(void)buttonRect;

	Common::Rect textRect;
	if (bsIdx < ARRAYSIZE(_helpDialogTextRects))
		textRect = _helpDialogTextRects[bsIdx];
	else
		textRect = _helpDialogButtonRects[bsIdx];

	if (!_vm->isVersionFamilyTlcV2()) {
		textRect.top += 3;
		textRect.bottom += 3;
	}
	if (bs.isAnimating()) {
		textRect.top += 2;
		textRect.bottom += 2;
	}
	return textRect;
}

const Common::Rect &ZoombiniDialogHelp::helpDialog_getTitleRect() const {
	return _vm->isVersionFamilyTlcV2() ? _helpDialogTlcTitleRect : _helpDialogTitleRect;
}

const Common::Rect &ZoombiniDialogHelp::helpDialog_getHeadRect() const {
	return _vm->isVersionFamilyTlcV2() ? _helpDialogTlcHeadRect : _helpDialogHeadRect;
}

const Common::Rect &ZoombiniDialogHelp::helpDialog_getBodyRect() const {
	return _vm->isVersionFamilyTlcV2() ? _helpDialogTlcBodyRect : _helpDialogBodyRect;
}

ZmbEventHandleResult ZoombiniDialogHelp::helpDialog_onMouseLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	(void)relPos;

	const ZmbEventHandleResult result = genericButton_onLButtonDown(feature, absPos, _helpDialogButtonStateMap,
																	static_cast<ZoombiniPage::ButtonGetRectFunc>(&ZoombiniDialogHelp::helpDialog_getButtonClickRect));

	// TLC Help narration is click-to-stop even when the click does not
	// activate a dialog button.
	if (_vm->isVersionFamilyTlcV2() &&
		_helpSoundQueue != ZoombiniSound::kInvalidSoundQueueHandle &&
		_vm->_sound->isSoundQueuePlaying(_helpSoundQueue))
		stopHelpVoice();

	return result;
}

Common::Rect ZoombiniDialogHelp::helpDialog_getButtonClickRect(ZmbFeature *feature, uint32 bsIdx, ButtonState &bs, const Common::Rect &buttonRect) {
	(void)feature;
	(void)bs;
	(void)buttonRect;

	return _helpDialogButtonRects[bsIdx];
}

ZmbEventHandleResult ZoombiniDialogHelp::helpDialog_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat) {
	ZmbEventHandleResult result = ZmbEventHandleResult::kPassthrough;

	if (_vm->isVersionFamilyTlcV2() && kbd.keycode == Common::KEYCODE_SPACE &&
		_helpSoundQueue != ZoombiniSound::kInvalidSoundQueueHandle &&
		_vm->_sound->isSoundQueuePlaying(_helpSoundQueue)) {
		stopHelpVoice();
		return ZmbEventHandleResult::kConsumed;
	}
	if (kbd.hasFlags(Common::KBD_CTRL) && kbd.keycode == Common::KEYCODE_q) {
		ButtonState &okayButton = _helpDialogButtonStateMap[kHelpDialogButton03_Okay];
		if (!kbdRepeat && !okayButton.isAnimating())
			okayButton.press(_vm, feature, _currentFrameCounter);
		return ZmbEventHandleResult::kConsumed;
	}

	// Enhanced keyboard shortcuts.
	if (!_vm->useEnhancedKbdShortcuts())
		return result;

	switch (classifyDialogKey(kbd)) {
	case kDialogKeyAccept:
	case kDialogKeyCancel:
		if (!kbdRepeat && !_helpDialogButtonStateMap[kHelpDialogButton03_Okay].isAnimating())
			_helpDialogButtonStateMap[kHelpDialogButton03_Okay].press(_vm, feature, _currentFrameCounter);
		result = ZmbEventHandleResult::kConsumed;
		break;
	case kDialogKeyNone:
		switch (getKeyboardNavDirection(kbd)) {
		case KBD_NAV_LEFT:
		case KBD_NAV_UP:
		case KBD_NAV_PAGEUP:
			if (!kbdRepeat && _helpDialogButtonStateMap[kHelpDialogButton01_Prev]._drawEnabled &&
				!_helpDialogButtonStateMap[kHelpDialogButton01_Prev].isAnimating()) {
				_helpDialogButtonStateMap[kHelpDialogButton01_Prev].press(_vm, feature, _currentFrameCounter);
				result = ZmbEventHandleResult::kConsumed;
			}
			break;
		case KBD_NAV_RIGHT:
		case KBD_NAV_DOWN:
		case KBD_NAV_PAGEDOWN:
			if (!kbdRepeat && _helpDialogButtonStateMap[kHelpDialogButton02_Next]._drawEnabled &&
				!_helpDialogButtonStateMap[kHelpDialogButton02_Next].isAnimating()) {
				_helpDialogButtonStateMap[kHelpDialogButton02_Next].press(_vm, feature, _currentFrameCounter);
				result = ZmbEventHandleResult::kConsumed;
			}
			break;
		default:
			break;
		}
		break;
	}
	if (result == ZmbEventHandleResult::kConsumed)
		stopHelpVoice();
	return result;
}

void ZoombiniDialogHelp::updateNavigationButtons() {
	const uint32 helpBodyCount = _showPickerUpdateHelpBody ? _normalHelpBodyCount : _pageHelpBodyStrs.size();
	if (helpBodyCount <= 1) {
		_helpDialogButtonStateMap[kHelpDialogButton01_Prev]._drawEnabled = false;
		_helpDialogButtonStateMap[kHelpDialogButton02_Next]._drawEnabled = false;
	} else if (_vm->isVersionFamilyTlcV2()) {
		_helpDialogButtonStateMap[kHelpDialogButton01_Prev]._drawEnabled = 0 < _pageHelpBodyIdx;
		_helpDialogButtonStateMap[kHelpDialogButton02_Next]._drawEnabled = _pageHelpBodyIdx + 1 < helpBodyCount;
	} else {
		_helpDialogButtonStateMap[kHelpDialogButton01_Prev]._drawEnabled = true;
		_helpDialogButtonStateMap[kHelpDialogButton02_Next]._drawEnabled = true;
	}
}

void ZoombiniDialogHelp::stopHelpVoice() {
	if (_helpSoundQueue != ZoombiniSound::kInvalidSoundQueueHandle)
		_vm->_sound->stopSoundQueue(_helpSoundQueue);
}

void ZoombiniDialogHelp::playHelpVoice() {
	if (!_vm->isVersionFamilyTlcV2())
		return;

	stopHelpVoice();

	if (!_vm->_state->getEnableHelpAudio())
		return;
	if (_helpSoundQueue == ZoombiniSound::kInvalidSoundQueueHandle)
		return;
	if (!_helpStrlResId)
		return;

	const uint32 computedVoiceResId = static_cast<uint32>(_helpStrlResId) + _pageHelpBodyIdx + kSysResSound20000_Base;
	if (0x7FFF < computedVoiceResId)
		return;
	const int16 voiceResId = static_cast<int16>(computedVoiceResId);

	ZmbResource voiceRes(ZmbResource::kSystem, voiceResId);
	if (!_vm->hasResource(ID_SND, voiceRes)) {
		warning("help: missing TLC help voice SND %d from HELP.MHK", voiceResId);
		return;
	}

	_vm->_sound->queueSound(_helpSoundQueue, voiceRes, Audio::Mixer::kSpeechSoundType, false);
}

} // End of namespace Mohawk
