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

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

#include "common/config-manager.h"

#include "mohawk/zoombini.h"
#include "mohawk/zoombini_graphics.h"
#include "mohawk/zoombini_metaengine.h"
#include "mohawk/zoombini_pages/dialog_options.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_sound.h"
#include "mohawk/zoombini_state.h"

namespace Mohawk {

constexpr uint16 ZoombiniDialogOptions::_optionDialogTextTops[8];
constexpr uint16 ZoombiniDialogOptions::_optionDialogTlcTextTops[10];

static Common::U32String formatOptionDialogShortcut(const Common::HardwareInput &input) {
	if (input.type != Common::kHardwareInputTypeKeyboard)
		return input.description;

	Common::U32String description = input.description;
	description.toUppercase();
	for (uint i = 0; i < description.size(); i++) {
		if (description[i] == U'+')
			description[i] = U' ';
	}
	return description;
}

ZoombiniDialogOptions::ZoombiniDialogOptions(MohawkEngine_Zoombini *vm) : ZoombiniDialog(vm, ZoombiniPageType::kDialogOptions) {
	ZmbResource soundResId(ZmbResource::kSystem, kSysResSound0999_ButtonSFX);
	const bool isTlc = _vm->isVersionFamilyTlcV2();
	const uint16 redPressedBaseIdx = isTlc ? 10 : 8;
	const uint16 togglePressedBaseIdx = isTlc ? 14 : 12;

	_redButtonStateMap[kOptionDialogButton01_NewGame] = ButtonState(ZoombiniText::kOptionsNewGame, soundResId, 0, redPressedBaseIdx, kSystemShape0001_03_OptionsRedButtonNormal, kSystemShape0001_04_OptionsRedButtonPressed);
	_redButtonStateMap[kOptionDialogButton02_LoadGame] = ButtonState(ZoombiniText::kOptionsLoadGame, soundResId, 1, redPressedBaseIdx + 1, kSystemShape0001_03_OptionsRedButtonNormal, kSystemShape0001_04_OptionsRedButtonPressed);
	_redButtonStateMap[kOptionDialogButton03_SaveGame] = ButtonState(ZoombiniText::kOptionsSaveGame, soundResId, 2, redPressedBaseIdx + 2, kSystemShape0001_03_OptionsRedButtonNormal, kSystemShape0001_04_OptionsRedButtonPressed);
	_redButtonStateMap[kOptionDialogButton04_Quit] = ButtonState(ZoombiniText::kOptionsQuit, soundResId, 3, redPressedBaseIdx + 3, kSystemShape0001_03_OptionsRedButtonNormal, kSystemShape0001_04_OptionsRedButtonPressed);

	_toggleButtonStateMap[kOptionDialogButton05_Sound] = ToggleButtonState(ZoombiniText::kOptionsSound, soundResId, 4, togglePressedBaseIdx, kOptionDialogToggleTrueNormalShape, kOptionDialogToggleTruePressedShape, kOptionDialogToggleFalseNormalShape, kOptionDialogToggleFalsePressedShape);
	_toggleButtonStateMap[kOptionDialogButton06_Music] = ToggleButtonState(ZoombiniText::kOptionsMusic, soundResId, 5, togglePressedBaseIdx + 1, kOptionDialogToggleTrueNormalShape, kOptionDialogToggleTruePressedShape, kOptionDialogToggleFalseNormalShape, kOptionDialogToggleFalsePressedShape);
	_toggleButtonStateMap[kOptionDialogButton07_StickyMouse] = ToggleButtonState(ZoombiniText::kOptionsStickyMouse, soundResId, 6, togglePressedBaseIdx + 2, kOptionDialogToggleTrueNormalShape, kOptionDialogToggleTruePressedShape, kOptionDialogToggleFalseNormalShape, kOptionDialogToggleFalsePressedShape);
	_toggleButtonStateMap[kOptionDialogButton08_Transitions] = ToggleButtonState(ZoombiniText::kOptionsTransitions, soundResId, 7, togglePressedBaseIdx + 3, kOptionDialogToggleTrueNormalShape, kOptionDialogToggleTruePressedShape, kOptionDialogToggleFalseNormalShape, kOptionDialogToggleFalsePressedShape);
	if (isTlc) {
		_toggleButtonStateMap[kOptionDialogButton09_TouchSense] = ToggleButtonState(ZoombiniText::kOptionsTouchSense, soundResId, 8, togglePressedBaseIdx + 4, kOptionDialogToggleTrueNormalShape, kOptionDialogToggleTruePressedShape, kOptionDialogToggleFalseNormalShape, kOptionDialogToggleFalsePressedShape);
		_toggleButtonStateMap[kOptionDialogButton10_HelpAudio] = ToggleButtonState(ZoombiniText::kOptionsHelpAudio, soundResId, 9, togglePressedBaseIdx + 5, kOptionDialogToggleTrueNormalShape, kOptionDialogToggleTruePressedShape, kOptionDialogToggleFalseNormalShape, kOptionDialogToggleFalsePressedShape);
	}

	_longButtonStateMap[kOptionDialogButton11_Okay] = ButtonState(ZoombiniText::kDialogButtonOkay, soundResId, 0, 2, kSystemShape0001_09_ShortGreenButtonNormal, kSystemShape0001_10_ShortGreenButtonPressed);
	_longButtonStateMap[kOptionDialogButton12_Credits] = ButtonState(ZoombiniText::kOptionsCredits, soundResId, 1, 3, kSystemShape0001_14_LongRedButtonNormal, kSystemShape0001_15_LongRedButtonPressed);
}

ZoombiniDialogOptions::~ZoombiniDialogOptions() {
}

void ZoombiniDialogOptions::loadFeatures() {
	loadScrbFeature(ZmbResource(ZmbResource::kSystem, kSysResShapeBitmap0001_Dialog), kSysResScrb0001_DialogOptionsFrame, 0,
					ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00001000_TOPMOST);

	for (Common::StableMap<uint32, ButtonState>::iterator it = _redButtonStateMap.begin(); it != _redButtonStateMap.end(); it++)
		it->second.reset();
	for (Common::StableMap<uint32, ToggleButtonState>::iterator it = _toggleButtonStateMap.begin(); it != _toggleButtonStateMap.end(); it++)
		it->second.reset();
	ZmbFeature::EventHooks hooks0002;
	hooks0002.setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniDialogOptions::redToggleButtons_onPreRenderShape));
	hooks0002.setPostRenderFunc(static_cast<ZmbFeature::OnPostRenderFunc>(&ZoombiniDialogOptions::redToggleButtons_onPostRender));
	hooks0002.setLButtonDownFunc(static_cast<ZmbFeature::OnLButtonDownFunc>(&ZoombiniDialogOptions::redToggleButtons_onLButtonDown));
	hooks0002.setKeyDownFunc(static_cast<ZmbFeature::OnKeyDownFunc>(&ZoombiniDialogOptions::redToggleButtons_onKeyDown));
	loadScrbFeature(ZmbResource(ZmbResource::kSystem, kSysResShapeBitmap0001_Dialog), kSysResScrb0002_DialogOptionsSmallButtons, 1,
					ZmbFeature::FLAG_04000000_OVERLAY,
					hooks0002);

	resetLongButtonStates();
	ZmbFeature::EventHooks hooks0003;
	hooks0003.setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniDialogOptions::longButtons_onPreRenderShape));
	hooks0003.setPostRenderFunc(static_cast<ZmbFeature::OnPostRenderFunc>(&ZoombiniDialogOptions::longButtons_onPostRender));
	hooks0003.setLButtonDownFunc(static_cast<ZmbFeature::OnLButtonDownFunc>(&ZoombiniDialogOptions::longButtons_onLButtonDown));
	hooks0003.setKeyDownFunc(static_cast<ZmbFeature::OnKeyDownFunc>(&ZoombiniDialogOptions::longButtons_onKeyDown));
	loadScrbFeature(ZmbResource(ZmbResource::kSystem, kSysResShapeBitmap0001_Dialog), kSysResScrb0003_DialogOptionsBigButtons, 9,
					ZmbFeature::FLAG_04000000_OVERLAY,
					hooks0003);
}

bool ZoombiniDialogOptions::getOptionDialogToggleValue(uint16 bsIdx) {
	switch (bsIdx) {
	case kOptionDialogButton05_Sound:
		return _vm->_state->getEnableSound();
	case kOptionDialogButton06_Music:
		return _vm->_state->getEnableMusic();
	case kOptionDialogButton07_StickyMouse:
		return _vm->_state->getEnableStickyMouse();
	case kOptionDialogButton08_Transitions:
		return _vm->_state->getEnableTransitions();
	case kOptionDialogButton09_TouchSense:
		return _vm->_state->getEnableTouchSense();
	case kOptionDialogButton10_HelpAudio:
		return _vm->_state->getEnableHelpAudio();
	default:
		error("options: invalid option dialog toggle button idx: %u", bsIdx);
		return false;
	}
}

void ZoombiniDialogOptions::redToggleButtons_onPreRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)hsGroup;

	genericButton_selectShapes(feature, hotspots, _redButtonStateMap);

	for (Common::StableMap<uint32, ToggleButtonState>::iterator it = _toggleButtonStateMap.begin(); it != _toggleButtonStateMap.end(); it++) {
		ToggleButtonState &bs = it->second;
		bs._toggleState = getOptionDialogToggleValue(it->first);
	}

	genericToggleButton_selectShapes(feature, hotspots, _toggleButtonStateMap);
}

void ZoombiniDialogOptions::redToggleButtons_onPostRender(ZmbFeature *feature) {
	ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;
	const bool portuguese10Layout = _vm->isGameVariant(MohawkGameFeatures::GF_ZMB_10_ESPT) && _vm->getLanguage() == Common::PT_PRT;

	{ // [Text Render] Dialog Title
		ZoombiniGraphics::TextConf titleConf;
		titleConf._fontUsage = ZoombiniFontUsage::kFontTitle;
		titleConf._hAlign = Graphics::kTextAlignCenter;
		titleConf._vAlign = Graphics::kTextAlignCenter;
		titleConf._outlineEffect = true;
		titleConf._outlinePalette = 0x0E;
		titleConf._textPalette = ZoombiniGraphics::kColor2D_Black;
		titleConf._wordWrap = portuguese10Layout;
		_vm->_gfx->drawText(screenKind, ZoombiniText::kOptionsTitle, getOptionDialogTitleRect(), titleConf);
	}

	{ // [Text Render] Toggle Title & Legend
		ZoombiniGraphics::TextConf bigConf;
		bigConf._fontUsage = ZoombiniFontUsage::kFontTitle;
		bigConf._textPalette = ZoombiniGraphics::kColor2D_Black;
		bigConf._wordWrap = portuguese10Layout;
		if (!_vm->isVersionFamilyTlcV2())
			_vm->_gfx->drawText(screenKind, ZoombiniText::kOptionsToggle, _optionDialogToggleRect, bigConf);
		_vm->_gfx->drawText(screenKind, ZoombiniText::kOptionsLegendOn, _optionDialogLegendOnRect, bigConf);
		_vm->_gfx->drawText(screenKind, ZoombiniText::kOptionsLegendOff, _optionDialogLegendOffRect, bigConf);
	}

	// [Text Render] Small Button Descriptions
	ZoombiniGraphics::TextConf tc;
	tc._textPalette = ZoombiniGraphics::kColor2D_Black;
	tc._wordWrap = portuguese10Layout;
	optionButtonTextRender(feature, _redButtonStateMap, static_cast<ButtonGetRectFunc>(&ZoombiniDialogOptions::redButtons_textRect), tc);
	optionToggleButtonTextRender(feature, _toggleButtonStateMap, static_cast<ToggleButtonGetRectFunc>(&ZoombiniDialogOptions::toggleButtons_textRect), tc);

	// [Post-Animation Events]
	genericButton_action(feature, _redButtonStateMap, static_cast<OnButtonActionFunc>(&ZoombiniDialogOptions::redButtons_onButtonAction));
	genericToggleButton_postAnimation(feature, _toggleButtonStateMap,
									  static_cast<OnToggleButtonPostAnimationFunc>(&ZoombiniDialogOptions::toggleButtons_onButtonAction));
}

Common::Rect ZoombiniDialogOptions::redButtons_textRect(ZmbFeature *feature, uint32 bsIdx, ButtonState &bs, const Common::Rect &drawnRect) {
	(void)feature;
	(void)bs;

	uint16 top = getOptionDialogTextTop(bsIdx);
	return Common::Rect(_optionDialogTextLeft, top, _optionDialogTextRight, top + drawnRect.height());
}

Common::Rect ZoombiniDialogOptions::toggleButtons_textRect(ZmbFeature *feature, uint32 bsIdx, ToggleButtonState &bs, const Common::Rect &drawnRect) {
	(void)feature;
	(void)bs;

	uint16 top = getOptionDialogTextTop(bsIdx);
	return Common::Rect(_optionDialogTextLeft, top, _optionDialogTextRight, top + drawnRect.height());
}

const char *ZoombiniDialogOptions::getOptionDialogActionId(ZoombiniText::Key textKey) const {
	switch (textKey) {
	case ZoombiniText::kOptionsNewGame:
		return MohawkMetaEngine_Zoombini::kActionNewGame;
	case ZoombiniText::kOptionsLoadGame:
		return Common::kStandardActionLoad;
	case ZoombiniText::kOptionsSaveGame:
		return Common::kStandardActionSave;
	case ZoombiniText::kOptionsQuit:
		return MohawkMetaEngine_Zoombini::kActionQuit;
	case ZoombiniText::kOptionsSound:
		return MohawkMetaEngine_Zoombini::kActionToggleDialogAndSfx;
	case ZoombiniText::kOptionsMusic:
		return MohawkMetaEngine_Zoombini::kActionToggleMusic;
	case ZoombiniText::kOptionsStickyMouse:
		return MohawkMetaEngine_Zoombini::kActionToggleStickyMouse;
	case ZoombiniText::kOptionsTransitions:
		return MohawkMetaEngine_Zoombini::kActionToggleTransitions;
	case ZoombiniText::kOptionsTouchSense:
		return MohawkMetaEngine_Zoombini::kActionToggleTouchSense;
	case ZoombiniText::kOptionsHelpAudio:
		return MohawkMetaEngine_Zoombini::kActionToggleHelpDialogNarration;
	default:
		return nullptr;
	}
}

Common::U32String ZoombiniDialogOptions::getOptionDialogText(ZoombiniText::Key textKey) const {
	const Common::U32String &originalText = _vm->_text->getLocalizedString(textKey);
	if (!ConfMan.getBool(MohawkMetaEngine_Zoombini::kOptionShowRemappedOptionDialogShortcuts))
		return originalText;

	const char *actionId = getOptionDialogActionId(textKey);
	if (!actionId)
		return originalText;

	Common::Keymapper *keymapper = _vm->getEventManager()->getKeymapper();
	if (!keymapper)
		return originalText;

	Common::Keymap *keymap = keymapper->getKeymap("zoombini");
	if (!keymap)
		return originalText;

	Common::Action *action = nullptr;
	for (Common::Action *candidate : keymap->getActions()) {
		if (Common::String(candidate->id).equals(actionId)) {
			action = candidate;
			break;
		}
	}
	if (!action)
		return originalText;

	const Common::Array<Common::HardwareInput> mappedInputs = keymap->getActionMapping(action);
	const Common::StringArray defaultMappings = keymap->getActionDefaultMappings(action);
	bool mappingsChanged = mappedInputs.size() != defaultMappings.size();
	for (uint i = 0; !mappingsChanged && i < mappedInputs.size(); i++) {
		bool foundDefault = false;
		for (uint j = 0; j < defaultMappings.size(); j++) {
			if (mappedInputs[i].id == defaultMappings[j]) {
				foundDefault = true;
				break;
			}
		}
		if (!foundDefault)
			mappingsChanged = true;
	}
	if (!mappingsChanged)
		return originalText;

	if (originalText.empty())
		return originalText;
	const size_t shortcutStart = originalText.rfind('(');
	const size_t shortcutEnd = originalText.rfind(')');
	if (shortcutStart == Common::U32String::npos || shortcutEnd == Common::U32String::npos ||
		shortcutEnd <= shortcutStart || shortcutEnd != originalText.size() - 1)
		return originalText;

	Common::U32String mappedText;
	for (uint i = 0; i < mappedInputs.size(); i++) {
		if (!mappedText.empty())
			mappedText += U", ";
		mappedText += formatOptionDialogShortcut(mappedInputs[i]);
	}

	Common::U32String filteredText = originalText.substr(0, shortcutStart);
	if (mappedText.empty()) {
		if (!filteredText.empty() && filteredText.lastChar() == ' ')
			filteredText.deleteLastChar();
	} else {
		filteredText += U"(";
		filteredText += mappedText;
		filteredText += U")";
	}
	filteredText += originalText.substr(shortcutEnd + 1);
	return filteredText;
}

Common::U32String ZoombiniDialogOptions::truncateOptionDialogText(const Common::U32String &text,
																  const Common::Rect &textRect, const ZoombiniGraphics::TextConf &tc) const {
	if (_vm->_gfx->getTextWidth(text, tc) <= textRect.width())
		return text;

	const size_t shortcutStart = text.rfind('(');
	const size_t shortcutEnd = text.rfind(')');
	if (shortcutStart == Common::U32String::npos || shortcutEnd == Common::U32String::npos ||
		shortcutEnd <= shortcutStart || shortcutEnd != text.size() - 1)
		return text;

	const Common::U32String prefix = text.substr(0, shortcutStart + 1);
	const Common::U32String shortcut = text.substr(shortcutStart + 1, shortcutEnd - shortcutStart - 1);
	const Common::U32String suffix = text.substr(shortcutEnd);
	const Common::U32String ellipsis(U"...");
	Common::U32String truncatedText = prefix;

	for (uint i = 0; i < shortcut.size(); i++) {
		Common::U32String candidate = truncatedText;
		candidate += shortcut[i];
		candidate += ellipsis;
		candidate += suffix;
		if (_vm->_gfx->getTextWidth(candidate, tc) > textRect.width())
			break;
		truncatedText += shortcut[i];
	}

	while (!truncatedText.empty() && truncatedText.lastChar() == ' ')
		truncatedText.deleteLastChar();
	truncatedText += ellipsis;
	truncatedText += suffix;
	return truncatedText;
}

void ZoombiniDialogOptions::optionButtonTextRender(ZmbFeature *feature,
												   Common::StableMap<uint32, ButtonState> &buttonStateMap,
												   ButtonGetRectFunc textRectFunc,
												   const ZoombiniGraphics::TextConf &tc) {
	ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;

	for (Common::StableMap<uint32, ButtonState>::iterator it = buttonStateMap.begin(); it != buttonStateMap.end(); it++) {
		ButtonState &bs = it->second;
		if (!bs._drawEnabled)
			continue;

		ZmbDrawRecord *record = feature->findDrawRecordByHotspotIdx(bs._hsNormalId, bs._hsPressedId);
		if (!record)
			continue;

		Common::Rect textRect = (this->*textRectFunc)(feature, it->first, bs, record->_drawnRect);
		if (bs._textKey != ZoombiniText::kNone) {
			const Common::U32String text = truncateOptionDialogText(getOptionDialogText(bs._textKey), textRect, tc);
			_vm->_gfx->drawText(screenKind, text, textRect, tc);
		}
	}
}

void ZoombiniDialogOptions::optionToggleButtonTextRender(ZmbFeature *feature,
														 Common::StableMap<uint32, ToggleButtonState> &buttonStateMap,
														 ToggleButtonGetRectFunc textRectFunc,
														 const ZoombiniGraphics::TextConf &tc) {
	ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;

	for (Common::StableMap<uint32, ToggleButtonState>::iterator it = buttonStateMap.begin(); it != buttonStateMap.end(); it++) {
		ToggleButtonState &bs = it->second;
		if (!bs._enabled)
			continue;

		ZmbDrawRecord *record = feature->findDrawRecordByHotspotIdx(bs._hsNormalId, bs._hsPressedId);
		if (!record)
			continue;

		Common::Rect textRect = (this->*textRectFunc)(feature, it->first, bs, record->_drawnRect);
		if (bs._textKey != ZoombiniText::kNone) {
			const Common::U32String text = truncateOptionDialogText(getOptionDialogText(bs._textKey), textRect, tc);
			_vm->_gfx->drawText(screenKind, text, textRect, tc);
		}
	}
}

const Common::Rect &ZoombiniDialogOptions::getOptionDialogTitleRect() const {
	return _vm->isVersionFamilyTlcV2() ? _optionDialogTlcTitleRect : _optionDialogTitleRect;
}

uint16 ZoombiniDialogOptions::getOptionDialogTextTop(uint32 bsIdx) const {
	if (_vm->isVersionFamilyTlcV2()) {
		if (bsIdx < ARRAYSIZE(_optionDialogTlcTextTops))
			return _optionDialogTlcTextTops[bsIdx];
	} else if (bsIdx < ARRAYSIZE(_optionDialogTextTops)) {
		return _optionDialogTextTops[bsIdx];
	}

	error("options: invalid option dialog text idx: %u", bsIdx);
	return 0;
}

void ZoombiniDialogOptions::redButtons_onButtonAction(ZmbFeature *feature, uint32 bsIdx, ButtonState &bs) {
	(void)feature;
	(void)bs;

	switch (bsIdx) {
	case kOptionDialogButton01_NewGame:
		_vm->_state->startNewGame(false);
		break;
	case kOptionDialogButton02_LoadGame:
		_vm->openLoadDialog(false);
		break;
	case kOptionDialogButton03_SaveGame:
		_vm->openSaveDialog();
		break;
	case kOptionDialogButton04_Quit:
		_vm->requestQuit();
		break;
	default:
		error("options: invalid option dialog red button event(%u)", bsIdx);
		break;
	}
}

void ZoombiniDialogOptions::toggleButtons_onButtonAction(ZmbFeature *feature, uint32 bsIdx, ToggleButtonState &bs) {
	(void)feature;
	(void)bs;

	switch (bsIdx) {
	case kOptionDialogButton05_Sound:
		_vm->_state->toggleSound(false);
		break;
	case kOptionDialogButton06_Music:
		_vm->_state->toggleMusic(false);
		break;
	case kOptionDialogButton07_StickyMouse:
		_vm->_state->toggleStickyMouse(false);
		break;
	case kOptionDialogButton08_Transitions:
		_vm->_state->toggleTransitions(false);
		break;
	case kOptionDialogButton09_TouchSense:
		_vm->_state->toggleTouchSense(false);
		break;
	case kOptionDialogButton10_HelpAudio:
		_vm->_state->toggleHelpAudio(false);
		break;
	default:
		error("options: invalid option dialog toggle button event(%u)", bsIdx);
		break;
	}
}

ZmbEventHandleResult ZoombiniDialogOptions::redToggleButtons_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	(void)relPos;

	ZmbEventHandleResult result = genericButton_onLButtonDown(feature, absPos, _redButtonStateMap);
	if (result == ZmbEventHandleResult::kConsumed)
		return result;

	return genericToggleButton_onLButtonDown(feature, absPos, _toggleButtonStateMap);
}

ZmbEventHandleResult ZoombiniDialogOptions::redToggleButtons_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat) {
	if (!kbd.hasFlags(Common::KBD_CTRL))
		return ZmbEventHandleResult::kPassthrough;

	ButtonState *buttonState = nullptr;
	ToggleButtonState *toggleState = nullptr;
	switch (kbd.keycode) {
	case Common::KEYCODE_n:
		buttonState = &_redButtonStateMap[kOptionDialogButton01_NewGame];
		break;
	case Common::KEYCODE_l:
		buttonState = &_redButtonStateMap[kOptionDialogButton02_LoadGame];
		break;
	case Common::KEYCODE_s:
		buttonState = &_redButtonStateMap[kOptionDialogButton03_SaveGame];
		break;
	case Common::KEYCODE_q:
		buttonState = &_redButtonStateMap[kOptionDialogButton04_Quit];
		break;
	case Common::KEYCODE_d:
		toggleState = &_toggleButtonStateMap[kOptionDialogButton05_Sound];
		break;
	case Common::KEYCODE_b:
		toggleState = &_toggleButtonStateMap[kOptionDialogButton06_Music];
		break;
	case Common::KEYCODE_j:
		toggleState = &_toggleButtonStateMap[kOptionDialogButton07_StickyMouse];
		break;
	case Common::KEYCODE_t:
		toggleState = &_toggleButtonStateMap[kOptionDialogButton08_Transitions];
		break;
	case Common::KEYCODE_k:
		if (_vm->isVersionFamilyTlcV2())
			toggleState = &_toggleButtonStateMap[kOptionDialogButton09_TouchSense];
		break;
	case Common::KEYCODE_a:
		if (_vm->isVersionFamilyTlcV2())
			toggleState = &_toggleButtonStateMap[kOptionDialogButton10_HelpAudio];
		break;
	default:
		break;
	}

	if (buttonState) {
		if (!kbdRepeat && !buttonState->isAnimating())
			buttonState->press(_vm, feature, _currentFrameCounter);
		return ZmbEventHandleResult::kConsumed;
	}
	if (toggleState) {
		if (!kbdRepeat && !toggleState->isAnimating())
			toggleState->press(_vm, feature, _currentFrameCounter);
		return ZmbEventHandleResult::kConsumed;
	}

	return ZmbEventHandleResult::kPassthrough;
}

void ZoombiniDialogOptions::longButtons_onPreRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)hsGroup;

	genericButton_selectShapes(feature, hotspots, _longButtonStateMap);
}

void ZoombiniDialogOptions::longButtons_onPostRender(ZmbFeature *feature) {
	// [Text Render] Big Button Descriptions
	ZoombiniGraphics::TextConf tc;
	tc._fontUsage = ZoombiniFontUsage::kFontTitle;
	tc._hAlign = Graphics::kTextAlignCenter;
	tc._vAlign = Graphics::kTextAlignCenter;
	genericButton_textRender(feature, _longButtonStateMap, tc, -1, 1);

	// [Post-Animation Events]
	genericButton_action(feature, _longButtonStateMap, static_cast<OnButtonActionFunc>(&ZoombiniDialogOptions::longButtons_onButtonAction));
}

void ZoombiniDialogOptions::longButtons_onButtonAction(ZmbFeature *feature, uint32 bsIdx, ButtonState &bs) {
	(void)feature;
	(void)bs;

	switch (bsIdx) {
	case kOptionDialogButton11_Okay:
		close();
		break;
	case kOptionDialogButton12_Credits:
		_vm->openCreditsDialog();
		break;
	default:
		error("options: invalid option dialog long button event(%u)", bsIdx);
		break;
	}
}

ZmbEventHandleResult ZoombiniDialogOptions::longButtons_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	(void)relPos;

	return genericButton_onLButtonDown(feature, absPos, _longButtonStateMap);
}

ZmbEventHandleResult ZoombiniDialogOptions::longButtons_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat) {
	switch (classifyDialogKey(kbd)) {
	case kDialogKeyAccept:
	case kDialogKeyCancel:
		if (!kbdRepeat && !_longButtonStateMap[kOptionDialogButton11_Okay].isAnimating())
			_longButtonStateMap[kOptionDialogButton11_Okay].press(_vm, feature, _currentFrameCounter);
		return ZmbEventHandleResult::kConsumed;
	case kDialogKeyNone:
		break;
	}
	return ZmbEventHandleResult::kPassthrough;
}

} // End of namespace Mohawk
