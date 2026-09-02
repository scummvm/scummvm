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

#ifndef MOHAWK_ZOOMBINI_PAGES_DIALOG_OPTION_H
#define MOHAWK_ZOOMBINI_PAGES_DIALOG_OPTION_H

#include "mohawk/zoombini_pages/dialog_base.h"

namespace Mohawk {

/**
 * Options dialog for state management and gameplay preferences.
 *
 * The dialog owns the visual state of both ordinary and toggle buttons.
 * Toggle values are read from the engine configuration when their value is displayed
 * and written back when the corresponding button action is committed.
 * Navigation buttons open the other modal dialogs or finish the options
 * interaction through the common dialog result path.
 */
class ZoombiniDialogOptions : public ZoombiniDialog {
public:
	/** Create an options dialog using the current engine settings. */
	ZoombiniDialogOptions(MohawkEngine_Zoombini *vm);
	/** Release option button state and dialog features. */
	~ZoombiniDialogOptions() override;

	/** Load option labels, toggles, and navigation features. */
	void loadFeatures() override;

protected:
	/** Normal shape used when a toggle is enabled. */
	static constexpr uint16 kOptionDialogToggleTrueNormalShape = ZoombiniPage::kSystemShape0001_05_OptionsOnButtonNormal;
	/** Pressed shape used when a toggle is enabled. */
	static constexpr uint16 kOptionDialogToggleTruePressedShape = ZoombiniPage::kSystemShape0001_07_OptionsOnButtonPressed;
	/** Normal shape used when a toggle is disabled. */
	static constexpr uint16 kOptionDialogToggleFalseNormalShape = ZoombiniPage::kSystemShape0001_06_OptionsOffButtonNormal;
	/** Pressed shape used when a toggle is disabled. */
	static constexpr uint16 kOptionDialogToggleFalsePressedShape = ZoombiniPage::kSystemShape0001_07_OptionsOnButtonPressed;

	/** Read the engine option represented by button index @p bsIdx. */
	bool getOptionDialogToggleValue(uint16 bsIdx);

	/** Replace toggle shapes with the current on/off visual state. */
	void redToggleButtons_onPreRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);
	/** Restore the options dialog after toggle rendering. */
	void redToggleButtons_onPostRender(ZmbFeature *feature);
	/** Return the text rectangle for an ordinary option button. */
	Common::Rect redButtons_textRect(ZmbFeature *feature, uint32 bsIdx, ButtonState &bs, const Common::Rect &drawnRect);
	/** Return the text rectangle for a toggle option. */
	Common::Rect toggleButtons_textRect(ZmbFeature *feature, uint32 bsIdx, ToggleButtonState &bs, const Common::Rect &drawnRect);
	/** Return the keymap action represented by an options-dialog text key. */
	const char *getOptionDialogActionId(ZoombiniText::Key textKey) const;
	/** Return an options-dialog label with a current remapped shortcut. */
	Common::U32String getOptionDialogText(ZoombiniText::Key textKey) const;
	/** Truncate an options-dialog label with an ellipsis when it exceeds its rectangle. */
	Common::U32String truncateOptionDialogText(const Common::U32String &text,
											   const Common::Rect &textRect,
											   const ZoombiniGraphics::TextConf &tc) const;
	/** Render ordinary option labels through the options-dialog text filter. */
	void optionButtonTextRender(ZmbFeature *feature,
								Common::StableMap<uint32, ButtonState> &buttonStateMap,
								ButtonGetRectFunc textRectFunc,
								const ZoombiniGraphics::TextConf &tc);
	/** Render toggle option labels through the options-dialog text filter. */
	void optionToggleButtonTextRender(ZmbFeature *feature,
									  Common::StableMap<uint32, ToggleButtonState> &buttonStateMap,
									  ToggleButtonGetRectFunc textRectFunc,
									  const ZoombiniGraphics::TextConf &tc);
	/** Return the options title rectangle for the current release layout. */
	const Common::Rect &getOptionDialogTitleRect() const;
	/** Return the top coordinate for option text @p bsIdx. */
	uint16 getOptionDialogTextTop(uint32 bsIdx) const;
	/** Apply an ordinary navigation-button action. */
	void redButtons_onButtonAction(ZmbFeature *feature, uint32 bsIdx, ButtonState &bs);
	/** Commit an option toggle action to the engine configuration. */
	void toggleButtons_onButtonAction(ZmbFeature *feature, uint32 bsIdx, ToggleButtonState &bs);
	/** Handle pointer input for ordinary and toggle buttons. */
	ZmbEventHandleResult redToggleButtons_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);
	/** Handle keyboard shortcuts for ordinary and toggle buttons. */
	ZmbEventHandleResult redToggleButtons_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat);

	/** Prepare the long navigation buttons before rendering. */
	void longButtons_onPreRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);
	/** Restore the options dialog after long-button rendering. */
	void longButtons_onPostRender(ZmbFeature *feature);
	/** Apply a long navigation-button action. */
	void longButtons_onButtonAction(ZmbFeature *feature, uint32 bsIdx, ButtonState &bs);
	/** Handle pointer input for long navigation buttons. */
	ZmbEventHandleResult longButtons_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);
	/** Handle keyboard input for long navigation buttons. */
	ZmbEventHandleResult longButtons_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat);

	// MapRect & MapSave data
	enum OptionDialogButtonIdx : uint32 {
		/** Start a new journey. */
		kOptionDialogButton01_NewGame = 0,
		/** Open the load-game dialog. */
		kOptionDialogButton02_LoadGame = 1,
		/** Open the save-game dialog. */
		kOptionDialogButton03_SaveGame = 2,
		/** Request game exit. */
		kOptionDialogButton04_Quit = 3,
		/** Toggle sound effects. */
		kOptionDialogButton05_Sound = 4,
		/** Toggle background music. */
		kOptionDialogButton06_Music = 5,
		/** Toggle sticky mouse behavior. */
		kOptionDialogButton07_StickyMouse = 6,
		/** Toggle transition animations. */
		kOptionDialogButton08_Transitions = 7,
		/** Toggle touch sensitivity. */
		kOptionDialogButton09_TouchSense = 8,
		/** Toggle help voice playback. */
		kOptionDialogButton10_HelpAudio = 9,
		/** Close the options dialog. */
		kOptionDialogButton11_Okay = 10,
		/** Open the credits dialog. */
		kOptionDialogButton12_Credits = 11,
	};

	/** State of the ordinary options navigation buttons. */
	Common::StableMap<uint32, ButtonState> _redButtonStateMap;
	/** State of the on/off option toggles. */
	Common::StableMap<uint32, ToggleButtonState> _toggleButtonStateMap;

	/**
	 * Dialog layout rectangles are instance members because Common::Rect requires
	 * runtime construction and ScummVM prohibits global C++ constructors.
	 */
	/** Standard options-dialog title rectangle. */
	const Common::Rect _optionDialogTitleRect = Common::Rect(0x00FA, 0x003A, 0x017C, 0x0050);
	/** TLC options-dialog title rectangle. */
	const Common::Rect _optionDialogTlcTitleRect = Common::Rect(0x00B9, 0x003A, 0x01BD, 0x0050);
	/** Toggle-button rectangle used by the options layout. */
	const Common::Rect _optionDialogToggleRect = Common::Rect(0x00A6, 0x00BE, 0x01D1, 0x00D2);
	/** Legend rectangle for the enabled state. */
	const Common::Rect _optionDialogLegendOnRect = Common::Rect(0x00C5, 0x0140, 0x0104, 0x0159);
	/** Legend rectangle for the disabled state. */
	const Common::Rect _optionDialogLegendOffRect = Common::Rect(0x00C5, 0x0159, 0x0104, 0x016D);
	/** Left edge of standard options text. */
	static constexpr uint16 _optionDialogTextLeft = 0x00C4;
	/** Right edge of standard options text. */
	static constexpr uint16 _optionDialogTextRight = 0x01D6;
	/** Text top coordinates for the standard options layout. */
	static constexpr uint16 _optionDialogTextTops[8] = {0x0066, 0x007C, 0x0091, 0x00A6, 0x00DD, 0x00F3, 0x0108, 0x011D};
	/** Text top coordinates for the TLC options layout. */
	static constexpr uint16 _optionDialogTlcTextTops[10] = {0x005A, 0x006F, 0x0084, 0x0099, 0x00BE, 0x00D3, 0x00E8, 0x00FD, 0x0112, 0x0127};
};

} // End of namespace Mohawk

#endif
