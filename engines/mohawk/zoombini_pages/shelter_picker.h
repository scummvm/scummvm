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

#ifndef MOHAWK_ZOOMBINI_PAGES_SHELTER_PICKER_H
#define MOHAWK_ZOOMBINI_PAGES_SHELTER_PICKER_H

#include "mohawk/zoombini_pages/shelter_base.h"

namespace Mohawk {

/**
 * Zoombini picker shelter page.
 *
 * The Picker either opens the saved-game selection flow or lets the player
 * generate and arrange the active pack. It owns the trait selectors, cave
 * mark controls, preview runners, and the final Go/Map decision that hands
 * the active pack to a route page.
 */
class ZoombiniShelterPicker : public ZoombiniShelter {
public:
	/** Select the interaction mode used by the picker. */
	enum PickerMode {
		/** Open the load dialog only when a saved game exists. */
		kPickerMode_LoadGame,
		/** Select Zoombinis. */
		kPickerMode_SelectZoombinis,
	};

	/** Create the picker page. */
	ZoombiniShelterPicker(MohawkEngine_Zoombini *vm);
	/** Stop pending picker voice and release picker features. */
	~ZoombiniShelterPicker() override;

	/** Select load-game or Zoombini-generation state. */
	void open() override;
	/** Select picker background music. */
	void setBackgroundMusic() override;
	/** Select the picker background bitmap. */
	void setBackgroundBitmap() override;
	/** Load picker controls, previews, and cave-mark features. */
	void loadFeatures() override;
	/** Return the original Picker script-sound range priority. */
	const ZoombiniPage::ScriptSoundPriorityRanges &getScriptSoundPriorityRanges() const override;
	/** Describe the Picker debugger subcommand. */
	Common::String debugGetPageCommandHelp() const override;
	/** Handle a Picker debugger subcommand from the console's page command. */
	bool debugDoPageCommand(int argc, const char **argv, Common::String &output) override;
	/** Describe the Picker built-in debug actions. */
	Common::String debugGetBuiltinDebugCommandHelp() const override;
	/** Run one Picker built-in debug action. */
	bool debugDoBuiltinDebugCommand(int argc, const char **argv, Common::String &output) override;

protected:
	/** Picker actions accepted by the built-in debug console. */
	enum class BuiltinDebugAction {
		kInvalid,
		kShiftGenerate,
		kWaveCycle,
		kUpdateHelp
	};
	static constexpr const char *kBuiltinDebugActionShiftGenerate = "shift-generate";
	static constexpr const char *kBuiltinDebugActionWaveCycle = "wave-cycle";
	static constexpr const char *kBuiltinDebugActionUpdateHelp = "update-help";
	/** Parse one console-facing Picker action name. */
	static BuiltinDebugAction parseBuiltinDebugAction(const Common::String &action);
	/** Run one typed Picker built-in debug action. */
	bool runBuiltinDebugAction(BuiltinDebugAction action, Common::String &output);
	/** Current Picker mode: saved-game loading or active-pack generation. */
	PickerMode _mode;

	/**
	 * Callbacks for the single picker UI runner.
	 *
	 * The original registers one callback-only runner with SCRB id 0, so that runner owns no
	 * hotspots at all: its pre-render merges the dirty rectangles of whatever UI state changed,
	 * and its post-render blits the right panel and then the trait matrix directly out of each
	 * group's own image resource. Reproduce that here. The shape-selection state still lives in
	 * the existing button state maps, so the selectors run against page-owned scratch hotspot
	 * arrays that only this page reads, never against the runner.
	 */
	/** Rebuild all three scratch hotspot arrays from their layout constants. */
	void pickerUI_resetHotspots();
	/** Report the dirty regions of the callback-only Picker UI runner. */
	bool pickerUI_onPreRender(ZmbFeature *feature);
	/** Draw the prepared Picker UI groups onto the current screen. */
	ZmbRenderResult pickerUI_onRender(ZmbFeature *feature);
	/** Complete the callback-only Picker UI render pass. */
	void pickerUI_onPostRender(ZmbFeature *feature);
	/** Dispatch a pointer click to the Picker UI groups under the pointer. */
	ZmbEventHandleResult pickerUI_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);
	/** Blit every prepared hotspot in @p hotspots from @p imgResource. */
	void pickerUI_drawGroup(const Common::Array<ZmbHotspot> &hotspots, ZmbResource imgResource);

	/** Select the current trait shapes for the matrix hotspots. */
	void pickerMatrix_onPreRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);
	/** Render the trait matrix group. */
	void pickerMatrix_onPostRender(ZmbFeature *feature);
	/** Apply a trait selection and update its button state. */
	void pickerMatrix_onButtonAction(ZmbFeature *feature, uint32 bsIdx, StickyButtonState &bs);
	/** Handle a click on a trait matrix selector. */
	ZmbEventHandleResult pickerMatrix_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);

	/** Select the current state of the right-panel buttons. */
	void pickerButtons_onPreRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);
	/** Render the right-panel button group. */
	void pickerButtons_onPostRender(ZmbFeature *feature);
	/** Apply a right-panel button action and update its state. */
	void pickerButtons_onButtonAction(ZmbFeature *feature, uint32 bsIdx, ButtonState &bs);
	/** Handle a click on a right-panel button. */
	ZmbEventHandleResult pickerButtons_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);
	/** Handle keyboard activation of a right-panel button. */
	ZmbEventHandleResult pickerButtons_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat);
	/** Finish keyboard activation of a right-panel button. */
	ZmbEventHandleResult pickerButtons_onKeyUp(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat);
	/** Mark Picker UI controls for rematerialization after a state change. */
	void requestPickerControlRematerialization();

	/** Select preview-part shapes for the generated Zoombini. */
	void zoombiniPreview_onPreRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);
	/** Handle a click on the generated Zoombini preview. */
	ZmbEventHandleResult zoombiniPreview_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);

	/** Render the one-time load-dialog overlay used on the first Picker visit. */
	ZmbRenderResult oneTimeLoadDialog_onRenderShape(ZmbFeature *feature);

	/** Process Picker-specific keyboard input before shared interactive shortcuts. */
	ZmbEventHandleResult onKeyDown(const Common::KeyState &kbd, bool kbdRepeat) override;
	/** Update Picker hover state and cursor feedback. */
	ZmbEventHandleResult onMouseMove(const Common::Point &absPos, const Common::Point &relPos) override;
	/** Dispatch a Picker click to the active controls or dragged Zoombini. */
	ZmbEventHandleResult onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) override;
	/** Rapid-fill empty seats when Dice receives a secondary click. */
	ZmbEventHandleResult onRButtonDown(const Common::Point &absPos, const Common::Point &relPos) override;
	/** Consume release for a secondary click captured by Dice. */
	ZmbEventHandleResult onRButtonUp(const Common::Point &absPos, const Common::Point &relPos) override;
	/** Resolve a dragged Zoombini release on a seat or the cave mark. */
	void endDrag(const Common::Point &dropPos) override;

	/** Randomize the preview and optionally fill every empty seat. */
	void randomizeTraitSelection(bool fillEmptySeats);
	/** Register the wave and boat SCRB runners that are not already loaded. */
	void registerWaveBoatFeatures();
	/** Preserve or advance the serialized wave and boat animation state, then apply it. */
	void updateWaveBoatAnimationState(bool preserveAnimationState);
	/** Return true when every seat is filled or the 625-generation cap is reached. */
	bool isGenerationComplete() const;
	/** Apply the synthetic counter prefix used by Shift+Generate. */
	bool debugApplySyntheticGeneratePrefix(Common::String &output);
	/** Return true when the nonempty Picker is full or the global generation cap is reached. */
	bool isDepartureReady() const;
	/** Reroll the preview traits and refresh the Go button from the resulting occupancy. */
	void applyDiceReroll(bool fillEmptySeats);
	/** Select the normal or rapid-fill Dice shapes from current input state. */
	void updateDiceButtonVisual(ZmbFeature *feature);
	/** Return whether a trait value can be generated in the current game state. */
	bool isZoombiniTraitGeneratable(ZmbTrait trait) const;
	/** Generate and assign a localized name for the preview Zoombini. */
	void generateZoombiniName();
	/** Apply the original bounded trait reroll and return whether it found a usable combination. */
	bool randomizePreviewTraits(bool randomizeAll);
	/** Reconcile pending Go state with the current seat occupancy. */
	void updatePendingGoTransition();
	/** Return the first empty Picker seat index, or -1 when every seat is occupied. */
	int16 findFirstEmptySeat() const;
	/** Compact the generated Zoombinis into their canonical seat positions. */
	void repackSeatPositions();
	/** Apply the cave-drop removal of one seated Zoombini and restore it as the preview. */
	void removeSeatedZoombini(ZmbSnoid *snoid);

	/** Page debugger subcommand that generates Zoombinis into every empty seat. */
	static constexpr const char *kPageCommandAutoseat = "autoseat";
	/** Page debugger subcommand that returns every seated Zoombini to the preview. */
	static constexpr const char *kPageCommandAutounseat = "autounseat";
	/** Return the full usage text for the Picker generation commands. */
	Common::String debugGetPickerCommandsHelp() const;
	/**
	 * Generate Zoombinis into every empty picker seat through the Shift+dice
	 * rapid fill path.
	 * @param output Diagnostic result text for the page debugger.
	 * @return Number generated, or -1 when the command could not run.
	 */
	int16 debugGenerateZoombinisIntoEmptySeats(Common::String &output);
	/**
	 * Remove every seated picker Zoombini through the cave-drop removal path.
	 * @param output Diagnostic result text for the page debugger.
	 * @return Number removed, or -1 when the command could not run.
	 */
	int16 debugUnseatAllZoombinis(Common::String &output);

	/** Start the Picker departure sequence after the active pack is complete. */
	void onGoButtonActivated() override;
	/** Refresh Go and Map controls from the current Picker state. */
	void updateDepartureState() override;
	/** Handle an attempted Go action while the active pack is incomplete. */
	void onDisabledGoButtonActivated() override;
	/** Open the route map from the Picker. */
	void onMapButtonActivated() override;
	/** Save Picker state before opening the route map. */
	void saveStateBeforeMapTransition() override;

	/** Return whether the current layout can display another Picker voice prompt. */
	bool hasPickerRoomForVoicePrompt() const;
	/** Return the voice played after the Picker introductory video. */
	int16 getAfterVideoVoiceSoundId();
	/** Return the voice played when departure cannot yet begin. */
	int16 getNoDepartureVoiceSoundId();
	/** Start a Picker voice with its semantic volume category and return its engine-owned sound handle. */
	Audio::SoundHandle *playPickerVoice(int16 soundId, Audio::Mixer::SoundType soundType);
	/** Start and track a voice that is deferred until Picker space is available. */
	void playPendingPickerVoice(int16 soundId);
	/** Stop and clear the deferred Picker voice. */
	void stopPendingPickerVoice();
	/** Select and start the entry narration after initial scene materialization. */
	void startPickerEntryNarration();

	/**
	 * Load Zoombini snoids from the active pack data.
	 * Load only active occupied entries.
	 * Assign each loaded Snoid to the first eligible runner within the seat hit radius.
	 *
	 * @param pack The active pack to load from.
	 * @return Number of loaded zoombini snoids.
	 */
	int16 loadZoombinisFromPack(ZmbStateActivePack &pack);

	// Constant
	/** Page-local Picker backgrounds, controls, previews, and sounds. */
	enum PageResourceId : int16 {
		kResBackground4000 = 4000,

		kResBitmapShapes4100_BackObjects = 4100,
		kResBitmapShapes4200_Buttons = 4200,
		kResBitmapShapes4300_ZoombiniPreview = 4300,
		kResBitmapShapes4400_PickerMatrix = 4400,

		kResNode1000_WalkNetwork = 1000,

		kResScrb4100_BackObjects = 4100,
		kResScrb4101_Star = 4101,
		kResScrb4102_Star = 4102,
		kResScrb4103_Star = 4103,
		/** Wave and boat background SCRBs, both self-animated through LOOP_ANIM. */
		kResScrb4104_Waves = 4104,
		kResScrb4105_Boat = 4105,
		kResScrb4106_RockShape = 4106,
		kResScrb4107_RockShape = 4107,
		kResScrb4108_RockShape = 4108,
		kResScrb4109_RockShape = 4109,
		kResScrb4110_CaveMark = 4110,

		// Sound resources.
		kResSound1000_PressMatrixButton = 1000,
		kResSound1004_ReleaseMatrixButton = 1004,
		kResSound1005_PressGenerateButton = 1005,
		kResSound1006_PressDiceButton = 1006,
		kResSound1007_RemoveZoombini = 1007,
		kResSound1008_AllZoombinisGenerated = 1008,
		kResSound30001_Isle = 30001,

		// Sound ranges used by getScriptSoundPriority().
		kResSoundRange1000_PressMatrixButton = 1000,
		kResSoundRange1007_RemoveZoombini = 1007,

		kResMidi30001_Isle = 30001,
	};

	enum Shape4200Id {
		kShape4200_01_GenerateButtonDisabled = 1,
		kShape4200_02_GenerateButtonNormal = 2,
		kShape4200_03_GenerateButtonPressed = 3,
		kShape4200_04_DiceButtonNormal = 4,
		kShape4200_05_DiceButtonPressed = 5,
		kShape4200_06_DiceArrowButtonNormal = 6,
		kShape4200_07_DiceArrowButtonPressed = 7,
		kShape4200_08_GoButtonDisabled = 8,
		kShape4200_09_GoButtonNormal = 9,
		kShape4200_10_GoButtonPressed = 10,
		kShape4200_11_MapButtonNormal = 11,
		kShape4200_12_MapButtonPressed = 12,
		kShape4200_13_NameBox = 13,
		kShape4200_14_GenerateButtonHover = 14,  // Z1-20U/TLC v2.0 release only
		kShape4200_15_DiceButtonHover = 15,      // Z1-20U/TLC v2.0 release only
		kShape4200_16_DiceArrowButtonHover = 16, // Z1-20U/TLC v2.0 release only
		kShape4200_17_GoButtonHover = 17,        // Z1-20U/TLC v2.0 release only
		kShape4200_18_MapButtonHover = 18,       // Z1-20U/TLC v2.0 release only
	};

	enum Shape4300Id {
		kShape4300_01_PreviewBody = 1,
		kShape4300_02_PreviewHair1,
		kShape4300_03_PreviewHair2,
		kShape4300_04_PreviewHair3,
		kShape4300_05_PreviewHair4,
		kShape4300_06_PreviewHair5,
		kShape4300_07_PreviewEye1,
		kShape4300_08_PreviewEye2,
		kShape4300_09_PreviewEye3,
		kShape4300_10_PreviewEye4,
		kShape4300_11_PreviewEye5,
		kShape4300_12_PreviewNose1,
		kShape4300_13_PreviewNose2,
		kShape4300_14_PreviewNose3,
		kShape4300_15_PreviewNose4,
		kShape4300_16_PreviewNose5,
		kShape4300_17_PreviewFeet1,
		kShape4300_18_PreviewFeet2,
		kShape4300_19_PreviewFeet3,
		kShape4300_20_PreviewFeet4,
		kShape4300_21_PreviewFeet5,
	};

	// MapRect & MapSave data
	/**
	 * Immutable layout rectangles are instance members because Common::Rect
	 * requires runtime construction and ScummVM prohibits global C++ constructors.
	 */
	/** Authored hit rectangles for the twenty trait matrix cells. */
	const Common::Rect _pickerMatrixRects[20] = {
		Common::Rect(0x0003, 0x0130, 0x002A, 0x015A),
		Common::Rect(0x002A, 0x0130, 0x0051, 0x015A),
		Common::Rect(0x0051, 0x0130, 0x0078, 0x015A),
		Common::Rect(0x0078, 0x0130, 0x009F, 0x015A),
		Common::Rect(0x009F, 0x0130, 0x00C6, 0x015A),
		Common::Rect(0x0003, 0x015C, 0x002A, 0x0186),
		Common::Rect(0x002A, 0x015C, 0x0051, 0x0186),
		Common::Rect(0x0051, 0x015C, 0x0078, 0x0186),
		Common::Rect(0x0078, 0x015C, 0x009F, 0x0186),
		Common::Rect(0x009F, 0x015C, 0x00C6, 0x0186),
		Common::Rect(0x0003, 0x0188, 0x002A, 0x01B2),
		Common::Rect(0x002A, 0x0188, 0x0051, 0x01B2),
		Common::Rect(0x0051, 0x0188, 0x0078, 0x01B2),
		Common::Rect(0x0078, 0x0188, 0x009F, 0x01B2),
		Common::Rect(0x009F, 0x0188, 0x00C6, 0x01B2),
		Common::Rect(0x0003, 0x01B4, 0x002A, 0x01DE),
		Common::Rect(0x002A, 0x01B4, 0x0051, 0x01DE),
		Common::Rect(0x0051, 0x01B4, 0x0078, 0x01DE),
		Common::Rect(0x0078, 0x01B4, 0x009F, 0x01DE),
		Common::Rect(0x009F, 0x01B4, 0x00C6, 0x01DE),
	};
	/** Press/release state for trait matrix selectors keyed by button ID. */
	Common::StableMap<uint32, StickyButtonState> _matrixButtonStateMap;
	/** Hit rectangles for trait matrix selectors keyed by button ID. */
	Common::HashMap<uint32, Common::Rect> _matrixButtonRectMap;
	/** Number of trait values shown in each matrix row. */
	static constexpr uint32 kMatrixColumns = 5u;

	static constexpr Common::Point _previewTraitOffsets[21] = {
		Common::Point(0x16, 0x17),                    // Body
		Common::Point(0x19, 0x1E),                    // Hair1
		Common::Point(0x1C, 0x1D),                    // Hair2
		Common::Point(0x13, 0x1B),                    // Hair3
		Common::Point(0x0A, 0x1F),                    // Hair4
		Common::Point(0x1D, 0x1E),                    // Hair5
		Common::Point(0x0F, 0x0B),                    // Eye1
		Common::Point(0x07, 0x0B),                    // Eye2
		Common::Point(0x11, 0x0B),                    // Eye3
		Common::Point(0x18, 0x09),                    // Eye4
		Common::Point(0x18, 0x06),                    // Eye5
		Common::Point(0x07, static_cast<int8>(0xFE)), // Nose1
		Common::Point(0x07, static_cast<int8>(0xFE)), // Nose2
		Common::Point(0x07, static_cast<int8>(0xFE)), // Nose3
		Common::Point(0x07, static_cast<int8>(0xFE)), // Nose4
		Common::Point(0x07, static_cast<int8>(0xFE)), // Nose5
		Common::Point(0x17, static_cast<int8>(0xEA)), // Feet1
		Common::Point(0x18, static_cast<int8>(0xEA)), // Feet2
		Common::Point(0x0D, static_cast<int8>(0xEC)), // Feet3
		Common::Point(0x0F, static_cast<int8>(0xEB)), // Feet4
		Common::Point(0x17, static_cast<int8>(0xE9)), // Feet5
	};

	enum PickerButtonHotspotIndex {
		kHotspotGenerateButtonNormal = 0,
		kHotspotDiceButtonNormal = 1,
		kHotspotGenerateButtonPressed = 2,
		kHotspotDiceButtonPressed = 3,
		kHotspotNameBox = 4,
	};

	enum PickerButtonsIndex {
		kPickerButtons_Generate = 0,
		kPickerButtons_Dice,
	};

	enum PickerWaveBoatAnimationState {
		kPickerWaveBoatBothRunning = 0,
		kPickerWaveBoatBothStopped,
		kPickerWaveOnlyRunning,
		kPickerBoatOnlyRunning,
	};

	/** Hit rectangle for the Generate button. */
	const Common::Rect _generateButtonRect = Common::Rect(0x00CD, 0x0130, 0x0104, 0x0156);
	/** Hit rectangle for the generated Zoombini preview. */
	const Common::Rect _previewZoombiniRect = Common::Rect(0x00CD, 0x015B, 0x0111, 0x01A4);
	/** Hit rectangle for the generated Zoombini name box. */
	const Common::Rect _nameBoxRect = Common::Rect(0x00C9, 0x01A3, 0x011F, 0x01B5);
	/** Hit rectangle for the Dice button. */
	const Common::Rect _diceButtonRect = Common::Rect(0x00CD, 0x01B8, 0x0104, 0x01DE);

	/** Hit rectangle for the Go button. */
	const Common::Rect _goButtonClickRect = Common::Rect(0x0258, 0x01B9, 0x027F, 0x01DE);
	/** Hit rectangle for the Map button. */
	const Common::Rect _mapButtonClickRect = Common::Rect(0x0258, 0x0193, 0x027F, 0x01B8);
	/** Hit rectangle for the Help button. */
	const Common::Rect _helpButtonClickRect = Common::Rect(0x0258, 0x016D, 0x027F, 0x0192);

	/**
	 * Embark priority order: seat indices checked when selecting which snoid to animate.
	 * These seats are in the upper-right corner, near the ladder.
	 *
	 * When 16 snoids are on the boat (0 remaining on picker), the Go button
	 * animates one snoid climbing the ladder. The engine checks seats in this
	 * priority order: if seat 11 is empty, check 12; if 12 is empty, check 6; etc.
	 */
	static constexpr int16 kEmbarkOrder[4]{
		11,
		12,
		6,
		7,
	};

	/** Embark destination point on the boat. */
	static constexpr Common::Point _embarkDestination = Common::Point(544, 264);

	/**
	 * Frame stagger between each embarking snoid.
	 * 60 Frames between each snoid's start
	 */
	static constexpr uint32 kEmbarkStagger = 60;

	/** Cave mark registration point. */
	static constexpr Common::Point _caveMarkRegPoint = Common::Point(172, 226);

	/** Radius used for picker click-zone hit testing. */
	static constexpr int16 kPickerClickZoneRadius = 60;

	/** Maximum number of deferred rapid-fill Snoids released together by Go. */
	static constexpr uint16 kRapidFillGoBurstSize = 4;

	enum ZoombiniPreviewIndex {
		kHotspotPreviewBody = 0,
		kHotspotPreviewHair,
		kHotspotPreviewEye,
		kHotspotPreviewNose,
		kHotspotPreviewFeet,
	};

	static constexpr Common::Point _zoombiniSeatPoints[16] = {
		{Common::Point(0x021E, 0x01BE)},
		{Common::Point(0x01F9, 0x01BF)},
		{Common::Point(0x01D2, 0x01C3)},
		{Common::Point(0x01A9, 0x01C0)},
		{Common::Point(0x017C, 0x01C2)},
		{Common::Point(0x0156, 0x01C3)},
		{Common::Point(0x020A, 0x0192)},
		{Common::Point(0x01E8, 0x0198)},
		{Common::Point(0x01BC, 0x01A0)},
		{Common::Point(0x0193, 0x019D)},
		{Common::Point(0x016C, 0x019D)},
		{Common::Point(0x01F2, 0x0168)},
		{Common::Point(0x01CF, 0x016F)},
		{Common::Point(0x01AA, 0x0172)},
		{Common::Point(0x0185, 0x0175)},
		{Common::Point(0x0160, 0x0176)},
	};

	/** Press/release state for right-panel buttons keyed by button ID. */
	Common::StableMap<uint32, ButtonState> _pickerButtonStateMap;
	/** Hit rectangles for right-panel buttons keyed by button ID. */
	Common::HashMap<uint32, Common::Rect> _pickerButtonRectMap;
	/**
	 * The single callback-only picker UI runner, covering every Picker control and preview.
	 * The aliases below keep the shared button helpers bound to that one owner.
	 */
	/** Shared callback-only feature alias used by the matrix group. */
	ZmbFeature *_pickerUIFeature = nullptr;
	/** Shared callback-only feature alias used by the matrix group. */
	ZmbFeature *_pickerMatrixFeature = nullptr;
	/** Shared callback-only feature alias used by the right-panel buttons. */
	ZmbFeature *_pickerButtonsFeature = nullptr;
	/** Shared callback-only feature alias used by the Zoombini preview. */
	ZmbFeature *_previewFeature = nullptr;

	/**
	 * Page-owned scratch hotspots, one array per UI group.
	 * The shared button-shape selectors write the shape to draw into these, and
	 * @ref ZoombiniShelterPicker::pickerUI_onRender() blits each array from its own resource.
	 * They are deliberately not attached to the runner, which owns no hotspots.
	 */
	/** Scratch hotspots for the trait matrix group. */
	Common::Array<ZmbHotspot> _matrixHotspots;
	/** Scratch hotspots for the right-panel button group. */
	Common::Array<ZmbHotspot> _buttonHotspots;
	/** Scratch hotspots for the Zoombini preview group. */
	Common::Array<ZmbHotspot> _previewHotspots;
	/** Immutable Go/Map hotspot templates owned by the Picker UI runner. */
	Common::Array<ZmbHotspot> _goMapBaseHotspots;
	/** Per-frame Go/Map hotspots owned by the Picker UI runner. */
	Common::Array<ZmbHotspot> _goMapHotspots;
	/** Immutable Help hotspot templates owned by the Picker UI runner. */
	Common::Array<ZmbHotspot> _helpBaseHotspots;
	/** Per-frame Help hotspots owned by the Picker UI runner. */
	Common::Array<ZmbHotspot> _helpHotspots;

	/** Wave background animation, omitted in less-action mode. */
	ZmbFeature *_wavesFeature = nullptr;
	/** Boat and water background animation, omitted in less-action mode. */
	ZmbFeature *_boatFeature = nullptr;

	/** Materialized Zoombini used by the trait preview. */
	ZmbSnoid _previewSnoid;
	/** Whether Shift currently selects Dice rapid fill. */
	bool _shiftRapidFillActive = false;
	/** Whether the pending Dice animation should execute rapid fill. */
	bool _diceRapidFillPending = false;
	/** Whether Dice captured the current right-button press. */
	bool _diceRightButtonCaptured = false;
	/** True on the first visit to the picker this game session. */
	bool _isFirstVisit = false;
	/** Whether Go waits for the last generated Zoombini walk to finish. */
	bool _pendingGoRequiresWalkCompletion = false;
	/** Whether the required final walk completion has been observed. */
	bool _pendingGoObservedWalkCompletion = false;
	/** Deferred Picker voice resource ID, or zero when none is queued. */
	int16 _pendingPickerVoiceSoundId = 0;
	/** Whether the deferred Picker voice owns an active sound handle. */
	bool _pendingPickerVoiceSoundHasHandle = false;
	/** Whether the standard lifecycle has requested Picker entry narration. */
	bool _pickerEntryNarrationPending = false;
	/** Handle for the deferred Picker voice. */
	Audio::SoundHandle _pendingPickerVoiceSoundHandle;

	/** Snoids whose walk completion may release a pending Go transition. */
	Common::Array<ZmbSnoid *> _pendingGoWalkers;

	/** Runner IDs created by the most recent Shift+Dice rapid fill. */
	Common::Array<uint16> _rapidFillSnoidIds;

	/**
	 * Seat-to-snoid mapping. Index is seat position (0-15), value is pointer to snoid.
	 * Updated when snoids are generated or removed.
	 */
	ZmbSnoid *_seatToSnoid[16] = {nullptr};

	/**
	 * Find a snoid at or near the given seat position.
	 * Used for embark animation to locate snoids by their seat.
	 */
	ZmbSnoid *findSnoidAtSeat(int16 seatIdx);

	/** Release a pending Go transition after its tracked walkers finish. */
	void updatePendingGoWalkCompletion();
	/** Release the bounded rapid-fill burst when Go begins. */
	void releaseRapidFillBurstForGo();
};

} // End of namespace Mohawk

#endif
