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
#include "pelrock/sound.h"

namespace Pelrock {

const int kQuestionMarkOffset = 3214046;
const int kInvLeftArrowOffset = 3215906;
const int kTransparentColor = 15;

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

static const char *inventorySounds[113] = {

	"HOJASZZZ.SMP", // 0 - Default leaf rustle
	"11ZZZZZZ.SMP", // 1 -
	"11ZZZZZZ.SMP",
	"11ZZZZZZ.SMP",
	"GLASS1ZZ.SMP", // 4 - Glass clink (brick)
	"11ZZZZZZ.SMP",
	"ELEC3ZZZ.SMP", // 6 - Electric zap
	"REMATERL.SMP", // 7 - Rematerialize
	"81ZZZZZZ.SMP", // 8 - (numbered SFX)
	"HOJASZZZ.SMP",
	"SSSHTZZZ.SMP", // 10 - Shushing
	"HOJASZZZ.SMP", // 11 - Default leaf rustle
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP", // 20
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP", // 30
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP", // 40
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP", // 50
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"BOTEZZZZ.SMP", // 60
	"BOTEZZZZ.SMP", // 61
	"BOTEZZZZ.SMP", // 62 - Bottle sound
	"BELCHZZZ.SMP", // 63 - Belch
	"BEAMZZZZ.SMP", // 64 - Beam/ray
	"ELVIS1ZZ.SMP", // 65 - Elvis impression
	"CAT_1ZZZ.SMP", // 66 - Cat sound
	"BOOOOOIZ.SMP", // 67 - Boing
	"DISCOSZZ.SMP", // 68 - Disco music
	"MONORLZZ.SMP", // 69 - Monorail
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"CARACOLA.SMP", // 73 - Seashell
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"WATER_2Z.SMP", // 76 - Water splash
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"EEEEKZZZ.SMP", // 79 - Shriek
	"REMATERL.SMP", // 80 - Rematerialize
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"ELVIS1ZZ.SMP", // 83 - Elvis impression
	"RIMSHOTZ.SMP", // 84 - Rimshot
	"HOJASZZZ.SMP",
	"WATER_2Z.SMP", // 86 - Water splash
	"MOTOSZZZ.SMP", // 87 - Motorcycle
	"HOJASZZZ.SMP",
	"TWANGZZZ.SMP", // 89 - Twang
	"HOJASZZZ.SMP",
	"QUAKE2ZZ.SMP", // 91 - Earthquake
	"HOJASZZZ.SMP",
	"SORBOZZZ.SMP", // 93 - Slurp
	"BOTEZZZZ.SMP", // 94 - Bottle sound
	"ELVIS1ZZ.SMP", // 95 - Elvis impression
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"LLAVESZZ.SMP", // 100 - Keys jingling
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"EVLLAUGH.SMP", // 104 - Evil laugh
	"HOJASZZZ.SMP",
	"BURROLZZ.SMP", // 106 - Donkey bray
	"HOJASZZZ.SMP",
	"TWANGZZZ.SMP", // 108
	"HOJASZZZ.SMP",
	"TWANGZZZ.SMP", // 110
	"ELVIS1ZZ.SMP", // 111 - Elvis impression
	"SEX3ZZZZ.SMP"  // 112 - Suggestive sound
};

class MenuManager {
public:
	MenuManager(Graphics::Screen *screen, PelrockEventManager *events, ResourceManager *res, SoundManager *sound);
	~MenuManager();
	void menuLoop();
	void drawScreen();
	void drawInventoryIcons();
	void loadMenu();
	byte _mainMenuPalette[768] = {0};

private:
	void checkMouseClick(int x, int y);
	bool selectInventoryItem(int i);
	void loadMenuTexts();
	void cleanUp();
	void drawButtons();
	void readButton(Common::File &alfred7, uint32 offset, byte *outBuffer[2], Common::Rect rect);
	MenuButton isButtonClicked(int x, int y);
	Graphics::Screen *_screen = nullptr;
	PelrockEventManager *_events = nullptr;
	ResourceManager *_res = nullptr;
	SoundManager *_sound = nullptr;
	byte *_mainMenu = nullptr;
	byte *_compositeBuffer = nullptr;

	Common::Rect _saveGameRect = Common::Rect(Common::Point(132, 186), 81, 34);
	byte *_saveButtons[2] = {nullptr};

	Common::Rect _loadGameRect = Common::Rect(Common::Point(133, 222), 80, 33);
	byte *_loadButtons[2] = {nullptr};

	Common::Rect _soundsRect = Common::Rect(Common::Point(134, 258), 77, 33);
	byte *_soundsButtons[2] = {nullptr};

	Common::Rect _exitToDosRect = Common::Rect(Common::Point(134, 293), 75, 30);
	byte *_exitToDosButtons[2] = {nullptr};

	Common::Rect _invLeft = Common::Rect(Common::Point(469, 88), 26, 37);
	byte *_inventoryLeftArrow[2] = {nullptr};

	Common::Rect _invRight = Common::Rect(Common::Point(463, 132), 26, 37);
	byte *_inventoryRightArrow[2] = {nullptr};

	Common::Rect _savesUp = Common::Rect(Common::Point(457, 189), 26, 24);
	byte *_savesUpArrows[2] = {nullptr};

	Common::Rect _savesDown = Common::Rect(Common::Point(450, 278), 26, 24);
	byte *_savesDownArrows[2] = {nullptr};

	Common::Rect _questionMarkRect = Common::Rect(Common::Point(217, 293), 31, 30);
	byte *_questionMark[2] = {nullptr};

	Common::Array<Common::StringArray> _menuTexts;
	// Temporary
	int _selectedInvIndex = 0;
	int _curInventoryPage = 0;
	Common::StringArray _menuText;
	Common::Array<Common::StringArray> _inventoryDescriptions;

	bool showButtons = true;
};

} // End of namespace Pelrock
#endif // PELROCK_MENU_H
