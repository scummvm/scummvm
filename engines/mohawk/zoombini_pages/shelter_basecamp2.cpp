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
#include "mohawk/zoombini_pages/shelter_basecamp2.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_sound.h"
#include "mohawk/zoombini_state.h"
#include "mohawk/zoombini_text.h"

namespace Mohawk {

constexpr Common::Point ZoombiniShelterBasecampTwo::_pedestalPoints[ZoombiniShelterBasecampTwo::kPedestalCount];
constexpr int16 ZoombiniShelterBasecampTwo::_storageMatrixX_anim[ZoombiniShelterBasecampTwo::kStorageAnimatedColumnCount];
constexpr int16 ZoombiniShelterBasecampTwo::_storageMatrixX_nonanim[ZoombiniShelterBasecampTwo::kStorageVisibleColumnCount];
constexpr int16 ZoombiniShelterBasecampTwo::_storageMatrixY_anim[ZoombiniShelterBasecampTwo::kStorageAnimatedColumnCount][ZoombiniShelterBasecampTwo::kStorageRowsPerColumn];
constexpr int16 ZoombiniShelterBasecampTwo::_storageMatrixY_nonanim[ZoombiniShelterBasecampTwo::kStorageVisibleColumnCount][ZoombiniShelterBasecampTwo::kStorageRowsPerColumn];

ZoombiniShelterBasecampTwo::ZoombiniShelterBasecampTwo(MohawkEngine_Zoombini *vm)
	: ZoombiniShelterBasecamp(vm, ZoombiniPageType::kBasecamp2,
							  Common::Rect(0x0037, 0x0013, 0x017F, 0x00FC), 0,
							  kResBitmapShape9000_Buttons, false, _pedestalPoints,
							  ZmbResource(ZmbResource::kPage, kResSound2000_StorageScrolling),
							  ZmbResource(ZmbResource::kPage, kResSound2001_StorageScrollEnd)) {
	memset(_buttonAnimRunnerIdxs, 0, sizeof(_buttonAnimRunnerIdxs));
}

ZoombiniShelterBasecampTwo::~ZoombiniShelterBasecampTwo() {
}

const ZoombiniPage::ScriptSoundPriorityRanges &ZoombiniShelterBasecampTwo::getScriptSoundPriorityRanges() const {
	static const ZoombiniPage::ScriptSoundPriorityRanges kRanges = {
		{kSysResSoundRange20000_Base, kSysResSoundRange29999_Last},
		{kResSoundRange2000_StorageScrolling, kResSoundRange2099_StorageRangeLast},
		{kResSoundRange6000_EasterEggRangeBase, kResSoundRange6099_EasterEggRangeLast}};
	return kRanges;
}

void ZoombiniShelterBasecampTwo::open() {
	// Basecamp visits always make the current game state eligible for saving.
	_vm->_state->markSaveBeforeQuitPending();
	openArchive(ZMB_MHK_BCTWO);
	loadNodePath(ZmbResource(ZmbResource::kPage, kResNode1000));
	// REGS 10000/10001 are a paired X/Y axis table for the cursor shapes in tBMP 10000:
	// one big-endian int16 per entry, reserved entry 0, then one entry per sub-image.
	loadShapeOffsetRegs(ZmbResource::kPage, kResRegs10000_Cursors);
	loadTerrainBitmap(kResBitmapTerrain100);
}

void ZoombiniShelterBasecampTwo::setBackgroundMusic() {
	// BC2 has no background music.
	// Ambient audio comes from SCRS feature animations (storage scroll and Easter eggs).
}

void ZoombiniShelterBasecampTwo::setBackgroundBitmap() {
	_vm->_gfx->setPalette(kResBackground5000);
	_vm->_gfx->drawBackground(kResBackground5000);
}

void ZoombiniShelterBasecampTwo::initStates() {
	ZmbStateFile &f = _vm->_state->_f;
	_storageLeftmostColumnIdx = f._storedChunkBC2.getLeftmostColumnIdx();
	// Trust complete trait records rather than the serialized header. The
	// original drag restore path could leave that header one or more entries low.
	synchronizeBasecampStoredCount(f._storedChunkBC2, &_storedCount);
	_storageLastOccupiedIdx = findLastOccupiedSlot();
	recalcStorageCapacity();
}

void ZoombiniShelterBasecampTwo::loadFeatures() {
	ZmbStateFile &f = _vm->_state->_f;

	// -- Preload shape bitmaps --
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Main));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape7000_Pedestal));
	_vm->_gfx->preloadImage(ZmbResource(ZmbResource::kPage, kResBitmapShape8000_Storage));
	_vm->_gfx->preloadImage(ZmbResource(ZmbResource::kPage, kResBitmapShape9000_Buttons));

	// Register the complete SCRB image groups without creating hidden runners.
	registerScrbImageGroup(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Main), kResScrb6000_EasterEggBonfireSticks, 14);
	registerScrbImageGroup(ZmbResource(ZmbResource::kPage, kResBitmapShape7000_Pedestal), kResScrb7000_Pedestal, kPedestalCount);

	// Configure all navigation controls before registering their shared runner.
	setGoButton(_goButtonClickRect, kShape9000_GoDisabled_15, kShape9000_GoEnabled_01, kShape9000_GoPressed_02);
	setMapButton(_mapButtonClickRect, kShape9000_MapNormal_05, kShape9000_MapPressed_06);
	setHelpButton(_helpButtonClickRect);
	buildEmbeddedControlHotspots(kResBitmapShape9000_Buttons, _controlGoMapBaseHotspots, _controlHelpBaseHotspots, true);

	{ // [*] Virtual Feature: Storage area (no SCRB; preRender=scroll SM, postRender=draw grid)
		ZmbFeature::EventHooks hooks;
		hooks.setPreRenderFunc(static_cast<ZmbFeature::OnPreRenderFunc>(&ZoombiniShelterBasecampTwo::storage_preRender));
		hooks.setPostRenderFunc(static_cast<ZmbFeature::OnPostRenderFunc>(&ZoombiniShelterBasecampTwo::storage_postRender));
		ZmbFeature *vf = loadScrbFeature(ZmbResource(ZmbResource::kPage, 0), 0, 6,
										 ZmbFeature::FLAG_00004000_NO_DIRTY_MERGE | ZmbFeature::FLAG_00008000_LOOP_ANIM,
										 hooks);
		vf->setClickRect(_storageRect);
		_storageFeature = vf;
	}

	{ // [*] Virtual Feature: Scroll-button panel (postRender draws scroll arrows via SHPL 9000)
		ZmbFeature::EventHooks hooks;
		hooks.setPostRenderFunc(static_cast<ZmbFeature::OnPostRenderFunc>(&ZoombiniShelterBasecampTwo::buttons_postRender));
		hooks.setLButtonDownFunc(static_cast<ZmbFeature::OnLButtonDownFunc>(&ZoombiniShelterBasecampTwo::scrollButton_onLButtonDown));
		_scrollButtonFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, 0), 0, 0,
											   ZmbFeature::FLAG_00001000_TOPMOST | ZmbFeature::FLAG_00008000_LOOP_ANIM,
											   hooks);
	}

	{ // [*] One callback-only runner for Go, Map, and Help.
		Common::Rect clickRect = _goButtonClickRect;
		clickRect.extend(_mapButtonClickRect);
		if (_vm->supportsOnScreenHelp())
			clickRect.extend(_helpButtonClickRect);
		ZmbFeature *feature = loadBasecampControlFeature(clickRect);
		bindEmbeddedControlFeature(feature);
	}

	// [*] SCRB 7000 ~ 7015: Pedestals (16 spots for the active Zoombini pack)
	loadBasecampPedestalFeatures(ZmbResource(ZmbResource::kPage, kResBitmapShape7000_Pedestal), kResScrb7000_Pedestal);

	// [*] SCRB 6000: Easter egg bonfire sticks (looping; fires once when armed)
	ZmbFeature *bonfireFeature = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Main),
		kResScrb6000_EasterEggBonfireSticks, 6,
		ZmbFeature::FLAG_00008000_LOOP_ANIM |
			ZmbFeature::FLAG_00080000_DEFER_ANIM |
			ZmbFeature::FLAG_00100000_PLAY_ONCE |
			ZmbFeature::FLAG_01000000_DEFER_RENDER |
			ZmbFeature::FLAG_04000000_OVERLAY);
	// The bonfire sequence owns a single-winner series of embedded SFX.
	bonfireFeature->setScriptSoundPolicy(ZmbFeature::ScriptSoundPolicy::kPriorityQueue);
	_bonfireAnimRunnerIdx = kResScrb6000_EasterEggBonfireSticks;

	// [*] SCRB 6005: Easter egg hand-like branch
	loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Main),
					kResScrb6005_EasterEggHandBranch, 6,
					ZmbFeature::FLAG_00008000_LOOP_ANIM |
						ZmbFeature::FLAG_00080000_DEFER_ANIM |
						ZmbFeature::FLAG_00100000_PLAY_ONCE);
	_buttonAnimRunnerIdxs[kEasterEggButton01_HandBranch] = kResScrb6005_EasterEggHandBranch;

	// [*] SCRB 6011: Easter egg odd tree leaves and acorns
	loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Main),
					kResScrb6011_EasterEggOddTreeLeavesAndAcorns, 6,
					ZmbFeature::FLAG_00008000_LOOP_ANIM |
						ZmbFeature::FLAG_00080000_DEFER_ANIM |
						ZmbFeature::FLAG_00100000_PLAY_ONCE);
	_buttonAnimRunnerIdxs[kEasterEggButton02_OddTree] = kResScrb6011_EasterEggOddTreeLeavesAndAcorns;

	// [*] SCRB 6010: Easter egg pipe organ
	loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Main),
					kResScrb6010_EasterEggPipeOrgan, 6,
					ZmbFeature::FLAG_00008000_LOOP_ANIM |
						ZmbFeature::FLAG_00080000_DEFER_ANIM |
						ZmbFeature::FLAG_00100000_PLAY_ONCE |
						ZmbFeature::FLAG_01000000_DEFER_RENDER);
	_buttonAnimRunnerIdxs[kEasterEggButton03_PipeOrgan] = kResScrb6010_EasterEggPipeOrgan;

	// [*] SCRB 6002: Easter egg crawling log (round-trip toggle, used with 6003)
	loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Main),
					kResScrb6002_EasterEggCrawlingLogAway, 6,
					ZmbFeature::FLAG_00008000_LOOP_ANIM |
						ZmbFeature::FLAG_00080000_DEFER_ANIM |
						ZmbFeature::FLAG_00100000_PLAY_ONCE |
						ZmbFeature::FLAG_04000000_OVERLAY);
	_buttonAnimRunnerIdxs[kEasterEggButton04_CrawlingLog] = kResScrb6002_EasterEggCrawlingLogAway;

	// [*] SCRB 6004: Easter egg geyser rock
	loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Main),
					kResScrb6004_EasterEggGeyserRock, 6,
					ZmbFeature::FLAG_00080000_DEFER_ANIM |
						ZmbFeature::FLAG_00100000_PLAY_ONCE |
						ZmbFeature::FLAG_04000000_OVERLAY);
	_buttonAnimRunnerIdxs[kEasterEggButton05_GeyserRock] = kResScrb6004_EasterEggGeyserRock;

	// [*] SCRB 6009: Easter egg thunderstorm
	loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Main),
					kResScrb6009_EasterEggThunderstorm, 6,
					ZmbFeature::FLAG_00008000_LOOP_ANIM |
						ZmbFeature::FLAG_00080000_DEFER_ANIM |
						ZmbFeature::FLAG_00100000_PLAY_ONCE |
						ZmbFeature::FLAG_01000000_DEFER_RENDER);
	_buttonAnimRunnerIdxs[kEasterEggButton06_Thunderstorm] = kResScrb6009_EasterEggThunderstorm;

	// [*] SCRB 6006: Easter egg top step
	loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Main),
					kResScrb6006_EasterEggTopStep, 6,
					ZmbFeature::FLAG_00008000_LOOP_ANIM |
						ZmbFeature::FLAG_00080000_DEFER_ANIM |
						ZmbFeature::FLAG_00100000_PLAY_ONCE |
						ZmbFeature::FLAG_01000000_DEFER_RENDER |
						ZmbFeature::FLAG_04000000_OVERLAY);
	_buttonAnimRunnerIdxs[kEasterEggButton07_TopStep] = kResScrb6006_EasterEggTopStep;

	// [*] SCRB 6007: Easter egg middle step
	loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Main),
					kResScrb6007_EasterEggMiddleStep, 6,
					ZmbFeature::FLAG_00008000_LOOP_ANIM |
						ZmbFeature::FLAG_00080000_DEFER_ANIM |
						ZmbFeature::FLAG_00100000_PLAY_ONCE |
						ZmbFeature::FLAG_01000000_DEFER_RENDER |
						ZmbFeature::FLAG_04000000_OVERLAY);
	_buttonAnimRunnerIdxs[kEasterEggButton08_MiddleStep] = kResScrb6007_EasterEggMiddleStep;

	// [*] SCRB 6008: Easter egg bottom step
	loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Main),
					kResScrb6008_EasterEggBottomStep, 6,
					ZmbFeature::FLAG_00008000_LOOP_ANIM |
						ZmbFeature::FLAG_00080000_DEFER_ANIM |
						ZmbFeature::FLAG_00100000_PLAY_ONCE |
						ZmbFeature::FLAG_01000000_DEFER_RENDER |
						ZmbFeature::FLAG_04000000_OVERLAY);
	_buttonAnimRunnerIdxs[kEasterEggButton09_BottomStep] = kResScrb6008_EasterEggBottomStep;

	// The bonfire button uses the runner in @ref ZoombiniShelterBasecampTwo::_bonfireAnimRunnerIdx.
	_buttonAnimRunnerIdxs[kEasterEggButton10_BonfireSticks] = _bonfireAnimRunnerIdx;

	// -- Zoombini Pack Management (mirrors BC1 two-phase loading) --

	// Load the incoming pack, then replace it with the BC2 resident snapshot.
	const int16 arrivingCount = beginBasecampPackLoad();

	// Phase 3: Handle arriving occupied zoombinis -> store to belt
	if (0 < arrivingCount && !f._zmbPackActive.getSkipOccupiedEntries()) {
		int16 occupiedInActive = 0;
		for (int16 i = 0; i < f._zmbPackActive.getPackZmbCount(); i++) {
			if (f._zmbPackActive.getEntry(i).getIsOccupied())
				occupiedInActive += 1;
		}
		if (0 < occupiedInActive) {
			int16 prevLastIdx = _storageLastOccupiedIdx;
			int16 wrapped = storeBasecampActivePackInStorage(StorageOccupancyTest::kHairOrEyes);
			synchronizeBasecampStoredCount(f._storedChunkBC2, &_storedCount);
			_storageLastOccupiedIdx = findLastOccupiedSlot();
			recalcStorageCapacity();
			if (wrapped) {
				_storageLeftmostColumnIdx = static_cast<int16>((prevLastIdx + 1) / kStorageRowsPerColumn % _storageColumnCount);
				recalcStorageCapacity();
			}
			f._zmbPackActive.setSkipOccupiedEntries(true);
		}
	}

	completeBasecampPackLoad(arrivingCount);

	// Play arrival voice line based on difficulty
	playArrivalVoice();

	// Persist leftmost column index back to state
	f._storedChunkBC2.setLeftmostColumnIdx(_storageLeftmostColumnIdx);
	f._storedChunkBC2.setStoredCount(_storedCount);
}

void ZoombiniShelterBasecampTwo::onPostRenderFrame() {
	if (!_bonfireButtonArmed)
		return;

	ZmbFeature *bonfireRunner = _scrbFeatures.find(_bonfireAnimRunnerIdx);
	if (!bonfireRunner || bonfireRunner->isRenderActivated())
		return;

	loadScrbOntoFeature(bonfireRunner, kResScrb6001_EasterEggBonfireFlames);
	bonfireRunner->setFlags(ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM);
}

bool ZoombiniShelterBasecampTwo::storage_preRender(ZmbFeature *feature) {
	if (_currentFrameCounter < _storageNextUpdateFrame)
		return true;

	_storageNextUpdateFrame = _currentFrameCounter + kStorageScrollInterval;
	updateHeldBasecampStorageScroll();
	if (_storageScrollDirection == StorageScrollDirection::kNone)
		return true;

	// Mark as rendering (reset sort rect to force re-sort)
	feature->setSortRect(Common::Rect());
	advanceBasecampStorageScroll();

	_vm->_state->_f._storedChunkBC2.setLeftmostColumnIdx(_storageLeftmostColumnIdx);
	return true;
}

void ZoombiniShelterBasecampTwo::storage_postRender(ZmbFeature *feature) {
	(void)feature;

	// Z-order: 1) honeycomb, 2) snoids, 3) lattice, 4) border
	ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;
	ZmbStateStoredChunk &chunk = _vm->_state->_f._storedChunkBC2;

	// Choose shapes based on scroll animation state
	uint16 honeycombShape, latticeShape;
	int16 latticeOffX, latticeOffY;
	if (_storageScrollAnimating) {
		honeycombShape = kShape8000_StorageAnim_Honeycomb;
		latticeShape = kShape8000_StorageAnim_Lattice;
		latticeOffX = -1 + 141; // 140
		latticeOffY = -3 + 28;  // 25
	} else {
		honeycombShape = kShape8000_StorageStill_Honeycomb;
		latticeShape = kShape8000_StorageStill_Lattice;
		latticeOffX = 141;
		latticeOffY = 28;
	}

	// Resource 8000 stores each shape N as a separate tBMP at resource 8000 + N - 1.
	// The @ref ZoombiniGraphics::drawShape() path uses @ref ZoombiniGraphics::decodeImages(),
	// which cannot decode these single-image resources.
	// Use @ref ZoombiniGraphics::drawBackground() instead.

	// 1) Draw honeycomb (background of storage belt)
	_vm->_gfx->drawImage(screenKind, kResBitmapShape8000_Storage + honeycombShape - 1, Common::Point(140, 23));

	// 2) Draw stored Zoombinis in grid (on top of honeycomb, below lattice)
	renderBasecampStorageEntries(chunk, _storageMatrixX_anim, _storageMatrixY_anim, _storageMatrixX_nonanim, _storageMatrixY_nonanim, StorageOccupancyTest::kHairOrEyes);

	// 3) Draw lattice (overlay on top of snoids)
	_vm->_gfx->drawImage(screenKind, kResBitmapShape8000_Storage + latticeShape - 1, Common::Point(latticeOffX, latticeOffY));

	// 4) Draw border (outermost frame)
	_vm->_gfx->drawImage(screenKind, kResBitmapShape8000_Storage + kShape8000_StorageBorder - 1, Common::Point(101, 0));
}

void ZoombiniShelterBasecampTwo::buttons_postRender(ZmbFeature *feature) {
	(void)feature;

	updateHeldBasecampStorageScroll();
	renderScrollButtons();
}

ZmbEventHandleResult ZoombiniShelterBasecampTwo::scrollButton_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	(void)feature;
	(void)relPos;

	for (int i = 0; i < kStorageScrollButtonCount; i++) {
		if (_scrollButtonRects[i].contains(absPos)) {
			beginBasecampStorageScroll(getBasecampStorageScrollDirection(i));
			return ZmbEventHandleResult::kConsumed;
		}
	}

	return ZmbEventHandleResult::kPassthrough;
}

bool ZoombiniShelterBasecampTwo::updateScrollButtonHover(const Common::Point &absPos) {
	// Z1-20U/TLC v2.0 release only: BC2 storage scroll buttons use
	// yellow-outline hover shapes.
	if (!_vm->isVersionFamilyTlcV2())
		return false;

	bool changed = false;
	for (uint i = 0; i < ARRAYSIZE(_scrollButtonHovered); i++) {
		bool hovered = _scrollButtonRects[i].contains(absPos);
		if (_scrollButtonHovered[i] == hovered)
			continue;

		_scrollButtonHovered[i] = hovered;
		changed = true;
	}

	if (!changed)
		return false;

	Common::Rect dirtyRect = _scrollButtonRects[0];
	for (uint i = 1; i < ARRAYSIZE(_scrollButtonHovered); i++)
		dirtyRect.extend(_scrollButtonRects[i]);

	addExternalDirtyRect(dirtyRect);
	if (_scrollButtonFeature)
		_scrollButtonFeature->setNeedsRedraw(true);
	return true;
}

void ZoombiniShelterBasecampTwo::updateScrollButtonCursor(const Common::Point &absPos) {
	uint16 cursorShapeIdx = ZmbHotspot::kShapeNone;
	for (uint buttonIdx = 0; buttonIdx < ARRAYSIZE(_scrollButtonRects); buttonIdx++) {
		if (_scrollButtonRects[buttonIdx].contains(absPos)) {
			cursorShapeIdx = kShape10000_ArrowLeftMax_01 + buttonIdx;
			break;
		}
	}

	if (cursorShapeIdx == _storageButtonCursorShapeIdx)
		return;

	if (cursorShapeIdx == ZmbHotspot::kShapeNone) {
		_vm->_cursor->setDefaultCursor();
	} else {
		Common::HashMap<int16, ZmbShapeOffsetRegs *>::iterator regsIt = _shapeOffsetRegsMap.find(kResRegs10000_Cursors);
		if (regsIt == _shapeOffsetRegsMap.end()) {
			error("basecamp2: required cursor REGS table is unavailable");
			return;
		}
		ZoombiniCursorManager *zmbCursor = dynamic_cast<ZoombiniCursorManager *>(_vm->_cursor);
		if (!zmbCursor) {
			error("basecamp2: cursor manager for Zoombini is unavailable");
			return;
		}
		zmbCursor->setShapeCursor(ZmbResource::kPage, kResBitmapShape10000_Cursors, cursorShapeIdx, regsIt->_value->getShapeDelta(cursorShapeIdx));
	}
	_storageButtonCursorShapeIdx = cursorShapeIdx;
}

bool ZoombiniShelterBasecampTwo::releaseHeldScrollButton() {
	if (_heldStorageScrollDirection == StorageScrollDirection::kNone)
		return false;

	releaseBasecampStorageScroll();

	Common::Rect dirtyRect = _scrollButtonRects[0];
	for (uint i = 1; i < ARRAYSIZE(_scrollButtonRects); i++)
		dirtyRect.extend(_scrollButtonRects[i]);

	addExternalDirtyRect(dirtyRect);
	if (_scrollButtonFeature)
		_scrollButtonFeature->setNeedsRedraw(true);

	return true;
}

// ---------------------------------------------------------------------------
// Page-level mouse handlers -- Snoid drag/drop
// Mouse-down begins a pickup; the matching mouse-up resolves the drop.
// ---------------------------------------------------------------------------

ZmbEventHandleResult ZoombiniShelterBasecampTwo::onBasecampNoSnoidLButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
	(void)relPos;
	if (updateButtonAnimations(absPos))
		return ZmbEventHandleResult::kConsumed;
	return ZmbEventHandleResult::kPassthrough;
}

ZmbEventHandleResult ZoombiniShelterBasecampTwo::onLButtonUp(const Common::Point &absPos, const Common::Point &relPos) {
	// Handle scroll button release
	if (releaseHeldScrollButton())
		return ZmbEventHandleResult::kConsumed;

	return ZoombiniInteractive::onLButtonUp(absPos, relPos);
}

ZmbEventHandleResult ZoombiniShelterBasecampTwo::onMouseMove(const Common::Point &absPos, const Common::Point &relPos) {
	if (_heldStorageScrollDirection != StorageScrollDirection::kNone) {
		const int16 scrollIdx = getBasecampStorageScrollButtonIndex(_heldStorageScrollDirection);
		bool heldButtonOutside = true;
		if (0 <= scrollIdx && static_cast<uint>(scrollIdx) < ARRAYSIZE(_scrollButtonRects))
			heldButtonOutside = !_scrollButtonRects[scrollIdx].contains(absPos);

		// Release an out-of-bounds held scroll button before the storage click handler clears its state.
		if (heldButtonOutside)
			releaseHeldScrollButton();
	}

	updateScrollButtonHover(isDragging() ? Common::Point(-1, -1) : absPos);
	updateScrollButtonCursor(isDragging() ? Common::Point(-1, -1) : absPos);

	// Delegate to parent for standard drag handling, including common pedestal highlighting.
	return ZoombiniInteractive::onMouseMove(absPos, relPos);
}

bool ZoombiniShelterBasecampTwo::updateButtonAnimations(const Common::Point &cursorPos) {
	// Check if cursor is over one of the decorative button hotspots
	// and trigger the corresponding animation if it's not already playing.

	for (uint16 buttonIdx = kEasterEggButton01_HandBranch; buttonIdx < kEasterEggButtonCount; buttonIdx++) {
		if (!_buttonHotspotRects[buttonIdx].contains(cursorPos))
			continue;

		// Get the runner for this button animation
		if (kEasterEggButtonCount <= buttonIdx || _buttonAnimRunnerIdxs[buttonIdx] == 0)
			continue;

		ZmbFeature *runner = _scrbFeatures.find(_buttonAnimRunnerIdxs[buttonIdx]);
		if (!runner)
			continue;

		// Skip if animation is already playing
		if (runner->isRenderActivated())
			continue;

		// Handle special button behaviors
		switch (buttonIdx) {
		case kEasterEggButton02_OddTree:
			// Cycles the odd-tree Easter egg: leaves/acorns -> bat -> woodpecker.
			{
				int16 curResId = runner->getScrbId();
				int16 nextResId;
				if (curResId == kResScrb6013_EasterEggOddTreeWoodpecker)
					nextResId = kResScrb6011_EasterEggOddTreeLeavesAndAcorns;
				else if (kResScrb6011_EasterEggOddTreeLeavesAndAcorns <= curResId && curResId < kResScrb6013_EasterEggOddTreeWoodpecker)
					nextResId = curResId + 1;
				else
					nextResId = kResScrb6011_EasterEggOddTreeLeavesAndAcorns;
				loadScrbOntoFeature(runner, nextResId);
			}
			break;
		case kEasterEggButton04_CrawlingLog:
			// Round-trip toggle for the crawling log.
			if (_roundTripToggle) {
				_roundTripToggle = false;
				loadScrbOntoFeature(runner, kResScrb6002_EasterEggCrawlingLogAway);
			} else {
				_roundTripToggle = true;
				loadScrbOntoFeature(runner, kResScrb6003_EasterEggCrawlingLogReturn);
			}
			break;
		case kEasterEggButton10_BonfireSticks:
			// Bonfire trigger: loads SCRB 0 and sets the armed flag.
			if (!_bonfireButtonArmed) {
				loadScrbOntoFeature(runner, 0);
				_bonfireButtonArmed = true;
			}
			break;
		default:
			// Other buttons: just activate the animation
			loadScrbOntoFeature(runner, 0);
			break;
		}

		// Loading schedules rendering, so dispatch only the initial frame sound here.
		// The bonfire button intentionally does not restart once armed.
		const int32 frameIdx = runner->getLastFrameIdx();
		ZmbResource soundResource;
		if (runner->getFrameSoundResource(frameIdx, soundResource)) {
			dispatchFeatureSound(runner, soundResource);
			runner->setLastSoundedFrameIdx(frameIdx);
		}
		return true; // Animation triggered
	}

	return false; // No animation triggered
}

void ZoombiniShelterBasecampTwo::playArrivalVoice() {
	// Select the arrival voice from the transient SFX group
	// when @ref ZoombiniGameState::_lastPageBeforeContainer identifies a puzzle.
	ZmbStateFile &f = _vm->_state->_f;

	int16 toPlaySoundId = 0;
	ZmbSfxGroupFlags sfxGroupFlags = ZmbSfxGroupFlags::kRandom_00;
	bool hasSfxGroupFlags = false;

	if (_vm->_state->_lastPageBeforeContainer != 0) {
		sfxGroupFlags = _vm->_state->getSfxGroupFlagsFromPageFlag(f._pageFlagBasecamp2);
		_vm->_state->_lastPageBeforeContainer = 0;
		hasSfxGroupFlags = true;
	}

	// Use Group 1 when Caves was not visited and only a small group is stored.
	if (sfxGroupFlags == ZmbSfxGroupFlags::kHardFirstTrigger_02 && f._pageFlagCaves.isZero() && f._zmbStoredBC2Count <= kPedestalCount) {
		sfxGroupFlags = ZmbSfxGroupFlags::kEasy_01;
		f._pageFlagBasecamp2.clearHardGroupTriggers();
	}

	// Expand the random choice range after a dedicated Group 2 visit marker has been set.
	const int16 voiceRandMax = f._pageFlagBasecamp2.hasHardGroupTrigger() ? 4 : 3;

	if (hasSfxGroupFlags) {
		switch (sfxGroupFlags) {
		case ZmbSfxGroupFlags::kRandom_00: {
			// Random from voiceRandMax voices
			const int16 r = _vm->_rnd->getRandomNumber(1, static_cast<uint16>(voiceRandMax));
			switch (r) {
			case 1:
				toPlaySoundId = kSysResSound20084_BC2Voice2;
				break;
			case 2:
				toPlaySoundId = kSysResSound20085_BC2Voice3;
				break;
			case 3:
				toPlaySoundId = kSysResSound20082_BC2Voice1;
				break;
			case 4:
				toPlaySoundId = kSysResSound20083_BC2Voice4;
				break;
			default:
				break;
			}
			break;
		}
		case ZmbSfxGroupFlags::kEasy_01:
		case ZmbSfxGroupFlags::kPractice_05:
			toPlaySoundId = kSysResSound20082_BC2Voice1;
			break;
		case ZmbSfxGroupFlags::kHardFirstTrigger_02:
		case ZmbSfxGroupFlags::kHardSecondTrigger_12:
			toPlaySoundId = kSysResSound20083_BC2Voice4;
			break;
		default:
			break;
		}
	}

	if (toPlaySoundId)
		_vm->_sound->playSound(ZmbResource(ZmbResource::kSystem, toPlaySoundId), Audio::Mixer::kSpeechSoundType, false);
}

ZmbSrcPageKind ZoombiniShelterBasecampTwo::getBasecampDepartureSourcePage() const {
	return ZmbSrcPageKind::kBC2_13;
}

void ZoombiniShelterBasecampTwo::onDisabledGoButtonActivated() {
	int16 soundId = 0;
	switch (_vm->_rnd->getRandomNumber(1, 3)) {
	case 1:
		soundId = kSysResSound20084_BC2Voice2;
		break;
	case 2:
		soundId = kSysResSound20085_BC2Voice3;
		break;
	case 3:
		soundId = kSysResSound20082_BC2Voice1;
		break;
	default:
		break;
	}

	if (soundId != 0)
		_vm->_sound->playSound(ZmbResource(ZmbResource::kSystem, soundId), Audio::Mixer::kSpeechSoundType, false);
}

void ZoombiniShelterBasecampTwo::renderScrollButtons() {
	static constexpr Common::Point kButtonPos[kStorageScrollButtonCount] = {
		Common::Point(0x0072, 0x0079),
		Common::Point(0x0083, 0x0075),
		Common::Point(0x0151, 0x006B),
		Common::Point(0x015E, 0x006E),
	};

	for (int scrollIdx = 0; scrollIdx < kStorageScrollButtonCount; scrollIdx++) {
		uint16 shapeIdx;
		if (_vm->isVersionFamilyTlcV2() && _scrollButtonHovered[scrollIdx])
			shapeIdx = static_cast<uint16>(kShape9000_ScrollLMaxHover_20 + scrollIdx);
		else if (_heldStorageScrollDirection == getBasecampStorageScrollDirection(scrollIdx))
			shapeIdx = static_cast<uint16>(kShape9000_ScrollLMaxPressed_08 + 2 * scrollIdx);
		else
			shapeIdx = static_cast<uint16>(kShape9000_ScrollLMaxNormal_07 + 2 * scrollIdx);

		_vm->_gfx->drawImage(ZoombiniGraphics::kShapeScreen, kResBitmapShape9000_Buttons + shapeIdx - 1, kButtonPos[scrollIdx]);
	}
}

int16 ZoombiniShelterBasecampTwo::findLastOccupiedSlot() {
	const ZmbStateStoredChunk &chunk = _vm->_state->_f._storedChunkBC2;
	const int16 lastOccupiedSlot = findLastBasecampStorageEntry(chunk, StorageOccupancyTest::kHairOrEyes);
	return lastOccupiedSlot < 0 ? 0 : lastOccupiedSlot;
}

void ZoombiniShelterBasecampTwo::recalcStorageCapacity() {
	recalculateBasecampStorageCapacity(_storageLastOccupiedIdx, _storedCount);
}

bool ZoombiniShelterBasecampTwo::expandBasecampStorageAtLeftBoundary() {
	ZmbStateStoredChunk &chunk = _vm->_state->_f._storedChunkBC2;
	if (!expandBasecampStorageEntriesAtLeftBoundary(chunk, StorageOccupancyTest::kHairOrEyes))
		return false;

	_storageLastOccupiedIdx += kStorageRowsPerColumn;
	recalcStorageCapacity();
	_storageLeftmostColumnIdx += 1;
	chunk.setLeftmostColumnIdx(_storageLeftmostColumnIdx);
	return true;
}

void ZoombiniShelterBasecampTwo::compactStorage() {
	_storageLastOccupiedIdx -= compactBasecampStorageEntries(StorageOccupancyTest::kHairOrEyes);
	_storageLastOccupiedIdx = MAX<int16>(_storageLastOccupiedIdx, 0);
}

void ZoombiniShelterBasecampTwo::resetStorageSortRect() {
	// Reset the storage feature's sort rect.
	ZmbFeature *storage = _storageFeature;
	if (storage)
		storage->setSortRect(Common::Rect());
}

int16 ZoombiniShelterBasecampTwo::findStorageSlotIndex(bool searchOccupied, const Common::Rect &clickRect, int16 leftmostColumnIdx) {
	const ZmbStateStoredChunk &chunk = _vm->_state->_f._storedChunkBC2;
	return findBasecampStorageSlotIndex(chunk, searchOccupied, clickRect, leftmostColumnIdx,
										_storageColumnCount, _storageCapacity,
										_storageMatrixX_nonanim, _storageMatrixY_nonanim,
										StorageOccupancyTest::kHairOrEyes);
}

ZmbStateStoredChunk &ZoombiniShelterBasecampTwo::getBasecampStorageChunk() {
	return _vm->_state->_f._storedChunkBC2;
}

void ZoombiniShelterBasecampTwo::refreshBasecampStorageState() {
	ZmbStateStoredChunk &chunk = getBasecampStorageChunk();
	synchronizeBasecampStoredCount(chunk, &_storedCount);
	_storageLastOccupiedIdx = findLastOccupiedSlot();
	recalcStorageCapacity();
}

int16 ZoombiniShelterBasecampTwo::getBasecampAvailableSnoidCount() const {
	const ZmbStateFile &f = _vm->_state->_f;
	return f._zmbPackIsle.getPackZmbCount() +
		   f._zmbStoredBC1Count + f._zmbStoredBC2Count;
}

ZmbStateActivePack &ZoombiniShelterBasecampTwo::getBasecampResidentPack() {
	return _vm->_state->_f._zmbPackBC2;
}

int16 &ZoombiniShelterBasecampTwo::getBasecampStoredPopulationCount() {
	return _vm->_state->_f._zmbStoredBC2Count;
}

int16 *ZoombiniShelterBasecampTwo::getBasecampRuntimeStoredCount() {
	return &_storedCount;
}

void ZoombiniShelterBasecampTwo::finalizeBasecampStorageForSave() {
	compactStorage();
	recalcStorageCapacity();
}

} // End of namespace Mohawk
