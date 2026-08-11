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

#ifndef TWINE_MENU_H
#define TWINE_MENU_H

#include "twine/twine.h"
#include "twine/text.h"

namespace TwinE {

#define MAX_BUTTONS 10
#define PLASMA_WIDTH 320
#define PLASMA_HEIGHT 50
#define kDemoMenu 9999
#define kQuitEngine 9998

class BodyData;
class SpriteData;

class MenuSettings {
private:
	enum MenuSettingsType {
		// button number
		MenuSettings_CurrentLoadedButton = 0,
		// is used to calc the height where the first button will appear
		MenuSettings_NumberOfButtons = 1,
		MenuSettings_ButtonsBoxHeight = 2,
		MenuSettings_TextBankId = 3,

		MenuSettings_FirstButtonState,
		MenuSettings_FirstButton
	};

	int16 _settings[4 + MAX_BUTTONS * 2] {0};
	Common::String _buttonTexts[MAX_BUTTONS];
	int8 _activeButtonIdx = 0;

public:
	TextId getButtonTextId(int buttonIndex) const {
		return (TextId)_settings[MenuSettings_FirstButton + buttonIndex * 2];
	}

	void reset() {
		for (int32 i = 0; i < MAX_BUTTONS; ++i) {
			_buttonTexts[i] = "";
		}
		_settings[MenuSettings_NumberOfButtons] = 0;
		setButtonsBoxHeight(0);
		setActiveButton(0);
	}

	// used to calc the height where the first button will appear
	void setButtonsBoxHeight(int16 height) {
		_settings[MenuSettings_ButtonsBoxHeight] = height;
	}

	void setActiveButton(int16 buttonIdx) {
		_activeButtonIdx = buttonIdx;
		_settings[MenuSettings_CurrentLoadedButton] = buttonIdx;
	}

	void setActiveButtonTextId(TextId textIndex) {
		setButtonTextId(getActiveButton(), textIndex);
	}

	void setButtonTextId(int16 buttonIdx, TextId textIndex) {
		_settings[MenuSettings_FirstButton + buttonIdx * 2] = (int16)textIndex;
		_buttonTexts[buttonIdx].clear();
	}

	TextId getActiveButtonTextId() const {
		return getButtonTextId(getActiveButton());
	}

	int16 getActiveButtonState() const {
		return getButtonState(getActiveButton());
	}

	int16 getButtonState(int buttonIndex) const {
		return _settings[MenuSettings_FirstButtonState + buttonIndex * 2];
	}

	const char *getButtonText(Text *text, int buttonIndex);

	int16 getActiveButton() const {
		return _activeButtonIdx;
	}

	int16 getButtonBoxHeight() const {
		return _settings[MenuSettings_ButtonsBoxHeight];
	}

	int16 getButtonCount() const {
		return _settings[MenuSettings_NumberOfButtons];
	}

	void setTextBankId(TextBankId textBankIndex) {
		_settings[MenuSettings_TextBankId] = (int16)textBankIndex;
	}

	void addButton(TextId textId, int16 state = 0) {
		const int16 i = _settings[MenuSettings_NumberOfButtons];
		_settings[i * 2 + MenuSettings_FirstButtonState] = state;
		_settings[i * 2 + MenuSettings_FirstButton] = (int16)textId;
		++_settings[MenuSettings_NumberOfButtons];
	}

	void addButton(const char *text, int16 state = 0) {
		const int16 i = _settings[MenuSettings_NumberOfButtons];
		_settings[i * 2 + MenuSettings_FirstButtonState] = state;
		// will return the button index
		_settings[i * 2 + MenuSettings_FirstButton] = i;
		_buttonTexts[i] = text;
		++_settings[MenuSettings_NumberOfButtons];
	}
};

class Menu {
private:
	TwinEEngine *_engine;
	/** Hero behaviour menu entity */
	BodyData *_behaviourEntity = nullptr;
	/** Behaviour menu anim state */
	uint _behaviourAnimState[4]; // winTab
	/** Behaviour menu anim data pointer */
	AnimTimerDataStruct _behaviourAnimData[4];

	int32 _inventorySelectedColor = COLOR_BLACK;
	int32 _inventorySelectedItem = 0; // currentSelectedObjectInInventory

	/** Plasma Effect pointer to file content: RESS.HQR:51 */
	uint8 *_plasmaEffectPtr = nullptr;

	MenuSettings _giveUpMenuWithSaveState;
	MenuSettings _volumeMenuState;
	MenuSettings _saveManageMenuState;
	MenuSettings _giveUpMenuState;
	MenuSettings _mainMenuState;
	MenuSettings _newGameMenuState;
	MenuSettings _advOptionsMenuState;
	MenuSettings _optionsMenuState;
	MenuSettings _languageMenuState;

	// objectRotation
	int16 _itemAngle[NUM_INVENTORY_ITEMS];
	/** Behaviour menu move pointer */
	RealValue _moveMenu;

	/**
	 * Draws main menu button
	 * @param buttonId current button identification from menu settings
	 * @param dialText
	 * @param hover flag to know if should draw as a hover button or not
	 */
	void drawButtonGfx(const MenuSettings *menuSettings, const Common::Rect &rect, int32 buttonId, const char *dialText, bool hover);
	void plasmaEffectRenderFrame();
	/**
	 * Process the menu button draw
	 * @param data menu settings array
	 * @param mode flag to know if should draw as a hover button or not
	 */
	int16 drawButtons(MenuSettings *menuSettings, bool hover);
	/** Used to run the advanced options menu */
	int32 advoptionsMenu();
	/** Used to run the volume menu */
	int32 volumeOptions();
	int32 languageMenu();
	/** Used to run the save game management menu */
	int32 savemanageMenu();
	void drawInfoMenu(int16 left, int16 top, int16 width);
	Common::Rect calcBehaviourRect(int32 left, int32 top, HeroBehaviourType behaviour) const;
	bool isBehaviourHovered(int32 left, int32 top, HeroBehaviourType behaviour) const;
	void drawBehaviour(int32 left, int32 top, HeroBehaviourType behaviour, int32 angle, bool cantDrawBox);
	void drawListInventory(int32 left, int32 top);
	void prepareAndDrawBehaviour(int32 left, int32 top, int32 angle, HeroBehaviourType behaviour); // DrawComportement
	void drawBehaviourMenu(int32 left, int32 top, int32 angle); // DrawMenuComportement
	Common::Rect calcItemRect(int32 left, int32 top, int32 item, int32 *centerX = nullptr, int32 *centerY = nullptr) const;
	// draw the 2d sprite of the item
	void drawOneInventory(int32 left, int32 top, int32 item);

	void drawSpriteAndString(int32 left, int32 top, const SpriteData &spriteData, const Common::String &str, int32 color = COLOR_GOLD);

public:
	Menu(TwinEEngine *engine);
	~Menu();

	/**
	 * Process the plasma effect
	 * @param color plasma effect start color
	 */
	void processPlasmaEffect(const Common::Rect &rect, int32 color);

	void drawHealthBar(int32 left, int32 right, int32 top, int32 barLeftPadding, int32 barHeight);
	void drawCloverLeafs(int32 newBoxLeft, int32 boxRight, int32 top);
	void drawMagicPointsBar(int32 left, int32 right, int32 top, int32 barLeftPadding, int32 barHeight);
	void drawCoins(int32 left, int32 top);
	void drawKeys(int32 left, int32 top);

	/**
	 * Draw the rect lines without filling the area
	 * @param left start width to draw the button
	 * @param top start height to draw the button
	 * @param right end width to draw the button
	 * @param bottom end height to draw the button
	 */
	void drawRectBorders(int32 left, int32 top, int32 right, int32 bottom, int32 colorLeftTop = COLOR_79, int32 colorRightBottom = COLOR_73);
	void drawRectBorders(const Common::Rect &rect, int32 colorLeftTop = COLOR_79, int32 colorRightBottom = COLOR_73);
	/**
	 * Where the main menu options are processed
	 * @param menuSettings menu settings array with the information to build the menu options
	 * @return pressed menu button identification
	 */
	void menuDemo();
	int32 doGameMenu(MenuSettings *menuSettings);

	bool init();

	/** Used to run the main menu */
	EngineState run();

	/** Used to run the in-game give-up menu */
	int32 quitMenu();

	void inGameOptionsMenu();

	/** Used to run the options menu */
	int32 optionsMenu();

	/** Process hero behaviour menu */
	void processBehaviourMenu(bool behaviourMenu); // MenuComportement

	int32 newGameClassicMenu();

	/** Process in-game inventory menu */
	void inventory();
};

} // namespace TwinE

#endif
