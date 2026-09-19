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

#ifndef MOHAWK_ZOOMBINI_PAGES_SHELTER_BASECAMP1_H
#define MOHAWK_ZOOMBINI_PAGES_SHELTER_BASECAMP1_H

#include "mohawk/zoombini_pages/shelter_base.h"

namespace Mohawk {

/**
 * Shleter Rock - First basecamp storage page.
 *
 * Basecamp1 displays the active pack on pedestals and in a horizontally
 * scrollable storage area. Dragging changes whether a Zoombini is occupied;
 * the map and Go controls then serialize that ownership through
 * @ref ZoombiniShelterBasecamp::saveBasecampPackState() before the next page
 * transition.
 */
class ZoombiniShelterBasecampOne : public ZoombiniShelterBasecamp {
public:
	/** Create the first basecamp page. */
	ZoombiniShelterBasecampOne(MohawkEngine_Zoombini *vm);
	/** Release basecamp storage and feature callbacks. */
	~ZoombiniShelterBasecampOne() override;

	/** Load the saved Basecamp1 storage state. */
	void open() override;
	/** Select the Basecamp1 background bitmap. */
	void setBackgroundBitmap() override;
	/** Restore Basecamp1 storage viewport state before feature registration. */
	void initStates() override;
	/** Load storage, pedestal, button, and decorative features. */
	void loadFeatures() override;
	/** Restart the global Snoid fidget inactivity period after Basecamp1 loads. */
	bool requiresFidgetActivityResetOnLoad() const override { return true; }
	/** Return the original Basecamp1 script-sound range priority. */
	const ZoombiniPage::ScriptSoundPriorityRanges &getScriptSoundPriorityRanges() const override;

protected:
	/** Dispatch fixed hidden decorations before shared Basecamp hit testing. */
	ZmbEventHandleResult onBasecampPreLButtonDown(const Common::Point &absPos, const Common::Point &relPos) override;
	/** Return the Basecamp1 resident pack. */
	ZmbStateActivePack &getBasecampResidentPack() override;
	/** Return the Basecamp1 logical population counter. */
	int16 &getBasecampStoredPopulationCount() override;
	/** Basecamp1 does not retain a separate runtime storage count. */
	int16 *getBasecampRuntimeStoredCount() override;
	/** Compact and commit Basecamp1 storage before a page exit. */
	void finalizeBasecampStorageForSave() override;
	/** Return the route-up or route-down transfer source selected by the Go button. */
	ZmbSrcPageKind getBasecampDepartureSourcePage() const override;
	/** Render the Basecamp1 storage virtual feature. */
	ZmbRenderResult storage_render(ZmbFeature *feature);
	/** Finish drawing the Basecamp1 storage feature. */
	void storage_postRender(ZmbFeature *feature);

	/** Select the release-specific scroll-button shapes. */
	void scroll_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);
	/** Finish rendering the storage scroll controls. */
	void scroll_postRender(ZmbFeature *feature);
	/** Begin holding a storage scroll control. */
	ZmbEventHandleResult scroll_lButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);
	/** Release a storage scroll control without discarding its pending movement. */
	ZmbEventHandleResult scroll_lButtonUp(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);
	/** Update storage scrolling while a scroll control is held. */
	ZmbEventHandleResult scroll_mouseMove(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);
	/** Release the currently held scroll button, if any. */
	bool releaseHeldScrollButton(ZmbFeature *feature);

	/** Dispatch a click to an Easter egg using its page-local hit rectangle. */
	ZmbEventHandleResult genericEasterEgg_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos, const Common::Rect &clickRect);

	/** Render the stone-man Easter egg. */
	void easterEggStoneMan_postRender(ZmbFeature *feature);
	/** Activate the stone-man Easter egg. */
	ZmbEventHandleResult easterEggStoneMan_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);
	/** Render the fish Easter egg. */
	void easterEggFish_postRender(ZmbFeature *feature);
	/** Activate the fish Easter egg. */
	ZmbEventHandleResult easterEggFish_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);
	/** Render the bear Easter egg. */
	void easterEggBear_postRender(ZmbFeature *feature);
	/** Activate the bear Easter egg. */
	ZmbEventHandleResult easterEggBear_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);
	/** Render the stone-face Easter egg. */
	void easterEggStoneFace_postRender(ZmbFeature *feature);
	/** Activate the stone-face Easter egg. */
	ZmbEventHandleResult easterEggStoneFace_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);
	/** Render the hollow-bugs Easter egg. */
	void easterEggHollowBugs_postRender(ZmbFeature *feature);
	/** Activate the hollow-bugs Easter egg. */
	ZmbEventHandleResult easterEggHollowBugs_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);

	/** Activate the bonfire Easter egg. */
	ZmbEventHandleResult easterEggBonfire_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);

	/**
	 * Apply Basecamp1's honeycomb layout and rectangle hit-test contract.
	 * Basecamp Two uses different coordinates and hit-test geometry.
	 */
	int16 findStorageSlotIndex(bool searchOccupied, const Common::Rect &clickRect, int16 leftmostColumnIdx) override;
	/** Return Basecamp1's stored chunk. */
	ZmbStateStoredChunk &getBasecampStorageChunk() override;
	/** Refresh Basecamp1 storage bounds and serialized count. */
	void refreshBasecampStorageState() override;
	/** Return the Isle-plus-Basecamp-One population available on this page. */
	int16 getBasecampAvailableSnoidCount() const override;

	/**
	 * Find the index one past the last occupied slot (scanning 624..0 for traits != 0).
	 * Returns 0 if no occupied slot found.
	 */
	int16 findLastOccupiedIdx() const;
	/** Apply Basecamp1 bookkeeping after opening a new left storage column. */
	bool expandBasecampStorageAtLeftBoundary() override;

	/**
	 * Recalculate the storage column count and capacity.
	 * Then clamp @ref ZoombiniShelterBasecampOne::_storageLeftmostColumnIdx.
	 */
	void calcStorageColumns();

	/** Select the current frame of the mushroom Easter egg. */
	int32 easterEggMushroom_selectRenderFrame(ZmbFeature *feature);
	/** Activate the mushroom Easter egg. */
	ZmbEventHandleResult easterEggMushroom_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);

	/** Handle activation of the Basecamp1 Go button. */
	void onGoButtonActivated() override;
	/** Handle activation of the secondary Basecamp1 Go control. */
	void onSecondGoButtonActivated() override;

	/** Page-local Basecamp1 storage, pedestal, and control resources. */
	enum PageResourceId : int16 {
		kResBackground1000 = 1000,

		kResBitmapShape1100 = 1100,
		kResBitmapShape1200_Pedestal = 1200,
		kResBitmapShape2000_Storage = 2000,
		kResBitmapShape2100_Buttons = 2100,
		kResBitmapShape9000_Cursors = 9000,

		kResNode1000 = 1000,

		kResRegs9000 = 9000,

		kResScrb1100_BottomShape1 = 1100,
		kResScrb1101_BottomShape2 = 1101,
		kResScrb1102_BottomShape3 = 1102,
		kResScrb1103_BottomShape4 = 1103,
		kResScrb1104_Bonfire = 1104,
		kResScrb1105_EasterEggPod = 1105,
		kResScrb1106_EasterEggStoneMan = 1106,
		kResScrb1107_EasterEggHollowBugs = 1107,
		kResScrb1108_EasterEggFish = 1108,
		kResScrb1109_EasterEggBear = 1109,
		kResScrb1110_EasterEggStoneFace = 1110,
		kResScrb1111_EasterEggMushroom1 = 1111,
		kResScrb1112_EasterEggMushroom2 = 1112,
		kResScrb1113_EasterEggMushroom3 = 1113,
		kResScrb1114_EasterEggMushroom4 = 1114,
		kResScrb1115_EasterEggMushroom5 = 1115,
		kResScrb1200_Pedestal = 1200,
		kResScrb1201_Pedestal = 1201,
		kResScrb1202_Pedestal = 1202,
		kResScrb1203_Pedestal = 1203,
		kResScrb1204_Pedestal = 1204,
		kResScrb1205_Pedestal = 1205,
		kResScrb1206_Pedestal = 1206,
		kResScrb1207_Pedestal = 1207,
		kResScrb1208_Pedestal = 1208,
		kResScrb1209_Pedestal = 1209,
		kResScrb1210_Pedestal = 1210,
		kResScrb1211_Pedestal = 1211,
		kResScrb1212_Pedestal = 1212,
		kResScrb1213_Pedestal = 1213,
		kResScrb1214_Pedestal = 1214,
		kResScrb1215_Pedestal = 1215,

		// Sound resources.
		kResSound1118_EasterEggMushroom1 = 1118,
		kResSound1119_EasterEggMushroom2 = 1119,
		kResSound1120_EasterEggMushroom3 = 1120,
		kResSound1121_EasterEggMushroom4 = 1121,
		kResSound1122_EasterEggMushroom5 = 1122,
		kResSound2000_StorageScrolling = 2000,
		kResSound2001_StorageScrollEnd = 2001,

		// Sound ranges used by getScriptSoundPriority().
		kResSoundRange1100_EasterEggRangeBase = 1100,
		kResSoundRange1199_EasterEggRangeLast = 1199,
		kResSoundRange2000_StorageScrollingBase = 2000,
		kResSoundRange2099_StorageRangeLast = 2099,
	};

	enum ShapeId : uint16 {
		kShapeStorage01_Honeycomb = 1,
		kShapeStorage02_Lattice = 2,
		kShapeStorage03_Honeycomb = 3,
		kShapeStorage04_Lattice = 4,
		kShapeStorage05_Border = 5,

		kShape2100_GoRouteUpButtonNormal_01 = 1,
		kShape2100_GoRouteUpButtonPressed_02 = 2,
		kShape2100_GoRouteDownButtonNormal_03 = 3,
		kShape2100_GoRouteDownButtonPressed_04 = 4,
		kShape2100_MapNormal_05 = 5,
		kShape2100_MapPressed_06 = 6,
		kShape2100_ScrollLeftFourNormal_07 = 7,
		kShape2100_ScrollLeftFourPressed_08 = 8,
		kShape2100_ScrollLeftOneNormal_09 = 9,
		kShape2100_ScrollLeftOnePressed_10 = 10,
		kShape2100_ScrollRightOneNormal_11 = 11,
		kShape2100_ScrollRightOnePressed_12 = 12,
		kShape2100_ScrollRightFourNormal_13 = 13,
		kShape2100_ScrollRightFourPressed_14 = 14,
		kShape2100_GoRouteUpButtonDisabled_15 = 15,
		kShape2100_GoRouteDownButtonDisabled_16 = 16,
		// Z1-20U/TLC v2.0 release only: storage scroll hover shapes.
		kShape2100_ScrollLeftFourHover_20 = 20,
		kShape2100_ScrollLeftOneHover_21 = 21,
		kShape2100_ScrollRightOneHover_22 = 22,
		kShape2100_ScrollRightFourHover_23 = 23,

		kShape9000_ArrowLeftMax_01 = 1,
		kShape9000_ArrowLeft_02 = 2,
		kShape9000_ArrowRight_03 = 3,
		kShape9000_ArrowRightMax_04 = 4,
	};

	// These points belong to Basecamp1's pedestal art and occupancy mapping.
	// Do not move them into the base class or reuse them for Basecamp Two.
	/** Pedestal positions and active-pack slot mapping for Basecamp1. */
	static constexpr Common::Point _pedestalPoints[kPedestalCount] = {
		Common::Point(0x0194, 0x014E),
		Common::Point(0x0185, 0x0162),
		Common::Point(0x016A, 0x0153),
		Common::Point(0x015E, 0x016B),
		Common::Point(0x0143, 0x015C),
		Common::Point(0x0130, 0x016F),
		Common::Point(0x0118, 0x015E),
		Common::Point(0x0106, 0x0170),
		Common::Point(0x00EE, 0x015A),
		Common::Point(0x00DD, 0x016E),
		Common::Point(0x00C1, 0x0160),
		Common::Point(0x00B0, 0x0173),
		Common::Point(0x0098, 0x015F),
		Common::Point(0x0085, 0x0171),
		Common::Point(0x006E, 0x0155),
		Common::Point(0x005E, 0x0169),
	};

	/**
	 * Immutable layout rectangles are instance members because Common::Rect
	 * requires runtime construction and ScummVM prohibits global C++ constructors.
	 */
	/** Hit rectangle for the route-up Go button. */
	const Common::Rect _goRouteUpButtonRect = Common::Rect(0x0257, 0x0136, 0x027E, 0x015B);
	/** Hit rectangle for the route-down Go button. */
	const Common::Rect _goRouteDownButtonRect = Common::Rect(0x0257, 0x0182, 0x027E, 0x01A7);
	/** Hit rectangle for the map button. */
	const Common::Rect _mapButtonClickRect = Common::Rect(0x0257, 0x015C, 0x027E, 0x0181);
	/** Hit rectangle for the help/save button. */
	const Common::Rect _helpButtonClickRect = Common::Rect(0x0257, 0x01A8, 0x027E, 0x01CD);
	/** Hit rectangle for the four-column left scroll button. */
	const Common::Rect _scrollLeftFourButtonRect = Common::Rect(0x0013, 0x004E, 0x0023, 0x00CD);
	/** Hit rectangle for the one-column left scroll button. */
	const Common::Rect _scrollLeftOneButtonRect = Common::Rect(0x0023, 0x004E, 0x0034, 0x00CD);
	/** Hit rectangle for the one-column right scroll button. */
	const Common::Rect _scrollRightOneButtonRect = Common::Rect(0x0101, 0x004E, 0x0111, 0x00CD);
	/** Hit rectangle for the four-column right scroll button. */
	const Common::Rect _scrollRightFourButtonRect = Common::Rect(0x0111, 0x004E, 0x0122, 0x00CD);
	/** Hit rectangle for the stone-man Easter egg. */
	const Common::Rect _easterEggStoneManRect = Common::Rect(0x020F, 0x005A, 0x0232, 0x0071);
	/** Hit rectangle for the fish Easter egg. */
	const Common::Rect _easterEggFishRect = Common::Rect(0x0226, 0x0077, 0x024C, 0x0080);
	/** Hit rectangle for the bear Easter egg. */
	const Common::Rect _easterEggBearRect = Common::Rect(0x024D, 0x0070, 0x027F, 0x00A2);
	/** Hit rectangle for the stone-face Easter egg. */
	const Common::Rect _easterEggStoneFaceRect = Common::Rect(0x0144, 0x011F, 0x0187, 0x0143);
	/** Hit rectangle for the hollow-bugs Easter egg. */
	const Common::Rect _easterEggHollowBugsRect = Common::Rect(0x0019, 0x01B0, 0x007A, 0x01DD);

	/** Pressed state of each storage scroll button. */
	Common::StableMap<uint32, ContinuousButtonState> _scrollButtonStateMap;
	/** Hit rectangles indexed by storage scroll-button ID. */
	Common::HashMap<uint32, Common::Rect> _scrollButtonRectMap;
	/** X coordinates of the six Basecamp1 storage columns. */
	static constexpr int16 _storageMatrixX1[kStorageAnimatedColumnCount]{
		0x37,
		0x5F,
		0x87,
		0xAF,
		0xD7,
		0xFF,
	};
	/** Right-side X coordinates of the six Basecamp1 storage columns. */
	static constexpr int16 _storageMatrixX2[kStorageAnimatedColumnCount]{
		0x4B,
		0x73,
		0x9B,
		0xC3,
		0xEB,
		0xFF,
	};
	/** Top Y coordinates for Basecamp1 storage cells. */
	static constexpr int16 _storageMatrixY1[kStorageAnimatedColumnCount][kStorageRowsPerColumn]{
		{0x25, 0x54, 0x83, 0xB2, 0xE1},
		{0x27, 0x56, 0x85, 0xB4, 0xE3},
		{0x28, 0x57, 0x86, 0xB5, 0xE4},
		{0x26, 0x55, 0x84, 0xB3, 0xE2},
		{0x22, 0x51, 0x80, 0xAF, 0xDE},
		{0x1B, 0x4A, 0x79, 0xA8, 0xD7},
	};
	/** Bottom Y coordinates for Basecamp1 storage cells. */
	static constexpr int16 _storageMatrixY2[kStorageAnimatedColumnCount][kStorageRowsPerColumn]{
		{0x27, 0x56, 0x85, 0xB4, 0xE3},
		{0x28, 0x57, 0x86, 0xB5, 0xE4},
		{0x27, 0x56, 0x85, 0xB4, 0xE3},
		{0x24, 0x53, 0x82, 0xB1, 0xE0},
		{0x1F, 0x4E, 0x7D, 0xAC, 0xDB},
		{0x1B, 0x4A, 0x79, 0xA8, 0xD7},
	};

	/** One past the highest occupied slot returned by @ref ZoombiniShelterBasecampOne::findLastOccupiedIdx(). */
	int16 _storageMaxCellIdx = 0;

	/** 1 = route up (north), 2 = route down (south). Set when departure starts. */
	int16 _departRouteDirection = 0;
};

} // End of namespace Mohawk

#endif
