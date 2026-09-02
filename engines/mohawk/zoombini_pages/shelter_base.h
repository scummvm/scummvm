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

#ifndef MOHAWK_ZOOMBINI_PAGES_SHELTER_BASE_H
#define MOHAWK_ZOOMBINI_PAGES_SHELTER_BASE_H

#include "mohawk/zoombini_pages/interactive_base.h"

namespace Mohawk {

struct ZmbStateStoredChunk;
struct ZmbStateStoredEntry;

/**
 * Base class for shelter pages where Zoombinis are stored and regrouped.
 *
 * Shelters are the storage side of the page graph: the Picker creates or
 * selects the active pack, Basecamp pages retain Zoombinis between puzzles,
 * and Town records the journey's long-term population. Concrete shelters own
 * their storage layout and departure rules while this class provides the
 * common interactive-page contract.
 */
class ZoombiniShelter : public ZoombiniInteractive {
public:
	/** Create a shelter page for the specified page type. */
	ZoombiniShelter(MohawkEngine_Zoombini *vm, ZoombiniPageType pageType);
	/** Release shelter features and interactive-page resources. */
	~ZoombiniShelter() override;
	bool canOpenSaveLoadDialog() const override { return true; }
};

/** Shared storage and drag state for the two Basecamp shelter pages. */
class ZoombiniShelterBasecamp : public ZoombiniShelter {
public:
	/** Describe the common Basecamp debugger subcommand. */
	Common::String debugGetPageCommandHelp() const override;
	/** Handle the common Basecamp pedestal debugger subcommands. */
	bool debugDoPageCommand(int argc, const char **argv, Common::String &output) override;

protected:
	ZoombiniShelterBasecamp(MohawkEngine_Zoombini *vm, ZoombiniPageType pageType,
							const Common::Rect &storageRect, int16 initialStorageCapacity,
							int16 controlBitmapBaseId, bool controlBitmapIsCompoundShapes,
							const Common::Point *pedestalPoints,
							ZmbResource storageScrollingSound, ZmbResource storageScrollEndSound);
	/** Number of pedestal seats on either Basecamp page. */
	static constexpr int16 kPedestalCount = 16;
	/** Number of storage scroll buttons on either Basecamp page. */
	static constexpr int16 kStorageScrollButtonCount = 4;
	/** Number of storage entries in one column. */
	static constexpr int16 kStorageRowsPerColumn = 5;
	/** Number of complete storage columns shown while the grid is still. */
	static constexpr int16 kStorageVisibleColumnCount = 5;
	/** Number of storage columns shown during a scrolling transition. */
	static constexpr int16 kStorageAnimatedColumnCount = 6;
	/** Number of serialized storage entries on either Basecamp page. */
	static constexpr int16 kStorageEntryCount = 625;
	/** Maximum number of columns represented by the serialized storage. */
	static constexpr int16 kStorageColumnCount = kStorageEntryCount / kStorageRowsPerColumn;
	/** Greatest valid leftmost column for a complete still viewport. */
	static constexpr int16 kStorageMaxLeftmostColumn = kStorageColumnCount - kStorageVisibleColumnCount;
	/** Number of storage slots inspected by a still-viewport hit test. */
	static constexpr int16 kStorageVisibleSlotCount = kStorageRowsPerColumn * kStorageVisibleColumnCount;
	/** Half-size of one empty storage-slot hit rectangle. */
	static constexpr int16 kStorageSlotHalfSize = 30;
	/** Minimum qualifying intersection area for an empty storage slot. */
	static constexpr int16 kStorageMinOverlapArea = 625;
	/** Number of state-machine steps requested by either outer scroll button. */
	static constexpr int16 kStorageOuterScrollStepCount = 5;
	/** Minimum dynamic storage capacity retained by either Basecamp page. */
	static constexpr int16 kStorageMinimumCapacity = 50;
	/** Extra serialized slots reserved when storage capacity is recalculated. */
	static constexpr int16 kStorageCapacityPadding = 10;
	/** Semantic order of the four Basecamp storage scroll buttons. */
	enum StorageScrollButtonIndex : uint32 {
		kStorageScrollButton01_LeftMaximum = 0,
		kStorageScrollButton02_LeftOne = 1,
		kStorageScrollButton03_RightOne = 2,
		kStorageScrollButton04_RightMaximum = 3,
	};
	/** Direction retained until a two-phase storage movement reaches a stable frame. */
	enum class StorageScrollDirection : int16 {
		kNone = 0,
		kLeftMaximum = 1,
		kLeftOne = 2,
		kRightOne = 3,
		kRightMaximum = 4,
	};
	/** Page debugger subcommand that seats stored Zoombinis automatically. */
	static constexpr const char *kPageCommandAutoseat = "autoseat";
	/** Page debugger subcommand that returns seated Zoombinis to storage. */
	static constexpr const char *kPageCommandAutounseat = "autounseat";
	/** Release shared Basecamp storage and drag state. */
	~ZoombiniShelterBasecamp() override;
	/** Dispatch the shared Basecamp controls, pickup gate, and Snoid drag start. */
	ZmbEventHandleResult onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) override;
	/** Give a concrete Basecamp page first refusal before shared control dispatch. */
	virtual ZmbEventHandleResult onBasecampPreLButtonDown(const Common::Point &absPos, const Common::Point &relPos);
	/** Handle a page-local background click after common Snoid pickup checks. */
	virtual ZmbEventHandleResult onBasecampNoSnoidLButtonDown(const Common::Point &absPos, const Common::Point &relPos);
	/** Forget the cached storage-button cursor before modal input takes ownership. */
	void clearHoverCursor() override;
	/** Save the current Basecamp pack before leaving for the map. */
	void saveStateBeforeMapTransition() override;
	/** Start the shared Basecamp departure walk toward the right edge. */
	void onGoButtonActivated() override;
	/** Play the departure sound and start a Basecamp walk toward @p target. */
	void startBasecampDeparture(const Common::Point &target);
	/** Serialize the selected Basecamp pack and enter its route transfer. */
	void executeDeparture() override;
	/** Save page-owned Basecamp storage and active-pack state. */
	void saveBasecampPackState(bool isDeparture);
	/** Return the resident pack owned by the concrete Basecamp page. */
	virtual ZmbStateActivePack &getBasecampResidentPack() = 0;
	/** Return the logical population counter owned by the concrete Basecamp page. */
	virtual int16 &getBasecampStoredPopulationCount() = 0;
	/** Return the optional runtime storage count maintained by the concrete page. */
	virtual int16 *getBasecampRuntimeStoredCount() = 0;
	/** Compact and commit page-owned storage bookkeeping before a page exit. */
	virtual void finalizeBasecampStorageForSave() = 0;
	/** Return the transfer source selected by the concrete Basecamp page. */
	virtual ZmbSrcPageKind getBasecampDepartureSourcePage() const = 0;
	/** Load the incoming pack and replace it with the page's resident snapshot. */
	int16 beginBasecampPackLoad();
	/** Load residents and finish the common Basecamp runner layout. */
	void completeBasecampPackLoad(int16 arrivingCount);
	/** Register the sixteen page-owned pedestal runners. */
	void loadBasecampPedestalFeatures(ZmbResource bitmap, uint16 firstScrbId);

	/** Recompute Go readiness from the field and all Zoombinis still available through this Basecamp. */
	void updateBasecampGoButtonState(int16 fieldSnoidCount, int16 availableSnoidCount);
	/** Initialize final-arrival state and Go readiness after Basecamp runners load. */
	void initializeBasecampGoButtonState(int16 fieldSnoidCount, int16 availableSnoidCount);
	/** Count field Snoids using the common Basecamp runner-type contract. */
	int16 getBasecampFieldSnoidCount() const;
	/** Complete the common Basecamp storage, pedestal, and terrain drop transaction. */
	void endDrag(const Common::Point &dropPos) override;
	/** Materialize one stored Zoombini as the runner used by a drag operation. */
	ZmbSnoid *loadBasecampStorageDragSnoid(const Common::Point &pickupPos);
	/** Clear the storage-specific part of the current drag origin. */
	void resetBasecampStorageDrag();
	/** Place @p snoid on a shared DRAW_ON_REG pedestal seat. */
	void placeSnoidOnBasecampPedestal(ZmbSnoid *snoid, int16 pedestalIdx);
	/** Serialize @p snoid into one Basecamp storage entry. */
	void writeBasecampStorageEntry(ZmbStateStoredEntry &entry, const ZmbSnoid &snoid);
	/** Serialize and unload @p snoid, then refresh the page-owned storage layout. */
	void storeBasecampDraggedSnoid(ZmbSnoid *snoid, int16 storageSlotIdx);
	/** Restore a rejected storage drag to its serialized origin and unload its runner. */
	void restoreBasecampStorageOrigin(ZmbSnoid *snoid);
	/** Recount serialized storage entries and update the optional page-local count. */
	int16 synchronizeBasecampStoredCount(ZmbStateStoredChunk &chunk, int16 *runtimeStoredCount);
	/** Return the stored chunk owned by the concrete Basecamp page. */
	virtual ZmbStateStoredChunk &getBasecampStorageChunk() = 0;
	/** Find a storage slot using the concrete Basecamp page's grid contract. */
	virtual int16 findStorageSlotIndex(
		bool searchOccupied, const Common::Rect &clickRect, int16 leftmostColumnIdx) = 0;
	/** Recompute page-owned storage counts, bounds, and capacity after a mutation. */
	virtual void refreshBasecampStorageState() = 0;
	/** Return every Zoombini still available through this Basecamp. */
	virtual int16 getBasecampAvailableSnoidCount() const = 0;
	/** Stored-trait fields used to recognize a physically occupied entry. */
	enum class StorageOccupancyTest {
		kHair,
		kHairOrEyes,
		kComplete
	};
	/** Return whether @p entry is occupied under the requested page storage rule. */
	bool isBasecampStorageEntryOccupied(const ZmbStateStoredEntry &entry, StorageOccupancyTest occupancyTest) const;
	/** Return the final occupied serialized slot, or -1 when storage is empty. */
	int16 findLastBasecampStorageEntry(const ZmbStateStoredChunk &chunk, StorageOccupancyTest occupancyTest) const;
	/** Copy occupied active-pack entries into the page-owned serialized storage. */
	int16 storeBasecampActivePackInStorage(StorageOccupancyTest occupancyTest);
	/** Remove complete empty columns at the front while retaining one spare column. */
	int16 compactBasecampStorageEntries(StorageOccupancyTest occupancyTest);
	/** Recompute the common storage capacity, viewport clamp, and serialized headers. */
	void recalculateBasecampStorageCapacity(int16 highestOccupiedSlotState, int16 storedCount);
	/** Open one serialized column at the left boundary when the page storage permits it. */
	bool expandBasecampStorageEntriesAtLeftBoundary(ZmbStateStoredChunk &chunk, StorageOccupancyTest occupancyTest);
	/** Apply page-owned bookkeeping after serialized storage expands at its left boundary. */
	virtual bool expandBasecampStorageAtLeftBoundary() = 0;

	/** Convert the common zero-based button order to its storage direction. */
	static StorageScrollDirection getBasecampStorageScrollDirection(uint32 buttonIdx);
	/** Convert a storage direction to its common zero-based button order. */
	static int16 getBasecampStorageScrollButtonIndex(StorageScrollDirection direction);
	/** Latch one held scroll button without discarding a pending animation phase. */
	void beginBasecampStorageScroll(StorageScrollDirection direction);
	/** Re-arm the held direction and update its boundary-dependent looping sound. */
	void updateHeldBasecampStorageScroll();
	/** Release the held button and sound while retaining pending storage movement. */
	void releaseBasecampStorageScroll();
	/** Advance one page-scheduled tick of the common two-phase storage movement. */
	void advanceBasecampStorageScroll();
	/** Return whether the selected direction can move from the current viewport. */
	bool canBasecampStorageScroll(StorageScrollDirection direction) const;
	/** Update the common scrolling/end sound transaction. */
	void updateBasecampStorageScrollSound(bool resetState);
	/**
	 * Search the visible storage window using page-owned matrix coordinates.
	 * @param occupancyTest Page-owned stored-trait occupancy rule.
	 */
	int16 findBasecampStorageSlotIndex(const ZmbStateStoredChunk &chunk, bool searchOccupied,
									   const Common::Rect &clickRect, int16 leftmostColumnIdx,
									   int16 storageColumnCount, int16 storageCapacity,
									   const int16 *storageMatrixX,
									   const int16 (*storageMatrixY)[kStorageRowsPerColumn],
									   StorageOccupancyTest occupancyTest) const;

	/** Render stored Zoombinis using page-owned animated and still grid coordinates. */
	void renderBasecampStorageEntries(ZmbStateStoredChunk &chunk,
									  const int16 *animatedMatrixX, const int16 (*animatedMatrixY)[kStorageRowsPerColumn],
									  const int16 *stillMatrixX, const int16 (*stillMatrixY)[kStorageRowsPerColumn],
									  StorageOccupancyTest occupancyTest);

	/** Register one callback-only Go, Map, and Help feature. */
	ZmbFeature *loadBasecampControlFeature(const Common::Rect &clickRect);
	/** Draw the page-qualified Go and Map shapes plus the shared Help shapes. */
	ZmbRenderResult basecampControls_render(ZmbFeature *feature);
	/** Prepare release-specific Go, Map, and Help shapes. */
	bool basecampControls_preRender(ZmbFeature *feature);
	/** Draw the system-owned Help shapes shared by both Basecamp pages. */
	void renderBasecampHelpButtons();
	/** Dispatch completed Go, Map, and Help actions. */
	void basecampControls_postRender(ZmbFeature *feature);
	/** Dispatch Go, Map, and Help presses. */
	ZmbEventHandleResult basecampControls_lButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);
	/**
	 * Save a complete shelter handoff snapshot, then select its live subset with
	 * the occupied and non-occupied skip flags.
	 * On departure, @p basecampPack retains both the resident and departing
	 * entries but materializes only non-occupied residents when reopened.
	 * @return The number of occupied Snoids that remain in @p activePack for departure.
	 */
	int16 splitActivePackForBasecamp(ZmbStateActivePack &activePack, ZmbStateActivePack &basecampPack, bool isDeparture);
	/** Return true when @p snoid is in a state that permits Basecamp dragging. */
	bool isBasecampSnoidDraggable(const ZmbSnoid &snoid) const;
	/**
	 * Move stored Zoombinis onto empty Basecamp pedestal seats.
	 * @param chunk Basecamp storage chunk whose entries are consumed.
	 * @param runtimeStoredCount Page-local storage count, used by Basecamp Two.
	 * @param output Diagnostic result text for the page debugger.
	 * @return Number moved, or -1 when the command could not run.
	 */
	int16 debugSeatStoredZoombinis(ZmbStateStoredChunk &chunk, int16 *runtimeStoredCount, Common::String &output);
	/**
	 * Return every seated pedestal Zoombini back into free storage entries.
	 * @param chunk Basecamp storage chunk receiving the returned entries.
	 * @param runtimeStoredCount Page-local storage count, used by Basecamp Two.
	 * @param output Diagnostic result text for the page debugger.
	 * @return Number returned, or -1 when the command could not run.
	 */
	int16 debugUnseatSeatedZoombinis(ZmbStateStoredChunk &chunk, int16 *runtimeStoredCount, Common::String &output);
	/** Parse and run one shared Basecamp pedestal debugger command. */
	int16 debugRunPedestalCommand(int argc, const char **argv, ZmbStateStoredChunk &chunk,
								  int16 *runtimeStoredCount, Common::String &output);
	/**
	 * Reject a pedestal debugger command while page input, departure, or Zoombini
	 * animations are active.
	 * @param commandName Invoked subcommand name used in the refusal message.
	 * @param output Diagnostic result text when rejected.
	 * @return True when pedestal commands may run.
	 */
	bool debugEnsurePedestalsIdle(const char *commandName, Common::String &output) const;
	/** Re-register occupied pack runners that already target a pedestal snap position. */
	void reconcileDrawOnRegPedestalOccupants();
	/** Return the full usage text for the Basecamp pedestal commands. */
	Common::String debugGetPedestalCommandsHelp() const;

	/** Click region of the page's storage panel. */
	Common::Rect _storageRect;
	/** Page-local tBMP base used by the Go and Map button shapes. */
	const int16 _controlBitmapBaseId;
	/** Whether the control bitmap contains a compound SHPL shape table. */
	const bool _controlBitmapIsCompoundShapes;
	/** Page-owned positions used by the common active-pack loader. */
	const Common::Point *_pedestalPoints;
	/** Immutable Go/Map hotspot templates for the combined control runner. */
	Common::Array<ZmbHotspot> _controlGoMapBaseHotspots;
	/** Per-frame Go/Map hotspots for the combined control runner. */
	Common::Array<ZmbHotspot> _controlGoMapHotspots;
	/** Immutable Help hotspot templates for the combined control runner. */
	Common::Array<ZmbHotspot> _controlHelpBaseHotspots;
	/** Per-frame Help hotspots for the combined control runner. */
	Common::Array<ZmbHotspot> _controlHelpHotspots;
	/** Cursor shape used while pointing at the storage panel controls. */
	uint16 _storageButtonCursorShapeIdx = ZmbHotspot::kShapeNone;
	/** Leftmost visible column in the storage grid. */
	int16 _storageLeftmostColumnIdx = 0;
	/** Total number of columns represented by the current storage capacity. */
	int16 _storageColumnCount = 0;
	/** Current storage-grid capacity in slots. */
	int16 _storageCapacity = 0;
	/** Page-qualified looping sound for storage movement. */
	const ZmbResource _storageScrollingSound;
	/** Page-qualified sound played when storage movement stops. */
	const ZmbResource _storageScrollEndSound;
	/** Direction of the storage button currently held by the player. */
	StorageScrollDirection _heldStorageScrollDirection = StorageScrollDirection::kNone;
	/** Pending storage movement retained until its animation phase settles. */
	StorageScrollDirection _storageScrollDirection = StorageScrollDirection::kNone;
	/** Whether the storage grid is showing its intermediate scrolling frame. */
	bool _storageScrollAnimating = false;
	/** Whether the boundary-aware storage scrolling sound is active. */
	bool _storageScrollSoundActive = false;
	/** True when all 625 Zoombinis were generated and fewer than 16 remain through this Basecamp. */
	bool _isFinalArrival = false;
	/** True when the current drag began in storage rather than on a pedestal. */
	bool _dragFromStorage = false;
	/** Storage slot where the current drag began, or -1 for a pedestal drag. */
	int16 _dragStorageOriginSlot = -1;
};

} // End of namespace Mohawk

#endif
