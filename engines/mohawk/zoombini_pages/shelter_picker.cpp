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
#include "mohawk/zoombini_pages/shelter_picker.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_sound.h"
#include "mohawk/zoombini_state.h"

namespace Mohawk {

constexpr int16 ZoombiniShelterPicker::kEmbarkOrder[4];
constexpr Common::Point ZoombiniShelterPicker::_previewTraitOffsets[21];
constexpr Common::Point ZoombiniShelterPicker::_embarkDestination;
constexpr Common::Point ZoombiniShelterPicker::_caveMarkRegPoint;
constexpr Common::Point ZoombiniShelterPicker::_zoombiniSeatPoints[16];

ZoombiniShelterPicker::ZoombiniShelterPicker(MohawkEngine_Zoombini *vm) : ZoombiniShelter(vm, ZoombiniPageType::kPicker),
																		  _previewSnoid(vm, 0, ZmbFeature::FLAG_00000001_TYPE_SNOID) {
	_mode = kPickerMode_SelectZoombinis;
	_clickZoneRadius = kPickerClickZoneRadius;
	_isFirstVisit = _vm->_state->isFirstLaunch();
	if (_isFirstVisit && 0 < _vm->_state->_r.getEntryCount() && _vm->_state->_currentSaveSlot == ZoombiniGameState::kUnsavedNewGame) {
		_mode = kPickerMode_LoadGame;
	}

	ZmbResource matrixPressSoundResId(ZmbResource::kPage, kResSound1000_PressMatrixButton);
	ZmbResource matrixReleaseSoundResId(ZmbResource::kPage, kResSound1004_ReleaseMatrixButton);
	for (uint32 i = 0; i < ARRAYSIZE(_pickerMatrixRects); i++) {
		_matrixButtonRectMap[i] = _pickerMatrixRects[i];

		uint16 normalShapeId = 2 * i + 1;
		uint16 pressedShapeId = 2 * i + 2;
		_matrixButtonStateMap[i] = StickyButtonState(matrixPressSoundResId, matrixReleaseSoundResId, i, i + 20, normalShapeId, pressedShapeId);
	}

	ZmbResource generateSoundResId(ZmbResource::kPage, kResSound1005_PressGenerateButton);
	ZmbResource diceSoundResId(ZmbResource::kPage, kResSound1006_PressDiceButton);
	_pickerButtonStateMap[kPickerButtons_Generate] = ButtonState(generateSoundResId, kHotspotGenerateButtonNormal, kHotspotGenerateButtonPressed, kShape4200_02_GenerateButtonNormal, kShape4200_03_GenerateButtonPressed);
	_pickerButtonStateMap[kPickerButtons_Generate].setDisabledState(kShape4200_01_GenerateButtonDisabled);
	// Z1-20U/TLC v2.0 release only: picker buttons gain yellow-outline hover states.
	if (_vm->isVersionFamilyTlcV2())
		_pickerButtonStateMap[kPickerButtons_Generate].setHoverState(kShape4200_14_GenerateButtonHover);
	_pickerButtonStateMap[kPickerButtons_Generate]._isPressDisabled = true; // Disabled until all matrix rows have a selection
	_pickerButtonStateMap[kPickerButtons_Dice] = ButtonState(diceSoundResId, kHotspotDiceButtonNormal, kHotspotDiceButtonPressed, kShape4200_04_DiceButtonNormal, kShape4200_05_DiceButtonPressed);
	if (_vm->isVersionFamilyTlcV2())
		_pickerButtonStateMap[kPickerButtons_Dice].setHoverState(kShape4200_15_DiceButtonHover);
	_pickerButtonRectMap[kPickerButtons_Generate] = _generateButtonRect;
	_pickerButtonRectMap[kPickerButtons_Dice] = _diceButtonRect;
}

ZoombiniShelterPicker::~ZoombiniShelterPicker() {
}

const ZoombiniPage::ScriptSoundPriorityRanges &ZoombiniShelterPicker::getScriptSoundPriorityRanges() const {
	static const ZoombiniPage::ScriptSoundPriorityRanges kRanges = {
		{kSysResSoundRange20000_Base, kSysResSoundRange29999_Last},
		{kResSoundRange1000_PressMatrixButton, kResSoundRange1007_RemoveZoombini}};
	return kRanges;
}

void ZoombiniShelterPicker::open() {
	// The preview name is selected before any Picker archive is opened.
	generateZoombiniName();

	if (_vm->isVersionFamilyTlcV2())
		openArchive(ZMB_MHK_MUSIC);
	else
		// v1.x MIDI BGM (tMID 30000-30001).
		// @ref ZoombiniPage::openMidiArchive() loads MIDIMPC.MHK
		// (Windows profile) or, when "use_mac_midi" is set, MIDIMAC.MHK
		// (Macintosh profile). Both hold the same tMID IDs.
		openMidiArchive();
	openArchive(ZMB_MHK_PICKER);
}

void ZoombiniShelterPicker::setBackgroundMusic() {
	// [Deliberate timing adaptation]
	// The original queues Picker music after feature materialization and after registering
	// its non-blocking startup Load dialog. ScummVM starts it at the standard background-
	// music lifecycle point because its Load dialog is synchronous. This keeps the music
	// audible while that dialog is open and does not add, remove, or move any RNG call.
	if (_vm->isVersionFamilyTlcV2())
		_vm->_sound->playSound(ZmbResource(ZmbResource::kPage, kResSound30001_Isle), Audio::Mixer::kMusicSoundType);
	else
		_vm->_midi->playMidi(ZmbResource(ZmbResource::kPage, kResMidi30001_Isle));

	_pickerEntryNarrationPending = true;
}

void ZoombiniShelterPicker::setBackgroundBitmap() {
	_vm->_gfx->setPalette(kResBackground4000);
	_vm->_gfx->drawBackground(kResBackground4000);
}

bool ZoombiniShelterPicker::hasPickerRoomForVoicePrompt() const {
	const ZmbStateFile &f = _vm->_state->_f;
	const int16 activeCount = static_cast<int16>(_snoidMap.size());
	const int32 remaining = 625 - static_cast<int32>(f._zmbStoredTownCount) - static_cast<int32>(f._zmbStoredBC2Count) - static_cast<int32>(f._zmbStoredBC1Count) - activeCount;

	return 0 < remaining && activeCount < 625;
}

int16 ZoombiniShelterPicker::getAfterVideoVoiceSoundId() {
	if (!hasPickerRoomForVoicePrompt())
		return 0;

	// Only rolls 1 and 10 trigger a voice.
	const int16 roll = _vm->_rnd->getRandomNumber(1, 20);
	if (roll == 1)
		return kSysResSound20043_PickerAfterVideoVoice;
	if (roll == 10)
		return kSysResSound20044_PickerAfterVideoVoice;
	return 0;
}

int16 ZoombiniShelterPicker::getNoDepartureVoiceSoundId() {
	if (!hasPickerRoomForVoicePrompt())
		return 0;

	// Choose one of the two no-departure prompts.
	if (_vm->_rnd->getRandomNumber(1, 2) == 1)
		return kSysResSound20043_PickerAfterVideoVoice;
	return kSysResSound20044_PickerAfterVideoVoice;
}

Audio::SoundHandle *ZoombiniShelterPicker::playPickerVoice(int16 soundId, Audio::Mixer::SoundType soundType) {
	if (soundId == 0)
		return nullptr;

	return _vm->_sound->playSound(ZmbResource(ZmbResource::kSystem, soundId), soundType, false);
}

void ZoombiniShelterPicker::playPendingPickerVoice(int16 soundId) {
	if (soundId == 0)
		return;

	Audio::SoundHandle *handle = playPickerVoice(soundId, Audio::Mixer::kSpeechSoundType);
	_pendingPickerVoiceSoundId = soundId;
	_pendingPickerVoiceSoundHasHandle = handle != nullptr;
	if (handle)
		_pendingPickerVoiceSoundHandle = *handle;
}

void ZoombiniShelterPicker::stopPendingPickerVoice() {
	if (_pendingPickerVoiceSoundHasHandle && _vm->_system->getMixer()->isSoundHandleActive(_pendingPickerVoiceSoundHandle))
		_vm->_system->getMixer()->stopHandle(_pendingPickerVoiceSoundHandle);
	_pendingPickerVoiceSoundHasHandle = false;
	_pendingPickerVoiceSoundId = 0;
}

void ZoombiniShelterPicker::startPickerEntryNarration() {
	if (!_pickerEntryNarrationPending)
		return;
	_pickerEntryNarrationPending = false;

	ZmbStateFile &f = _vm->_state->_f;
	if (f._currentRoute == 1) {
		playPendingPickerVoice(getAfterVideoVoiceSoundId());
	} else {
		_vm->_state->getSfxGroupFlagsFromPageFlag(f._pageFlagIsle);
		if (f._zmbGeneratedCount < 625 && _isFirstVisit)
			playPendingPickerVoice(kSysResSound20042_PickerAfterVideoVoice);
	}
}

void ZoombiniShelterPicker::loadFeatures() {
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShapes4400_PickerMatrix));    // to shape slot 0
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShapes4200_Buttons));         // to shape slot 2
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShapes4300_ZoombiniPreview)); // to shape slot 1
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShapes4100_BackObjects));     // main shape

	// Snoids arriving on the picker settle directly to idle with no turn-around.
	_vm->setArrivalTurnDirection(ArrivalTurnDirection::kIdle);

	// Load NODE 1000: the path snoids walk when entering the corral.
	// Its first waypoint at (148, 215) is the entry point.
	loadNodePath(ZmbResource(ZmbResource::kPage, kResNode1000_WalkNetwork));

	// Background Animation: Stars
	loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShapes4100_BackObjects), kResScrb4101_Star, 10, ZmbFeature::FLAG_00008000_LOOP_ANIM);
	loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShapes4100_BackObjects), kResScrb4102_Star, 11, ZmbFeature::FLAG_00008000_LOOP_ANIM);
	loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShapes4100_BackObjects), kResScrb4103_Star, 12, ZmbFeature::FLAG_00008000_LOOP_ANIM);

	// Wave and boat background animations are omitted in less-action mode.
	if (!_vm->_state->isLessActionEnabled())
		registerWaveBoatFeatures();
	else
		_vm->_state->_f._pickerWaveBoatAnimationState = kPickerWaveBoatBothStopped;
	updateWaveBoatAnimationState(true);

	// Background Objects
	loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShapes4100_BackObjects), kResScrb4100_BackObjects, 0,
					ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_20000000_ZSORT_BOTTOM | ZmbFeature::FLAG_40000000_ZSORT_LEFT);

	// Cave Mark on Hover with Zoombinis
	// - Appears only when a user is holding a zoombini
	loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShapes4100_BackObjects), kResScrb4110_CaveMark, 6, _caveMarkRegPoint,
					ZmbFeature::FLAG_01000000_DEFER_RENDER | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00002000_DRAW_ON_REG);

	// [*] Rocks near Cave Entrance
	// Part of Rocks - Bottom from Entrance
	loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShapes4100_BackObjects), kResScrb4106_RockShape, 0, ZmbFeature::FLAG_00000000_TYPE_SHAPES);
	// Part of Rocks - Bottom-Left from Entrance
	loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShapes4100_BackObjects), kResScrb4107_RockShape, 0, ZmbFeature::FLAG_00000000_TYPE_SHAPES);
	// Part of Rocks - Left from Entrance
	loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShapes4100_BackObjects), kResScrb4108_RockShape, 0, ZmbFeature::FLAG_00000000_TYPE_SHAPES);
	// Part of Rocks - Bottom-Right from Entrance
	loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShapes4100_BackObjects), kResScrb4109_RockShape, 0, ZmbFeature::FLAG_00000000_TYPE_SHAPES);

	setGoButton(_goButtonClickRect, kShape4200_08_GoButtonDisabled, kShape4200_09_GoButtonNormal, kShape4200_10_GoButtonPressed);
	setMapButton(_mapButtonClickRect, kShape4200_11_MapButtonNormal, kShape4200_12_MapButtonPressed);
	setHelpButton(_helpButtonClickRect);
	buildEmbeddedControlHotspots(kResBitmapShapes4200_Buttons, _goMapBaseHotspots, _helpBaseHotspots);

	{ // [*] The one callback-only picker UI runner: every control and preview.
		// SCRB id 0, so this runner owns no hotspots. Its render callback blits the right panel
		// and then the trait matrix straight out of tBMP 4200, 4300, and 4400, matching the
		// original's single post-render callback.
		//
		pickerUI_resetHotspots();

		ZmbFeature::EventHooks hooks;
		hooks.setPreRenderFunc(static_cast<ZmbFeature::OnPreRenderFunc>(&ZoombiniShelterPicker::pickerUI_onPreRender));
		hooks.setRenderFunc(static_cast<ZmbFeature::OnRenderFunc>(&ZoombiniShelterPicker::pickerUI_onRender));
		hooks.setPostRenderFunc(static_cast<ZmbFeature::OnPostRenderFunc>(&ZoombiniShelterPicker::pickerUI_onPostRender));
		hooks.setLButtonDownFunc(static_cast<ZmbFeature::OnLButtonDownFunc>(&ZoombiniShelterPicker::pickerUI_onLButtonDown));
		hooks.setKeyDownFunc(static_cast<ZmbFeature::OnKeyDownFunc>(&ZoombiniShelterPicker::pickerButtons_onKeyDown));
		hooks.setKeyUpFunc(static_cast<ZmbFeature::OnKeyUpFunc>(&ZoombiniShelterPicker::pickerButtons_onKeyUp));

		// The runner's own resource is tBMP 4200 because the shared button-shape selector bounds
		// its hover shape against the owning feature's resource, and the right-panel buttons are
		// the only group with a hover state.
		_pickerUIFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShapes4200_Buttons), 0, 0,
										   ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00001000_TOPMOST,
										   hooks);
		// The runner owns no hotspots, so give it the union of all control layout rects as
		// its first-draw coverage fallback. From the first drawn frame onward the render callback's
		// measured draw rectangle supplies the real dirty region.
		Common::Rect uiRect = _pickerMatrixRects[0];
		for (uint32 i = 1; i < ARRAYSIZE(_pickerMatrixRects); i++)
			uiRect.extend(_pickerMatrixRects[i]);
		uiRect.extend(_generateButtonRect);
		uiRect.extend(_diceButtonRect);
		uiRect.extend(_nameBoxRect);
		uiRect.extend(_previewZoombiniRect);
		uiRect.extend(_goButtonClickRect);
		uiRect.extend(_mapButtonClickRect);
		if (_vm->supportsOnScreenHelp())
			uiRect.extend(_helpButtonClickRect);
		_pickerUIFeature->setClickRect(uiRect);

		_pickerMatrixFeature = _pickerUIFeature;
		_pickerButtonsFeature = _pickerUIFeature;
		_previewFeature = _pickerUIFeature;
		bindEmbeddedControlFeature(_pickerUIFeature);
	}

	// Transfer the Isle pack only after every page-owned feature has been registered.
	ZmbStateFile &f = _vm->_state->_f;
	f._zmbPackIsle.copyTo(f._zmbPackActive);
	f._zmbPackIsle.clearEntries();
	loadZoombinisFromPack(f._zmbPackActive);

	// Assign each authored seat to the first eligible runner within the original radius.
	memset(_seatToSnoid, 0, sizeof(_seatToSnoid));
	Common::Array<uint16> assignedRunnerIds;
	assignIdleSnoidsToSlots(_zoombiniSeatPoints, 16, 500, false, assignedRunnerIds);
	for (int16 snoidIdx = 0; snoidIdx < 16; snoidIdx++) {
		ZmbSnoid *matchedSnoid = getSnoid(assignedRunnerIds[snoidIdx]);
		if (matchedSnoid) {
			_seatToSnoid[snoidIdx] = matchedSnoid;
			matchedSnoid->setPointLoc(_zoombiniSeatPoints[snoidIdx]);
		}
	}

	// Materialize the registered features and Snoids before controls, dialogs, and entry audio.
	renderFeatures();

	// [*] Callback-only runner - open loadDialog if required
	if (_mode == kPickerMode_LoadGame) {
		ZmbFeature::EventHooks hooks;
		hooks.setRenderFunc(static_cast<ZmbFeature::OnRenderFunc>(&ZoombiniShelterPicker::oneTimeLoadDialog_onRenderShape));

		loadScrbFeature(ZmbResource(ZmbResource::kPage, 0), 0, 0, ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00001000_TOPMOST, hooks);
	} else
		startPickerEntryNarration();
}

void ZoombiniShelterPicker::pickerUI_resetHotspots() {
	// Rebuild every group from its constants. The selectors adjust positions relative to the value
	// they find, so they must always start from the pristine layout rather than last frame's result.
	_matrixHotspots.clear();
	for (uint32 i = 0; i < ARRAYSIZE(_pickerMatrixRects); i++)
		_matrixHotspots.push_back(ZmbHotspot(i, 2 * i + 1, 0, _pickerMatrixRects[i]));
	for (uint32 i = 0; i < ARRAYSIZE(_pickerMatrixRects); i++)
		_matrixHotspots.push_back(ZmbHotspot(i + 20, 2 * i + 2, 0, _pickerMatrixRects[i]));

	_buttonHotspots.clear();
	_buttonHotspots.push_back(ZmbHotspot(kHotspotGenerateButtonNormal, kShape4200_02_GenerateButtonNormal, 0, _generateButtonRect));
	_buttonHotspots.push_back(ZmbHotspot(kHotspotDiceButtonNormal, kShape4200_04_DiceButtonNormal, 0, _diceButtonRect));
	_buttonHotspots.push_back(ZmbHotspot(kHotspotGenerateButtonPressed, kShape4200_03_GenerateButtonPressed, 0, _generateButtonRect));
	_buttonHotspots.push_back(ZmbHotspot(kHotspotDiceButtonPressed, kShape4200_05_DiceButtonPressed, 0, _diceButtonRect));
	_buttonHotspots.push_back(ZmbHotspot(kHotspotNameBox, kShape4200_13_NameBox, 0, _nameBoxRect));

	// The preview is composed inside the Picker UI feature so every trait change
	// rematerializes one ordered body, hair, eye, nose, and feet hotspot group.
	const int16 previewTraitOffsetX = _previewZoombiniRect.left + 39;
	const int16 previewTraitOffsetY = _previewZoombiniRect.top + 31;
	_previewHotspots.clear();
	_previewHotspots.push_back(ZmbHotspot(kHotspotPreviewBody, kShape4300_01_PreviewBody, 0, previewTraitOffsetX, previewTraitOffsetY));
	_previewHotspots.push_back(ZmbHotspot(kHotspotPreviewHair, ZmbHotspot::kShapeNone, 0, previewTraitOffsetX, previewTraitOffsetY));
	_previewHotspots.push_back(ZmbHotspot(kHotspotPreviewEye, ZmbHotspot::kShapeNone, 0, previewTraitOffsetX, previewTraitOffsetY));
	_previewHotspots.push_back(ZmbHotspot(kHotspotPreviewNose, ZmbHotspot::kShapeNone, 0, previewTraitOffsetX, previewTraitOffsetY));
	_previewHotspots.push_back(ZmbHotspot(kHotspotPreviewFeet, ZmbHotspot::kShapeNone, 0, previewTraitOffsetX, previewTraitOffsetY));

	_goMapHotspots = _goMapBaseHotspots;
	_helpHotspots = _helpBaseHotspots;
}

bool ZoombiniShelterPicker::pickerUI_onPreRender(ZmbFeature *feature) {
	// Prepare every UI group. The selectors also advance the press-animation state and ask the
	// runner to rematerialize while an animation is in flight, so they must run in pre-render.
	pickerUI_resetHotspots();
	pickerMatrix_onPreRenderShape(feature, nullptr, _matrixHotspots);
	pickerButtons_onPreRenderShape(feature, nullptr, _buttonHotspots);
	zoombiniPreview_onPreRenderShape(feature, nullptr, _previewHotspots);
	goMapButtons_preRenderShape(feature, nullptr, _goMapHotspots);
	helpButton_preRenderShape(feature, nullptr, _helpHotspots);
	return true;
}

void ZoombiniShelterPicker::pickerUI_drawGroup(const Common::Array<ZmbHotspot> &hotspots, ZmbResource imgResource) {
	for (uint32 hotspotIdx = 0; hotspotIdx < hotspots.size(); hotspotIdx++) {
		const ZmbHotspot &hs = hotspots[hotspotIdx];
		if (hs._shapeIdx <= ZmbHotspot::kShapeNone)
			continue;
		_vm->_gfx->drawShape(ZoombiniGraphics::kShapeScreen, imgResource, static_cast<uint16>(hs._shapeIdx), hs.getPos());
	}
}

ZmbRenderResult ZoombiniShelterPicker::pickerUI_onRender(ZmbFeature *feature) {
	(void)feature;

	// Draw order follows the original's post-render callback: right panel first, matrix on top.
	pickerUI_drawGroup(_buttonHotspots, ZmbResource(ZmbResource::kPage, kResBitmapShapes4200_Buttons));
	pickerUI_drawGroup(_previewHotspots, ZmbResource(ZmbResource::kPage, kResBitmapShapes4300_ZoombiniPreview));
	pickerUI_drawGroup(_goMapHotspots, ZmbResource(ZmbResource::kPage, kResBitmapShapes4200_Buttons));
	pickerUI_drawGroup(_helpHotspots, ZmbResource(ZmbResource::kSystem, kSysResShapeBitmap0001_Dialog));
	pickerUI_drawGroup(_matrixHotspots, ZmbResource(ZmbResource::kPage, kResBitmapShapes4400_PickerMatrix));
	return ZmbRenderResult::kRendered;
}

void ZoombiniShelterPicker::pickerUI_onPostRender(ZmbFeature *feature) {
	pickerMatrix_onPostRender(feature);
	pickerButtons_onPostRender(feature);
	goMapButtons_onPostRender(feature);
	helpButton_onPostRender(feature);
}

ZmbEventHandleResult ZoombiniShelterPicker::pickerUI_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	// Central dispatch, matching the original's single click handler.
	ZmbEventHandleResult result = pickerMatrix_onLButtonDown(feature, absPos, relPos);
	if (result == ZmbEventHandleResult::kConsumed)
		return result;

	result = pickerButtons_onLButtonDown(feature, absPos, relPos);
	if (result == ZmbEventHandleResult::kConsumed)
		return result;

	if (_nameBoxRect.contains(absPos)) {
		stopPendingPickerVoice();
		if (isZoombiniTraitGeneratable(_previewSnoid._trait)) {
			_vm->_sound->playSound(ZmbResource(ZmbResource::kPage, kResSound1000_PressMatrixButton), Audio::Mixer::kSFXSoundType);
			generateZoombiniName();
			requestPickerControlRematerialization();
		}
		return ZmbEventHandleResult::kConsumed;
	}

	result = zoombiniPreview_onLButtonDown(feature, absPos, relPos);
	if (result == ZmbEventHandleResult::kConsumed)
		return result;

	result = goMapButtons_onLButtonDown(feature, absPos, relPos);
	if (result == ZmbEventHandleResult::kConsumed)
		return result;

	return helpButton_onLButtonDown(feature, absPos, relPos);
}

void ZoombiniShelterPicker::pickerMatrix_onPreRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)hsGroup;

	genericStickyButton_selectShapes(feature, hotspots, _matrixButtonStateMap);
}

void ZoombiniShelterPicker::pickerMatrix_onPostRender(ZmbFeature *feature) {
	(void)feature;
}

void ZoombiniShelterPicker::pickerMatrix_onButtonAction(ZmbFeature *feature, uint32 bsIdx, StickyButtonState &bs) {
	(void)feature;

	uint32 row = bsIdx / kMatrixColumns;
	uint32 column = bsIdx % kMatrixColumns;

	if (bs.isStuck()) {
		// Enforce mutual exclusion: in each row only one button may be stuck at a time
		switch (row) {
		case 0: // Hair
			_previewSnoid._trait._hair = column + 1;
			break;
		case 1: // Eye
			_previewSnoid._trait._eyes = column + 1;
			break;
		case 2: // Nose
			_previewSnoid._trait._nose = column + 1;
			break;
		case 3: // Feet
			_previewSnoid._trait._feet = column + 1;
			break;
		default:
			error("picker: invalid ButtonState index %u", bsIdx);
			break;
		}

		for (uint32 colIdx = 0; colIdx < kMatrixColumns; colIdx++) {
			uint32 sibIdx = row * kMatrixColumns + colIdx;
			if (sibIdx != bsIdx)
				_matrixButtonStateMap[sibIdx].reset();
		}
	} else {
		// Clear the trait if the button is unstuck
		switch (row) {
		case 0: // Hair
			_previewSnoid._trait._hair = ZmbTrait::TRAIT_NONE;
			break;
		case 1: // Eye
			_previewSnoid._trait._eyes = ZmbTrait::TRAIT_NONE;
			break;
		case 2: // Nose
			_previewSnoid._trait._nose = ZmbTrait::TRAIT_NONE;
			break;
		case 3: // Feet
			_previewSnoid._trait._feet = ZmbTrait::TRAIT_NONE;
			break;
		default:
			error("picker: invalid ButtonState index %u", bsIdx);
			break;
		}
	}

	// Enable Generate only while the roster has capacity and every row has a usable selection.
	// The name stays unchanged when the user manually selects traits.
	_pickerButtonStateMap[kPickerButtons_Generate]._isPressDisabled = isGenerationComplete() || !isZoombiniTraitGeneratable(_previewSnoid._trait);
	requestPickerControlRematerialization();
}

ZmbEventHandleResult ZoombiniShelterPicker::pickerMatrix_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	(void)relPos;

	for (Common::HashMap<uint32, Common::Rect>::const_iterator it = _matrixButtonRectMap.begin(); it != _matrixButtonRectMap.end(); it++) {
		if (it->_value.contains(absPos)) {
			stopPendingPickerVoice();
			break;
		}
	}

	return genericStickyButton_onLButtonDown(feature, absPos, _matrixButtonStateMap, _matrixButtonRectMap, static_cast<OnStickyButtonActionFunc>(&ZoombiniShelterPicker::pickerMatrix_onButtonAction));
}

void ZoombiniShelterPicker::pickerButtons_onPreRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)hsGroup;

	// Disable generation once the page or global roster is full.
	const bool generationComplete = isGenerationComplete();
	_pickerButtonStateMap[kPickerButtons_Generate]._isPressDisabled = generationComplete || !isZoombiniTraitGeneratable(_previewSnoid._trait);
	setGoButtonsEnabled(isDepartureReady());

	genericButton_selectShapes(feature, hotspots, _pickerButtonStateMap);
}

void ZoombiniShelterPicker::pickerButtons_onPostRender(ZmbFeature *feature) {
	updatePendingGoTransition();

	// Draw the name only when all four traits form a generatable combination.
	if (isZoombiniTraitGeneratable(_previewSnoid._trait)) {
		ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;
		const bool portuguese10Layout = _vm->isGameVariant(MohawkGameFeatures::GF_ZMB_10_ESPT) && _vm->getLanguage() == Common::PT_PRT;
		const bool polish20Layout = _vm->isVersionFamilyTlcV2() && _vm->getLanguage() == Common::PL_POL;

		Common::Rect nameTextRect = _nameBoxRect;
		nameTextRect.left += 4;
		nameTextRect.top += (portuguese10Layout || polish20Layout) ? 1 : 4;

		ZoombiniGraphics::TextConf tc;
		tc._hAlign = Graphics::TextAlign::kTextAlignCenter;
		tc._vAlign = Graphics::TextAlign::kTextAlignCenter;
		_vm->_gfx->drawText(screenKind, _previewSnoid._name, nameTextRect, tc);
	}

	genericButton_action(feature, _pickerButtonStateMap, static_cast<OnButtonActionFunc>(&ZoombiniShelterPicker::pickerButtons_onButtonAction));
}

void ZoombiniShelterPicker::requestPickerControlRematerialization() {
	if (_pickerMatrixFeature)
		_pickerMatrixFeature->requestVisualRematerialization();
	if (_pickerButtonsFeature)
		_pickerButtonsFeature->requestVisualRematerialization();
	if (_previewFeature)
		_previewFeature->requestVisualRematerialization();
}

void ZoombiniShelterPicker::updatePendingGoTransition() {
	if (_departureState != ZmbDepartureState::kAnimating)
		return;

	updatePendingGoWalkCompletion();

	// Wait for departure sound to finish
	if (!isDepartSfxDone())
		return;

	// When one idle Snoid was sent toward the boat, the first walk completion
	// after the Go press releases the transition. A Snoid that was already
	// walking in may satisfy this condition.
	if (_pendingGoRequiresWalkCompletion && !_pendingGoObservedWalkCompletion)
		return;

	_departureState = ZmbDepartureState::kCompleted;
	_pendingGoRequiresWalkCompletion = false;
	_pendingGoObservedWalkCompletion = false;
	_pendingGoWalkers.clear();
	_vm->setNextPage(ZoombiniPageType::kXfer);
	close();
}

void ZoombiniShelterPicker::updateDepartureState() {
	// updatePendingGoTransition() owns the Picker-specific sound and walk gates.
}

ZmbSnoid *ZoombiniShelterPicker::findSnoidAtSeat(int16 seatIdx) {
	if (seatIdx < 0 || 16 <= seatIdx)
		return nullptr;
	return _seatToSnoid[seatIdx];
}

void ZoombiniShelterPicker::updatePendingGoWalkCompletion() {
	if (_pendingGoObservedWalkCompletion)
		return;

	for (ZmbSnoid *snoid : _pendingGoWalkers) {
		if (!snoid)
			continue;

		const SnoidAnimState state = snoid->getAnimState();
		if (state != kSnoidAnimState007_Depart && state != kSnoidAnimState112_Path) {
			_pendingGoObservedWalkCompletion = true;
			return;
		}
	}
}

void ZoombiniShelterPicker::releaseRapidFillBurstForGo() {
	const uint32 currentFrame = getCurrentFrameCounter();

	for (uint16 orderIndex = 0; orderIndex < kRapidFillGoBurstSize; orderIndex++) {
		ZmbSnoid *snoid = findSnoidAtSeat(kEmbarkOrder[orderIndex]);
		if (!snoid || !snoid->hasDeferredAnimationStart())
			continue;

		bool isRapidFillSnoid = false;
		for (uint16 snoidId : _rapidFillSnoidIds) {
			if (snoidId == snoid->getId()) {
				isRapidFillSnoid = true;
				break;
			}
		}
		if (!isRapidFillSnoid)
			continue;

		// Release the two upper-right seat pairs on the next animation tick.
		// Their shared source and path make the interrupted rapid fill emerge as one burst.
		snoid->setDelayUntilFrame(currentFrame);
	}
}

void ZoombiniShelterPicker::pickerButtons_onButtonAction(ZmbFeature *feature, uint32 bsIdx, ButtonState &bs) {
	stopPendingPickerVoice();

	if (bs._isPressDisabled)
		return;

	switch (bsIdx) {
	case kPickerButtons_Generate: {
		ZmbStateFile &f = _vm->_state->_f;
		if (_vm->_state->_f.getDebugEnabled() && _shiftRapidFillActive && _snoidMap.empty()) {
			Common::String ignoredOutput;
			debugApplySyntheticGeneratePrefix(ignoredOutput);
		}
		if (isGenerationComplete())
			break;
		if (!isZoombiniTraitGeneratable(_previewSnoid._trait))
			break;
		const int16 seatIdx = findFirstEmptySeat();
		if (seatIdx < 0)
			break;
		_vm->_state->markSaveBeforeQuitPending();

		// Record another generated twin for this preview trait combination.
		const int16 snoidTraitId = _previewSnoid._trait.snoidId();
		if (f._twinGenStatus[snoidTraitId] < 2)
			f._twinGenStatus[snoidTraitId] += 1;

		// Add new entry to the active pack for later transfer to basecamp
		if (f._zmbPackActive.getPackZmbCount() < 16) {
			ZmbStateActiveEntry entry;
			entry.setTraits(_previewSnoid._trait);
			entry.setIsOccupied(true);
			entry.setU32Name(_vm, _previewSnoid._name);
			f._zmbPackActive.appendEntry(entry);
		}

		// Create a new snoid walking from the NODE 1000 entry point to the next seat.
		// The source (148, 215) is the first waypoint of NODE 1000.
		// Use @ref kSnoidAnimState007_Depart so the snoid walks in rather than teleporting directly to the seat.
		Common::Point spawnPos(148, 215); // NODE 1000 entry point
		{
			Common::HashMap<int16, ZmbNode *>::const_iterator nodeIt = _nodeMap.find(1000);
			if (nodeIt != _nodeMap.end() && !nodeIt->_value->_waypoints.empty())
				spawnPos = nodeIt->_value->_waypoints[0];
		}
		const Common::Point seatPos = _zoombiniSeatPoints[seatIdx];
		const uint16 newSnoidId = allocateDynamicPackSnoidId();
		const byte idleTickCounter = static_cast<byte>(_vm->_rnd->getRandomNumber(0, 64));
		ZmbSnoid *snoid = loadSnoidFromPack(newSnoidId, spawnPos, ZmbFeature::FLAG_00000001_TYPE_SNOID);
		if (snoid) {
			snoid->setIdleTickCounter(idleTickCounter);
			snoid->_trait = _previewSnoid._trait;
			snoid->_name = _previewSnoid._name;
			// Picker-generated runners always occupy their active-pack slots.
			snoid->_packIsOccupied = true;
			snoid->setupIdleHotspots();
			snoid->setFacingLeft(seatPos.x < spawnPos.x);
			snoid->setAnimTargetPos(seatPos);
			snoid->setAnimState(kSnoidAnimState007_Depart, nullptr);
			// Track seat-to-snoid mapping for embark animation
			_seatToSnoid[seatIdx] = snoid;
		}
		// Track the pending walk-in.
		_vm->_walkersInProgress += 1;

		// Increment total generated count
		f._zmbGeneratedCount += 1;

		// Immediately enable or disable Go from current departure readiness.
		// This runs before the shared button pre-render callback,
		// so the visual updates in the same frame the departure gate changes.
		setGoButtonsEnabled(isDepartureReady());

		// Generate a new name for the next preview snoid
		generateZoombiniName();

		break;
	}
	case kPickerButtons_Dice: {
		const bool fillEmptySeats = _shiftRapidFillActive || _diceRapidFillPending;
		_diceRapidFillPending = false;
		applyDiceReroll(fillEmptySeats);
		updateDiceButtonVisual(feature);
		break;
	}
	default:
		error("picker: pickerButtons_onPostAnimation: invalid ButtonState index %u", bsIdx);
		break;
	}

	requestPickerControlRematerialization();
}

ZmbEventHandleResult ZoombiniShelterPicker::zoombiniPreview_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	(void)feature;
	(void)relPos;

	if (!_previewZoombiniRect.contains(absPos))
		return ZmbEventHandleResult::kPassthrough;

	stopPendingPickerVoice();

	// Select a preview voice from its randomized voice group.
	const int16 voiceGroup = _vm->_rnd->getRandomNumber(0, 12);
	const int16 soundId = _previewSnoid.getVoiceResId(voiceGroup);
	if (0 < soundId)
		playPickerVoice(soundId, Audio::Mixer::kSFXSoundType);

	return ZmbEventHandleResult::kConsumed;
}

ZmbEventHandleResult ZoombiniShelterPicker::pickerButtons_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	(void)relPos;

	for (Common::HashMap<uint32, Common::Rect>::const_iterator it = _pickerButtonRectMap.begin(); it != _pickerButtonRectMap.end(); it++) {
		if (it->_value.contains(absPos)) {
			stopPendingPickerVoice();
			break;
		}
	}
	if (_diceButtonRect.contains(absPos) &&
		ZmbTrait::SNOID_MAX <= _vm->_state->_f._zmbGeneratedCount) {
		_diceRapidFillPending = false;
		updateDiceButtonVisual(feature);
		_vm->_sound->playSound(ZmbResource(ZmbResource::kPage, kResSound1008_AllZoombinisGenerated), Audio::Mixer::kSFXSoundType);
		return ZmbEventHandleResult::kConsumed;
	}

	return genericButton_onLButtonDown(feature, absPos, _pickerButtonStateMap, _pickerButtonRectMap);
}

ZmbEventHandleResult ZoombiniShelterPicker::pickerButtons_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat) {
	(void)kbdRepeat;

	if (kbd.hasFlags(Common::KBD_SHIFT)) {
		_shiftRapidFillActive = true;
		updateDiceButtonVisual(feature);
	}

	return ZmbEventHandleResult::kPassthrough;
}

ZmbEventHandleResult ZoombiniShelterPicker::pickerButtons_onKeyUp(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat) {
	(void)kbdRepeat;

	if (!kbd.hasFlags(Common::KBD_SHIFT)) {
		_shiftRapidFillActive = false;
		updateDiceButtonVisual(feature);
	}

	return ZmbEventHandleResult::kPassthrough;
}

void ZoombiniShelterPicker::updateDiceButtonVisual(ZmbFeature *feature) {
	const bool generationAvailable = _vm->_state->_f._zmbGeneratedCount < ZmbTrait::SNOID_MAX;
	const bool rapidFill = generationAvailable && (_shiftRapidFillActive || _diceRapidFillPending);
	ButtonState &diceButton = _pickerButtonStateMap[kPickerButtons_Dice];
	diceButton._shapeNormalIdx = rapidFill ? kShape4200_06_DiceArrowButtonNormal : kShape4200_04_DiceButtonNormal;
	diceButton._shapePressedIdx = rapidFill ? kShape4200_07_DiceArrowButtonPressed : kShape4200_05_DiceButtonPressed;
	if (_vm->isVersionFamilyTlcV2()) {
		diceButton.setHoverState(rapidFill ? kShape4200_16_DiceArrowButtonHover : kShape4200_15_DiceButtonHover);
	}
	addExternalDirtyRect(feature->getZSortRect());
	feature->requestVisualRematerialization();
}

void ZoombiniShelterPicker::zoombiniPreview_onPreRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)feature;
	(void)hsGroup;

	// Body is always visible
	ZmbHotspot &hsBody = hotspots[kHotspotPreviewBody];
	hsBody._shapeIdx = kShape4300_01_PreviewBody;
	hsBody._x -= _previewTraitOffsets[0].x;
	hsBody._y -= _previewTraitOffsets[0].y;

	// For each trait row, map its selected column to the corresponding preview shape.
	// Each trait row begins at a stride of @ref ZoombiniShelterPicker::kMatrixColumns.
	const uint32 numTraitRows = ARRAYSIZE(_pickerMatrixRects) / kMatrixColumns;
	for (uint32 traitIdx = 0; traitIdx < numTraitRows; traitIdx++) {
		uint32 rowStart = traitIdx * kMatrixColumns;
		uint32 hsId = kHotspotPreviewHair + traitIdx;
		uint16 rowShapeIdxBase = kShape4300_02_PreviewHair1 + traitIdx * kMatrixColumns;

		ZmbHotspot &hs = hotspots[hsId];
		uint16 shapeIdx = ZmbHotspot::kShapeNone;
		for (uint32 col = 0; col < kMatrixColumns; col++) {
			Common::StableMap<uint32, StickyButtonState>::iterator it = _matrixButtonStateMap.find(rowStart + col);
			if (it != _matrixButtonStateMap.end() && it->second.isStuck()) {
				shapeIdx = rowShapeIdxBase + col;
				break;
			}
		}
		if (0 < shapeIdx) {
			hs._shapeIdx = shapeIdx;
			hs._x -= _previewTraitOffsets[shapeIdx - 1].x;
			hs._y -= _previewTraitOffsets[shapeIdx - 1].y;
		}
	}
}

ZmbRenderResult ZoombiniShelterPicker::oneTimeLoadDialog_onRenderShape(ZmbFeature *feature) {
	_vm->_gfx->flushScreens(); // Ensure screen is up-to-date before opening a first-screen loadDialog
	// The original selects the entry narration after registering the non-blocking Load dialog but before input can reach it.
	// Keep that RNG boundary immediately before the synchronous ScummVM dialog.
	startPickerEntryNarration();
	ZoombiniDialogResult dialogResult = _vm->openLoadDialog(true);
	if (dialogResult == ZoombiniDialogResult::kYes) {
		close();
	} else if (dialogResult == ZoombiniDialogResult::kNo) {
		_vm->_state->markGameStateReady();
	}

	// The one-time dialog has resolved; the picker now operates as the regular
	// Zoombini selection screen for the rest of this visit.
	_mode = kPickerMode_SelectZoombinis;

	// This function must be called only once
	feature->scheduleClose();

	return ZmbRenderResult::kRendered;
}

// ---------------------------------------------------------------------------
// Page-level mouse handlers -- Snoid drag/drop
// ---------------------------------------------------------------------------

ZmbEventHandleResult ZoombiniShelterPicker::onKeyDown(const Common::KeyState &kbd, bool kbdRepeat) {
	if (kbd.hasFlags(Common::KBD_CTRL) && kbd.keycode == Common::KEYCODE_w) {
		// Cycle both-running, both-stopped, waves-running, and boat-running states.
		if (_vm->_state->isLessActionEnabled())
			registerWaveBoatFeatures();
		updateWaveBoatAnimationState(false);
		return ZmbEventHandleResult::kConsumed;
	}

	// Lowercase r arms the one-shot version-update header for the next Help dialog.
	// If that dialog still belongs to Picker, it also loads the update body.
	if (kbd.ascii == 'r' && kbd.hasFlags(0))
		_vm->requestPickerUpdateHelp();

	// The hidden selector does not consume the key, so shared handling still runs.
	return ZoombiniInteractive::onKeyDown(kbd, kbdRepeat);
}

ZmbEventHandleResult ZoombiniShelterPicker::onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
	// Complete a pending departure before handling other clicks.
	if (isDeparturePending()) {
		_departureState = ZmbDepartureState::kCompleted;
		_hasDepartSfxHandle = false;
		_pendingGoRequiresWalkCompletion = false;
		_pendingGoObservedWalkCompletion = false;
		_pendingGoWalkers.clear();
		_vm->setNextPage(ZoombiniPageType::kXfer);
		close();
		return ZmbEventHandleResult::kConsumed;
	}

	// Let the base class handle button/feature clicks first.
	if (isPointInControlButtonRect(absPos))
		stopPendingPickerVoice();
	ZmbEventHandleResult result = ZoombiniInteractive::onLButtonDown(absPos, relPos);
	if (result == ZmbEventHandleResult::kConsumed)
		return result;

	// Don't start a new drag while already dragging
	if (isDragging())
		return ZmbEventHandleResult::kPassthrough;

	// Do not drag while a Snoid is walking in through a departure or path state.
	for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
		SnoidAnimState st = (*it)->getAnimState();
		if (st == kSnoidAnimState007_Depart || st == kSnoidAnimState112_Path)
			return ZmbEventHandleResult::kPassthrough;
	}

	// Find snoid under cursor
	ZmbSnoid *snoid = findSnoidAtPoint(absPos);
	if (!snoid)
		return ZmbEventHandleResult::kPassthrough;

	// Only idle (0), arrive (4), or fidget (6) are draggable.
	SnoidAnimState state = snoid->getAnimState();
	if (state != kSnoidAnimState000_Idle && state != kSnoidAnimState004_Arrive && state != kSnoidAnimState006_Fidget)
		return ZmbEventHandleResult::kPassthrough;

	stopPendingPickerVoice();
	startSnoidDrag(snoid, absPos);
	return ZmbEventHandleResult::kConsumed;
}

ZmbEventHandleResult ZoombiniShelterPicker::onMouseMove(const Common::Point &absPos, const Common::Point &relPos) {
	// Z1-20U/TLC v2.0 release only: picker-local buttons use hover shapes.
	if (_vm->isVersionFamilyTlcV2())
		genericButton_updateHoverState(_pickerButtonsFeature, absPos, _pickerButtonStateMap, _pickerButtonRectMap);
	return ZoombiniInteractive::onMouseMove(absPos, relPos);
}

ZmbEventHandleResult ZoombiniShelterPicker::onRButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
	if (isDeparturePending())
		return ZmbEventHandleResult::kPassthrough;
	if (!_diceButtonRect.contains(absPos) || !_pickerButtonsFeature)
		return ZmbEventHandleResult::kPassthrough;

	_diceRightButtonCaptured = true;
	_diceRapidFillPending = true;
	updateDiceButtonVisual(_pickerButtonsFeature);
	const ZmbEventHandleResult result = pickerButtons_onLButtonDown(_pickerButtonsFeature, absPos, relPos);
	if (result == ZmbEventHandleResult::kConsumed)
		return result;

	_diceRightButtonCaptured = false;
	_diceRapidFillPending = false;
	updateDiceButtonVisual(_pickerButtonsFeature);
	return ZmbEventHandleResult::kPassthrough;
}

ZmbEventHandleResult ZoombiniShelterPicker::onRButtonUp(const Common::Point &absPos, const Common::Point &relPos) {
	(void)absPos;
	(void)relPos;
	if (!_diceRightButtonCaptured)
		return ZmbEventHandleResult::kPassthrough;

	_diceRightButtonCaptured = false;
	return ZmbEventHandleResult::kConsumed;
}

void ZoombiniShelterPicker::endDrag(const Common::Point &dropPos) {
	(void)dropPos;
	const bool droppedOnCave = 0 <= _dragHighlightSlot;
	ZmbSnoid *snoid = finishSnoidDrag();

	if (droppedOnCave) {
		// --- Dropped on cave entrance: remove the snoid ---
		removeSeatedZoombini(snoid);

		// Play remove SFX
		_vm->_sound->playSound(ZmbResource(ZmbResource::kPage, kResSound1007_RemoveZoombini), Audio::Mixer::kSFXSoundType);

		// Refresh Go after changing both occupancy and the global count.
		setGoButtonsEnabled(isDepartureReady());

		// Refresh Generate after opening a seat while preserving the global cap.
		_pickerButtonStateMap[kPickerButtons_Generate]._isPressDisabled = isGenerationComplete() || !isZoombiniTraitGeneratable(_previewSnoid._trait);
		requestPickerControlRematerialization();
	} else {
		// Not on the cave: the Picker loads no terrain bitmap, so the generic
		// drop controller rejects the release and returns the Snoid to its
		// pickup seat through the Arrive state, which settles facing right.
		settleSnoidAfterTerrainDrop(snoid, _dragOrigPos);
	}
}

void ZoombiniShelterPicker::removeSeatedZoombini(ZmbSnoid *snoid) {
	// Copy traits back to preview panel
	_previewSnoid._trait = snoid->_trait;
	_previewSnoid._name = snoid->_name;

	// Update matrix selection buttons to match the restored preview traits
	for (uint32 row = 0; row < 4; row++) {
		const byte traitVal = _previewSnoid._trait.getTraitValue(ZmbTrait::traitKindFromIndex(static_cast<int>(row)));
		for (uint32 col = 0; col < kMatrixColumns; col++) {
			_matrixButtonStateMap[row * kMatrixColumns + col]._isStuck = (traitVal == col + 1);
		}
	}

	// Decrement twin status
	ZmbStateFile &f = _vm->_state->_f;
	int16 snoidTraitId = snoid->_trait.snoidId();
	if (0 < f._twinGenStatus[snoidTraitId])
		f._twinGenStatus[snoidTraitId] -= 1;

	// Decrement total generated count
	if (0 < f._zmbGeneratedCount)
		f._zmbGeneratedCount -= 1;

	// Remove snoid from screen
	for (uint16 seatIdx = 0; seatIdx < ARRAYSIZE(_seatToSnoid); seatIdx++) {
		if (_seatToSnoid[seatIdx] == snoid) {
			_seatToSnoid[seatIdx] = nullptr;
			break;
		}
	}
	uint16 removedId = snoid->getId();

	// Queue the departing Zoombini's visual area for a background recompose
	// before the runner disappears, or the seat keeps showing the ghost.
	const Common::Rect oldRect = snoid->getZSortRect();
	if (!oldRect.isEmpty())
		addExternalDirtyRect(oldRect);

	unloadSnoid(removedId);

	// Repack remaining Snoids within their visual rows.
	repackSeatPositions();
}

int16 ZoombiniShelterPicker::findFirstEmptySeat() const {
	for (uint16 seatIdx = 0; seatIdx < ARRAYSIZE(_seatToSnoid); seatIdx++) {
		if (!_seatToSnoid[seatIdx])
			return static_cast<int16>(seatIdx);
	}

	return -1;
}

void ZoombiniShelterPicker::repackSeatPositions() {
	// Compact each visual row independently.
	// Every shifted Snoid walks directly to the hole while NODE/PATH traversal is temporarily disabled,
	// so it cannot follow Picker path 1000 during this special operation.
	ZmbStateFile &f = _vm->_state->_f;

	for (int16 emptySeatIdx = 0; emptySeatIdx < 15; emptySeatIdx++) {
		if (_seatToSnoid[emptySeatIdx])
			continue;

		int16 rowEnd = 15;
		if (emptySeatIdx < 6)
			rowEnd = 5;
		else if (emptySeatIdx < 11)
			rowEnd = 10;

		for (int16 candidateSeatIdx = emptySeatIdx + 1; candidateSeatIdx <= rowEnd; candidateSeatIdx++) {
			ZmbSnoid *shiftedSnoid = _seatToSnoid[candidateSeatIdx];
			if (!shiftedSnoid)
				continue;

			shiftedSnoid->initDirectWalkToTarget(_zoombiniSeatPoints[emptySeatIdx]);
			_seatToSnoid[emptySeatIdx] = shiftedSnoid;
			_seatToSnoid[candidateSeatIdx] = nullptr;
			break;
		}
	}

	// Rebuild the serialized pack in visual seat order.
	f._zmbPackActive.clearEntries();
	for (uint16 seatIdx = 0; seatIdx < 16; seatIdx++) {
		ZmbSnoid *s = _seatToSnoid[seatIdx];
		if (!s)
			continue;

		ZmbStateActiveEntry entry;
		entry.setTraits(s->_trait);
		entry.setIsOccupied(true);
		entry.setU32Name(_vm, s->_name);
		f._zmbPackActive.appendEntry(entry);
	}
}

void ZoombiniShelterPicker::onGoButtonActivated() {
	if (isDepartureActive())
		return;

	stopPendingPickerVoice();

	// Play the Picker departure sound.
	playDepartSfx();

	ZmbStateFile &f = _vm->_state->_f;
	// Clear only the Isle pack count; the active pack continues to Xfer and Bridge.
	f._zmbPackIsle.clearEntries();

	// Snoids arriving at the next page will do a turn-around-left animation.
	_vm->setArrivalTurnDirection(ArrivalTurnDirection::kRight);

	_vm->_xferSrcPage = ZmbSrcPageKind::kPicker_01;
	setGoButtonsEnabled(false);

	releaseRapidFillBurstForGo();

	_pendingGoRequiresWalkCompletion = false;
	_pendingGoObservedWalkCompletion = false;
	_pendingGoWalkers.clear();

	// Only the first idle Snoid from priority seats [11, 12, 6, 7] embarks.
	// This snoid walks to the embark destination (544, 264) on the boat.
	for (uint16 orderIdx = 0; orderIdx < 4; orderIdx++) {
		int16 seatIdx = kEmbarkOrder[orderIdx];
		if (seatIdx < 0 || 16 <= seatIdx)
			continue;

		ZmbSnoid *snoid = _seatToSnoid[seatIdx];
		if (!snoid)
			continue;

		// Only take snoids that are idle (not still walking in, etc.)
		if (snoid->getAnimState() != kSnoidAnimState000_Idle)
			continue;

		// Set destination and begin walking animation
		snoid->setAnimTargetPos(_embarkDestination);
		snoid->setAnimState(kSnoidAnimState007_Depart, nullptr);

		_pendingGoRequiresWalkCompletion = true;
		break;
	}

	// Rebase the completion test without cancelling delayed walk-ins.
	// Any Snoid walking now may provide the first completion after this Go press.
	for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
		ZmbSnoid *snoid = *it;
		const SnoidAnimState state = snoid->getAnimState();
		if (state == kSnoidAnimState007_Depart || state == kSnoidAnimState112_Path)
			_pendingGoWalkers.push_back(snoid);
	}

	// Keep Picker visible until the departure SFX and rebased walk condition finish.
	_departureState = ZmbDepartureState::kAnimating;
	updatePendingGoTransition();
}

void ZoombiniShelterPicker::onDisabledGoButtonActivated() {
	stopPendingPickerVoice();
	playPendingPickerVoice(getNoDepartureVoiceSoundId());
}

void ZoombiniShelterPicker::onMapButtonActivated() {
	stopPendingPickerVoice();
	ZoombiniInteractive::onMapButtonActivated();
}

void ZoombiniShelterPicker::saveStateBeforeMapTransition() {
	// Copy active pack -> Isle pack (preserve snoids on the picker for when user returns).
	// Then clear active pack.
	ZmbStateFile &f = _vm->_state->_f;

	// Write current Snoid runners back to @ref ZmbStateFile::_zmbPackActive.
	saveSnoidsToPack();

	// BC0 (isle), clear active.
	f._zmbPackActive.setSkipOccupiedEntries(false);
	f._zmbPackActive.setSkipUnoccupiedEntries(false);
	f._zmbPackActive.copyTo(f._zmbPackIsle);
	f._zmbPackActive.clearEntries();
}

bool ZoombiniShelterPicker::randomizePreviewTraits(bool randomizeAll) {
	bool isGeneratable = false;
	int attempt = 0;
	while (!isGeneratable && attempt < 64) {
		attempt += 1;
		if (attempt < 64) {
			for (int i = 0; i < 4; i++) {
				// Randomize an empty slot or every slot during a full reroll.
				if (!_previewSnoid._trait[i] || randomizeAll)
					_previewSnoid._trait[i] = static_cast<byte>(_vm->_rnd->getRandomNumber(1, 5));
			}
		} else {
			// The bounded random phase falls back to the last available combination.
			randomizeAll = true;
			for (int hi = 0; hi < 5; hi++) {
				for (int ei = 0; ei < 5; ei++) {
					for (int ni = 0; ni < 5; ni++) {
						for (int fi = 0; fi < 5; fi++) {
							int16 snoidId = static_cast<int16>(125 * hi + 25 * ei + 5 * ni + fi);
							if (_vm->_state->_f._twinGenStatus[snoidId] < 2) {
								_previewSnoid._trait._hair = static_cast<byte>(hi + 1);
								_previewSnoid._trait._eyes = static_cast<byte>(ei + 1);
								_previewSnoid._trait._nose = static_cast<byte>(ni + 1);
								_previewSnoid._trait._feet = static_cast<byte>(fi + 1);
							}
						}
					}
				}
			}
		}

		isGeneratable = isZoombiniTraitGeneratable(_previewSnoid._trait);
		// If still invalid, use a full reroll on the next retry.
		if (!isGeneratable)
			randomizeAll = true;
	}

	// Generate a new name only after a full reroll.
	// If the first partial fill succeeds, keep the existing name.
	if (randomizeAll)
		generateZoombiniName();
	return isGeneratable;
}

void ZoombiniShelterPicker::registerWaveBoatFeatures() {
	if (!_wavesFeature) {
		_wavesFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShapes4100_BackObjects), kResScrb4104_Waves, 7,
										ZmbFeature::FLAG_08000000_REGION_TRACK | ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00004000_NO_DIRTY_MERGE);
	}
	if (!_boatFeature) {
		_boatFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShapes4100_BackObjects), kResScrb4105_Boat, 9,
									   ZmbFeature::FLAG_08000000_REGION_TRACK | ZmbFeature::FLAG_00008000_LOOP_ANIM);
	}
}

void ZoombiniShelterPicker::updateWaveBoatAnimationState(bool preserveAnimationState) {
	// The animation state changes only when both background runners are available.
	if (!_wavesFeature || !_boatFeature)
		return;

	uint16 &animationState = _vm->_state->_f._pickerWaveBoatAnimationState;
	if (!preserveAnimationState)
		animationState += 1;
	if (kPickerBoatOnlyRunning < animationState)
		animationState = kPickerWaveBoatBothRunning;

	const bool wavesRunning = animationState == kPickerWaveBoatBothRunning ||
							  animationState == kPickerWaveOnlyRunning;
	const bool boatRunning = animationState == kPickerWaveBoatBothRunning ||
							 animationState == kPickerBoatOnlyRunning;

	if (wavesRunning) {
		_wavesFeature->activateRender();
	} else {
		_wavesFeature->deactivateRender();
	}

	if (boatRunning) {
		_boatFeature->activateRender();
	} else {
		_boatFeature->deactivateRender();
	}
}

bool ZoombiniShelterPicker::isGenerationComplete() const {
	return 16u <= _snoidMap.size() || ZmbTrait::SNOID_MAX <= _vm->_state->_f._zmbGeneratedCount;
}

bool ZoombiniShelterPicker::isDepartureReady() const {
	const uint32 loadedSnoidCount = _snoidMap.size();
	if (loadedSnoidCount == 0)
		return false;

	return 16u <= loadedSnoidCount || ZmbTrait::SNOID_MAX <= _vm->_state->_f._zmbGeneratedCount;
}

void ZoombiniShelterPicker::applyDiceReroll(bool fillEmptySeats) {
	randomizeTraitSelection(fillEmptySeats);
	setGoButtonsEnabled(isDepartureReady());
}

void ZoombiniShelterPicker::randomizeTraitSelection(bool fillEmptySeats) {
	// A complete usable preview rerolls every trait; otherwise preserve selected rows.
	bool isGeneratable = randomizePreviewTraits(isZoombiniTraitGeneratable(_previewSnoid._trait));

	// Sync matrix sticky-button states to reflect the rolled traits
	for (uint32 row = 0; row < 4; row++) {
		for (uint32 col = 0; col < kMatrixColumns; col++) {
			uint32 matrixButtonIdx = row * kMatrixColumns + col;
			_matrixButtonStateMap[matrixButtonIdx]._isStuck = (_previewSnoid._trait[row] == col + 1);
		}
	}

	if (fillEmptySeats) {
		ZmbStateFile &f = _vm->_state->_f;
		_rapidFillSnoidIds.clear();
		Common::Point spawnPos(148, 215);
		{
			Common::HashMap<int16, ZmbNode *>::const_iterator nodeIt = _nodeMap.find(1000);
			if (nodeIt != _nodeMap.end() && !nodeIt->_value->_waypoints.empty())
				spawnPos = nodeIt->_value->_waypoints[0];
		}

		// Less Action spreads entries farther apart to reduce simultaneous motion.
		uint32 nextStartFrame = getCurrentFrameCounter();
		const bool lessAction = _vm->_state->isLessActionEnabled();

		while (isGeneratable && !isGenerationComplete()) {
			const int16 seatIdx = findFirstEmptySeat();
			if (seatIdx < 0)
				break;

			_vm->_state->markSaveBeforeQuitPending();

			// Record another generated twin for this trait combination.
			const int16 snoidTraitId = _previewSnoid._trait.snoidId();
			if (f._twinGenStatus[snoidTraitId] < 2)
				f._twinGenStatus[snoidTraitId] += 1;

			// Add to active pack
			if (f._zmbPackActive.getPackZmbCount() < 16) {
				ZmbStateActiveEntry entry;
				entry.setTraits(_previewSnoid._trait);
				entry.setIsOccupied(true);
				entry.setU32Name(_vm, _previewSnoid._name);
				f._zmbPackActive.appendEntry(entry);
			}

			// Spawn snoid walking to its seat, with deferred start for staggering.
			const Common::Point seatPos = _zoombiniSeatPoints[seatIdx];
			const uint16 newSnoidId = allocateDynamicPackSnoidId();
			const byte idleTickCounter = static_cast<byte>(_vm->_rnd->getRandomNumber(0, 64));
			ZmbSnoid *snoid = loadSnoidFromPack(newSnoidId, spawnPos, ZmbFeature::FLAG_00000001_TYPE_SNOID);
			if (snoid) {
				snoid->setIdleTickCounter(idleTickCounter);
				_rapidFillSnoidIds.push_back(newSnoidId);
				snoid->_trait = _previewSnoid._trait;
				snoid->_name = _previewSnoid._name;
				// Inherited via struct copy; picker-generated runners are always occupied.
				snoid->_packIsOccupied = true;
				snoid->setupIdleHotspots();
				snoid->setFacingLeft(seatPos.x < spawnPos.x);
				snoid->setAnimTargetPos(seatPos);
				snoid->setAnimState(kSnoidAnimState007_Depart, nullptr);
				if (getCurrentFrameCounter() < nextStartFrame) {
					snoid->setDelayUntilFrame(nextStartFrame);
				}
				// Track seat-to-snoid mapping for embark animation
				_seatToSnoid[seatIdx] = snoid;
			}
			// Keep the page locked until this generated Snoid finishes walking.
			_vm->_walkersInProgress += 1;

			f._zmbGeneratedCount += 1;

			if (lessAction)
				nextStartFrame += _vm->_rnd->getRandomNumber(120, 180);
			else
				nextStartFrame += _vm->_rnd->getRandomNumber(60, 120);

			// The original prepares another full preview even after filling the last seat.
			isGeneratable = randomizePreviewTraits(true);
		}

		// Re-sync matrix buttons to reflect the current (next) preview trait
		for (uint32 row = 0; row < 4; row++) {
			for (uint32 col = 0; col < kMatrixColumns; col++) {
				_matrixButtonStateMap[row * kMatrixColumns + col]._isStuck = (_previewSnoid._trait[row] == col + 1);
			}
		}
	}

	// Enable Generate button only when result is generatable
	_pickerButtonStateMap[kPickerButtons_Generate]._isPressDisabled = !isGeneratable;
}

bool ZoombiniShelterPicker::isZoombiniTraitGeneratable(ZmbTrait trait) const {
	if (!trait.isComplete())
		return false;
	int16 snoidId = trait.snoidId();
	return _vm->_state->_f._twinGenStatus[snoidId] < 2;
}

void ZoombiniShelterPicker::generateZoombiniName() {
	_previewSnoid._name = _vm->_text->pickNextZoombiniName();
}

int16 ZoombiniShelterPicker::loadZoombinisFromPack(ZmbStateActivePack &pack) {
	// Clear seat-to-snoid mapping
	memset(_seatToSnoid, 0, sizeof(_seatToSnoid));
	Common::Array<ZmbSnoid *> loadedSnoids;
	const int16 count = loadDynamicSnoidsFromPack(pack, _zoombiniSeatPoints, ARRAYSIZE(_zoombiniSeatPoints), false, &loadedSnoids);
	for (uint16 i = 0; i < loadedSnoids.size() && i < ARRAYSIZE(_seatToSnoid); i++)
		_seatToSnoid[i] = loadedSnoids[i];
	return count;
}

// [*] Page debugger subcommands

Common::String ZoombiniShelterPicker::debugGetPageCommandHelp() const {
	Common::String output;
	output += "  ";
	output += kPageCommandAutoseat;
	output += "\n";
	output += "      Generate Zoombinis into every empty picker seat.\n";
	output += "  ";
	output += kPageCommandAutounseat;
	output += "\n";
	output += "      Remove every seated Zoombini, as if dropped into the cave.\n";
	return output;
}

Common::String ZoombiniShelterPicker::debugGetBuiltinDebugCommandHelp() const {
	Common::String output;
	output += Common::String::format("  %-21s (%s)\n", "Shift+Generate button", kBuiltinDebugActionShiftGenerate);
	output += "    With debug enabled, exact Shift, an empty Picker, and an otherwise enabled Generate action,\n";
	output += "    set generated and Town-stored counts to 624, then create one normal final visible Zoombini.\n";
	output += "    This does not fill the seats. ScummVM blocks saving the synthetic state until Load or New Game.\n";
	output += Common::String::format("  %-21s (%s)\n", "Ctrl+W", kBuiltinDebugActionWaveCycle);
	output += "    Without requiring debug mode, advance 0=both moving, 1=both frozen, 2=waves only, 3=boat only.\n";
	output += "    Frozen layers remain visible. The selector changes only when both SCRB 4104/4105 runners exist.\n";
	output += "    The key is consumed and keyboard repeat continues cycling it.\n";
	output += Common::String::format("  %-21s (%s)\n", "r", kBuiltinDebugActionUpdateHelp);
	output += "    Without requiring debug mode, arm the one-shot update header for the next Help dialog.\n";
	output += "    The update body appears only if that dialog is still Picker Help; this key does not open Help and is not consumed.\n";
	return output;
}

bool ZoombiniShelterPicker::debugApplySyntheticGeneratePrefix(Common::String &output) {
	if (_mode != kPickerMode_SelectZoombinis) {
		output = "Shift+Generate requires the Zoombini selection mode.\n";
		return false;
	}
	if (!_snoidMap.empty()) {
		output = "Shift+Generate requires an empty Picker.\n";
		return false;
	}

	ZmbStateFile &f = _vm->_state->_f;
	f._zmbGeneratedCount = ZmbTrait::SNOID_MAX - 1;
	f._zmbStoredTownCount = ZmbTrait::SNOID_MAX - 1;
	_vm->_state->markUnsafeSyntheticDebugState();
	_vm->_state->markSaveBeforeQuitPending();
	output = "Synthetic generated and Town counts set to 624.\n";
	return true;
}

bool ZoombiniShelterPicker::debugDoBuiltinDebugCommand(int argc, const char **argv, Common::String &output) {
	if (argc != 3) {
		output = Common::String::format("Usage: page builtin_debug <%s|%s|%s>\n",
										kBuiltinDebugActionShiftGenerate, kBuiltinDebugActionWaveCycle, kBuiltinDebugActionUpdateHelp);
		return true;
	}
	const BuiltinDebugAction action = parseBuiltinDebugAction(argv[2]);
	if (action == BuiltinDebugAction::kInvalid) {
		output = Common::String::format("Usage: page builtin_debug <%s|%s|%s>\n",
										kBuiltinDebugActionShiftGenerate, kBuiltinDebugActionWaveCycle, kBuiltinDebugActionUpdateHelp);
		return true;
	}
	return runBuiltinDebugAction(action, output);
}

ZoombiniShelterPicker::BuiltinDebugAction ZoombiniShelterPicker::parseBuiltinDebugAction(const Common::String &action) {
	if (action.equalsIgnoreCase(kBuiltinDebugActionShiftGenerate))
		return BuiltinDebugAction::kShiftGenerate;
	if (action.equalsIgnoreCase(kBuiltinDebugActionWaveCycle))
		return BuiltinDebugAction::kWaveCycle;
	if (action.equalsIgnoreCase(kBuiltinDebugActionUpdateHelp))
		return BuiltinDebugAction::kUpdateHelp;
	return BuiltinDebugAction::kInvalid;
}

bool ZoombiniShelterPicker::runBuiltinDebugAction(BuiltinDebugAction action, Common::String &output) {
	if (action == BuiltinDebugAction::kWaveCycle) {
		if (_vm->_state->isLessActionEnabled())
			registerWaveBoatFeatures();
		updateWaveBoatAnimationState(false);
		_vm->_state->markDebugStateMutation();
		_vm->_state->markSaveBeforeQuitPending();
		output = Common::String::format("Picker wave/boat animation state: %u.\n", _vm->_state->_f._pickerWaveBoatAnimationState);
		return false;
	}
	if (action == BuiltinDebugAction::kUpdateHelp) {
		_vm->requestPickerUpdateHelp();
		output = "Picker version-update Help selector armed.\n";
		return false;
	}
	if (action != BuiltinDebugAction::kShiftGenerate) {
		return true;
	}

	ButtonState &generateButton = _pickerButtonStateMap[kPickerButtons_Generate];
	if (generateButton._isPressDisabled) {
		output = "The Generate button is disabled for the current preview.\n";
		return true;
	}
	if (!debugApplySyntheticGeneratePrefix(output))
		return true;

	const int16 beforeCount = _vm->_state->_f._zmbGeneratedCount;
	pickerButtons_onButtonAction(_pickerButtonsFeature, kPickerButtons_Generate, generateButton);
	if (_vm->_state->_f._zmbGeneratedCount == beforeCount) {
		output += "The normal Generate action did not create the final Zoombini.\n";
		return true;
	}

	output += "The normal Generate action created the final visible Zoombini; saving is blocked until load or new game.\n";
	return false;
}

Common::String ZoombiniShelterPicker::debugGetPickerCommandsHelp() const {
	Common::String output;
	output += "Generate Zoombinis into empty seats, or remove every seated Zoombini.\n";
	output += "Usage: page ";
	output += kPageCommandAutoseat;
	output += "|";
	output += kPageCommandAutounseat;
	output += "\n";
	output += "  autoseat places a freshly rolled Zoombini directly onto every empty\n";
	output += "  seat, honoring twin limits and the 625-generation cap. autounseat is\n";
	output += "  its reverse: each seated Zoombini is removed through the cave-drop\n";
	output += "  path, restoring twin limits and counts.\n\n";
	output += "Options:\n";
	output += "  -h, --help  Show this help text and exit.\n";
	return output;
}

int16 ZoombiniShelterPicker::debugGenerateZoombinisIntoEmptySeats(Common::String &output) {
	if (isDeparturePending() || isDragging()) {
		output = Common::String::format("page %s cannot run while Picker input or departure animation is active.\n", kPageCommandAutoseat);
		return -1;
	}

	if (_mode != kPickerMode_SelectZoombinis) {
		output = "Picker is showing the one-time load dialog; generation needs the Zoombini selection mode.\n";
		return -1;
	}

	const int16 beforeCount = static_cast<int16>(_snoidMap.size());
	if (16 <= beforeCount) {
		output = "All 16 picker seats are already occupied.\n";
		return 0;
	}

	// Play the same dice press sound the interactive button click plays.
	_vm->_sound->playSound(ZmbResource(ZmbResource::kPage, kResSound1006_PressDiceButton), Audio::Mixer::kSFXSoundType, false);

	stopPendingPickerVoice();

	ZmbStateFile &f = _vm->_state->_f;

	// Unlike the Shift+dice rapid fill, which walks every Zoombini in from the
	// cave with staggered starts, the debugger command places each rolled
	// Zoombini directly onto its seat. The state result is identical: pack
	// entries, twin tracking, names, and generation counts all advance alike.
	int16 addedCount = 0;
	while (!isGenerationComplete()) {
		const int16 seatIdx = findFirstEmptySeat();
		if (seatIdx < 0)
			break;

		// Roll a fresh usable trait combination for the next Zoombini.
		if (!randomizePreviewTraits(true))
			break;

		_vm->_state->markSaveBeforeQuitPending();

		const uint16 newSnoidId = allocateDynamicPackSnoidId();
		const uint8 idleTickCounter = static_cast<uint8>(_vm->_rnd->getRandomNumber(0, 64));
		ZmbSnoid *snoid = loadSnoidFromPack(newSnoidId, _zoombiniSeatPoints[seatIdx],
											ZmbFeature::FLAG_00000001_TYPE_SNOID);
		if (!snoid)
			break;

		snoid->setIdleTickCounter(idleTickCounter);
		snoid->_trait = _previewSnoid._trait;
		snoid->_name = _previewSnoid._name;
		// Picker-generated runners always occupy their active-pack slots.
		snoid->_packIsOccupied = true;
		snoid->setupIdleHotspots();
		_seatToSnoid[seatIdx] = snoid;

		// Record another generated twin for this trait combination.
		const int16 snoidTraitId = snoid->_trait.snoidId();
		if (f._twinGenStatus[snoidTraitId] < 2)
			f._twinGenStatus[snoidTraitId] += 1;

		// Add a matching entry to the active pack for later transfer to basecamp.
		if (f._zmbPackActive.getPackZmbCount() < 16) {
			ZmbStateActiveEntry entry;
			entry.setTraits(snoid->_trait);
			entry.setIsOccupied(true);
			entry.setU32Name(_vm, snoid->_name);
			f._zmbPackActive.appendEntry(entry);
		}

		// Make the newly placed Zoombini visible immediately.
		prepareSnoidVisualCoverage(snoid, true);
		addExternalDirtyRect(snoid->getClickRect());
		snoid->setNeedsRedraw(true);

		f._zmbGeneratedCount += 1;
		addedCount += 1;
	}

	// Re-sync matrix buttons to reflect the final preview traits
	for (uint32 row = 0; row < 4; row++) {
		for (uint32 col = 0; col < kMatrixColumns; col++) {
			_matrixButtonStateMap[row * kMatrixColumns + col]._isStuck = (_previewSnoid._trait[row] == col + 1);
		}
	}

	setGoButtonsEnabled(isDepartureReady());
	_pickerButtonStateMap[kPickerButtons_Generate]._isPressDisabled = isGenerationComplete() || !isZoombiniTraitGeneratable(_previewSnoid._trait);
	requestPickerControlRematerialization();

	if (addedCount == 0) {
		output = "No new Zoombinis could be generated; usable trait combinations are exhausted.\n";
		return 0;
	}

	const int16 afterCount = static_cast<int16>(_snoidMap.size());
	output = Common::String::format("Placed %d Zoombini(s) directly onto picker seats (%d/16 occupied).\n", addedCount, afterCount);
	return addedCount;
}

int16 ZoombiniShelterPicker::debugUnseatAllZoombinis(Common::String &output) {
	if (isDeparturePending() || isDragging()) {
		output = Common::String::format("page %s cannot run while Picker input or departure animation is active.\n", kPageCommandAutounseat);
		return -1;
	}

	if (_mode != kPickerMode_SelectZoombinis) {
		output = "Picker is showing the one-time load dialog; removal needs the Zoombini selection mode.\n";
		return -1;
	}

	for (uint16 seatIdx = 0; seatIdx < ARRAYSIZE(_seatToSnoid); seatIdx++) {
		const ZmbSnoid *snoid = _seatToSnoid[seatIdx];
		if (snoid && snoid->getAnimState() != kSnoidAnimState000_Idle) {
			output = "Picker Zoombinis are still walking; try again when they are idle.\n";
			return -1;
		}
	}

	int16 removedCount = 0;
	while (true) {
		// Remove from the highest occupied seat first so repacking never has to shift survivors.
		ZmbSnoid *snoid = nullptr;
		for (int16 seatIdx = 15; 0 <= seatIdx; seatIdx -= 1) {
			if (_seatToSnoid[seatIdx]) {
				snoid = _seatToSnoid[seatIdx];
				break;
			}
		}
		if (!snoid)
			break;

		removeSeatedZoombini(snoid);
		removedCount += 1;
	}

	// Play remove SFX once for the whole batch
	_vm->_sound->playSound(ZmbResource(ZmbResource::kPage, kResSound1007_RemoveZoombini), Audio::Mixer::kSFXSoundType);

	setGoButtonsEnabled(isDepartureReady());
	_pickerButtonStateMap[kPickerButtons_Generate]._isPressDisabled = isGenerationComplete() || !isZoombiniTraitGeneratable(_previewSnoid._trait);
	requestPickerControlRematerialization();

	if (removedCount == 0) {
		output = "No Zoombinis are seated on the picker seats.\n";
		return 0;
	}

	output = Common::String::format("Removed %d Zoombini(s) from the picker seats; the last one is restored as the preview.\n", removedCount);
	return removedCount;
}

bool ZoombiniShelterPicker::debugDoPageCommand(int argc, const char **argv, Common::String &output) {
	if (argc != 2 || (scumm_stricmp(argv[1], kPageCommandAutoseat) != 0 && scumm_stricmp(argv[1], kPageCommandAutounseat) != 0)) {
		output = debugGetPickerCommandsHelp();
		return true;
	}

	int16 changedCount;
	if (scumm_stricmp(argv[1], kPageCommandAutoseat) == 0)
		changedCount = debugGenerateZoombinisIntoEmptySeats(output);
	else
		changedCount = debugUnseatAllZoombinis(output);
	if (0 < changedCount)
		_vm->_state->markDebugStateMutation();
	return changedCount < 0;
}

} // End of namespace Mohawk
