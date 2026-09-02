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

#include "mohawk/zoombini_pages/shelter_base.h"

#include "mohawk/zoombini.h"
#include "mohawk/zoombini_graphics.h"
#include "mohawk/zoombini_sound.h"
#include "mohawk/zoombini_state.h"

namespace Mohawk {

ZoombiniShelter::ZoombiniShelter(MohawkEngine_Zoombini *vm, ZoombiniPageType pageType)
	: ZoombiniInteractive(vm, pageType) {
}

ZoombiniShelter::~ZoombiniShelter() {
}

Common::String ZoombiniShelterBasecamp::debugGetPageCommandHelp() const {
	Common::String output;
	output += "  ";
	output += kPageCommandAutoseat;
	output += "\n";
	output += "      Fill empty Basecamp pedestal seats from the storage area.\n";
	output += "  ";
	output += kPageCommandAutounseat;
	output += "\n";
	output += "      Return seated pedestal Zoombinis to the storage area.\n";
	return output;
}

bool ZoombiniShelterBasecamp::debugDoPageCommand(int argc, const char **argv, Common::String &output) {
	const int16 changedCount = debugRunPedestalCommand(argc, argv, getBasecampStorageChunk(), getBasecampRuntimeStoredCount(), output);
	if (0 <= changedCount) {
		refreshBasecampStorageState();
		updateBasecampGoButtonState(getBasecampFieldSnoidCount(), getBasecampAvailableSnoidCount());
		if (0 < changedCount)
			_vm->_state->markDebugStateMutation();
	}
	return changedCount < 0;
}

ZoombiniShelterBasecamp::ZoombiniShelterBasecamp(MohawkEngine_Zoombini *vm,
												 ZoombiniPageType pageType,
												 const Common::Rect &storageRect,
												 int16 initialStorageCapacity,
												 int16 controlBitmapBaseId,
												 bool controlBitmapIsCompoundShapes,
												 const Common::Point *pedestalPoints,
												 ZmbResource storageScrollingSound,
												 ZmbResource storageScrollEndSound)
	: ZoombiniShelter(vm, pageType),
	  _storageRect(storageRect),
	  _controlBitmapBaseId(controlBitmapBaseId),
	  _controlBitmapIsCompoundShapes(controlBitmapIsCompoundShapes),
	  _pedestalPoints(pedestalPoints),
	  _storageColumnCount(initialStorageCapacity / kStorageRowsPerColumn),
	  _storageCapacity(initialStorageCapacity),
	  _storageScrollingSound(storageScrollingSound),
	  _storageScrollEndSound(storageScrollEndSound) {
	static_assert(kStorageEntryCount == ZmbTrait::SNOID_MAX, "Basecamp storage must cover every possible Zoombini");
}

ZoombiniShelterBasecamp::~ZoombiniShelterBasecamp() {
}

ZmbEventHandleResult ZoombiniShelterBasecamp::onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
	ZmbEventHandleResult result = onBasecampPreLButtonDown(absPos, relPos);
	if (result == ZmbEventHandleResult::kConsumed)
		return result;

	result = ZoombiniInteractive::onLButtonDown(absPos, relPos);
	if (result == ZmbEventHandleResult::kConsumed)
		return result;

	if (isDragging() || isDeparturePending())
		return ZmbEventHandleResult::kPassthrough;

	ZmbSnoid *snoid = findSnoidAtPoint(absPos);

	// Walk-in destinations are not registered as occupied until their Snoids
	// arrive. Keep field and storage pickup disabled while those destinations
	// are pending, without blocking page-local background interactions.
	if (0 < _vm->_walkersInProgress) {
		if (snoid || _storageRect.contains(absPos))
			return ZmbEventHandleResult::kPassthrough;
		return onBasecampNoSnoidLButtonDown(absPos, relPos);
	}

	if (!snoid)
		snoid = loadBasecampStorageDragSnoid(absPos);
	if (!snoid)
		return onBasecampNoSnoidLButtonDown(absPos, relPos);

	if (!isBasecampSnoidDraggable(*snoid))
		return ZmbEventHandleResult::kPassthrough;

	startSnoidDrag(snoid, absPos);
	return ZmbEventHandleResult::kConsumed;
}

ZmbEventHandleResult ZoombiniShelterBasecamp::onBasecampPreLButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
	(void)absPos;
	(void)relPos;
	return ZmbEventHandleResult::kPassthrough;
}

ZmbEventHandleResult ZoombiniShelterBasecamp::onBasecampNoSnoidLButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
	(void)absPos;
	(void)relPos;
	return ZmbEventHandleResult::kPassthrough;
}

void ZoombiniShelterBasecamp::clearHoverCursor() {
	_storageButtonCursorShapeIdx = ZmbHotspot::kShapeNone;
}

void ZoombiniShelterBasecamp::saveStateBeforeMapTransition() {
	saveSnoidsToPack();
	saveBasecampPackState(false);
}

void ZoombiniShelterBasecamp::onGoButtonActivated() {
	startBasecampDeparture(Common::Point(680, 316));
	_departureState = ZmbDepartureState::kAnimating;
}

void ZoombiniShelterBasecamp::startBasecampDeparture(const Common::Point &target) {
	playDepartSfx();
	startDepartWalkAnimation(target);
}

void ZoombiniShelterBasecamp::executeDeparture() {
	saveSnoidsToPack();
	saveBasecampPackState(true);

	_vm->_xferSrcPage = getBasecampDepartureSourcePage();
	_vm->setNextPage(ZoombiniPageType::kXfer);
	close();
}

void ZoombiniShelterBasecamp::saveBasecampPackState(bool isDeparture) {
	ZmbStateFile &f = _vm->_state->_f;
	const int16 departingCount = splitActivePackForBasecamp(f._zmbPackActive, getBasecampResidentPack(), isDeparture);
	getBasecampStoredPopulationCount() -= departingCount;
	finalizeBasecampStorageForSave();
}

int16 ZoombiniShelterBasecamp::beginBasecampPackLoad() {
	ZmbStateFile &f = _vm->_state->_f;
	int16 arrivingCount = 0;
	if (0 < f._zmbPackActive.getPackZmbCount()) {
		arrivingCount = loadDynamicSnoidsFromPack(f._zmbPackActive, _pedestalPoints, kPedestalCount, false, nullptr);
	}

	getBasecampStoredPopulationCount() += arrivingCount;
	ZmbStateActivePack &residentPack = getBasecampResidentPack();
	residentPack.copyTo(f._zmbPackActive);
	residentPack.clearEntries();
	residentPack.setSkipOccupiedEntries(true);
	residentPack.setSkipUnoccupiedEntries(true);
	return arrivingCount;
}

void ZoombiniShelterBasecamp::completeBasecampPackLoad(int16 arrivingCount) {
	loadDynamicSnoidsFromPack(_vm->_state->_f._zmbPackActive, _pedestalPoints, kPedestalCount, true, nullptr);
	layoutStaticAndWalkIn(-20, arrivingCount == 0);
	renderFeatures();
	if (arrivingCount != 0)
		assignStaggeredWalkDelays(30, 45);

	initializeBasecampGoButtonState(static_cast<int16>(_snoidMap.size()), getBasecampAvailableSnoidCount());
}

void ZoombiniShelterBasecamp::loadBasecampPedestalFeatures(ZmbResource bitmap, uint16 firstScrbId) {
	for (uint16 pedestalIdx = 0; pedestalIdx < kPedestalCount; pedestalIdx++) {
		loadScrbFeature(bitmap, firstScrbId + pedestalIdx, 7, _pedestalPoints[pedestalIdx],
						ZmbFeature::FLAG_00002000_DRAW_ON_REG |
							ZmbFeature::FLAG_00008000_LOOP_ANIM |
							ZmbFeature::FLAG_00080000_DEFER_ANIM |
							ZmbFeature::FLAG_01000000_DEFER_RENDER);
	}
}

void ZoombiniShelterBasecamp::updateBasecampGoButtonState(int16 fieldSnoidCount, int16 availableSnoidCount) {
	bool canGoEnabled = kPedestalCount <= fieldSnoidCount;
	if (_isFinalArrival)
		canGoEnabled = 0 < fieldSnoidCount && availableSnoidCount <= fieldSnoidCount;
	setGoButtonsEnabled(canGoEnabled);
}

void ZoombiniShelterBasecamp::initializeBasecampGoButtonState(int16 fieldSnoidCount, int16 availableSnoidCount) {
	_isFinalArrival = ZmbTrait::SNOID_MAX <= _vm->_state->_f._zmbGeneratedCount &&
					  availableSnoidCount < kPedestalCount;
	updateBasecampGoButtonState(fieldSnoidCount, availableSnoidCount);
}

int16 ZoombiniShelterBasecamp::getBasecampFieldSnoidCount() const {
	int16 fieldSnoidCount = 0;
	for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
		if ((*it)->hasFlag(ZmbFeature::FLAG_00000001_TYPE_SNOID))
			fieldSnoidCount += 1;
	}
	return fieldSnoidCount;
}

void ZoombiniShelterBasecamp::endDrag(const Common::Point &dropPos) {
	(void)dropPos;
	ZmbSnoid *snoid = finishSnoidDrag();
	const Common::Point releasedSnoidPos = snoid->getPointLoc();
	const Common::Rect dropRect = snoid->getClickRect();
	const int16 pedestalIdx = hitTestDrawOnRegSlot(releasedSnoidPos, _clickZoneRadius, true);
	bool placedOnPedestal = false;
	bool storedInStorage = false;
	bool settledAtRelease = false;

	if (0 <= pedestalIdx) {
		placeSnoidOnBasecampPedestal(snoid, pedestalIdx);
		placedOnPedestal = true;
	} else if (dropRect.intersects(_storageRect)) {
		const int16 storageSlotIdx = findStorageSlotIndex(false, dropRect, _storageLeftmostColumnIdx);
		if (0 <= storageSlotIdx) {
			storeBasecampDraggedSnoid(snoid, storageSlotIdx);
			storedInStorage = true;
		} else if (_dragFromStorage) {
			restoreBasecampStorageOrigin(snoid);
			storedInStorage = true;
		} else {
			settleSnoidAtTarget(snoid, _dragOrigPos);
			settledAtRelease = _dragOrigPos == releasedSnoidPos;
		}
	} else {
		settledAtRelease = settleSnoidAfterTerrainDrop(snoid, _dragOrigPos);
	}

	if (!placedOnPedestal && !storedInStorage) {
		if (_dragFromStorage) {
			if (settledAtRelease) {
				snoid->_packIsOccupied = false;
			} else {
				restoreBasecampStorageOrigin(snoid);
			}
		} else {
			snoid->_packIsOccupied = false;
			if (0 <= _dragSourceDrawOnRegSlot && !settledAtRelease) {
				setDrawOnRegOccupant(_dragSourceDrawOnRegSlot, snoid->getId());
				snoid->_packIsOccupied = true;
			}
		}
	}

	updateBasecampGoButtonState(getBasecampFieldSnoidCount(), getBasecampAvailableSnoidCount());
	resetBasecampStorageDrag();
}

ZmbSnoid *ZoombiniShelterBasecamp::loadBasecampStorageDragSnoid(const Common::Point &pickupPos) {
	if (!_storageRect.contains(pickupPos))
		return nullptr;

	const Common::Rect clickRect(pickupPos.x, pickupPos.y, pickupPos.x, pickupPos.y);
	const int16 storageSlotIdx = findStorageSlotIndex(true, clickRect, _storageLeftmostColumnIdx);
	if (storageSlotIdx < 0)
		return nullptr;

	ZmbStateStoredChunk &chunk = getBasecampStorageChunk();
	ZmbStateStoredEntry &entry = chunk.getEntry(storageSlotIdx);
	ZmbSnoid *snoid = loadSnoidFromPack(allocateDynamicPackSnoidId(), pickupPos, ZmbFeature::FLAG_00000001_TYPE_SNOID);
	if (!snoid)
		return nullptr;

	snoid->_trait = entry.getTraits();
	snoid->_name = entry.getName(_vm);
	snoid->_packIsOccupied = false;
	snoid->setupIdleHotspots();

	// Physical occupancy is trait-owned. The name and last rendered rectangle
	// remain residual until this entry is overwritten or rendered again.
	entry.setTraits(ZmbTrait());
	refreshBasecampStorageState();
	_dragFromStorage = true;
	_dragStorageOriginSlot = storageSlotIdx;
	return snoid;
}

void ZoombiniShelterBasecamp::resetBasecampStorageDrag() {
	_dragFromStorage = false;
	_dragStorageOriginSlot = -1;
}

void ZoombiniShelterBasecamp::placeSnoidOnBasecampPedestal(ZmbSnoid *snoid, int16 pedestalIdx) {
	setDrawOnRegOccupant(pedestalIdx, snoid->getId());
	snoid->_packIsOccupied = true;
	snoid->setAnimTargetPos(_drawOnRegSnapPositions[pedestalIdx]);
	snoid->setAnimState(kSnoidAnimState004_Arrive);
}

void ZoombiniShelterBasecamp::writeBasecampStorageEntry(ZmbStateStoredEntry &entry, const ZmbSnoid &snoid) {
	entry.setTraits(snoid._trait);
	entry.setName(_vm, snoid._name);
	entry._rect = Common::Rect();
}

void ZoombiniShelterBasecamp::storeBasecampDraggedSnoid(ZmbSnoid *snoid, int16 storageSlotIdx) {
	ZmbStateStoredChunk &chunk = getBasecampStorageChunk();
	writeBasecampStorageEntry(chunk.getEntry(storageSlotIdx), *snoid);
	unloadSnoid(snoid->getId());
	refreshBasecampStorageState();
}

void ZoombiniShelterBasecamp::restoreBasecampStorageOrigin(ZmbSnoid *snoid) {
	if (_dragStorageOriginSlot < 0)
		return;

	storeBasecampDraggedSnoid(snoid, _dragStorageOriginSlot);
}

int16 ZoombiniShelterBasecamp::synchronizeBasecampStoredCount(ZmbStateStoredChunk &chunk, int16 *runtimeStoredCount) {
	const int16 storedCount = chunk.getPopulatedEntryCount();
	chunk.setStoredCount(storedCount);
	if (runtimeStoredCount)
		*runtimeStoredCount = storedCount;
	return storedCount;
}

bool ZoombiniShelterBasecamp::isBasecampStorageEntryOccupied(
	const ZmbStateStoredEntry &entry, StorageOccupancyTest occupancyTest) const {
	const ZmbTrait &traits = entry.getTraits();
	if (occupancyTest == StorageOccupancyTest::kComplete)
		return traits.isComplete();
	if (occupancyTest == StorageOccupancyTest::kHairOrEyes)
		return traits._hair != ZmbTrait::TRAIT_NONE || traits._eyes != ZmbTrait::TRAIT_NONE;
	return traits._hair != ZmbTrait::TRAIT_NONE;
}

int16 ZoombiniShelterBasecamp::findLastBasecampStorageEntry(
	const ZmbStateStoredChunk &chunk, StorageOccupancyTest occupancyTest) const {
	for (int16 entryIdx = kStorageEntryCount - 1; 0 <= entryIdx; entryIdx--) {
		if (isBasecampStorageEntryOccupied(chunk.getEntry(entryIdx), occupancyTest))
			return entryIdx;
	}
	return -1;
}

int16 ZoombiniShelterBasecamp::storeBasecampActivePackInStorage(StorageOccupancyTest occupancyTest) {
	ZmbStateStoredChunk &chunk = getBasecampStorageChunk();
	const ZmbStateActivePack &activePack = _vm->_state->_f._zmbPackActive;
	int16 occupiedCount = 0;
	for (int16 entryIdx = 0; entryIdx < activePack.getPackZmbCount(); entryIdx++) {
		if (activePack.getEntry(entryIdx).getIsOccupied())
			occupiedCount += 1;
	}

	const int16 storageInsertIdx = findLastBasecampStorageEntry(chunk, occupancyTest) + 1;
	if (kStorageEntryCount - 1 < occupiedCount + storageInsertIdx) {
		int16 activeIdx = 0;
		int16 storedCount = 0;
		for (int16 storIdx = 0; storedCount < occupiedCount && storIdx < kStorageEntryCount; storIdx++) {
			if (isBasecampStorageEntryOccupied(chunk.getEntry(storIdx), occupancyTest))
				continue;
			while (activeIdx < activePack.getPackZmbCount() && !activePack.getEntry(activeIdx).getIsOccupied())
				activeIdx += 1;
			if (activePack.getPackZmbCount() <= activeIdx)
				break;
			ZmbStateStoredEntry &storageEntry = chunk.getEntry(storIdx);
			storageEntry.setTraits(activePack.getEntry(activeIdx).getTraits());
			storageEntry._rect = Common::Rect();
			storageEntry.setName(_vm, activePack.getEntry(activeIdx).getU32Name(_vm));
			activeIdx += 1;
			storedCount += 1;
		}
		return 0;
	}

	int16 activeIdx = 0;
	for (int16 arrvIdx = 0; arrvIdx < occupiedCount; arrvIdx++) {
		while (activeIdx < activePack.getPackZmbCount() && !activePack.getEntry(activeIdx).getIsOccupied())
			activeIdx += 1;
		if (activePack.getPackZmbCount() <= activeIdx)
			break;
		ZmbStateStoredEntry &storageEntry = chunk.getEntry(storageInsertIdx + arrvIdx);
		storageEntry.setTraits(activePack.getEntry(activeIdx).getTraits());
		storageEntry._rect = Common::Rect();
		storageEntry.setName(_vm, activePack.getEntry(activeIdx).getU32Name(_vm));
		activeIdx += 1;
	}
	return 1;
}

int16 ZoombiniShelterBasecamp::compactBasecampStorageEntries(StorageOccupancyTest occupancyTest) {
	ZmbStateStoredChunk &chunk = getBasecampStorageChunk();
	int16 leadingEmptyCount = -kStorageRowsPerColumn;
	for (int16 entryIdx = 0; entryIdx < kStorageEntryCount; entryIdx++) {
		if (isBasecampStorageEntryOccupied(chunk.getEntry(entryIdx), occupancyTest))
			break;
		leadingEmptyCount += 1;
	}
	if (leadingEmptyCount < kStorageRowsPerColumn)
		return 0;

	const int16 shiftCount = static_cast<int16>(kStorageRowsPerColumn * (leadingEmptyCount / kStorageRowsPerColumn));
	for (int16 entryIdx = shiftCount; entryIdx < kStorageEntryCount; entryIdx++) {
		chunk.getEntry(entryIdx - shiftCount) = chunk.getEntry(entryIdx);
		chunk.getEntry(entryIdx).setTraits(ZmbTrait());
	}

	_storageLeftmostColumnIdx -= shiftCount / kStorageRowsPerColumn;
	_storageLeftmostColumnIdx = MAX<int16>(_storageLeftmostColumnIdx, 0);
	chunk.setLeftmostColumnIdx(_storageLeftmostColumnIdx);
	return shiftCount;
}

void ZoombiniShelterBasecamp::recalculateBasecampStorageCapacity(int16 highestOccupiedSlotState, int16 storedCount) {
	_storageCapacity = static_cast<int16>(kStorageRowsPerColumn * ((highestOccupiedSlotState + kStorageCapacityPadding) / kStorageRowsPerColumn));
	_storageCapacity = CLIP<int16>(_storageCapacity, kStorageMinimumCapacity, kStorageEntryCount);
	_storageColumnCount = _storageCapacity / kStorageRowsPerColumn;

	const int16 maxLeftmostColumn = _storageColumnCount - kStorageVisibleColumnCount;
	_storageLeftmostColumnIdx = MIN<int16>(_storageLeftmostColumnIdx, maxLeftmostColumn);

	ZmbStateStoredChunk &chunk = getBasecampStorageChunk();
	chunk.setLeftmostColumnIdx(_storageLeftmostColumnIdx);
	chunk.setStoredCount(storedCount);
}

bool ZoombiniShelterBasecamp::expandBasecampStorageEntriesAtLeftBoundary(ZmbStateStoredChunk &chunk,
																		 StorageOccupancyTest occupancyTest) {
	bool hasFirstColumnEntry = false;
	for (int16 entryIdx = 0; !hasFirstColumnEntry && entryIdx < kStorageRowsPerColumn; entryIdx++)
		hasFirstColumnEntry = isBasecampStorageEntryOccupied(chunk.getEntry(entryIdx), occupancyTest);
	if (!hasFirstColumnEntry || kStorageColumnCount <= _storageColumnCount)
		return false;

	bool lastColumnEmpty = true;
	const int16 lastColumnStart = kStorageEntryCount - kStorageRowsPerColumn;
	for (int16 entryIdx = lastColumnStart; lastColumnEmpty && entryIdx < kStorageEntryCount; entryIdx++)
		lastColumnEmpty = !isBasecampStorageEntryOccupied(chunk.getEntry(entryIdx), occupancyTest);
	if (!lastColumnEmpty)
		return false;

	const int16 lastSourceEntryIdx = kStorageEntryCount - kStorageRowsPerColumn - 1;
	for (int16 entryIdx = lastSourceEntryIdx; 0 <= entryIdx; entryIdx--) {
		chunk.getEntry(entryIdx + kStorageRowsPerColumn) = chunk.getEntry(entryIdx);
		chunk.getEntry(entryIdx).setTraits(ZmbTrait());
	}
	return true;
}

ZoombiniShelterBasecamp::StorageScrollDirection ZoombiniShelterBasecamp::getBasecampStorageScrollDirection(uint32 buttonIdx) {
	if (kStorageScrollButtonCount <= buttonIdx)
		return StorageScrollDirection::kNone;
	return static_cast<StorageScrollDirection>(buttonIdx + 1);
}

int16 ZoombiniShelterBasecamp::getBasecampStorageScrollButtonIndex(StorageScrollDirection direction) {
	if (direction == StorageScrollDirection::kNone)
		return -1;
	return static_cast<int16>(direction) - 1;
}

void ZoombiniShelterBasecamp::beginBasecampStorageScroll(StorageScrollDirection direction) {
	_heldStorageScrollDirection = direction;
	_storageScrollDirection = direction;
	updateBasecampStorageScrollSound(false);
}

void ZoombiniShelterBasecamp::updateHeldBasecampStorageScroll() {
	if (_heldStorageScrollDirection == StorageScrollDirection::kNone)
		return;

	_storageScrollDirection = _heldStorageScrollDirection;
	updateBasecampStorageScrollSound(false);
}

void ZoombiniShelterBasecamp::releaseBasecampStorageScroll() {
	_heldStorageScrollDirection = StorageScrollDirection::kNone;
	updateBasecampStorageScrollSound(true);
}

void ZoombiniShelterBasecamp::advanceBasecampStorageScroll() {
	if (_storageScrollDirection == StorageScrollDirection::kNone)
		return;

	switch (_storageScrollDirection) {
	case StorageScrollDirection::kLeftMaximum:
	case StorageScrollDirection::kLeftOne: {
		int16 steps;
		if (_storageScrollDirection == StorageScrollDirection::kLeftMaximum)
			steps = kStorageOuterScrollStepCount;
		else
			steps = 1;
		if (_storageScrollDirection == StorageScrollDirection::kLeftMaximum &&
			!_storageScrollAnimating && _storageLeftmostColumnIdx < kStorageOuterScrollStepCount)
			steps = 0;
		if (0 < steps && _storageLeftmostColumnIdx == 0 && expandBasecampStorageAtLeftBoundary())
			updateBasecampStorageScrollSound(false);

		for (; 0 < steps; steps--) {
			if (_storageScrollAnimating || _storageLeftmostColumnIdx <= 0) {
				_storageScrollAnimating = false;
			} else {
				_storageLeftmostColumnIdx -= 1;
				if (_storageLeftmostColumnIdx < 0) {
					_storageLeftmostColumnIdx = 0;
					steps = 1;
				}
				_storageScrollAnimating = true;
			}
		}
		break;
	}
	case StorageScrollDirection::kRightOne:
	case StorageScrollDirection::kRightMaximum: {
		int16 maxColumn = _storageColumnCount - kStorageVisibleColumnCount;
		maxColumn = MIN<int16>(maxColumn, kStorageMaxLeftmostColumn);

		int16 steps;
		if (_storageScrollDirection == StorageScrollDirection::kRightMaximum)
			steps = kStorageOuterScrollStepCount;
		else
			steps = 1;
		if (_storageScrollDirection == StorageScrollDirection::kRightMaximum &&
			!_storageScrollAnimating && maxColumn < _storageLeftmostColumnIdx + kStorageOuterScrollStepCount)
			steps = 0;

		for (; 0 < steps; steps--) {
			if (_storageScrollAnimating) {
				_storageScrollAnimating = false;
				_storageLeftmostColumnIdx += 1;
				if (maxColumn <= _storageLeftmostColumnIdx) {
					_storageLeftmostColumnIdx = maxColumn;
					steps = 1;
				}
			} else if (_storageLeftmostColumnIdx + 1 <= kStorageMaxLeftmostColumn) {
				_storageScrollAnimating = _storageLeftmostColumnIdx < maxColumn;
			}
		}
		break;
	}
	case StorageScrollDirection::kNone:
		break;
	}

	if (!_storageScrollAnimating)
		_storageScrollDirection = StorageScrollDirection::kNone;
}

bool ZoombiniShelterBasecamp::canBasecampStorageScroll(StorageScrollDirection direction) const {
	switch (direction) {
	case StorageScrollDirection::kLeftMaximum:
		return kStorageOuterScrollStepCount - 1 < _storageLeftmostColumnIdx;
	case StorageScrollDirection::kLeftOne:
		return 0 < _storageLeftmostColumnIdx;
	case StorageScrollDirection::kRightOne:
		return _storageLeftmostColumnIdx < _storageColumnCount - kStorageVisibleColumnCount &&
			   _storageLeftmostColumnIdx + 1 <= kStorageMaxLeftmostColumn;
	case StorageScrollDirection::kRightMaximum:
		return _storageLeftmostColumnIdx < _storageColumnCount - (kStorageVisibleColumnCount + kStorageOuterScrollStepCount - 1) &&
			   _storageLeftmostColumnIdx + kStorageOuterScrollStepCount <= kStorageMaxLeftmostColumn;
	case StorageScrollDirection::kNone:
		return false;
	}
	return false;
}

void ZoombiniShelterBasecamp::updateBasecampStorageScrollSound(bool resetState) {
	const bool canScroll = canBasecampStorageScroll(_storageScrollDirection);
	bool playScrollingSound = false;
	bool playScrollEndSound = false;

	if (canScroll != _storageScrollSoundActive) {
		_storageScrollSoundActive = canScroll;
		playScrollingSound = canScroll;
		playScrollEndSound = !canScroll;
	}

	if (resetState) {
		if (_storageScrollSoundActive)
			playScrollEndSound = true;
		_storageScrollSoundActive = false;
		playScrollingSound = false;
	}

	if (playScrollEndSound) {
		_vm->_sound->stopSound(_storageScrollingSound);
		_vm->_sound->playSound(_storageScrollEndSound, Audio::Mixer::kSFXSoundType, false);
	} else if (playScrollingSound) {
		_vm->_sound->playSound(_storageScrollingSound, Audio::Mixer::kSFXSoundType, true);
	}
}

int16 ZoombiniShelterBasecamp::findBasecampStorageSlotIndex(const ZmbStateStoredChunk &chunk,
															bool searchOccupied, const Common::Rect &clickRect,
															int16 leftmostColumnIdx, int16 storageColumnCount,
															int16 storageCapacity, const int16 *storageMatrixX,
															const int16 (*storageMatrixY)[kStorageRowsPerColumn],
															StorageOccupancyTest occupancyTest) const {
	int16 bestSlotIdx = -1;
	int16 bestArea = 0;
	int16 entryIdx = kStorageRowsPerColumn * (leftmostColumnIdx % storageColumnCount);
	int16 columnIdx = 0;
	int16 rowIdx = 0;
	const Common::Point clickPoint(clickRect.left, clickRect.top);

	for (int16 visibleSlotIdx = 0; visibleSlotIdx < kStorageVisibleSlotCount; visibleSlotIdx++) {
		const int16 wrappedEntryIdx = entryIdx % storageCapacity;
		const ZmbStateStoredEntry &entry = chunk.getEntry(wrappedEntryIdx);
		const bool isOccupied = isBasecampStorageEntryOccupied(entry, occupancyTest);

		if (searchOccupied == isOccupied) {
			if (isOccupied) {
				if (entry._rect.contains(clickPoint))
					return wrappedEntryIdx;
			} else {
				const int16 x = storageMatrixX[columnIdx];
				const int16 y = storageMatrixY[columnIdx][rowIdx];
				const Common::Rect slotRect(x - kStorageSlotHalfSize, y - kStorageSlotHalfSize, x + kStorageSlotHalfSize, y + kStorageSlotHalfSize);
				const Common::Rect intersection = clickRect.findIntersectingRect(slotRect);
				if (!intersection.isEmpty()) {
					const int16 area = intersection.width() * intersection.height();
					if (kStorageMinOverlapArea < area && bestArea < area) {
						bestArea = area;
						bestSlotIdx = wrappedEntryIdx;
					}
				}
			}
		}

		entryIdx += 1;
		rowIdx += 1;
		if (kStorageRowsPerColumn <= rowIdx) {
			rowIdx = 0;
			columnIdx += 1;
		}
	}

	return bestSlotIdx;
}

void ZoombiniShelterBasecamp::renderBasecampStorageEntries(ZmbStateStoredChunk &chunk,
														   const int16 *animatedMatrixX, const int16 (*animatedMatrixY)[kStorageRowsPerColumn],
														   const int16 *stillMatrixX, const int16 (*stillMatrixY)[kStorageRowsPerColumn],
														   StorageOccupancyTest occupancyTest) {
	const int16 leftmostColumn = _storageLeftmostColumnIdx % _storageColumnCount;
	int16 visibleColumnCount;
	if (_storageScrollAnimating)
		visibleColumnCount = kStorageAnimatedColumnCount;
	else
		visibleColumnCount = kStorageVisibleColumnCount;
	int16 storageEntryIdx = kStorageRowsPerColumn * leftmostColumn;
	int16 columnIdx = 0;
	int16 rowIdx = 0;

	for (int16 visibleEntryIdx = 0;
		 visibleEntryIdx < visibleColumnCount * kStorageRowsPerColumn;
		 visibleEntryIdx++) {
		ZmbStateStoredEntry &entry = chunk.getEntry(storageEntryIdx % _storageCapacity);
		if (isBasecampStorageEntryOccupied(entry, occupancyTest)) {
			int16 x;
			int16 y;
			if (_storageScrollAnimating) {
				x = animatedMatrixX[columnIdx];
				y = animatedMatrixY[columnIdx][rowIdx];
			} else {
				x = stillMatrixX[columnIdx];
				y = stillMatrixY[columnIdx][rowIdx];
			}
			entry._rect = renderStoredSnoid(ZoombiniGraphics::kShapeScreen, entry.getTraits(), Common::Point(x, y));
		}

		storageEntryIdx += 1;
		rowIdx += 1;
		if (kStorageRowsPerColumn <= rowIdx) {
			rowIdx = 0;
			columnIdx += 1;
		}
	}
}

ZmbFeature *ZoombiniShelterBasecamp::loadBasecampControlFeature(const Common::Rect &clickRect) {
	ZmbFeature::EventHooks hooks;
	hooks.setPreRenderFunc(static_cast<ZmbFeature::OnPreRenderFunc>(&ZoombiniShelterBasecamp::basecampControls_preRender));
	hooks.setRenderFunc(static_cast<ZmbFeature::OnRenderFunc>(&ZoombiniShelterBasecamp::basecampControls_render));
	hooks.setPostRenderFunc(static_cast<ZmbFeature::OnPostRenderFunc>(&ZoombiniShelterBasecamp::basecampControls_postRender));
	hooks.setLButtonDownFunc(static_cast<ZmbFeature::OnLButtonDownFunc>(&ZoombiniShelterBasecamp::basecampControls_lButtonDown));

	ZmbFeature *feature = loadScrbFeature(ZmbResource(ZmbResource::kPage, 0), 0, 0, ZmbFeature::FLAG_00001000_TOPMOST, hooks);
	feature->setClickRect(clickRect);
	return feature;
}

ZmbRenderResult ZoombiniShelterBasecamp::basecampControls_render(ZmbFeature *feature) {
	(void)feature;

	for (uint32 hotspotIdx = 0; hotspotIdx < _controlGoMapHotspots.size(); hotspotIdx++) {
		const ZmbHotspot &hotspot = _controlGoMapHotspots[hotspotIdx];
		if (hotspot._shapeIdx <= ZmbHotspot::kShapeNone)
			continue;
		if (_controlBitmapIsCompoundShapes) {
			_vm->_gfx->drawShape(ZoombiniGraphics::kShapeScreen,
								 ZmbResource(ZmbResource::kPage, _controlBitmapBaseId),
								 static_cast<uint16>(hotspot._shapeIdx),
								 hotspot.getPos());
		} else {
			_vm->_gfx->drawImage(ZoombiniGraphics::kShapeScreen, _controlBitmapBaseId + hotspot._shapeIdx - 1, hotspot.getPos());
		}
	}
	renderBasecampHelpButtons();
	return ZmbRenderResult::kRendered;
}

bool ZoombiniShelterBasecamp::basecampControls_preRender(ZmbFeature *feature) {
	_controlGoMapHotspots = _controlGoMapBaseHotspots;
	_controlHelpHotspots = _controlHelpBaseHotspots;
	goMapButtons_preRenderShape(feature, nullptr, _controlGoMapHotspots);
	helpButton_preRenderShape(feature, nullptr, _controlHelpHotspots);
	return true;
}

void ZoombiniShelterBasecamp::renderBasecampHelpButtons() {
	for (uint32 i = 0; i < _controlHelpHotspots.size(); i++) {
		const ZmbHotspot &hotspot = _controlHelpHotspots[i];
		if (hotspot._shapeIdx <= ZmbHotspot::kShapeNone)
			continue;
		_vm->_gfx->drawShape(ZoombiniGraphics::kShapeScreen,
							 ZmbResource(ZmbResource::kSystem, kSysResShapeBitmap0001_Dialog),
							 static_cast<uint16>(hotspot._shapeIdx), hotspot.getPos());
	}
}

void ZoombiniShelterBasecamp::basecampControls_postRender(ZmbFeature *feature) {
	goMapButtons_onPostRender(feature);
	helpButton_onPostRender(feature);
}

ZmbEventHandleResult ZoombiniShelterBasecamp::basecampControls_lButtonDown(ZmbFeature *feature,
																		   const Common::Point &absPos,
																		   const Common::Point &relPos) {
	ZmbEventHandleResult result = goMapButtons_onLButtonDown(feature, absPos, relPos);
	if (result == ZmbEventHandleResult::kConsumed)
		return result;
	return helpButton_onLButtonDown(feature, absPos, relPos);
}

int16 ZoombiniShelterBasecamp::splitActivePackForBasecamp(ZmbStateActivePack &activePack, ZmbStateActivePack &basecampPack, bool isDeparture) {
	if (!isDeparture) {
		// A map transition keeps the complete snapshot at this shelter. Both
		// categories remain materializable when the shelter is reopened.
		activePack.setSkipOccupiedEntries(false);
		activePack.setSkipUnoccupiedEntries(false);
		activePack.copyTo(basecampPack);
		activePack.clearEntries();
		return 0;
	}

	// The shelter pack intentionally remains a full handoff snapshot. Its
	// occupied entries are the departing party and become stale at the shelter,
	// so 1/0 materializes only the non-occupied residents on the next visit.
	activePack.setSkipOccupiedEntries(true);
	activePack.setSkipUnoccupiedEntries(false);
	activePack.copyTo(basecampPack);
	// The active pack uses the inverse 0/1 filter so only the occupied departing
	// party continues onto the route.
	activePack.setSkipOccupiedEntries(false);
	activePack.setSkipUnoccupiedEntries(true);

	int16 departingCount = 0;
	for (int16 i = 0; i < activePack.getPackZmbCount(); i++) {
		if (activePack.getEntry(i).getIsOccupied())
			departingCount += 1;
	}
	return departingCount;
}

bool ZoombiniShelterBasecamp::isBasecampSnoidDraggable(const ZmbSnoid &snoid) const {
	switch (snoid.getAnimState()) {
	case kSnoidAnimState008_ScriptReject:
	case kSnoidAnimState009_ScriptNormal:
	case kSnoidAnimState001_TurnLeft:
	case kSnoidAnimState002_TurnRight:
	case kSnoidAnimState007_Depart:
	case kSnoidAnimState112_Path:
	case kSnoidAnimState010_ArrivalMotion:
		return false;
	default:
		return true;
	}
}

Common::String ZoombiniShelterBasecamp::debugGetPedestalCommandsHelp() const {
	Common::String output;
	output += "Fill empty Basecamp pedestal seats, or return seated Zoombinis to storage.\n";
	output += "Usage: page ";
	output += kPageCommandAutoseat;
	output += "|";
	output += kPageCommandAutounseat;
	output += "\n";
	output += "  autoseat preserves seated Zoombinis and consumes only the stored entries\n";
	output += "  needed to fill the remaining pedestal seats. autounseat is its reverse:\n";
	output += "  every seated Zoombini goes back into the first free storage entry.\n\n";
	output += "Options:\n";
	output += "  -h, --help  Show this help text and exit.\n";
	return output;
}

int16 ZoombiniShelterBasecamp::debugRunPedestalCommand(int argc, const char **argv,
													   ZmbStateStoredChunk &chunk,
													   int16 *runtimeStoredCount, Common::String &output) {
	if (argc != 2 || (scumm_stricmp(argv[1], kPageCommandAutoseat) != 0 &&
					  scumm_stricmp(argv[1], kPageCommandAutounseat) != 0)) {
		output = debugGetPedestalCommandsHelp();
		return -1;
	}

	if (scumm_stricmp(argv[1], kPageCommandAutoseat) == 0)
		return debugSeatStoredZoombinis(chunk, runtimeStoredCount, output);
	return debugUnseatSeatedZoombinis(chunk, runtimeStoredCount, output);
}

bool ZoombiniShelterBasecamp::debugEnsurePedestalsIdle(const char *commandName, Common::String &output) const {
	if (isDragging() || isDeparturePending() || _goButtonPressPending) {
		output = Common::String::format("page %s cannot run while Basecamp input or departure animation is active.\n", commandName);
		return false;
	}

	if (_drawOnRegCount < kPedestalCount) {
		output = "Basecamp pedestal runners are not ready.\n";
		return false;
	}

	for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
		ZmbSnoid *snoid = *it;
		if (snoid && snoid->isPackSnoid() && !isBasecampSnoidDraggable(*snoid)) {
			output = "Basecamp Zoombini animations are still in progress; try again when they are idle.\n";
			return false;
		}
	}

	return true;
}

void ZoombiniShelterBasecamp::reconcileDrawOnRegPedestalOccupants() {
	// Reconcile occupied pack runners with their target seat before searching for empty seats.
	// Walk-in animation can temporarily leave the DRAW_ON_REG occupancy table unset even though
	// the runner already owns an occupied pack entry and has a pedestal target.
	for (int16 pedestalIdx = 0; pedestalIdx < kPedestalCount; pedestalIdx += 1) {
		if (getDrawOnRegOccupant(pedestalIdx) != 0)
			continue;

		for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
			ZmbSnoid *snoid = *it;
			if (!snoid || !snoid->isPackSnoid() || !snoid->_packIsOccupied)
				continue;
			if (snoid->getAnimTargetPos() != _drawOnRegSnapPositions[pedestalIdx])
				continue;
			if (findDrawOnRegSlotByOccupant(snoid->getId()) < 0)
				setDrawOnRegOccupant(pedestalIdx, snoid->getId());
			break;
		}
	}
}

int16 ZoombiniShelterBasecamp::debugSeatStoredZoombinis(ZmbStateStoredChunk &chunk, int16 *runtimeStoredCount, Common::String &output) {
	if (!debugEnsurePedestalsIdle(kPageCommandAutoseat, output))
		return -1;

	reconcileDrawOnRegPedestalOccupants();

	int16 occupiedPedestalCount = 0;
	for (int16 pedestalIdx = 0; pedestalIdx < kPedestalCount; pedestalIdx += 1) {
		if (getDrawOnRegOccupant(pedestalIdx) != 0)
			occupiedPedestalCount += 1;
	}

	if (occupiedPedestalCount == kPedestalCount) {
		output = "All 16 Basecamp pedestal seats are already occupied.\n";
		return 0;
	}

	const Common::Point storagePoint = _storageRect.center();
	int16 movedCount = 0;
	for (int16 pedestalIdx = 0; pedestalIdx < kPedestalCount; pedestalIdx += 1) {
		if (getDrawOnRegOccupant(pedestalIdx) != 0)
			continue;

		int16 storIdx = -1;
		for (int16 entryIdx = 0; entryIdx < chunk.getEntryCapacity(); entryIdx += 1) {
			if (chunk.getEntry(entryIdx).getTraits().isComplete()) {
				storIdx = entryIdx;
				break;
			}
		}
		if (storIdx < 0)
			break;

		ZmbStateStoredEntry &storedEntry = chunk.getEntry(storIdx);
		Common::Point snoidPoint = storagePoint;
		if (!storedEntry._rect.isEmpty())
			snoidPoint = storedEntry._rect.center();

		ZmbSnoid *snoid = loadSnoidFromPack(allocateDynamicPackSnoidId(), snoidPoint, ZmbFeature::FLAG_00000001_TYPE_SNOID);
		if (!snoid) {
			output = "Could not materialize a stored Zoombini for the pedestal command.\n";
			break;
		}

		if (!storedEntry._rect.isEmpty())
			addExternalDirtyRect(storedEntry._rect);
		snoid->_trait = storedEntry.getTraits();
		snoid->_name = storedEntry.getName(_vm);
		snoid->_packIsOccupied = true;
		snoid->setupIdleHotspots();
		placeSnoidOnBasecampPedestal(snoid, pedestalIdx);
		prepareSnoidVisualCoverage(snoid, true);
		addExternalDirtyRect(snoid->getClickRect());
		snoid->setNeedsRedraw(true);

		storedEntry.setTraits(ZmbTrait());
		storedEntry._rect = Common::Rect();
		movedCount += 1;
	}
	synchronizeBasecampStoredCount(chunk, runtimeStoredCount);

	const int16 finalPedestalCount = occupiedPedestalCount + movedCount;
	if (0 < movedCount) {
		output = Common::String::format("Moved %d stored Zoombini(s) onto Basecamp pedestals (%d/16 occupied).\n", movedCount, finalPedestalCount);
	} else {
		output = Common::String::format("No stored Zoombinis were available; Basecamp pedestals remain at %d/16 occupied.\n", occupiedPedestalCount);
	}
	return movedCount;
}

int16 ZoombiniShelterBasecamp::debugUnseatSeatedZoombinis(ZmbStateStoredChunk &chunk, int16 *runtimeStoredCount, Common::String &output) {
	if (!debugEnsurePedestalsIdle(kPageCommandAutounseat, output))
		return -1;

	reconcileDrawOnRegPedestalOccupants();

	int16 seatedCount = 0;
	for (int16 pedestalIdx = 0; pedestalIdx < kPedestalCount; pedestalIdx += 1) {
		if (getDrawOnRegOccupant(pedestalIdx) != 0)
			seatedCount += 1;
	}

	if (seatedCount == 0) {
		output = "No Zoombinis are seated on the Basecamp pedestals.\n";
		return 0;
	}

	int16 returnedCount = 0;
	bool storageFull = false;
	for (int16 pedestalIdx = 0; pedestalIdx < kPedestalCount; pedestalIdx += 1) {
		const uint16 occupantId = getDrawOnRegOccupant(pedestalIdx);
		if (occupantId == 0)
			continue;

		int16 freeEntryIdx = -1;
		for (int16 entryIdx = 0; entryIdx < chunk.getEntryCapacity(); entryIdx += 1) {
			if (!chunk.getEntry(entryIdx).getTraits().isComplete()) {
				freeEntryIdx = entryIdx;
				break;
			}
		}
		if (freeEntryIdx < 0) {
			storageFull = true;
			break;
		}

		ZmbSnoid *snoid = nullptr;
		for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
			if (*it && (*it)->getId() == occupantId) {
				snoid = *it;
				break;
			}
		}
		if (!snoid) {
			setDrawOnRegOccupant(pedestalIdx, 0);
			continue;
		}

		ZmbStateStoredEntry &entry = chunk.getEntry(freeEntryIdx);
		writeBasecampStorageEntry(entry, *snoid);

		// Queue the departing Zoombini's visual area for a background recompose
		// before the runner disappears, or the pedestal keeps showing the ghost.
		const Common::Rect oldRect = snoid->getZSortRect();
		if (!oldRect.isEmpty())
			addExternalDirtyRect(oldRect);

		unloadSnoid(occupantId);
		setDrawOnRegOccupant(pedestalIdx, 0);
		returnedCount += 1;
	}

	synchronizeBasecampStoredCount(chunk, runtimeStoredCount);

	const int16 remainingSeatedCount = seatedCount - returnedCount;
	if (storageFull) {
		output = Common::String::format("Storage is full; returned %d of %d seated Zoombini(s) (%d/16 still seated).\n",
										returnedCount, seatedCount, remainingSeatedCount);
	} else {
		output = Common::String::format("Returned %d seated Zoombini(s) to storage (%d/16 still seated).\n", returnedCount, remainingSeatedCount);
	}
	return returnedCount;
}

} // End of namespace Mohawk
