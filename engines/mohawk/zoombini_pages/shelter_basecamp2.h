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

#ifndef MOHAWK_ZOOMBINI_PAGES_SHELTER_BASECAMP2_H
#define MOHAWK_ZOOMBINI_PAGES_SHELTER_BASECAMP2_H

#include "mohawk/zoombini_pages/shelter_base.h"

namespace Mohawk {

/**
 * Shade Tree - Second basecamp storage page.
 *
 * Basecamp2 is the larger intermediate storage area. It manages a
 * dynamically compacted storage grid, a pedestal row for the active pack,
 * and page-local Go/map/save and scroll controls. Departure serialization
 * preserves the occupied/non-occupied distinction used by the next route
 * page.
 */
class ZoombiniShelterBasecampTwo : public ZoombiniShelterBasecamp {
public:
	/** Create the second basecamp page. */
	ZoombiniShelterBasecampTwo(MohawkEngine_Zoombini *vm);
	/** Release Basecamp2 storage and feature callbacks. */
	~ZoombiniShelterBasecampTwo() override;

	/** Load the saved Basecamp2 storage state. */
	void open() override;
	/** Select Basecamp2 background music. */
	void setBackgroundMusic() override;
	/** Select the Basecamp2 background bitmap. */
	void setBackgroundBitmap() override;
	/** Restore Basecamp2 storage state before registering feature runners. */
	void initStates() override;
	/** Load storage, pedestal, button, and decorative features. */
	void loadFeatures() override;
	/** Restart the global Snoid fidget inactivity period after Basecamp2 loads. */
	bool requiresFidgetActivityResetOnLoad() const override { return true; }
	/** Return the original Basecamp2 script-sound range priority. */
	const ZoombiniPage::ScriptSoundPriorityRanges &getScriptSoundPriorityRanges() const override;
	/** Complete a drag or release a pressed control. */
	ZmbEventHandleResult onLButtonUp(const Common::Point &absPos, const Common::Point &relPos) override;
	/** Update storage hover, drag, and scroll-button state. */
	ZmbEventHandleResult onMouseMove(const Common::Point &absPos, const Common::Point &relPos) override;

protected:
	/** Trigger decorative buttons after shared Basecamp pickup checks find no Snoid. */
	ZmbEventHandleResult onBasecampNoSnoidLButtonDown(const Common::Point &absPos, const Common::Point &relPos) override;
	/** Play the original random feedback when Go is disabled. */
	void onDisabledGoButtonActivated() override;
	/** Advance the armed bonfire sequence after rendering and before ambient audio. */
	void onPostRenderFrame() override;

	/** Return the Basecamp2 resident pack. */
	ZmbStateActivePack &getBasecampResidentPack() override;
	/** Return the Basecamp2 logical population counter. */
	int16 &getBasecampStoredPopulationCount() override;
	/** Return the Basecamp2 runtime storage count. */
	int16 *getBasecampRuntimeStoredCount() override;
	/** Compact and commit Basecamp2 storage before a page exit. */
	void finalizeBasecampStorageForSave() override;
	/** Return the fixed Basecamp2 transfer source. */
	ZmbSrcPageKind getBasecampDepartureSourcePage() const override;
	// [*] Virtual feature callbacks -- Storage
	/** Render the Basecamp2 storage virtual feature. */
	bool storage_preRender(ZmbFeature *feature);
	/** Finish drawing the Basecamp2 storage feature. */
	void storage_postRender(ZmbFeature *feature);

	// [*] Virtual feature callbacks -- Scroll buttons area
	/** Finish rendering the Basecamp2 scroll-button overlay. */
	void buttons_postRender(ZmbFeature *feature);
	/** Begin holding a Basecamp2 scroll button. */
	ZmbEventHandleResult scrollButton_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);
	/** Update the hover state of the Basecamp2 scroll buttons. */
	bool updateScrollButtonHover(const Common::Point &absPos);
	/** Select the cursor shape for the hovered scroll button. */
	void updateScrollButtonCursor(const Common::Point &absPos);
	/** Release the currently held scroll button, if any. */
	bool releaseHeldScrollButton();
	/** Render the four BC2 storage scroll buttons. */
	void renderScrollButtons();

	// [*] Storage management helpers
	/** Return the last occupied slot in the compacted storage grid. */
	int16 findLastOccupiedSlot();
	/** Recalculate capacity after a storage slot becomes occupied. */
	void recalcStorageCapacity();
	/** Apply Basecamp2 bookkeeping after opening a new left storage column. */
	bool expandBasecampStorageAtLeftBoundary() override;
	/** Compact occupied storage entries toward the first storage slot. */
	void compactStorage();

	// Apply Basecamp2's column layout and rectangle hit-test contract.
	// Basecamp One uses different coordinates and the same selection rule.
	/** Find a storage slot using Basecamp2's grid geometry. */
	int16 findStorageSlotIndex(bool searchOccupied, const Common::Rect &clickRect, int16 leftmostColumnIdx) override;
	/** Return Basecamp2's stored chunk. */
	ZmbStateStoredChunk &getBasecampStorageChunk() override;
	/** Refresh Basecamp2 storage bounds, capacity, and serialized count. */
	void refreshBasecampStorageState() override;
	/** Return the Isle-plus-Basecamp-One-plus-Basecamp-Two available population. */
	int16 getBasecampAvailableSnoidCount() const override;

	/**
	 * This path checks button animation hotspots and triggers decorative animations.
	 * @param cursorPos Current cursor position to check against hotspots.
	 * @return True if a button animation was triggered, false otherwise.
	 */
	bool updateButtonAnimations(const Common::Point &cursorPos);

	/** Plays random arrival voice line based on difficulty. */
	void playArrivalVoice();

	// -----------------------------------------------------------------------
	// Resource IDs
	// -----------------------------------------------------------------------
	/** Page-local Basecamp2 storage, pedestal, and control resources. */
	enum PageResourceId : int16 {
		kResBackground5000 = 5000,

		kResBitmapTerrain100 = 100,
		kResBitmapShape6000_Main = 6000,
		kResBitmapShape7000_Pedestal = 7000,
		kResBitmapShape8000_Storage = 8000,
		kResBitmapShape9000_Buttons = 9000,
		kResBitmapShape10000_Cursors = 10000,

		kResNode1000 = 1000,

		kResRegs10000_Cursors = 10000,

		// SCRBs within tBMP group 6000
		kResScrb6000_EasterEggBonfireSticks = 6000,
		kResScrb6001_EasterEggBonfireFlames = 6001,
		kResScrb6002_EasterEggCrawlingLogAway = 6002,
		kResScrb6003_EasterEggCrawlingLogReturn = 6003,
		kResScrb6004_EasterEggGeyserRock = 6004,
		kResScrb6005_EasterEggHandBranch = 6005,
		kResScrb6006_EasterEggTopStep = 6006,
		kResScrb6007_EasterEggMiddleStep = 6007,
		kResScrb6008_EasterEggBottomStep = 6008,
		kResScrb6009_EasterEggThunderstorm = 6009,
		kResScrb6010_EasterEggPipeOrgan = 6010,
		kResScrb6011_EasterEggOddTreeLeavesAndAcorns = 6011,
		kResScrb6012_EasterEggOddTreeBat = 6012,
		kResScrb6013_EasterEggOddTreeWoodpecker = 6013,
		// SCRBs within tBMP group 7000 (pedestals 7000..7015)
		kResScrb7000_Pedestal = 7000,

		// Sound resources.
		kResSound2000_StorageScrolling = 2000,
		kResSound2001_StorageScrollEnd = 2001,

		// Sound ranges used by getScriptSoundPriority().
		kResSoundRange2000_StorageScrolling = 2000,
		kResSoundRange2099_StorageRangeLast = 2099,
		kResSoundRange6000_EasterEggRangeBase = 6000,
		kResSoundRange6099_EasterEggRangeLast = 6099,
	};

	// -----------------------------------------------------------------------
	// Shape indices within SHPL 8000 (storage) and SHPL 9000 (buttons)
	// -----------------------------------------------------------------------
	enum ShapeId : uint16 {
		// SHPL 8000 -- storage panel
		kShape8000_StorageAnim_Honeycomb = 1,
		kShape8000_StorageAnim_Lattice = 2,
		kShape8000_StorageStill_Honeycomb = 3,
		kShape8000_StorageStill_Lattice = 4,
		kShape8000_StorageBorder = 5,

		// SHPL 9000 -- individually stored button images.
		kShape9000_GoEnabled_01 = 1,
		kShape9000_GoPressed_02 = 2,
		// Index 3 is unused by the loop.
		kShape9000_MapNormal_05 = 5,
		kShape9000_MapPressed_06 = 6,
		// Indices 7-14: scroll arrows with two normal and two pressed shapes per direction.
		kShape9000_ScrollLMaxNormal_07 = 7,
		kShape9000_ScrollLMaxPressed_08 = 8,
		kShape9000_ScrollLOneNormal_09 = 9,
		kShape9000_ScrollLOnePressed_10 = 10,
		kShape9000_ScrollROneNormal_11 = 11,
		kShape9000_ScrollROnePressed_12 = 12,
		kShape9000_ScrollRMaxNormal_13 = 13,
		kShape9000_ScrollRMaxPressed_14 = 14,
		kShape9000_GoDisabled_15 = 15,
		// Z1-20U/TLC v2.0 release only: page-local hover shapes.
		kShape9000_GoHover_17 = 17,
		kShape9000_MapHover_19 = 19,
		kShape9000_ScrollLMaxHover_20 = 20,
		kShape9000_ScrollLOneHover_21 = 21,
		kShape9000_ScrollROneHover_22 = 22,
		kShape9000_ScrollRMaxHover_23 = 23,
		// Help/Save button (slot 3): uses shape 24 via the SCRB shape-table path rather than an individual tBMP.
		// Resources 9000-9022 cover the page-local Go, Map, and scroll shapes.
		// No tBMP 9023 or 9024 resources exist.
		// The SCRB runner renders the help button automatically.
		// These constants are kept for documentation only.
		kShape9000_HelpNormal_24 = 24,
		kShape9000_HelpPressed_25 = 25,

		// tBMP 10000 -- storage scroll cursors.
		kShape10000_ArrowLeftMax_01 = 1,
		kShape10000_ArrowLeft_02 = 2,
		kShape10000_ArrowRight_03 = 3,
		kShape10000_ArrowRightMax_04 = 4,
	};

	/** Fixed indices for the Basecamp2 decorative animation runners. */
	enum EasterEggButtonIdx : uint16 {
		kEasterEggButton01_HandBranch = 0,
		kEasterEggButton02_OddTree = 1,
		kEasterEggButton03_PipeOrgan = 2,
		kEasterEggButton04_CrawlingLog = 3,
		kEasterEggButton05_GeyserRock = 4,
		kEasterEggButton06_Thunderstorm = 5,
		kEasterEggButton07_TopStep = 6,
		kEasterEggButton08_MiddleStep = 7,
		kEasterEggButton09_BottomStep = 8,
		kEasterEggButton10_BonfireSticks = 9,
		kEasterEggButtonCount = 10,
	};

	// -----------------------------------------------------------------------
	// Static position data
	// -----------------------------------------------------------------------

	/**
	 * Pedestal positions for the 16 active Zoombinis (SCRB 7000..7015).
	 * These coordinates and their occupancy mapping are Basecamp2-owned and
	 * must not be moved into the base class or reused for Basecamp One.
	 */
	/** Pedestal positions and active-pack slot mapping for Basecamp2. */
	static constexpr Common::Point _pedestalPoints[kPedestalCount] = {
		Common::Point(0x01EA, 0x0174), //  0
		Common::Point(0x01CA, 0x0167), //  1
		Common::Point(0x01C2, 0x0180), //  2
		Common::Point(0x019C, 0x0178), //  3
		Common::Point(0x0189, 0x018E), //  4
		Common::Point(0x016D, 0x0182), //  5
		Common::Point(0x015C, 0x0195), //  6
		Common::Point(0x0141, 0x0185), //  7
		Common::Point(0x0130, 0x019A), //  8
		Common::Point(0x0116, 0x018D), //  9
		Common::Point(0x0108, 0x01A1), // 10
		Common::Point(0x00EA, 0x0190), // 11
		Common::Point(0x00DA, 0x01A4), // 12
		Common::Point(0x00C5, 0x018E), // 13
		Common::Point(0x00B1, 0x01A2), // 14
		Common::Point(0x0098, 0x0193), // 15
	};

	/**
	 * Storage grid X positions for animation-mode display (6 columns visible).
	 * Indexed by visible column.
	 */
	/** X coordinates of the six visible animated storage columns. */
	static constexpr int16 _storageMatrixX_anim[kStorageAnimatedColumnCount] = {
		0x008F,
		0x00B7,
		0x00DF,
		0x0107,
		0x012F,
		0x0157,
	};

	/**
	 * Storage grid X positions for non-animation (still) display (5 columns visible).
	 * Indexed by visible column.
	 */
	/** X coordinates of the five visible still storage columns. */
	static constexpr int16 _storageMatrixX_nonanim[kStorageVisibleColumnCount] = {
		0x00A3,
		0x00CB,
		0x00F3,
		0x011B,
		0x0143,
	};

	/**
	 * Storage-grid Y positions for animation-mode display: 6 columns x 5 rows.
	 * Indexed by visible column and row.
	 */
	/** Y coordinates for animated storage cells, indexed by column and row. */
	static constexpr int16 _storageMatrixY_anim[kStorageAnimatedColumnCount][kStorageRowsPerColumn] = {
		{0x0035, 0x0064, 0x0093, 0x00C2, 0x00F4}, // col 0
		{0x0037, 0x0065, 0x0094, 0x00C4, 0x00F3}, // col 1
		{0x0038, 0x0067, 0x0095, 0x00C4, 0x00F3}, // col 2
		{0x0036, 0x0064, 0x0093, 0x00C3, 0x00F2}, // col 3
		{0x002F, 0x0061, 0x0090, 0x00BF, 0x00ED}, // col 4
		{0x002B, 0x005A, 0x008A, 0x00B8, 0x00E7}, // col 5
	};

	/**
	 * Storage-grid Y positions for still display: 5 columns x 5 rows.
	 * Indexed by visible column and row.
	 */
	/** Y coordinates for still storage cells, indexed by column and row. */
	static constexpr int16 _storageMatrixY_nonanim[kStorageVisibleColumnCount][kStorageRowsPerColumn] = {
		{0x0037, 0x0066, 0x0095, 0x00C4, 0x00F3}, // col 0
		{0x0037, 0x0066, 0x0096, 0x00C4, 0x00F3}, // col 1
		{0x0036, 0x0066, 0x0095, 0x00C4, 0x00F3}, // col 2
		{0x0034, 0x0063, 0x0092, 0x00C1, 0x00F0}, // col 3
		{0x002E, 0x005D, 0x008D, 0x00BB, 0x00EB}, // col 4
	};

	/**
	 * Clickable hotspot rectangles indexed by @ref ZoombiniShelterBasecampTwo::EasterEggButtonIdx.
	 * The first hotspot shares the storage drag region with the hand-branch animation.
	 */
	const Common::Rect _buttonHotspotRects[kEasterEggButtonCount] = {
		Common::Rect(52, 290, 136, 332),  // [0] storage drag area
		Common::Rect(469, 169, 521, 241), // [1] odd tree
		Common::Rect(499, 289, 566, 308), // [2] pipe organ
		Common::Rect(455, 301, 504, 319), // [3] crawling log
		Common::Rect(568, 304, 604, 320), // [4] geyser rock
		Common::Rect(570, 36, 624, 66),   // [5] thunderstorm
		Common::Rect(229, 304, 273, 317), // [6] top step
		Common::Rect(242, 324, 292, 336), // [7] middle step
		Common::Rect(253, 348, 305, 361), // [8] bottom step
		Common::Rect(520, 259, 545, 300), // [9] bonfire sticks
	};

	/**
	 * Click region of the full storage panel virtual feature.
	 * BC2 columns are wider-spaced than BC1:
	 *   Non-anim (5 cols): X = 0x00A3, 0x00CB, 0x00F3, 0x011B, 0x0143
	 *   Anim     (6 cols): X = 0x008F, 0x00B7, 0x00DF, 0x0107, 0x012F, 0x0157
	 * Rightmost anim column centre at 0x0157=343, plus ~30px snoid radius -> right >= 373.
	 * Use 0x017F=383 to provide a safe margin.
	 * Keep the left, top, and bottom edges aligned with the BC1 layout.
	 */
	/**
	 * Immutable layout rectangles are instance members because Common::Rect
	 * requires runtime construction and ScummVM prohibits global C++ constructors.
	 */
	/** Go button click rect (derived from render position 0x0257,0x0140 + button size). */
	const Common::Rect _goButtonClickRect = Common::Rect(0x0257, 0x0140, 0x027E, 0x0165);

	/** Map button click rect (derived from render position 0x0257,0x0166 + button size). */
	const Common::Rect _mapButtonClickRect = Common::Rect(0x0257, 0x0166, 0x027E, 0x018B);

	/** Help/Save button click rectangle. */
	const Common::Rect _helpButtonClickRect = Common::Rect(0x0257, 0x018C, 0x027E, 0x01B1);

	/**
	 * Scroll button click rects (decoded, 36-byte stride).
	 * [0] = Scroll Left Max (slot 4), [1] = Scroll Left One (slot 5),
	 * [2] = Scroll Right One (slot 6), [3] = Scroll Right Max (slot 7).
	 */
	const Common::Rect _scrollButtonRects[kStorageScrollButtonCount] = {
		Common::Rect(0x0072, 0x0079, 0x0083, 0x00D0), // [0] Left Max
		Common::Rect(0x0083, 0x0075, 0x0090, 0x00CD), // [1] Left One
		Common::Rect(0x0151, 0x006B, 0x015E, 0x00C3), // [2] Right One
		Common::Rect(0x015E, 0x006E, 0x0169, 0x00C6), // [3] Right Max
	};

	// -----------------------------------------------------------------------
	// Runtime state
	// -----------------------------------------------------------------------

	/** Runner index of the virtual storage feature. */
	ZmbFeature *_storageFeature = nullptr;

	/** Virtual runner that renders the storage scroll buttons. */
	ZmbFeature *_scrollButtonFeature = nullptr;

	/** Runner index of the bonfire-sticks Easter egg SCRB. */
	uint16 _bonfireAnimRunnerIdx = 0;

	/**
	 * Runner indices for the Easter egg SCRBs, indexed by
	 * @ref ZoombiniShelterBasecampTwo::EasterEggButtonIdx.
	 * @ref ZoombiniShelterBasecampTwo::kEasterEggButton10_BonfireSticks also stores
	 * @ref ZoombiniShelterBasecampTwo::_bonfireAnimRunnerIdx.
	 */
	uint16 _buttonAnimRunnerIdxs[kEasterEggButtonCount] = {0};

	/** Index of the last occupied storage slot. */
	int16 _storageLastOccupiedIdx = 0;

	/** Number of Zoombinis currently in storage. */
	int16 _storedCount = 0;

	/** Frame counter threshold for the next storage scroll update. */
	uint32 _storageNextUpdateFrame = 0;

	/** Per-frame update interval for the storage scroll animation. */
	static constexpr uint32 kStorageScrollInterval = 2;

	/** Z1-20U/TLC v2.0 release only: yellow-outline hover state for scroll buttons. */
	bool _scrollButtonHovered[kStorageScrollButtonCount] = {false, false, false, false};
	/** True when the bonfire-sticks Easter egg has been armed. */
	bool _bonfireButtonArmed = false;

	/**
	 * Round-trip toggle for the crawling-log Easter egg.
	 * Starts at 1.
	 */
	bool _roundTripToggle = true;
};

} // End of namespace Mohawk

#endif
