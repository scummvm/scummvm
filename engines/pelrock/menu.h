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
#ifndef PELROCK_MENU_H
#define PELROCK_MENU_H

#include "graphics/font.h"
#include "graphics/screen.h"

#include "pelrock/events.h"
#include "pelrock/resources.h"

namespace Pelrock {

const int kQuestionMarkOffset = 3214046;
const int kInvLeftArrowOffset = 3215906;

const int kQuestionMarkPosX = 217;
const int kQuestionMarkPosY = 293;
const int kQuestionMarkWidth = 31;
const int kQuestionMarkHeight = 30;
const int kQuestionMarkStride = 30 * 31;

const int kInventoryArrowWidth = 26;
const int kInventoryArrowHeight = 37;
const int kInventoryArrowStride = kInventoryArrowWidth * kInventoryArrowHeight;

enum MenuButton {
	QUESTION_MARK_BUTTON,
	INVENTORY_PREV_BUTTON,
	INVENTORY_NEXT_BUTTON,
	SAVEGAME_PREV_BUTTON,
	SAVEGAME_NEXT_BUTTON,
	EXIT_MENU_BUTTON,
	SAVE_GAME_BUTTON,
	LOAD_GAME_BUTTON,
	SOUNDS_BUTTON,
	NO_BUTTON
};

class MenuManager {
public:
	MenuManager(Graphics::Screen *screen, PelrockEventManager *events, ResourceManager *res);
	~MenuManager();
	void menuLoop();
	void loadMenu();
	byte _mainMenuPalette[768] = {0};

private:
	void checkMouseClick(int x, int y);
	void loadMenuTexts();
	void tearDown();
	void drawButtons();
	void drawColoredText(Graphics::ManagedSurface *surface, const Common::String &text, int x, int y, int w, Graphics::Font *font);
	MenuButton isButtonClicked(int x, int y);
	Graphics::Screen *_screen = nullptr;
	PelrockEventManager *_events = nullptr;
	ResourceManager *_res = nullptr;
	byte *_mainMenu = nullptr;
	byte *_compositeBuffer = nullptr;
	byte *_inventoryLeftArrow[2] = { nullptr };
	byte *_inventoryRightArrow[2] = { nullptr };
	byte *_savesArrows[2] = { nullptr };
	byte *_questionMark[2] = {nullptr};
	Common::Array<Common::StringArray> _menuTexts;
	// Temporary
	int _selectedInvIndex = 0;
	int _curInventoryPage = 0;
	Common::StringArray _menuText;
	Common::Array<Common::StringArray> _inventoryDescriptions;
};

} // End of namespace Pelrock
#endif // PELROCK_MENU_H
