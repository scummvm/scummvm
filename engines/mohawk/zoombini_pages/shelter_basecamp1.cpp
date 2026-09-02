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
#include "mohawk/cursors.h"
#include "mohawk/mohawk.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"

#include "mohawk/zoombini.h"
#include "mohawk/zoombini_graphics.h"
#include "mohawk/zoombini_pages/shelter_basecamp1.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_sound.h"
#include "mohawk/zoombini_state.h"
#include "mohawk/zoombini_text.h"

namespace Mohawk {

constexpr Common::Point ZoombiniShelterBasecampOne::_pedestalPoints[ZoombiniShelterBasecampOne::kPedestalCount];
constexpr int16 ZoombiniShelterBasecampOne::_storageMatrixX1[ZoombiniShelterBasecampOne::kStorageAnimatedColumnCount];
constexpr int16 ZoombiniShelterBasecampOne::_storageMatrixX2[ZoombiniShelterBasecampOne::kStorageAnimatedColumnCount];
constexpr int16 ZoombiniShelterBasecampOne::_storageMatrixY1[ZoombiniShelterBasecampOne::kStorageAnimatedColumnCount][ZoombiniShelterBasecampOne::kStorageRowsPerColumn];
constexpr int16 ZoombiniShelterBasecampOne::_storageMatrixY2[ZoombiniShelterBasecampOne::kStorageAnimatedColumnCount][ZoombiniShelterBasecampOne::kStorageRowsPerColumn];

ZoombiniShelterBasecampOne::ZoombiniShelterBasecampOne(MohawkEngine_Zoombini *vm)
	: ZoombiniShelterBasecamp(vm, ZoombiniPageType::kBasecamp1,
							  Common::Rect(0x0037, 0x0013, 0x00FF, 0x00FC), kStorageMinimumCapacity,
							  kResBitmapShape2100_Buttons, true, _pedestalPoints,
							  ZmbResource(ZmbResource::kPage, kResSound2000_StorageScrolling),
							  ZmbResource(ZmbResource::kPage, kResSound2001_StorageScrollEnd)) {
	_scrollButtonStateMap[kStorageScrollButton01_LeftMaximum] = ContinuousButtonState(0, 4, kShape2100_ScrollLeftFourNormal_07, kShape2100_ScrollLeftFourPressed_08);
	_scrollButtonStateMap[kStorageScrollButton02_LeftOne] = ContinuousButtonState(1, 5, kShape2100_ScrollLeftOneNormal_09, kShape2100_ScrollLeftOnePressed_10);
	_scrollButtonStateMap[kStorageScrollButton03_RightOne] = ContinuousButtonState(2, 6, kShape2100_ScrollRightOneNormal_11, kShape2100_ScrollRightOnePressed_12);
	_scrollButtonStateMap[kStorageScrollButton04_RightMaximum] = ContinuousButtonState(3, 7, kShape2100_ScrollRightFourNormal_13, kShape2100_ScrollRightFourPressed_14);

	// Z1-20U/TLC v2.0 release only: storage scroll buttons gain yellow-outline
	// hover bitmap states in addition to the page-local cursor arrows.
	if (_vm->isVersionFamilyTlcV2()) {
		_scrollButtonStateMap[kStorageScrollButton01_LeftMaximum].setHoverState(kShape2100_ScrollLeftFourHover_20);
		_scrollButtonStateMap[kStorageScrollButton02_LeftOne].setHoverState(kShape2100_ScrollLeftOneHover_21);
		_scrollButtonStateMap[kStorageScrollButton03_RightOne].setHoverState(kShape2100_ScrollRightOneHover_22);
		_scrollButtonStateMap[kStorageScrollButton04_RightMaximum].setHoverState(kShape2100_ScrollRightFourHover_23);
	}

	_scrollButtonRectMap[kStorageScrollButton01_LeftMaximum] = _scrollLeftFourButtonRect;
	_scrollButtonRectMap[kStorageScrollButton02_LeftOne] = _scrollLeftOneButtonRect;
	_scrollButtonRectMap[kStorageScrollButton03_RightOne] = _scrollRightOneButtonRect;
	_scrollButtonRectMap[kStorageScrollButton04_RightMaximum] = _scrollRightFourButtonRect;
}

ZoombiniShelterBasecampOne::~ZoombiniShelterBasecampOne() {
}

const ZoombiniPage::ScriptSoundPriorityRanges &ZoombiniShelterBasecampOne::getScriptSoundPriorityRanges() const {
	static const ZoombiniPage::ScriptSoundPriorityRanges kRanges = {
		{kSysResSoundRange20000_Base, kSysResSoundRange29999_Last},
		{kResSoundRange2000_StorageScrollingBase, kResSoundRange2099_StorageRangeLast},
		{kResSoundRange1100_EasterEggRangeBase, kResSoundRange1199_EasterEggRangeLast}};
	return kRanges;
}

void ZoombiniShelterBasecampOne::open() {
	// Basecamp visits always make the current game state eligible for saving.
	_vm->_state->markSaveBeforeQuitPending();
	openArchive(ZMB_MHK_BASECAMP);

	// Both Go routes use the BASECAMP NODE/PATH 1000 graph before their final off-screen segment.
	loadNodePath(ZmbResource(ZmbResource::kPage, kResNode1000));
	// REGS 9000/9001 are a paired X/Y axis table for the cursor shapes in tBMP 9000:
	// one big-endian int16 per entry, reserved entry 0, then one entry per sub-image.
	loadShapeOffsetRegs(ZmbResource::kPage, kResRegs9000);
}

void ZoombiniShelterBasecampOne::setBackgroundBitmap() {
	_vm->_gfx->setPalette(kResBackground1000);
	_vm->_gfx->drawBackground(kResBackground1000);
}

void ZoombiniShelterBasecampOne::initStates() {
	ZmbStateFile &f = _vm->_state->_f;
	_storageLeftmostColumnIdx = f._storedChunkBC1.getLeftmostColumnIdx();
	// Complete trait records are the physical belt occupants. Older original
	// saves may have a lower header after a restored drag failed to increment it.
	f._storedChunkBC1.setStoredCount(f._storedChunkBC1.getPopulatedEntryCount());
	_storageMaxCellIdx = findLastOccupiedIdx();
	calcStorageColumns();
}

void ZoombiniShelterBasecampOne::loadFeatures() {
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape1100));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape1200_Pedestal));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape2000_Storage));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape2100_Buttons));

	{ // [*] Callback-only runner: Storage (refers to tBMP 2000)
		// Register a resource-ID-0 runner for the stored-Zoombini grid viewer.
		ZmbFeature::EventHooks hooksStorage;
		hooksStorage.setRenderFunc(static_cast<ZmbFeature::OnRenderFunc>(&ZoombiniShelterBasecampOne::storage_render));
		hooksStorage.setPostRenderFunc(static_cast<ZmbFeature::OnPostRenderFunc>(&ZoombiniShelterBasecampOne::storage_postRender));
		ZmbFeature *storageFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, 0), 0, 6,
													 ZmbFeature::FLAG_00004000_NO_DIRTY_MERGE | ZmbFeature::FLAG_00008000_LOOP_ANIM,
													 hooksStorage);
		storageFeature->setClickRect(_storageRect);
	}

	// Configure the controls before registering their shared callback runner.
	setGoButton(_goRouteUpButtonRect, kShape2100_GoRouteUpButtonDisabled_15, kShape2100_GoRouteUpButtonNormal_01, kShape2100_GoRouteUpButtonPressed_02);
	setSecondGoButton(_goRouteDownButtonRect, kShape2100_GoRouteDownButtonDisabled_16, kShape2100_GoRouteDownButtonNormal_03, kShape2100_GoRouteDownButtonPressed_04);
	setMapButton(_mapButtonClickRect, kShape2100_MapNormal_05, kShape2100_MapPressed_06);
	setHelpButton(_helpButtonClickRect);
	buildEmbeddedControlHotspots(kResBitmapShape2100_Buttons, _controlGoMapBaseHotspots, _controlHelpBaseHotspots);

	{ // [*] Callback-only runner: Storage Scroll Buttons (refers to tBMP 2100)
		// Register a resource-ID-0 runner for scroll-button drawing and input.
		ZmbFeature::EventHooks hooksScroll;
		hooksScroll.setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniShelterBasecampOne::scroll_preRenderShape));
		hooksScroll.setPostRenderFunc(static_cast<ZmbFeature::OnPostRenderFunc>(&ZoombiniShelterBasecampOne::scroll_postRender));
		hooksScroll.setLButtonDownFunc(static_cast<ZmbFeature::OnLButtonDownFunc>(&ZoombiniShelterBasecampOne::scroll_lButtonDown));
		hooksScroll.setLButtonUpFunc(static_cast<ZmbFeature::OnLButtonUpFunc>(&ZoombiniShelterBasecampOne::scroll_lButtonUp));
		hooksScroll.setMouseMoveFunc(static_cast<ZmbFeature::OnMouseMoveFunc>(&ZoombiniShelterBasecampOne::scroll_mouseMove));

		Common::Array<ZmbHotspot> scrollHotspots;
		scrollHotspots.push_back(ZmbHotspot(0, kShape2100_ScrollLeftFourNormal_07, 0, _scrollLeftFourButtonRect));
		scrollHotspots.push_back(ZmbHotspot(1, kShape2100_ScrollLeftOneNormal_09, 0, _scrollLeftOneButtonRect));
		scrollHotspots.push_back(ZmbHotspot(2, kShape2100_ScrollRightOneNormal_11, 0, _scrollRightOneButtonRect));
		scrollHotspots.push_back(ZmbHotspot(3, kShape2100_ScrollRightFourNormal_13, 0, _scrollRightFourButtonRect));
		scrollHotspots.push_back(ZmbHotspot(4, kShape2100_ScrollLeftFourPressed_08, 0, _scrollLeftFourButtonRect));
		scrollHotspots.push_back(ZmbHotspot(5, kShape2100_ScrollLeftOnePressed_10, 0, _scrollLeftOneButtonRect));
		scrollHotspots.push_back(ZmbHotspot(6, kShape2100_ScrollRightOnePressed_12, 0, _scrollRightOneButtonRect));
		scrollHotspots.push_back(ZmbHotspot(7, kShape2100_ScrollRightFourPressed_14, 0, _scrollRightFourButtonRect));

		// FLAG_00001000_TOPMOST | FLAG_00008000_LOOP_ANIM.
		// LOOP_ANIM checked first -> loopAnimList (rendered behind sorted features).
		loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape2100_Buttons), 0, scrollHotspots, 0,
						ZmbFeature::FLAG_00001000_TOPMOST | ZmbFeature::FLAG_00008000_LOOP_ANIM,
						hooksScroll);
	}

	{ // [*] One callback-only runner for Go, Map, and Help.
		Common::Rect clickRect = _goRouteUpButtonRect;
		clickRect.extend(_goRouteDownButtonRect);
		clickRect.extend(_mapButtonClickRect);
		if (_vm->supportsOnScreenHelp())
			clickRect.extend(_helpButtonClickRect);
		ZmbFeature *feature = loadBasecampControlFeature(clickRect);
		bindEmbeddedControlFeature(feature);
	}

	// [*] SCRB 1200 ~ 1215: Pedestals
	loadBasecampPedestalFeatures(ZmbResource(ZmbResource::kPage, kResBitmapShape1200_Pedestal), kResScrb1200_Pedestal);

	{ // [*] SCRB 1106, 1108, 1109, 1110, 1107: Easter Eggs
		ZmbFeature::EventHooks hooksStoneMan;
		hooksStoneMan.setPostRenderFunc(static_cast<ZmbFeature::OnPostRenderFunc>(&ZoombiniShelterBasecampOne::easterEggStoneMan_postRender));
		hooksStoneMan.setLButtonDownFunc(static_cast<ZmbFeature::OnLButtonDownFunc>(&ZoombiniShelterBasecampOne::easterEggStoneMan_onLButtonDown));
		loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1100), kResScrb1106_EasterEggStoneMan, 6,
						ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_01000000_DEFER_RENDER,
						hooksStoneMan);

		ZmbFeature::EventHooks hooksFish;
		hooksFish.setPostRenderFunc(static_cast<ZmbFeature::OnPostRenderFunc>(&ZoombiniShelterBasecampOne::easterEggFish_postRender));
		hooksFish.setLButtonDownFunc(static_cast<ZmbFeature::OnLButtonDownFunc>(&ZoombiniShelterBasecampOne::easterEggFish_onLButtonDown));
		loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1100), kResScrb1108_EasterEggFish, 6,
						ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_01000000_DEFER_RENDER,
						hooksFish);

		ZmbFeature::EventHooks hooksBear;
		hooksBear.setPostRenderFunc(static_cast<ZmbFeature::OnPostRenderFunc>(&ZoombiniShelterBasecampOne::easterEggBear_postRender));
		hooksBear.setLButtonDownFunc(static_cast<ZmbFeature::OnLButtonDownFunc>(&ZoombiniShelterBasecampOne::easterEggBear_onLButtonDown));
		loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1100), kResScrb1109_EasterEggBear, 6,
						ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE,
						hooksBear);

		ZmbFeature::EventHooks hooksStoneFace;
		hooksStoneFace.setPostRenderFunc(static_cast<ZmbFeature::OnPostRenderFunc>(&ZoombiniShelterBasecampOne::easterEggStoneFace_postRender));
		hooksStoneFace.setLButtonDownFunc(static_cast<ZmbFeature::OnLButtonDownFunc>(&ZoombiniShelterBasecampOne::easterEggStoneFace_onLButtonDown));
		// Both Z-sort barriers keep Snoids from being sorted in front of StoneFace.
		loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1100), kResScrb1110_EasterEggStoneFace, 6,
						ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_10000000_ZSORT_RIGHT | ZmbFeature::FLAG_40000000_ZSORT_LEFT,
						hooksStoneFace);

		ZmbFeature::EventHooks hooksHollowBugs;
		hooksHollowBugs.setPostRenderFunc(static_cast<ZmbFeature::OnPostRenderFunc>(&ZoombiniShelterBasecampOne::easterEggHollowBugs_postRender));
		hooksHollowBugs.setLButtonDownFunc(static_cast<ZmbFeature::OnLButtonDownFunc>(&ZoombiniShelterBasecampOne::easterEggHollowBugs_onLButtonDown));
		loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1100), kResScrb1107_EasterEggHollowBugs, 6,
						ZmbFeature::FLAG_00001000_TOPMOST | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_01000000_DEFER_RENDER,
						hooksHollowBugs);
	}

	// [*] SCRB 1111 ~ 1115: Easter Egg Mushrooms
	// Pre-render clears render activation each frame, but post-render draws non-DEFER_RENDER features regardless,
	// so these mushrooms remain visible.
	// Initialization and each click call @ref ZmbFeature::activateRender().
	// One pre-render pass then applies the selected mushroom color before the skip-render flag deactivates it.
	for (uint32 i = 0; i <= kResScrb1115_EasterEggMushroom5 - kResScrb1111_EasterEggMushroom1; i++) {
		ZmbFeature::EventHooks hooks;
		hooks.setSelectRenderFrameFunc(static_cast<ZmbFeature::OnSelectRenderFrameFunc>(&ZoombiniShelterBasecampOne::easterEggMushroom_selectRenderFrame));
		hooks.setLButtonDownFunc(static_cast<ZmbFeature::OnLButtonDownFunc>(&ZoombiniShelterBasecampOne::easterEggMushroom_onLButtonDown));
		ZmbFeature *feature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1100), i + kResScrb1111_EasterEggMushroom1, 0,
											  ZmbFeature::FLAG_00020000_SKIP_RENDER,
											  hooks);
		feature->activateRender(); // Load the initial color on the first pre-render pass.
	}

	{ // [*] SCRB 1104: Bonfire (randomly animates; clicking triggers Pod animation)
		ZmbFeature::EventHooks hooks;
		hooks.setLButtonDownFunc(static_cast<ZmbFeature::OnLButtonDownFunc>(&ZoombiniShelterBasecampOne::easterEggBonfire_onLButtonDown));
		loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1100), kResScrb1104_Bonfire, 6,
						ZmbFeature::FLAG_00040000_CHAIN_SCRIPT | ZmbFeature::FLAG_02000000_RANDOM_FRAME,
						hooks);
	}

	// [*] SCRB 1100 ~ 1103: Bottom shapes
	// These static shapes use a zero frame interval.
	for (uint32 i = kResScrb1100_BottomShape1; i <= kResScrb1103_BottomShape4; i++) {
		loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1100), i, 0, ZmbFeature::FLAG_00000000_TYPE_SHAPES);
	}

	// [*] Zoombini Pack Management
	ZmbStateFile &f = _vm->_state->_f;

	// Load the incoming pack, then replace it with the BC1 resident snapshot.
	const int16 arrivingCount = beginBasecampPackLoad();

	// Step 4: Storage grid management (insert arriving zoombinis into BC1 storage)
	if (0 < arrivingCount && !f._zmbPackActive.getSkipOccupiedEntries()) {
		int16 occupiedInActive = 0;
		for (int16 i = 0; i < f._zmbPackActive.getPackZmbCount(); i++) {
			if (f._zmbPackActive.getEntry(i).getIsOccupied())
				occupiedInActive += 1;
		}
		if (0 < occupiedInActive) {
			// Insert arriving zoombinis into the BC1 storage grid.
			int16 prevMaxCellIdx = _storageMaxCellIdx;
			int16 storeResult = storeBasecampActivePackInStorage(StorageOccupancyTest::kComplete);

			// Update stored count and recalculate column layout.
			f._storedChunkBC1.setStoredCount(f._storedChunkBC1.getPopulatedEntryCount());
			_storageMaxCellIdx = findLastOccupiedIdx();
			calcStorageColumns();

			// Adding one advances from the last occupied slot to the next free slot's column.
			// Without it, the scroll column lands one column too early when (prevMaxCellIdx % 5 == 4).
			if (storeResult == 1) {
				_storageLeftmostColumnIdx = static_cast<int16>(((prevMaxCellIdx + 1) / 5) % _storageColumnCount);
				calcStorageColumns();
			}
			// The occupied BC1 residents were transferred to storage.
			// Do not load them onto the 16 pedestal seats again below.
			f._zmbPackActive.setSkipOccupiedEntries(true);
		}
	}

	completeBasecampPackLoad(arrivingCount);

	// Arrival voice sound (Only played when the player arrives from completing a puzzle)
	int16 arriveSoundId = 0;
	ZmbSfxGroupFlags sfxGroupFlags = ZmbSfxGroupFlags::kRandom_00;
	bool hasSfxGroupFlags = false;
	if (_vm->_state->_lastPageBeforeContainer != 0) {
		ZmbSfxGroupFlags rawSfxGroupFlags = _vm->_state->getSfxGroupFlagsFromPageFlag(f._pageFlagBasecamp1);
		_vm->_state->_lastPageBeforeContainer = 0;
		// Use Group 1 when no later-route puzzle was visited and only a small group is stored.
		if (rawSfxGroupFlags == ZmbSfxGroupFlags::kHardFirstTrigger_02 && f._pageFlagFerry.isZero() && f._pageFlagFleens.isZero() && f._zmbStoredBC1Count <= kPedestalCount) {
			rawSfxGroupFlags = ZmbSfxGroupFlags::kEasy_01;
			f._pageFlagBasecamp1.clearHardGroupTriggers();
		}
		sfxGroupFlags = rawSfxGroupFlags;
		hasSfxGroupFlags = true;
	}

	// Expand the random choice range after a dedicated Group 2 visit marker has been set.
	const int16 randFlag = f._pageFlagBasecamp1.hasHardGroupTrigger() ? 6 : 4;
	if (_isFinalArrival) {
		if (hasSfxGroupFlags) {
			const int16 r = _vm->_rnd->getRandomNumber(1, 3);
			switch (r) {
			case 1:
				arriveSoundId = kSysResSound20051_ArriveBC1Voice;
				break;
			case 2:
				arriveSoundId = kSysResSound20053_ArriveBC1Voice;
				break;
			case 3:
				arriveSoundId = kSysResSound20054_ArriveBC1Voice;
				break;
			}
		}
	} else if (hasSfxGroupFlags) {
		switch (sfxGroupFlags) {
		case ZmbSfxGroupFlags::kRandom_00: {
			// Randomly select from the currently unlocked arrival voices.
			const int16 r = _vm->_rnd->getRandomNumber(1, static_cast<uint16>(randFlag));
			switch (r) {
			case 1:
				arriveSoundId = kSysResSound20049_ArriveBC1Voice;
				break;
			case 2:
				arriveSoundId = kSysResSound20051_ArriveBC1Voice;
				break;
			case 3:
				arriveSoundId = kSysResSound20053_ArriveBC1Voice;
				break;
			case 4:
				arriveSoundId = kSysResSound20054_ArriveBC1Voice;
				break;
			case 5:
				arriveSoundId = kSysResSound20050_ArriveBC1Voice;
				break;
			case 6:
				arriveSoundId = kSysResSound20052_ArriveBC1Voice;
				break;
			default:
				break;
			}
			break;
		}
		case ZmbSfxGroupFlags::kEasy_01:
		case ZmbSfxGroupFlags::kPractice_05:
			arriveSoundId = kSysResSound20049_ArriveBC1Voice;
			break;
		case ZmbSfxGroupFlags::kHardFirstTrigger_02:
			arriveSoundId = kSysResSound20050_ArriveBC1Voice;
			break;
		case ZmbSfxGroupFlags::kHardSecondTrigger_12:
			arriveSoundId = kSysResSound20052_ArriveBC1Voice;
			break;
		default:
			break;
		}
	}

	if (arriveSoundId)
		_vm->_sound->playSound(ZmbResource(ZmbResource::kSystem, arriveSoundId), Audio::Mixer::kSpeechSoundType);
}

void ZoombiniShelterBasecampOne::onGoButtonActivated() {
	startBasecampDeparture(Common::Point(680, 316));
	_departRouteDirection = 1;
	_departureState = ZmbDepartureState::kAnimating;
}

void ZoombiniShelterBasecampOne::onSecondGoButtonActivated() {
	// SFX 996, walk snoids to (680, 400), stagger 45 frames, fade out when SFX finishes.
	startBasecampDeparture(Common::Point(680, 400));
	_departRouteDirection = 2;
	_departureState = ZmbDepartureState::kAnimating;
}

ZmbSrcPageKind ZoombiniShelterBasecampOne::getBasecampDepartureSourcePage() const {
	if (_departRouteDirection == 1)
		return ZmbSrcPageKind::kBC1North_05;
	return ZmbSrcPageKind::kBC1South_06;
}

ZmbRenderResult ZoombiniShelterBasecampOne::storage_render(ZmbFeature *feature) {
	(void)feature;

	ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;

	// Draw only the honeycomb background here (behind snoids).
	// @ref ZoombiniShelterBasecampOne::storage_postRender() draws the lattice overlay and border over the snoids.
	// The complete order is honeycomb, snoids, lattice, then border.
	uint16 matrixShapeId = _storageScrollAnimating ? kShapeStorage01_Honeycomb : kShapeStorage03_Honeycomb;
	ZmbResource storageBitmap = ZmbResource(ZmbResource::kPage, kResBitmapShape2000_Storage);
	_vm->_gfx->drawShape(screenKind, storageBitmap, matrixShapeId, Common::Point(53, 6));
	return ZmbRenderResult::kRendered;
}

void ZoombiniShelterBasecampOne::storage_postRender(ZmbFeature *feature) {
	(void)feature;

	// Draw stored zoombinis, then the lattice grid overlay, then the border.
	ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;
	ZmbStateStoredChunk &chunk = _vm->_state->_f._storedChunkBC1;
	renderBasecampStorageEntries(chunk, _storageMatrixX1, _storageMatrixY1, _storageMatrixX2, _storageMatrixY2, StorageOccupancyTest::kComplete);

	// Lattice overlay (drawn on top of snoids) and border (drawn last)
	ZmbResource storageBitmap = ZmbResource(ZmbResource::kPage, kResBitmapShape2000_Storage);
	uint16 latticeShapeId = _storageScrollAnimating ? kShapeStorage02_Lattice : kShapeStorage04_Lattice;
	uint16 latticePosY = _storageScrollAnimating ? 9 : 12;
	_vm->_gfx->drawShape(screenKind, storageBitmap, latticeShapeId, Common::Point(53, latticePosY));
	_vm->_gfx->drawShape(screenKind, storageBitmap, kShapeStorage05_Border, Common::Point(31, 0));
}

void ZoombiniShelterBasecampOne::scroll_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)hsGroup;

	continuousButton_selectShapes(feature, hotspots, _scrollButtonStateMap);
}

void ZoombiniShelterBasecampOne::scroll_postRender(ZmbFeature *feature) {
	(void)feature;

	updateHeldBasecampStorageScroll();
	advanceBasecampStorageScroll();
	_vm->_state->_f._storedChunkBC1.setLeftmostColumnIdx(_storageLeftmostColumnIdx);
}

ZmbEventHandleResult ZoombiniShelterBasecampOne::scroll_lButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	(void)relPos;

	ZmbDrawRecord *drawRecord = feature->findDrawRecordAtPoint(absPos);
	if (!drawRecord)
		return ZmbEventHandleResult::kPassthrough;

	for (Common::StableMap<uint32, ContinuousButtonState>::iterator it = _scrollButtonStateMap.begin(); it != _scrollButtonStateMap.end(); it++) {
		ContinuousButtonState &bs = it->second;

		if (!bs._enabled)
			continue;

		// Find the pressed button
		if (drawRecord->_hs._hsId != bs._hsNormalIdx && drawRecord->_hs._hsId != bs._hsPressedIdx)
			continue;

		releaseHeldScrollButton(feature);
		bs.press();
		beginBasecampStorageScroll(getBasecampStorageScrollDirection(it->first));
		feature->setNeedsRedraw(true);
		return ZmbEventHandleResult::kConsumed;
	}

	return ZmbEventHandleResult::kPassthrough;
}

ZmbEventHandleResult ZoombiniShelterBasecampOne::scroll_lButtonUp(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	(void)absPos;
	(void)relPos;

	if (releaseHeldScrollButton(feature))
		return ZmbEventHandleResult::kConsumed;

	return ZmbEventHandleResult::kPassthrough;
}

bool ZoombiniShelterBasecampOne::releaseHeldScrollButton(ZmbFeature *feature) {
	bool released = false;
	Common::Rect dirtyRect;

	if (feature)
		dirtyRect = feature->getZSortRect();

	for (Common::StableMap<uint32, ContinuousButtonState>::iterator it = _scrollButtonStateMap.begin(); it != _scrollButtonStateMap.end(); it++) {
		uint32 buttonIdx = it->first;
		ContinuousButtonState &bs = it->second;

		if (!bs._pressed)
			continue;

		bs.release();
		released = true;

		Common::HashMap<uint32, Common::Rect>::const_iterator rit = _scrollButtonRectMap.find(buttonIdx);
		if (rit != _scrollButtonRectMap.end()) {
			if (dirtyRect.isEmpty())
				dirtyRect = rit->_value;
			else
				dirtyRect.extend(rit->_value);
		}
	}

	if (!released)
		return false;

	releaseBasecampStorageScroll();

	if (!dirtyRect.isEmpty())
		addExternalDirtyRect(dirtyRect);
	if (feature)
		feature->setNeedsRedraw(true);

	return true;
}

ZmbEventHandleResult ZoombiniShelterBasecampOne::scroll_mouseMove(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	(void)relPos;

	uint16 cursorShapeIdx = ZmbHotspot::kShapeNone;
	// Z1-20U/TLC v2.0 release only: 1.x keeps the arrow cursor behavior but
	// does not swap storage button shapes on hover.
	bool hoverChanged = false;
	Common::Rect hoverDirtyRect = feature->getZSortRect();
	bool releaseHeldButton = false;

	ZmbDrawRecord *drawRecord = feature->findDrawRecordAtPoint(absPos);
	for (Common::StableMap<uint32, ContinuousButtonState>::iterator it = _scrollButtonStateMap.begin(); it != _scrollButtonStateMap.end(); it++) {
		uint32 buttonIdx = it->first;
		ContinuousButtonState &bs = it->second;

		if (!bs._enabled)
			continue;

		Common::HashMap<uint32, Common::Rect>::const_iterator rit = _scrollButtonRectMap.find(buttonIdx);
		if (_vm->isVersionFamilyTlcV2() && bs.hasHoverState() && rit != _scrollButtonRectMap.end()) {
			if (hoverDirtyRect.isEmpty())
				hoverDirtyRect = rit->_value;
			else
				hoverDirtyRect.extend(rit->_value);
		}

		const bool buttonHovered = drawRecord &&
								   (drawRecord->_hs._hsId == bs._hsNormalIdx || drawRecord->_hs._hsId == bs._hsPressedIdx);
		if (_vm->isVersionFamilyTlcV2() && bs.hasHoverState())
			hoverChanged |= bs.setHovered(buttonHovered);

		if (bs._pressed && rit != _scrollButtonRectMap.end() && !rit->_value.contains(absPos))
			releaseHeldButton = true;

		if (!buttonHovered)
			continue;

		// The button is being hovered. Set corresponding bitmap as a cursor.
		switch (buttonIdx) {
		case kStorageScrollButton01_LeftMaximum:
			cursorShapeIdx = kShape9000_ArrowLeftMax_01;
			break;
		case kStorageScrollButton02_LeftOne:
			cursorShapeIdx = kShape9000_ArrowLeft_02;
			break;
		case kStorageScrollButton03_RightOne:
			cursorShapeIdx = kShape9000_ArrowRight_03;
			break;
		case kStorageScrollButton04_RightMaximum:
			cursorShapeIdx = kShape9000_ArrowRightMax_04;
			break;
		default:
			error("basecamp1: scroll_mouseMove: invalid buttonIdx %u", buttonIdx);
			break;
		}
	}

	// Check held-button bounds before releasing the common scroll sound state.
	if (releaseHeldButton)
		releaseHeldScrollButton(feature);

	if (hoverChanged) {
		if (!hoverDirtyRect.isEmpty())
			addExternalDirtyRect(hoverDirtyRect);
		feature->setNeedsRedraw(true);
	}

	if (cursorShapeIdx != _storageButtonCursorShapeIdx) {
		if (cursorShapeIdx == ZmbHotspot::kShapeNone) {
			_vm->_cursor->setDefaultCursor();
		} else {
			ZmbShapeOffsetRegs *regs = _shapeOffsetRegsMap[kResRegs9000];
			ZoombiniCursorManager *zmbCursor = dynamic_cast<ZoombiniCursorManager *>(_vm->_cursor);
			zmbCursor->setShapeCursor(ZmbResource::kPage, kResBitmapShape9000_Cursors, cursorShapeIdx, regs->getShapeDelta(cursorShapeIdx));
		}
		_storageButtonCursorShapeIdx = cursorShapeIdx;
	}

	return ZmbEventHandleResult::kPassthrough;
}

ZmbRenderResult ZoombiniShelterBasecampOne::virt03_render(ZmbFeature *feature) {
	(void)feature;

	return ZmbRenderResult::kRendered;
}

void ZoombiniShelterBasecampOne::virt03_postRender(ZmbFeature *feature) {
	(void)feature;
}

ZmbEventHandleResult ZoombiniShelterBasecampOne::genericEasterEgg_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos, const Common::Rect &clickRect) {
	(void)relPos;

	if (!clickRect.contains(absPos))
		return ZmbEventHandleResult::kPassthrough;

	// The render gate owns reentry for hidden animations.
	// Reloading the current SCRB resets frame, sound, timing, and dirty state for every replay.
	if (feature->isRenderActivated())
		return ZmbEventHandleResult::kConsumed;

	loadScrbOntoFeature(feature, 0);
	return ZmbEventHandleResult::kConsumed;
}

void ZoombiniShelterBasecampOne::easterEggStoneMan_postRender(ZmbFeature *feature) {
	(void)feature;
}

ZmbEventHandleResult ZoombiniShelterBasecampOne::easterEggStoneMan_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	return genericEasterEgg_onLButtonDown(feature, absPos, relPos, _easterEggStoneManRect);
}

void ZoombiniShelterBasecampOne::easterEggFish_postRender(ZmbFeature *feature) {
	(void)feature;
}

ZmbEventHandleResult ZoombiniShelterBasecampOne::easterEggFish_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	return genericEasterEgg_onLButtonDown(feature, absPos, relPos, _easterEggFishRect);
}

void ZoombiniShelterBasecampOne::easterEggBear_postRender(ZmbFeature *feature) {
	(void)feature;
}

ZmbEventHandleResult ZoombiniShelterBasecampOne::easterEggBear_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	return genericEasterEgg_onLButtonDown(feature, absPos, relPos, _easterEggBearRect);
}

void ZoombiniShelterBasecampOne::easterEggStoneFace_postRender(ZmbFeature *feature) {
	(void)feature;
}

ZmbEventHandleResult ZoombiniShelterBasecampOne::easterEggStoneFace_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	return genericEasterEgg_onLButtonDown(feature, absPos, relPos, _easterEggStoneFaceRect);
}

void ZoombiniShelterBasecampOne::easterEggHollowBugs_postRender(ZmbFeature *feature) {
	(void)feature;
}

ZmbEventHandleResult ZoombiniShelterBasecampOne::easterEggHollowBugs_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	return genericEasterEgg_onLButtonDown(feature, absPos, relPos, _easterEggHollowBugsRect);
}

int16 ZoombiniShelterBasecampOne::findLastOccupiedIdx() const {
	const ZmbStateStoredChunk &chunk = _vm->_state->_f._storedChunkBC1;
	return findLastBasecampStorageEntry(chunk, StorageOccupancyTest::kComplete) + 1;
}

bool ZoombiniShelterBasecampOne::expandBasecampStorageAtLeftBoundary() {
	ZmbStateStoredChunk &chunk = _vm->_state->_f._storedChunkBC1;
	if (!expandBasecampStorageEntriesAtLeftBoundary(chunk, StorageOccupancyTest::kComplete))
		return false;

	_storageMaxCellIdx += kStorageRowsPerColumn;
	calcStorageColumns();
	_storageLeftmostColumnIdx += 1;
	chunk.setLeftmostColumnIdx(_storageLeftmostColumnIdx);
	return true;
}

void ZoombiniShelterBasecampOne::calcStorageColumns() {
	ZmbStateStoredChunk &chunk = _vm->_state->_f._storedChunkBC1;
	recalculateBasecampStorageCapacity(_storageMaxCellIdx, chunk.getStoredCount());
}

/**
 * Find the storage entry index at the given click area.
 *
 * Iterates through the 25 visible storage slots (5 columns x 5 rows) starting
 * from the current leftmost visible column. For occupied slots, checks if the
 * cursor point falls within the zoombini's stored rect (immediate return).
 * For empty slots, constructs a 60x60 rect centered on the grid position and
 * returns the empty slot with the largest qualifying intersection area.
 *
 * @param searchOccupied If true, search for occupied slots; if false, search for empty slots.
 * @param clickRect The click area rectangle. For occupied search, only the top-left point is used.
 * @param leftmostColumnIdx The current leftmost visible column index (0~120).
 * @return The storage entry index (0~624), or -1 if no matching slot found.
 */
int16 ZoombiniShelterBasecampOne::findStorageSlotIndex(bool searchOccupied, const Common::Rect &clickRect, int16 leftmostColumnIdx) {
	const ZmbStateStoredChunk &chunk = _vm->_state->_f._storedChunkBC1;
	return findBasecampStorageSlotIndex(chunk, searchOccupied, clickRect, leftmostColumnIdx,
										kStorageColumnCount, kStorageEntryCount,
										_storageMatrixX2, _storageMatrixY2, StorageOccupancyTest::kHair);
}

ZmbStateStoredChunk &ZoombiniShelterBasecampOne::getBasecampStorageChunk() {
	return _vm->_state->_f._storedChunkBC1;
}

void ZoombiniShelterBasecampOne::refreshBasecampStorageState() {
	ZmbStateStoredChunk &chunk = getBasecampStorageChunk();
	synchronizeBasecampStoredCount(chunk, nullptr);
	_storageMaxCellIdx = findLastOccupiedIdx();
	calcStorageColumns();
}

int16 ZoombiniShelterBasecampOne::getBasecampAvailableSnoidCount() const {
	const ZmbStateFile &f = _vm->_state->_f;
	return f._zmbPackIsle.getPackZmbCount() + f._zmbStoredBC1Count;
}

ZmbEventHandleResult ZoombiniShelterBasecampOne::easterEggBonfire_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	(void)relPos;

	if (!feature->findDrawRecordAtPoint(absPos))
		return ZmbEventHandleResult::kPassthrough;

	// SCRB 1105 starts the embedded infinite-loop SND 1123 at frame zero.
	// Keep this chained bonfire runner in the priority queue so the return to SCRB 1104
	// replaces the loop instead of leaving an orphaned mixer stream behind.
	feature->setScriptSoundPolicy(ZmbFeature::ScriptSoundPolicy::kPriorityQueue);
	loadScrbOntoFeature(feature, kResScrb1105_EasterEggPod);
	const int32 frameIdx = feature->getLastFrameIdx();
	if (feature->enqueueFrameSound(frameIdx))
		feature->setLastSoundedFrameIdx(frameIdx);
	return ZmbEventHandleResult::kConsumed;
}

// Find a mushroom runner by click-rectangle containment.
// Features without @ref ZmbFeature::FLAG_01000000_DEFER_RENDER remain drawn after pre-render deactivates them.
// @ref ZmbFeature::activateRender() updates @ref ZmbFeature::_lastFrameIdx from the selected color for one pass.
int32 ZoombiniShelterBasecampOne::easterEggMushroom_selectRenderFrame(ZmbFeature *feature) {
	uint16 stateIdx = feature->getId() - kResScrb1111_EasterEggMushroom1;
	assert(stateIdx <= kResScrb1115_EasterEggMushroom5 - kResScrb1111_EasterEggMushroom1);
	return _vm->_state->_f._bcOneMushroomColors[stateIdx];
}

ZmbEventHandleResult ZoombiniShelterBasecampOne::easterEggMushroom_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	(void)relPos;

	// Always hittable.
	if (!feature->isPointInClickRect(absPos))
		return ZmbEventHandleResult::kPassthrough;

	uint16 mushroomIdx = feature->getId() - kResScrb1111_EasterEggMushroom1;
	assert(mushroomIdx <= kResScrb1115_EasterEggMushroom5 - kResScrb1111_EasterEggMushroom1);

	uint16 colorCount = static_cast<uint16>(feature->getMaxFrameIdx() + 1);
	_vm->_state->_f._bcOneMushroomColors[mushroomIdx] = (_vm->_state->_f._bcOneMushroomColors[mushroomIdx] + 1) % colorCount;

	// Activate one pre-render pass to update @ref ZmbFeature::_lastFrameIdx.
	// The shape remains visible because the feature does not use @ref ZmbFeature::FLAG_01000000_DEFER_RENDER.
	feature->activateRender();

	int16 soundId = static_cast<int16>(mushroomIdx + kResSound1118_EasterEggMushroom1);
	_vm->_sound->playSound(ZmbResource(ZmbResource::kPage, soundId), Audio::Mixer::kSFXSoundType, false);

	return ZmbEventHandleResult::kConsumed;
}

ZmbStateActivePack &ZoombiniShelterBasecampOne::getBasecampResidentPack() {
	return _vm->_state->_f._zmbPackBC1;
}

int16 &ZoombiniShelterBasecampOne::getBasecampStoredPopulationCount() {
	return _vm->_state->_f._zmbStoredBC1Count;
}

int16 *ZoombiniShelterBasecampOne::getBasecampRuntimeStoredCount() {
	return nullptr;
}

void ZoombiniShelterBasecampOne::finalizeBasecampStorageForSave() {
	_storageMaxCellIdx -= compactBasecampStorageEntries(StorageOccupancyTest::kComplete);
	_storageMaxCellIdx = MAX<int16>(_storageMaxCellIdx, 0);
	calcStorageColumns();
}

ZmbEventHandleResult ZoombiniShelterBasecampOne::onBasecampPreLButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
	// Hidden decorations use fixed page zones because their dormant runners have no live click coverage.
	// Keep this dispatch independent of runner Z-order and ordinary feature hit testing.
	if (!isDragging() && !isDeparturePending() && !_goButtonPressPending &&
		!findSnoidAtPoint(absPos) && !_storageRect.contains(absPos)) {
		const Common::Rect *hiddenAnimRects[] = {
			&_easterEggStoneManRect,
			&_easterEggFishRect,
			&_easterEggBearRect,
			&_easterEggStoneFaceRect,
			&_easterEggHollowBugsRect};
		const int16 hiddenAnimScrbIds[] = {
			kResScrb1106_EasterEggStoneMan,
			kResScrb1108_EasterEggFish,
			kResScrb1109_EasterEggBear,
			kResScrb1110_EasterEggStoneFace,
			kResScrb1107_EasterEggHollowBugs};
		for (uint32 i = 0; i < ARRAYSIZE(hiddenAnimRects); i++) {
			if (!hiddenAnimRects[i]->contains(absPos))
				continue;
			ZmbFeature *feature = _scrbFeatures.find(hiddenAnimScrbIds[i]);
			if (feature)
				return genericEasterEgg_onLButtonDown(feature, absPos, relPos, *hiddenAnimRects[i]);
			break;
		}
	}

	return ZmbEventHandleResult::kPassthrough;
}

} // End of namespace Mohawk
