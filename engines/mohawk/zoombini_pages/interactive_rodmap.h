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

#ifndef MOHAWK_ZOOMBINI_PAGES_INTERACTIVE_RODMAP_H
#define MOHAWK_ZOOMBINI_PAGES_INTERACTIVE_RODMAP_H

#include "mohawk/zoombini_pages/interactive_base.h"

namespace Mohawk {

/**
 * Interactive route map page used between puzzle visits.
 *
 * The RodMap presents route and page features, tracks the current journey or
 * practice selection, and turns the selected map target into the next page
 * transition. It also owns the page-local hover overlays and the practice
 * pack preparation used when the player enters a puzzle without journey data.
 *
 * The map is intentionally an interactive page rather than a puzzle: it does
 * not consume puzzle chances and its feature callbacks are concerned with
 * selection, hover rendering, labels, and route statistics.
 */
class ZoombiniInteractiveRodMap : public ZoombiniInteractive {
public:
	/** Create the route map page. */
	ZoombiniInteractiveRodMap(MohawkEngine_Zoombini *vm);
	/** Release map features and page-local resources. */
	~ZoombiniInteractiveRodMap() override;
	bool canOpenSaveLoadDialog() const override { return true; }

	/** Initialize the current journey/practice map state. */
	void open() override;
	/** Select the map background bitmap for the detected release. */
	void setBackgroundBitmap() override;
	/** Load map, route, page-icon, and control features. */
	void loadFeatures() override;
	/** Return the original journey-map script-sound range priority. */
	const ZoombiniPage::ScriptSoundPriorityRanges &getScriptSoundPriorityRanges() const override;
	/** Advance map hover and animation state by one engine frame. */
	void onAnimFrame() override;

	/** Update hovered page and map controls from mouse movement. */
	ZmbEventHandleResult onMouseMove(const Common::Point &absPos, const Common::Point &relPos) override;
	/** Handle keyboard navigation and activation of the selected map control. */
	ZmbEventHandleResult onKeyDown(const Common::KeyState &kbd, bool kbdRepeat) override;

protected:
	/** RodMap actions accepted by the built-in debug console and keyboard dispatcher. */
	enum class BuiltinDebugAction {
		kInvalid,
		kAdd,
		kRemove,
		kXfer
	};
	static constexpr const char *kBuiltinDebugActionAdd = "add";
	static constexpr const char *kBuiltinDebugActionRemove = "remove";
	static constexpr const char *kBuiltinDebugActionXfer = "xfer";
	/** Run one RodMap built-in debug action from @c page @c builtin_debug. */
	bool debugDoBuiltinDebugCommand(int argc, const char **argv, Common::String &output) override;
	/** Describe the RodMap built-in debug actions exposed by the debugger. */
	Common::String debugGetBuiltinDebugCommandHelp() const override;
	/** Handle RodMap built-in debug keys while the serialized debug flag is enabled. */
	ZmbEventHandleResult onDebugKeyDown(const Common::KeyState &kbd) override;
	/** Parse one console-facing RodMap action name. */
	static BuiltinDebugAction parseBuiltinDebugAction(const Common::String &action);
	/** Run one typed RodMap debug action with an optional XFER selector. */
	bool runBuiltinDebugAction(BuiltinDebugAction action, char argument, Common::String &output);
	/** Start the practice-mode debug XFER selected by a letter from A through P. */
	bool runBuiltinXferSelector(char selector, Common::String &output);

	/** Enable or disable practice mode and refresh the map controls. */
	void setPracticeMode(bool setPracticeMode);
	/** Toggle between journey mode and practice mode. */
	void togglePracticeMode();
	/** Rebuild every map feature whose output depends on the current mode. */
	void refreshModeDependentFeatures(bool clearHover);
	/** Select the requested practice difficulty from the level legend. */
	void selectPracticeLevel(ZmbFeature *feature, uint16 selectedLevel);
	/** Load the controls that are visible in the route-map panel. */
	void loadRodmapPanelFeatures();
	/** Release the route-map panel features before rebuilding them. */
	void unloadRodmapPanelFeatures();
	/** Restart a map feature after its release-specific state changes. */
	void restartRodmapFeature(ZmbFeature *feature);
	/** Rebuild route and page-icon state from the current game state. */
	void refreshRouteAndPageFeatures();
	/** Redraw the route labels for the current language and map layout. */
	void redrawRouteNames();
	/** Update the page currently highlighted by the mouse or keyboard. */
	void updateHoveredPage(ZoombiniPageType pageType);
	/** Clear the hovered-page overlay and optionally reload page icons. */
	void clearHoveredPage(bool reloadPageIcons);
	/** Return whether the route-map entry at the given index can be selected. */
	bool isPageHoverable(uint32 pageIndex);
	/** Return whether practice mode may leave the map through the page type. */
	bool isPracticeExitAvailable(ZoombiniPageType pageType) const;
	/** Read the completed difficulty for a page from a saved state. */
	static uint16 getPuzzleLevel(const ZmbStateFile &state, uint16 pageFlagIndex);

	/**
	 * Fill the active pack with the selected debug count of random-trait Zoombinis for practice mode.
	 * Generates occupied entries with random trait values 1-5
	 * for each of the four traits (hair, eye, nose, feet), and clears their names.
	 * Called before navigating to any puzzle page in practice mode
	 * so that the destination puzzle always receives a full pack.
	 */
	void generatePracticePack();

	/** 1000: Patch page shapeid on hovered */
	void patchPageShape1000_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);
	/** 1000: Run page */
	ZmbEventHandleResult runPage1000_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);
	/** 1001: Patch route shapeid to match the route's level */
	void patchRouteShape1001_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);
	/** 1005: Page text box appears only after page icon has been hovered */
	bool drawAfterPageIconHover1005_preRender(ZmbFeature *feature);
	/**
	 * 1005: Gate shape blitting on hover state.
	 * Shape blitting and text drawing share the current hover rectangle gate.
	 * This custom renderer keeps both operations under that gate.
	 */
	ZmbRenderResult renderAfterPageIconHover1005(ZmbFeature *feature);
	/** 1005: Select text of the page */
	void textPageName1005_postRender(ZmbFeature *feature);
	/** 1006: Select shape of menu button when clicked */
	void optionButton1006_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);
	/** 1006: PostAnimation of option button */
	void optionButton1006_postRender(ZmbFeature *feature);
	/** Draw the release-specific label inside the option button. */
	void optionButton1006_renderTlcLabel();
	/** Update the hover state of the release-specific option label. */
	void optionButton1006_updateTlcHover(const Common::Point &absPos);
	/** 1006: Clicked on menu button */
	ZmbEventHandleResult optionButton1006_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);
	/** 1004: (Practice mode only) Change shape of current selected level */
	void patchSelectedLevelShape1004_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);
	/** 1004: Draw text of the level legend */
	void textLegend1004_postRender(ZmbFeature *feature);
	/** 1004: (Practice mode only) Clicked on level */
	ZmbEventHandleResult legendLevel1004_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);
	/** 1004: (Practice mode only) Key down on level */
	ZmbEventHandleResult legendLevel1004_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat);
	/** 1002: Draw text of the journey stat */
	void textJourneyStat1002_postRender(ZmbFeature *feature);
	/** 1003: Combobox which selects Practice vs Journey */
	void drawComboBox1003_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);
	/** 1003: Clicked on Practice/Journey Combobox */
	ZmbEventHandleResult selectMode1003_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);
	/** Virtual Feature: Draw route names on the map */
	void textRouteNames_postRender(ZmbFeature *feature);

	/** Return the SCRB resource used for the page-name hover tooltip. */
	int16 getHoverTooltipScrbId() const;
	/** Return the release-specific SCRB resource used by the option button. */
	int16 getOptionButtonScrbId() const;
	/** Return the release-specific SCRB resource used by the level legend. */
	int16 getLevelLegendScrbId() const;
	/** Return the map rectangle used to draw one route name. */
	const Common::Rect &getRouteNameRect(uint32 routeIndex) const;
	/** Return the clickable rectangle for one difficulty level. */
	const Common::Rect &getLevelLegendClickRect(uint32 levelIndex) const;

	// Constant
	/** Page-local background, bitmap, and SCRB resource IDs. */
	enum PageResourceId : int16 {
		kResBackground300 = 300,

		kResBitmapShape1000 = 1000,

		kResScrbPageIcon1000 = 1000,
		kResScrbRoute1001 = 1001,
		kResScrbJourneyStats1002 = 1002,
		kResScrbUsModeCombobox1003 = 1003,
		kResScrbEuropeLevelLegend1003 = 1003,
		kResScrbUsLevelLegend1004 = 1004,
		kResScrbEuropePageNameHover1004 = 1004,
		kResScrbUsPageNameHover1005 = 1005,
		kResScrbEuropeMenuButton1005 = 1005,
		kResScrbUsMenuButton1006 = 1006,
		// Z1-20U/TLC v2.0 release only: hover state for SCRB 1006.
		kResScrbMenuButtonHover1007 = 1007,
	};

	enum ShapeId : uint16 {
		kResShapeBridge01 = 1,
		kResShapeTunnels02 = 2,
		kResShapePizza03 = 3,
		kResShapeBcOne04 = 4,
		kResShapeFerry05 = 5,
		kResShapeLilly06 = 6,
		kResShapeSlides07 = 7,
		kResShapeFleens08 = 8,
		kResShapeHotel09 = 9,
		kResShapeNet10 = 10,
		kResShapeBcTwo11 = 11,
		kResShapeCaves12 = 12,
		kResShapeSmoke13 = 13,
		kResShapeMaze14 = 14,
		kResShapeTown15 = 15,
		kResShapePicker16 = 16,

		kResShapeRouteBigBadHungryP0_17 = 17,
		kResShapeRouteBigBadHungryP1_18 = 18,
		kResShapeRouteBigBadHungryP2_19 = 19,
		kResShapeRouteBigBadHungryP3_20 = 20,
		kResShapeRouteWhosBayouP0_21 = 21,
		kResShapeRouteWhosBayouP1_22 = 22,
		kResShapeRouteWhosBayouP2_23 = 23,
		kResShapeRouteWhosBayouP3_24 = 24,
		kResShapeRouteDeepDarkForestP0_25 = 25,
		kResShapeRouteDeepDarkForestP1_26 = 26,
		kResShapeRouteDeepDarkForestP2_27 = 27,
		kResShapeRouteDeepDarkForestP3_28 = 28,
		kResShapeRouteMontDespairP0_29 = 29,
		kResShapeRouteMontDespairP1_30 = 30,
		kResShapeRouteMontDespairP2_31 = 31,
		kResShapeRouteMontDespairP3_32 = 32,

		kShapeOptionButtonPressed = 92,
		kShapeOptionButtonNormal = 93,

		kShapeComboBoxRedCircle111 = 111,
		kShapeComboBoxOutline112 = 112,
	};

	/** Page type whose map icon is currently highlighted. */
	ZoombiniPageType _lastHoveredPageType = ZoombiniPageType::kNone;

	/**
	 * Immutable layout rectangles are instance members because Common::Rect
	 * requires runtime construction and ScummVM prohibits global C++ constructors.
	 */
	/** Screen rectangles for the four journey route labels. */
	const Common::Rect _routeNameRects[4] = {
		Common::Rect(15, 247, 140, 275),
		Common::Rect(214, 47, 384, 62),
		Common::Rect(327, 414, 439, 444),
		Common::Rect(507, 193, 587, 223),
	};
	/** Screen rectangles for the localized European route labels. */
	const Common::Rect _europeRouteNameRects[4] = {
		Common::Rect(10, 150, 170, 275),
		Common::Rect(214, 47, 384, 62),
		Common::Rect(327, 390, 439, 480),
		Common::Rect(490, 195, 610, 253),
	};
	/** Text keys for the four route labels. */
	static constexpr ZoombiniText::Key _routeNameTextKey[4] = {
		ZoombiniText::kRoute1,
		ZoombiniText::kRoute2,
		ZoombiniText::kRoute3,
		ZoombiniText::kRoute4,
	};

	/** Palette entries used by the four difficulty legend items. */
	static constexpr uint16 _levelLegendPalettes[4] = {
		0x00EC,
		0x00EA,
		0x00E8,
		0x00EE,
	};
	// Clickable Pos & Rect
	/** Clickable rectangles for the US difficulty legend. */
	const Common::Rect _levelLegendClickRects[4] = {
		Common::Rect(0x01D0, 0x018B, 0x0264, 0x0199),
		Common::Rect(0x01D0, 0x0199, 0x0264, 0x01A8),
		Common::Rect(0x01D0, 0x01A8, 0x0264, 0x01B6),
		Common::Rect(0x01D0, 0x01B6, 0x0264, 0x01C6),
	};
	/** Clickable rectangles for the European difficulty legend. */
	const Common::Rect _europeLevelLegendClickRects[4] = {
		Common::Rect(449, 395, 597, 409),
		Common::Rect(449, 409, 597, 424),
		Common::Rect(449, 424, 597, 438),
		Common::Rect(449, 438, 597, 454),
	};
	/** Map coordinates used as the centers of the sixteen page icons. */
	static constexpr Common::Point _pageClickPoints[16] = {
		{Common::Point(0x003C, 0x0190)},
		{Common::Point(0x00AA, 0x0157)},
		{Common::Point(0x00A0, 0x0105)},
		{Common::Point(0x00DE, 0x0139)},
		{Common::Point(0x00EC, 0x010D)},
		{Common::Point(0x00EA, 0x0092)},
		{Common::Point(0x011E, 0x0052)},
		{Common::Point(0x016A, 0x00B9)},
		{Common::Point(0x0141, 0x0159)},
		{Common::Point(0x01AB, 0x0160)},
		{Common::Point(0x01E4, 0x0145)},
		{Common::Point(0x01BA, 0x00D3)},
		{Common::Point(0x01EA, 0x00B6)},
		{Common::Point(0x01E7, 0x005F)},
		{Common::Point(0x021C, 0x007D)},
		{Common::Point(0x0242, 0x0041)},
	};
	/** Hit rectangles derived from the page-icon centers at page load. */
	Common::Rect _pageClickRects[16];
	/** Page types corresponding to the sixteen page-icon positions. */
	static constexpr ZoombiniPageType _pageClickTypes[16] = {
		ZoombiniPageType::kPicker,
		ZoombiniPageType::kBridge,
		ZoombiniPageType::kTunnels,
		ZoombiniPageType::kPizza,
		ZoombiniPageType::kBasecamp1,
		ZoombiniPageType::kFerry,
		ZoombiniPageType::kLilly,
		ZoombiniPageType::kSlides,
		ZoombiniPageType::kFleens,
		ZoombiniPageType::kHotel,
		ZoombiniPageType::kNet,
		ZoombiniPageType::kBasecamp2,
		ZoombiniPageType::kCaves,
		ZoombiniPageType::kSmoke,
		ZoombiniPageType::kMaze,
		ZoombiniPageType::kTown,
	};
	/** Shape IDs corresponding to the sixteen page-icon positions. */
	static constexpr uint16 _pageClickShapes[16] = {
		kResShapePicker16,
		kResShapeBridge01,
		kResShapeTunnels02,
		kResShapePizza03,
		kResShapeBcOne04,
		kResShapeFerry05,
		kResShapeLilly06,
		kResShapeSlides07,
		kResShapeFleens08,
		kResShapeHotel09,
		kResShapeNet10,
		kResShapeBcTwo11,
		kResShapeCaves12,
		kResShapeSmoke13,
		kResShapeMaze14,
		kResShapeTown15,
	};

	/** Hit rectangles for the journey/practice mode selector. */
	const Common::Rect _modeSelectClickRects[2] = {
		Common::Rect(23, 131, 197, 152),
		Common::Rect(23, 153, 197, 175),
	};
	/** Text rectangle used by the TLC-specific option-button label. */
	const Common::Rect _tlcOptionButtonTextRect = Common::Rect(0x01E8, 0x0018, 0x021E, 0x0024);

	/** Indexes of controls owned by the route-map page. */
	enum RodmapButtonIdx {
		kRodmapButton_OptionDialog = 0,
	};

	// Option Button SCRB does not have two hotspots, so normal hsId and pressed hsId are equal.
	/** Button sound resource used by the route-map option control. */
	ZmbResource soundResId = ZmbResource(ZmbResource::kSystem, kSysResSound0999_ButtonSFX);
	/** Pressed and normal visual state for the option button. */
	ButtonState _optionButtonState = ButtonState(soundResId, 0, 0, kShapeOptionButtonNormal, kShapeOptionButtonPressed);
	/** Feature that renders and handles the page icons. */
	ZmbFeature *_pageIconFeature = nullptr;
	/** Feature that renders the route segments. */
	ZmbFeature *_routeSegmentsFeature = nullptr;
	/** Feature that renders the hovered page name. */
	ZmbFeature *_hoverNameFeature = nullptr;
	/** Feature that renders and handles the option button. */
	ZmbFeature *_optionButtonFeature = nullptr;
	/** Feature that renders and handles the difficulty legend. */
	ZmbFeature *_levelLegendFeature = nullptr;
	/** Feature that renders the journey statistics. */
	ZmbFeature *_journeyStatFeature = nullptr;
	/** Feature that selects journey mode or practice mode. */
	ZmbFeature *_modeComboFeature = nullptr;
	/** Virtual feature that draws the route names. */
	ZmbFeature *_routeNamesFeature = nullptr;
	/** Whether the TLC-specific option-button label is hovered. */
	bool _optionButtonTlcHovered = false;
	/** Frame at which the current page hover tooltip expires. */
	uint32 _hoverTimeoutFrame = 0;
	/** Most recent absolute mouse position received by the map. */
	Common::Point _lastMouseAbsPos;
	/** Whether _lastMouseAbsPos contains a valid input position. */
	bool _hasLastMouseAbsPos = false;
	/** Number of Zoombinis generated by the practice-pack debug controls. */
	int16 _builtinPracticePackCount = 16;
	/** Whether Shift+T armed the one-letter debug XFER selector. */
	bool _builtinXferSelectorArmed = false;

	/**
	 * Map of route keys (17 ~ 32) to its level.
	 * Value of 0 means the route is not yet visited.
	 * Value of 1 ~ 4 means the route level is 1 to 4.
	 */
	Common::StableMap<uint16, int16> _pageRouteLevelMap;
	/**
	 * Map of page-icon shapes (1 ~ 16) to their visibility state.
	 * The Picker is normally hidden and only appears while it is hovered.
	 */
	Common::StableMap<uint16, int16> _pageNodeLevelMap;
	/** Build the route-to-difficulty lookup from the current save state. */
	void buildPageRouteLevelMap();
};

} // End of namespace Mohawk

#endif
