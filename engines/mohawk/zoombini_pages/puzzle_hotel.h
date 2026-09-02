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

#ifndef MOHAWK_ZOOMBINI_PAGES_PUZZLE_HOTEL_H
#define MOHAWK_ZOOMBINI_PAGES_PUZZLE_HOTEL_H

#include "common/array.h"
#include "mohawk/zoombini_pages/puzzle_base.h"

namespace Mohawk {

/**
 * Hotel Dimensia puzzle page (@ref ZoombiniPageType::kHotel).
 * Route 3, Puzzle 2
 *
 * Zoombinis must be assigned to hotel rooms based on trait matching.
 * Room assignments become more complex at higher difficulty levels with different SCRB sets loaded.
 *
 */
class ZoombiniPuzzleHotel : public ZoombiniPuzzle {
public:
	/** Create the Hotel Dimensia puzzle page. */
	ZoombiniPuzzleHotel(MohawkEngine_Zoombini *vm);
	/** Release room, guide, and Zoombini features. */
	~ZoombiniPuzzleHotel() override;
	/** Static puzzle name used by debug answer descriptions. */
	static constexpr const char *kPageName = ZmbXferRouteInfo::kXferPageNameHotelDimensia;
	static constexpr int kRouteNumber = 3;
	static constexpr int kRoutePuzzleIdx = 2;
	/** Return the puzzle name used by debug answer descriptions. */
	const char *getPageName() const override { return kPageName; }
	int getRouteNumber() const override { return kRouteNumber; }
	int getRoutePuzzleIdx() const override { return kRoutePuzzleIdx; }

	/** Generate room rules and initialize the active pack. */
	void open() override;
	/** Select the Hotel Dimensia puzzle music. */
	void setBackgroundMusic() override;
	/** Select the difficulty-specific hotel background. */
	void setBackgroundBitmap() override;
	/** Initialize puzzle states and departure transfer source. */
	void initStates() override;
	/** Load rooms, guide, counter, and Zoombini features. */
	void loadFeatures() override;
	/** Select the fixed Hotel Dimensia F1 narrator prompt. */
	void initHelpPrompt() override;

	/** Return the original Hotel Dimensia script-sound range priority. */
	const ZoombiniPage::ScriptSoundPriorityRanges &getScriptSoundPriorityRanges() const override;
	/** Classify Ulla's localized dialogue separately from Hotel effects. */
	Audio::Mixer::SoundType getFeatureSoundType(const ZmbFeature *feature, ZmbResource resource) const override;
	/** Advance room placement, Ulla, and rejection animations after rendering. */
	void onPostRenderFrame() override;
	/** Process room, Ulla, and placement animation callbacks. */
	void onFeatureAnimEvent(ZmbFeature *feature, int16 eventCode) override;

	/** Start a room placement drag or handle a control click. */
	ZmbEventHandleResult onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) override;

protected:
	/** Hotel Dimensia actions accepted by the built-in debug console and keyboard dispatcher. */
	enum class BuiltinDebugAction {
		kInvalid,
		kReinitCounter,
		kInfo,
		kStartMode,
		kIncrementCounter,
		kAllowAll,
		kCycleRoom,
		kCycleRoomAlternate
	};
	static constexpr const char *kBuiltinDebugActionReinitCounter = "reinit-counter";
	static constexpr const char *kBuiltinDebugActionInfo = "info";
	static constexpr const char *kBuiltinDebugActionStartMode = "start-mode";
	static constexpr const char *kBuiltinDebugActionIncrementCounter = "increment-counter";
	static constexpr const char *kBuiltinDebugActionAllowAll = "allow-all";
	static constexpr const char *kBuiltinDebugActionCycleRoom = "cycle-room";
	static constexpr const char *kBuiltinDebugActionCycleRoomAlternate = "cycle-room-alt";
	/** Depart with accepted room occupants after completion. */
	void onGoButtonActivated() override;
	/** Describe the generated room rules for diagnostics. */
	Common::String debugGetAnswer() const override;
	/** Describe the Hotel Dimensia built-in debug actions. */
	Common::String debugGetBuiltinDebugCommandHelp() const override;
	/** Run one Hotel Dimensia built-in debug action. */
	bool debugDoBuiltinDebugCommand(int argc, const char **argv, Common::String &output) override;
	/** Handle Hotel Dimensia built-in debug keys. */
	ZmbEventHandleResult onDebugKeyDown(const Common::KeyState &kbd) override;
	/** Parse one console-facing Hotel Dimensia action name. */
	static BuiltinDebugAction parseBuiltinDebugAction(const Common::String &action);
	/** Run one typed Hotel Dimensia built-in debug action. */
	bool runBuiltinDebugAction(BuiltinDebugAction action, Common::String &output);
	/** Report Hotel Dimensia mistake-budget usage for diagnostics. */
	ZmbChanceInfo debugGetChances() const override;
	/** Hotel Dimensia supports debugger chance adjustment. */
	bool debugCanSetChances() const override { return true; }
	/** Set Hotel Dimensia mistake-budget usage for diagnostics. */
	bool debugSetChances(int16 remaining) override;
	/** Select a room-specific Zoombini under the cursor. */
	ZmbSnoid *findSnoidAtPoint(const Common::Point &pos) override;
	/** Return whether @p snoid was accepted into a room. */
	bool isPlacedSnoid(const ZmbSnoid *snoid) const;
	/** Begin the level-4 drag that keeps an accepted Snoid fixed in its room. */
	void startPlacedSnoidDrag(ZmbSnoid *snoid, const Common::Point &mousePos);
	bool isSnoidDragPositionFixed() const override { return _acceptedSnoidDrag; }

private:
	/** Page-local Hotel Dimensia room, Ulla, and placement resources. */
	enum PageResourceId : int16 {
		kResBackground5000 = 5000,
		kResBackground5001_Gameplay = 5001,
		kResBackground5002_Level4 = 5002,

		// Terrain bitmaps
		kResBitmapTerrain100 = 100,

		// Shape bitmaps
		kResBitmapShape6000_Counter = 6000,
		kResBitmapShape7000_Main = 7000,
		kResBitmapShape7500_Guide = 7500,
		kResBitmapShape8000_Buttons = 8000,
		kResBitmapShape9000_Level4 = 9000,
		kResBitmapShape10000_RoomDrop = 10000,
		kResBitmapShape11000_Forbidden = 11000,
		kResBitmapShape11500_Label = 11500,
		kResBitmapShape11800_Room = 11800,
		kResBitmapShape12000_Level4Forbidden = 12000,

		kResRegs9000_Level4 = 9000,
		kResRegs9002_Level4 = 9002,
		kResRegs11000_Level3 = 11000,
		kResRegs11002_Level3Shape = 11002,
		kResRegs11004_Level3Cell = 11004,
		kResRegs12004_Level4 = 12004,

		kResScrb6000_CounterBase = 6000,
		kResScrb6013_RoomDisplayBase = 6013,
		kResScrb6038_RoomIconBase = 6038,
		kResScrb6063_RoomIconWinBase = 6063,
		kResScrb7000_RoomBase = 7000,
		kResScrb7007_EscalationBase = 7007,
		kResScrb7500_GuideBase = 7500,
		kResScrb7503_GuideReactionBase = 7503,
		kResScrb7506_GuideReactionLast = 7506,
		kResScrb9000_Level4DisplayBase = 9000,
		kResScrb9007_Level4BackgroundBase = 9007,
		kResScrb10000_RoomDropBase = 10000,
		kResScrb10025_Level4RoomDropBase = 10025,
		kResScrb11000_ForbiddenBase = 11000,
		kResScrb11500_LabelBase = 11500,
		kResScrb11800_Room = 11800,
		kResScrb12000_Level4ForbiddenBase = 12000,

		kResScrs13000_NormalBase = 13000,
		kResScrs13025_Level4NormalBase = 13025,
		kResScrs13030_NormalBase = 13030,
		kResScrs13035_NormalBase = 13035,
		kResScrs13040_NormalBase = 13040,
		kResScrs13045_Level4Base = 13045,
		kResScrs14000_RejectBase = 14000,
		kResScrs14025_Level4RejectBase = 14025,

		// Sound resources.
		kResSound6006_Overflow = 6006,
		kResSound7012_DebugRoomRestart = 7012,
		kResSound7500_Level4Overflow = 7500,
		kResSound7000_UllaSpeechSingle = 7000,
		kResSound7002_UllaSpeechBase = 7002,
		kResSound7023_UllaSpeechLast = 7023,
		kResSound7500_UllaSpeechBase = 7500,
		kResSound7535_UllaSpeechLast = 7535,

		// Sound ranges used by getScriptSoundPriority().
		kResSoundRange0099_PageSingle = 99,
		kResSoundRange0425_PageRangeBase = 425,
		kResSoundRange0499_PageRangeLast = 499,
		kResSoundRange6000_PageRangeBase = 6000,
		kResSoundRange6004_PagePriorityBase = 6004,
		kResSoundRange6006_Overflow = 6006,
		kResSoundRange6099_PageRangeLast = 6099,
		kResSoundRange7000_SystemRangeBase = 7000,
		kResSoundRange7999_SystemRangeLast = 7999,
		kResSoundRange8900_PagePriorityBase = 8900,
		kResSoundRange8901_PagePriorityLast = 8901,
		kResSoundRange9000_PageRangeBase = 9000,
		kResSoundRange9004_PagePriorityBase = 9004,
		kResSoundRange9006_PagePriorityLast = 9006,
		kResSoundRange9999_PageRangeLast = 9999,
		kResSoundRange10000_PageRangeBase = 10000,
		kResSoundRange10999_PageRangeLast = 10999,

		kResMidi30020_HotelBgmBase = 30020,
	};

	/** Snoid callbacks used while placing a guest in a room. */
	enum RoomEventCode : int16 {
		kRoomEventToggleSnoidFacing00 = 0,
		kRoomEventCommitPlacedSnoid15 = 15
	};

	/** Purpose of the animation currently stored in @ref _ullaAnimFeature. */
	enum UllaAnimPurpose : byte {
		/** Distant opening animation from the SCRB 7000 family. */
		kUllaAnimIntro00 = 0,
		/** Close-view prompt from the SCRB 7500 family. */
		kUllaAnimPrompt01 = 1,
		/** Completion or terminal-failure reaction from SCRB 7503-7509. */
		kUllaAnimCompletion02 = 2,
		/** Non-blocking response. */
		kUllaAnimResponse03 = 3
	};

	/** Hotel-specific trait-axis order used by room-rule generation and lookup tables. */
	enum class TraitAxis : int16 {
		/** Feet trait axis. */
		kFeet00 = 0,
		/** Nose trait axis. */
		kNose01 = 1,
		/** Eye trait axis. */
		kEyes02 = 2,
		/** Hair trait axis. */
		kHair03 = 3
	};
	/** Number of Hotel trait axes. */
	static constexpr int16 kTraitAxisCount = 4;

	/** Count generated axes whose value range is below @p threshold. */
	int countLimitedAxes(int threshold) const;

	// --- Initialization helpers ---
	/** Load active-pack Snoids into Hotel placement state. */
	void loadZoombinisFromPack();
	/** Count trait variants used by the room rules. */
	void computeTraitVariantCounts();
	/** Generate difficulty-specific room constraints. */
	void generateRoomRules();
	/** Register level 3 forbidden markers for the distant opening view. */
	void registerInitialForbiddenScrbs();

	// --- Opening-to-close-view transition ---
	/** Transition from the opening room view to the close placement view. */
	void setupCloseView();
	/** Register room display SCRBs used by the close view. */
	void registerDisplayScrbs();
	/** Remove the currently loaded Ulla animation feature. */
	void unloadUllaAnimFeature();
	/** Replace Ulla's animation feature with a new image, SCRB, and purpose. */
	void replaceUllaAnimFeature(int16 imageId, int16 scrbId, uint32 flags, UllaAnimPurpose purpose);
	/** Return the level 4 foreground position for a room slot. */
	Common::Point getLevel4DisplayPosition(int16 roomSlot) const;
	/** Return the level 4 background position for a room slot. */
	Common::Point getLevel4BackgroundPosition(int16 roomSlot) const;
	/** Load a level 4 room SCRB onto a feature. */
	void loadLevel4RoomScrb(ZmbFeature *feature, int16 scrbId);
	/** Reassign remaining unaccepted Snoids to the first requested pedestal positions. */
	void reassignPedestalPositions(int16 count);

	// --- Trait validation ---
	// Axis arguments use numbered order for readable ScummVM call sites.
	// Axis 1 maps to Row, axis 2 maps to Column, and axis 3 maps to SubColumn.
	// This interface order does not change the validation or grid-storage behavior.
	/**
	 * Validate placement at levels 1-3 using two axes.
	 * @param slot Zero-based room slot.
	 * @param axis1Val Axis 1 Row value.
	 * @param axis2Val Axis 2 Column value.
	 * @return True if the placement is valid.
	 */
	bool validate2TraitPlacement(int16 slot, int16 axis1Val, int16 axis2Val) const;
	/**
	 * Validate placement at level 4 using three axes.
	 * @param slot Zero-based room slot.
	 * @param axis1Val Axis 1 Row value.
	 * @param axis2Val Axis 2 Column value.
	 * @param axis3Val Axis 3 SubColumn value.
	 * @return True if the placement is valid.
	 */
	bool validate3TraitPlacement(int16 slot, int16 axis1Val, int16 axis2Val, int16 axis3Val) const;

	// --- Trait grid management ---
	/**
	 * Fill Row and Column constraints after accepting a Zoombini at levels 1-3.
	 * @param roomSlot Zero-based room slot.
	 * @param axis1Val Axis 1 Row value.
	 * @param axis2Val Axis 2 Column value.
	 */
	void fillCellRow(int16 roomSlot, int16 axis1Val, int16 axis2Val);
	/**
	 * Set level-4 Row, Column, and SubColumn constraints.
	 * @param roomSlot Zero-based room slot.
	 * @param axis1Val Axis 1 Row value.
	 * @param axis2Val Axis 2 Column value.
	 * @param axis3Val Axis 3 SubColumn value.
	 */
	void setCellTraitsIn3Grids(int16 roomSlot, int16 axis1Val, int16 axis2Val, int16 axis3Val);

	// --- Zoombini placement ---
	/** Start the SCRS entrance animation for a Zoombini accepted into a room. */
	void placeZoombiniInRoom(int16 roomSlot, ZmbSnoid *snoid);
	/** Start the rejection animation for a wrong placement. */
	void startRejectedPlacement(ZmbSnoid *snoid);
	/** Start the rejected Snoid's return to the resting pack. */
	void startRejectedReturn(ZmbSnoid *snoid);
	/** Commit an accepted placement after its animation completes. */
	void finalizeAcceptedPlacement();
	/** Commit a rejected placement after its animation completes. */
	void finalizeRejectedPlacement(ZmbSnoid *snoid);
	/** Scatter remaining Snoids around the return position. */
	void scatterSnoidsNearReturn(int16 returnX);
	/** Find the deferred reject-return state for a Snoid. */
	int findRejectReturnState(ZmbSnoid *snoid) const;

	// --- Win condition ---
	/** Register checkpoint callbacks on hotspot runners. */
	void registerWinCheckpoints();

	// --- Palette effect on wrong placement ---
	/** Dim palette by 88/90/92% based on difficulty over @p duration milliseconds. */
	void dimPaletteOnError(uint32 duration = 500);
	/** Rebuild cumulative palette dimming from the current chance counter. */
	void refreshChancePalette();

	// --- Drag-and-drop ---
	/** Resolve a Hotel drop and begin accepted or rejected placement. */
	void endDrag(const Common::Point &mousePos) override;
	/** Find the first matching room slot for the drop position, or -1 if none. */
	int16 getDropTargetSlot(const Common::Point &dropPos) const;

	// --- Trait access ---
	/** Return whether @p axis is a defined Hotel trait axis. */
	static bool isValidTraitAxis(TraitAxis axis);
	/** Convert a Hotel trait axis to its zero-based lookup-table index. */
	static int16 traitAxisIndex(TraitAxis axis);
	/** Return the shared trait kind represented by a Hotel trait axis. */
	static ZmbTrait::TraitKind traitKindForAxis(TraitAxis axis);
	/** Return @p trait's value for a Hotel trait axis. */
	static byte getAxisTraitValue(const ZmbTrait &trait, TraitAxis axis);

	// -----------------------------------------------------------------------
	// Static data tables
	// -----------------------------------------------------------------------
	/**
	 * Room center positions for levels 1-3 (25 slots).
	 * These levels use five columns and five rows.
	 * Each serialized entry contains little-endian int16 X and Y coordinates.
	 */
	static constexpr Common::Point kRoomPositions25[25]{
		Common::Point(0x0087, 0x004e), // [0]  col0 row0  (135, 78)
		Common::Point(0x008a, 0x008e), // [1]  col0 row1  (138, 142)
		Common::Point(0x008e, 0x00cc), // [2]  col0 row2  (142, 204)
		Common::Point(0x0092, 0x0107), // [3]  col0 row3  (146, 263)
		Common::Point(0x0095, 0x0144), // [4]  col0 row4  (149, 324)
		Common::Point(0x00df, 0x0054), // [5]  col1 row0  (223, 84)
		Common::Point(0x00de, 0x0093), // [6]  col1 row1  (222, 147)
		Common::Point(0x00e3, 0x00d2), // [7]  col1 row2  (227, 210)
		Common::Point(0x00e4, 0x010b), // [8]  col1 row3  (228, 267)
		Common::Point(0x00ea, 0x0148), // [9]  col1 row4  (234, 328)
		Common::Point(0x013b, 0x0058), // [10] col2 row0  (315, 88)
		Common::Point(0x0137, 0x0098), // [11] col2 row1  (311, 152)
		Common::Point(0x0139, 0x00d5), // [12] col2 row2  (313, 213)
		Common::Point(0x013a, 0x0110), // [13] col2 row3  (314, 272)
		Common::Point(0x013a, 0x014d), // [14] col2 row4  (314, 333)
		Common::Point(0x0192, 0x005e), // [15] col3 row0  (402, 94)
		Common::Point(0x018e, 0x009d), // [16] col3 row1  (398, 157)
		Common::Point(0x018f, 0x00dc), // [17] col3 row2  (399, 220)
		Common::Point(0x018d, 0x0115), // [18] col3 row3  (397, 277)
		Common::Point(0x018c, 0x0152), // [19] col3 row4  (396, 338)
		Common::Point(0x01eb, 0x0064), // [20] col4 row0  (491, 100)
		Common::Point(0x01e9, 0x00a4), // [21] col4 row1  (489, 164)
		Common::Point(0x01e9, 0x00e2), // [22] col4 row2  (489, 226)
		Common::Point(0x01e8, 0x011c), // [23] col4 row3  (488, 284)
		Common::Point(0x01e5, 0x015a), // [24] col4 row4  (485, 346)
	};
	/**
	 * Room center positions for level 4 (125 slots).
	 * Room centers for level 4 use five Columns, each with five Rows and five SubColumns.
	 */
	static constexpr Common::Point kRoomPositions125[125]{
		// Column 0 (entries 0-24)
		Common::Point(0x0010, 0x0028), // [0]   (16, 40)
		Common::Point(0x0027, 0x0032), // [1]   (39, 50)
		Common::Point(0x003c, 0x0036), // [2]   (60, 54)
		Common::Point(0x0056, 0x003a), // [3]   (86, 58)
		Common::Point(0x006f, 0x003c), // [4]   (111, 60)
		Common::Point(0x0013, 0x0073), // [5]   (19, 115)
		Common::Point(0x002a, 0x007d), // [6]   (42, 125)
		Common::Point(0x003f, 0x0081), // [7]   (63, 129)
		Common::Point(0x0059, 0x0085), // [8]   (89, 133)
		Common::Point(0x0072, 0x0087), // [9]   (114, 135)
		Common::Point(0x0015, 0x00bc), // [10]  (21, 188)
		Common::Point(0x002c, 0x00c6), // [11]  (44, 198)
		Common::Point(0x0041, 0x00ca), // [12]  (65, 202)
		Common::Point(0x005b, 0x00ce), // [13]  (91, 206)
		Common::Point(0x0074, 0x00d0), // [14]  (116, 208)
		Common::Point(0x0018, 0x0105), // [15]  (24, 261)
		Common::Point(0x002f, 0x010f), // [16]  (47, 271)
		Common::Point(0x0044, 0x0113), // [17]  (68, 275)
		Common::Point(0x005e, 0x0117), // [18]  (94, 279)
		Common::Point(0x0077, 0x0119), // [19]  (119, 281)
		Common::Point(0x001c, 0x014d), // [20]  (28, 333)
		Common::Point(0x0033, 0x0157), // [21]  (51, 343)
		Common::Point(0x0049, 0x015b), // [22]  (73, 347)
		Common::Point(0x0063, 0x015f), // [23]  (99, 351)
		Common::Point(0x007c, 0x0161), // [24]  (124, 353)
		// Column 1 (entries 25-49)
		Common::Point(0x008e, 0x0036), // [25]  (142, 54)
		Common::Point(0x00a5, 0x003c), // [26]  (165, 60)
		Common::Point(0x00ba, 0x0040), // [27]  (186, 64)
		Common::Point(0x00d4, 0x0044), // [28]  (212, 68)
		Common::Point(0x00ed, 0x0046), // [29]  (237, 70)
		Common::Point(0x0091, 0x0081), // [30]  (145, 129)
		Common::Point(0x00a8, 0x008b), // [31]  (168, 139)
		Common::Point(0x00bd, 0x008f), // [32]  (189, 143)
		Common::Point(0x00d7, 0x0093), // [33]  (215, 147)
		Common::Point(0x00f0, 0x0095), // [34]  (240, 149)
		Common::Point(0x0093, 0x00ca), // [35]  (147, 202)
		Common::Point(0x00aa, 0x00d4), // [36]  (170, 212)
		Common::Point(0x00bf, 0x00d8), // [37]  (191, 216)
		Common::Point(0x00d9, 0x00dc), // [38]  (217, 220)
		Common::Point(0x00f2, 0x00de), // [39]  (242, 222)
		Common::Point(0x0096, 0x0113), // [40]  (150, 275)
		Common::Point(0x00ad, 0x011d), // [41]  (173, 285)
		Common::Point(0x00c2, 0x0121), // [42]  (194, 289)
		Common::Point(0x00dc, 0x0125), // [43]  (220, 293)
		Common::Point(0x00f5, 0x0127), // [44]  (245, 295)
		Common::Point(0x009a, 0x015b), // [45]  (154, 347)
		Common::Point(0x00b1, 0x0165), // [46]  (177, 357)
		Common::Point(0x00c6, 0x0169), // [47]  (198, 361)
		Common::Point(0x00e0, 0x016d), // [48]  (224, 365)
		Common::Point(0x00f9, 0x016f), // [49]  (249, 367)
		// Column 2 (entries 50-74)
		Common::Point(0x010c, 0x003b), // [50]  (268, 59)
		Common::Point(0x0123, 0x0045), // [51]  (291, 69)
		Common::Point(0x0138, 0x0049), // [52]  (312, 73)
		Common::Point(0x0152, 0x004d), // [53]  (338, 77)
		Common::Point(0x016b, 0x004f), // [54]  (363, 79)
		Common::Point(0x010f, 0x0086), // [55]  (271, 134)
		Common::Point(0x0126, 0x0090), // [56]  (294, 144)
		Common::Point(0x013b, 0x0094), // [57]  (315, 148)
		Common::Point(0x0155, 0x0098), // [58]  (341, 152)
		Common::Point(0x016e, 0x009a), // [59]  (366, 154)
		Common::Point(0x0111, 0x00cf), // [60]  (273, 207)
		Common::Point(0x0128, 0x00d9), // [61]  (296, 217)
		Common::Point(0x013c, 0x00dd), // [62]  (316, 221)
		Common::Point(0x0156, 0x00e1), // [63]  (342, 225)
		Common::Point(0x016f, 0x00e3), // [64]  (367, 227)
		Common::Point(0x0114, 0x0118), // [65]  (276, 280)
		Common::Point(0x012b, 0x0122), // [66]  (299, 290)
		Common::Point(0x0140, 0x0126), // [67]  (320, 294)
		Common::Point(0x015a, 0x012a), // [68]  (346, 298)
		Common::Point(0x0173, 0x012c), // [69]  (371, 300)
		Common::Point(0x0118, 0x0163), // [70]  (280, 355)
		Common::Point(0x012f, 0x016d), // [71]  (303, 365)
		Common::Point(0x0144, 0x0171), // [72]  (324, 369)
		Common::Point(0x015e, 0x0175), // [73]  (350, 373)
		Common::Point(0x0177, 0x0177), // [74]  (375, 375)
		// Column 3 (entries 75-99)
		Common::Point(0x0188, 0x003f), // [75]  (392, 63)
		Common::Point(0x019f, 0x0049), // [76]  (415, 73)
		Common::Point(0x01b5, 0x004d), // [77]  (437, 77)
		Common::Point(0x01cf, 0x0051), // [78]  (463, 81)
		Common::Point(0x01e8, 0x0053), // [79]  (488, 83)
		Common::Point(0x018b, 0x008a), // [80]  (395, 138)
		Common::Point(0x01a2, 0x0094), // [81]  (418, 148)
		Common::Point(0x01b7, 0x0098), // [82]  (439, 152)
		Common::Point(0x01d1, 0x009c), // [83]  (465, 156)
		Common::Point(0x01ea, 0x009e), // [84]  (490, 158)
		Common::Point(0x018d, 0x00d3), // [85]  (397, 211)
		Common::Point(0x01a4, 0x00dd), // [86]  (420, 221)
		Common::Point(0x01b9, 0x00e1), // [87]  (441, 225)
		Common::Point(0x01d3, 0x00e5), // [88]  (467, 229)
		Common::Point(0x01ec, 0x00e7), // [89]  (492, 231)
		Common::Point(0x0190, 0x011c), // [90]  (400, 284)
		Common::Point(0x01a7, 0x0126), // [91]  (423, 294)
		Common::Point(0x01bc, 0x012a), // [92]  (444, 298)
		Common::Point(0x01d6, 0x012e), // [93]  (470, 302)
		Common::Point(0x01ef, 0x0130), // [94]  (495, 304)
		Common::Point(0x0194, 0x0164), // [95]  (404, 356)
		Common::Point(0x01ab, 0x016e), // [96]  (427, 366)
		Common::Point(0x01c0, 0x0172), // [97]  (448, 370)
		Common::Point(0x01da, 0x0176), // [98]  (474, 374)
		Common::Point(0x01f3, 0x0178), // [99]  (499, 376)
		// Column 4 (entries 100-124)
		Common::Point(0x0204, 0x0048), // [100] (516, 72)
		Common::Point(0x021b, 0x0052), // [101] (539, 82)
		Common::Point(0x0230, 0x0056), // [102] (560, 86)
		Common::Point(0x024a, 0x005a), // [103] (586, 90)
		Common::Point(0x0263, 0x005c), // [104] (611, 92)
		Common::Point(0x0207, 0x0093), // [105] (519, 147)
		Common::Point(0x021e, 0x009d), // [106] (542, 157)
		Common::Point(0x0233, 0x00a1), // [107] (563, 161)
		Common::Point(0x024d, 0x00a5), // [108] (589, 165)
		Common::Point(0x0266, 0x00a7), // [109] (614, 167)
		Common::Point(0x0209, 0x00dc), // [110] (521, 220)
		Common::Point(0x0220, 0x00e6), // [111] (544, 230)
		Common::Point(0x0237, 0x00ea), // [112] (567, 234)
		Common::Point(0x0251, 0x00ee), // [113] (593, 238)
		Common::Point(0x026a, 0x00f0), // [114] (618, 240)
		Common::Point(0x020c, 0x0125), // [115] (524, 293)
		Common::Point(0x0223, 0x012f), // [116] (547, 303)
		Common::Point(0x0238, 0x0133), // [117] (568, 307)
		Common::Point(0x0252, 0x0137), // [118] (594, 311)
		Common::Point(0x026b, 0x0139), // [119] (619, 313)
		Common::Point(0x0210, 0x016d), // [120] (528, 365)
		Common::Point(0x0227, 0x0177), // [121] (551, 375)
		Common::Point(0x023c, 0x017b), // [122] (572, 379)
		Common::Point(0x0256, 0x017f), // [123] (598, 383)
		Common::Point(0x026f, 0x0181), // [124] (623, 385)
	};
	/** SubColumn X-offsets for level-4 room-runner registration. */
	static constexpr int16 kSubColumnOffsetX[5]{
		0,
		23,
		46,
		69,
		94,
	};
	/** SubColumn Y-offsets for level-4 room-runner registration. */
	static constexpr int16 kSubColumnOffsetY[5]{
		0,
		7,
		11,
		14,
		17,
	};
	/** SubColumn Y-offsets for level-4 room background registration. */
	static constexpr int16 kBackgroundSubColumnOffsetY[5]{
		0,
		10,
		16,
		20,
		23,
	};
	/** Pedestal positions for the 20 compact zoombini pack slots in the opening. */
	static constexpr Common::Point kIntroSnoidPositions[20]{
		Common::Point(455, 423),
		Common::Point(432, 421),
		Common::Point(412, 420),
		Common::Point(395, 425),
		Common::Point(379, 418),
		Common::Point(365, 433),
		Common::Point(352, 412),
		Common::Point(340, 433),
		Common::Point(328, 418),
		Common::Point(314, 432),
		Common::Point(295, 421),
		Common::Point(279, 430),
		Common::Point(264, 437),
		Common::Point(259, 421),
		Common::Point(244, 432),
		Common::Point(226, 421),
		Common::Point(211, 427),
		Common::Point(195, 419),
		Common::Point(176, 423),
		Common::Point(158, 431),
	};
	/** Pedestal positions for the 16 normal-size zoombinis on the puzzle board. */
	static constexpr Common::Point kGameplaySnoidPositions[16]{
		Common::Point(504, 458),
		Common::Point(467, 453),
		Common::Point(428, 453),
		Common::Point(384, 454),
		Common::Point(344, 451),
		Common::Point(297, 454),
		Common::Point(270, 441),
		Common::Point(244, 453),
		Common::Point(217, 448),
		Common::Point(188, 453),
		Common::Point(160, 449),
		Common::Point(130, 455),
		Common::Point(103, 446),
		Common::Point(74, 454),
		Common::Point(50, 445),
		Common::Point(17, 453),
	};

	// -----------------------------------------------------------------------
	// Difficulty / setup
	// -----------------------------------------------------------------------

	/** Difficulty-specific counter step reached by the initial fill: 5, 2, 4, or 2. */
	int16 _initialCounterStep = 5;
	/** Number of active room slots: 25 at levels 1-3 and 125 at level 4. */
	int16 _roomSlotCount = 0;

	// -----------------------------------------------------------------------
	// These feature runners are assigned by @ref ZoombiniPuzzleHotel::loadFeatures()
	// or @ref ZoombiniPuzzleHotel::setupCloseView().
	// -----------------------------------------------------------------------
	/** Ulla runner reused for the opening, prompt, escalation, and completion animations. */
	ZmbFeature *_ullaAnimFeature = nullptr;
	/**
	 * Difficulty-specific opening label overlay from SCRB 11500-11502.
	 * Freed when the close view is installed.
	 */
	ZmbFeature *_openingLabelFeature = nullptr;
	/** Opening-room overlay runner for SCRB 11800. */
	ZmbFeature *_openingRoomFeature = nullptr;
	/** Per-slot room displays from SCRB 6013+slot or the level-4 9002 family. */
	ZmbFeature *_roomDisplayFeatures[125] = {};
	/** Per-slot platform and checkpoint features from SCRB 6038+slot. */
	ZmbFeature *_roomIconFeatures[125] = {};
	/** Per-slot level-4 room-background features from the SCRB 9007 family. */
	ZmbFeature *_roomBackgroundFeatures[125] = {};
	/** Hidden DRAW_ON_REG features that own the room drop targets. */
	ZmbFeature *_roomDropTargetFeatures[125] = {};
	/** Close-view board mask from SCRB 11503-11505. */
	ZmbFeature *_boardMaskFeature = nullptr;
	/** Per-slot boarded-room marker features. */
	ZmbFeature *_forbiddenMarkerFeatures[125] = {};
	/** Mistake-counter feature loaded from SCRB 6000 plus the current step. */
	ZmbFeature *_mistakeCounterFeature = nullptr;
	/** W-command offset cycling through room SCRBs 7000-7009. */
	int16 _debugRoomAnimCycle = 25;
	/** E-command offset cycling through room SCRBs 7010-7017. */
	int16 _debugRoomAnimCycleAlternate = 25;
	/** H/I level-2 empty-room restart state: 0=idle, 1=armed, 2=run, 7012=consumed. */
	int16 _debugRoomRestartState = 0;

	// -----------------------------------------------------------------------
	// Trait constraint grids
	// -----------------------------------------------------------------------
	/**
	 * Axis-1 constraints replicated across room slots at levels 1-3 and stored per Row at level 4.
	 * 0 = unset.
	 */
	int16 _axis1TraitConstraints[25] = {};
	/**
	 * Axis-2 constraints replicated across room slots at levels 2-3 and stored per Column at level 4.
	 * 0 = unset.
	 */
	int16 _axis2TraitConstraints[25] = {};
	/**
	 * Axis-3 constraint per SubColumn at level 4.
	 * Zero means unset.
	 */
	int16 _axis3TraitConstraints[5] = {};
	/** Debug-only bypass for the level 2-4 multi-axis validators. */
	bool _debugTraitBypass = false;

	/** Whether the level-3 generation snapshot is available to diagnostics. */
	bool _level3GenerationSnapshotValid = false;
	/** Axis-1 constraint snapshot captured before level-3 room constraints are cleared. */
	int16 _level3GeneratedAxis1Traits[25] = {};
	/** Axis-2 constraint snapshot captured before level-3 room constraints are cleared. */
	int16 _level3GeneratedAxis2Traits[25] = {};
	/** Number of active-pack Zoombinis matching each generated level-3 room. */
	int16 _level3RoomMatchCounts[25] = {};

	// -----------------------------------------------------------------------
	// Puzzle axis selection
	// -----------------------------------------------------------------------
	/** Hotel trait axis selected for grid axis 1. */
	TraitAxis _axis1TraitAxis = TraitAxis::kFeet00;
	/** Hotel trait axis selected for grid axis 2. */
	TraitAxis _axis2TraitAxis = TraitAxis::kFeet00;
	/** Hotel trait axis selected for grid axis 3 at level 4. */
	TraitAxis _axis3TraitAxis = TraitAxis::kFeet00;
	/** Number of distinct active-pack values for each @ref TraitAxis. */
	int16 _traitVariantCounts[kTraitAxisCount] = {};
	// -----------------------------------------------------------------------
	// Room grid state
	// -----------------------------------------------------------------------
	/**
	 * Per-slot room state: -1=boarded, 0=empty, and 1-6=accepted-Snoid display depth.
	 * The active prefix has 25 entries at levels 1-3 and 125 at level 4.
	 */
	int16 _roomSlotStates[125] = {};
	/** Random 0-3 marker variant for each generated boarded room, in scan order. */
	int16 _forbiddenMarkerVariants[8] = {};

	// -----------------------------------------------------------------------
	// Placement tracking
	// -----------------------------------------------------------------------
	/** Current target room slot (0-based). */
	int16 _targetRoomSlot = -1;
	/** Number of Zoombinis accepted into rooms. */
	int16 _acceptedSnoidCount = 0;
	/** Current mistake-counter step; terminal failure begins at step 12. */
	int16 _mistakeCounterStep = 1;
	/** Pack Snoid IDs that have already been accepted into a room. */
	Common::Array<uint16> _acceptedSnoidIds;
	/** Whether the current drag belongs to a level-4 accepted room occupant. */
	bool _acceptedSnoidDrag = false;
	/** Whether the fixed drag temporarily removed a room clip. */
	bool _acceptedSnoidDragHadConstraint = false;
	/** Aggregate click rectangle restored when the fixed drag ends. */
	Common::Rect _acceptedSnoidDragClickRect;
	/** Room clip restored when the fixed drag ends. */
	Common::Rect _acceptedSnoidDragConstraint;

	// -----------------------------------------------------------------------
	// Pending placement
	// -----------------------------------------------------------------------
	/** Dropped Snoid waiting to finish common arrival before Hotel grading. */
	ZmbSnoid *_pendingDropSnoid = nullptr;
	/** Grading result retained while @ref _pendingDropSnoid finishes arrival. */
	bool _pendingDropAccepted = false;
	/** Snoid currently owned by an accepted-door or rejected-door script. */
	ZmbSnoid *_placementScriptSnoid = nullptr;
	/** Final room position committed after an accepted entrance script. */
	Common::Point _acceptedRoomPosition;
	/** Visual constraint installed by event 15 for the active placement script. */
	Common::Rect _placementScriptRect;
	enum SnoidScriptPhase : byte {
		kSnoidScriptNone00 = 0,
		kSnoidScriptAcceptedEntrance01 = 1,
		kSnoidScriptRejectedDoor02 = 2
	};
	/** Current accepted/rejected Snoid script phase. */
	SnoidScriptPhase _placementScriptPhase = kSnoidScriptNone00;
	struct RejectReturnState {
		/** Snoid awaiting its return animation. */
		ZmbSnoid *snoid = nullptr;
		/** Body arrangement to restore before the return script. */
		int16 pendingBodyArrangement = 0;
	};
	/** Snoids waiting for their serialized rejection return. */
	Common::Array<RejectReturnState> _rejectReturnStates;

	// -----------------------------------------------------------------------
	// Puzzle state flags
	// -----------------------------------------------------------------------
	/** Whether the next non-boarded placement establishes the first room rule. */
	bool _firstPlacementPending = true;
	/** Whether the player may pick up and grade another Snoid room placement. */
	bool _roomPlacementInputEnabled = true;
	/** Whether a rejected-door or rejected-return animation is playing. */
	bool _rejectionAnimActive = false;
	// -----------------------------------------------------------------------
	// These state-machine completion flags are set by @ref ZoombiniPuzzleHotel::onFeatureAnimEvent().
	// -----------------------------------------------------------------------
	/**
	 * True while the distant opening animation owns the S1 completion trigger.
	 * Its live branch refreshes the guide timer and excludes all S2/S3 logic.
	 */
	bool _openingAnimActive = true;
	/** Whether S1 completion requested @ref ZoombiniPuzzleHotel::setupCloseView(). */
	bool _openingAnimCompleted = false;
	/** Whether the close-view Ulla prompt completed and the counter may start. */
	bool _guidePromptCompleted = false;
	/**
	 * Page-local next-frame latch for the counter-fill sequence.
	 * The sequence advances on the next frame without waiting for its timing slot to become idle.
	 */
	bool _initialCounterFillStepPending = false;
	/** Whether a rejection-owned counter animation completed. */
	bool _rejectionCounterAnimCompleted = false;
	/** Whether the current counter SCRB belongs to the rejection callback chain. */
	bool _rejectionCounterCallbackPending = false;
	/** Whether the Ulla completion or terminal-failure reaction completed. */
	bool _completionAnimCompleted = false;
	// -----------------------------------------------------------------------
	// Counter / reset
	// -----------------------------------------------------------------------
	/** Whether the player skipped the distant opening sequence. */
	bool _openingSkipped = false;
	/** Whether a click may currently skip the distant opening sequence. */
	bool _openingSkipEnabled = true;
	/** Number of terminal-failure sequences entered; also locks further input. */
	int16 _terminalFailureCount = 0;
	/** Whether level 4 still owes its post-freeze random draw and Ulla reaction sound. */
	bool _level4FailureReactionPending = false;

	/**
	 * Frame deadline for the terminal-failure render freeze.
	 * @ref ZoombiniPuzzleHotel::onPostRenderFrame() pauses interactive dispatch until this deadline,
	 * holding the terminal-failure animation for 60 frames.
	 */
	uint32 _failureFreezeDeadlineFrame = 0;

	// -----------------------------------------------------------------------
	// Ulla animation state
	// -----------------------------------------------------------------------
	/** Purpose of the animation stored in @ref _ullaAnimFeature. */
	UllaAnimPurpose _ullaAnimPurpose = kUllaAnimIntro00;
	/** Distant-opening SCRB variant in the 7000 family, from 0 through 6. */
	int16 _openingAnimVariant = 0;
	/** Whether opening variants 0 or 4 still require the close-view Ulla prompt. */
	bool _guidePromptPending = false;
	/** Frame baseline used for the delayed close-view Ulla prompt. */
	uint32 _guidePromptDelayStartFrame = 0;
};

} // End of namespace Mohawk

#endif
