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
#include "graphics/managed_surface.h"
#include "graphics/screen.h"

#include "pelrock/events.h"
#include "pelrock/resources.h"
#include "pelrock/sound.h"

namespace Pelrock {

const uint32 kQuestionMarkOffset = 3214046;
const uint32 kInvLeftArrowOffset = 3215906;
const uint32 kSoundControlOffset = 3037008;
const uint32 kSoundMasterOffset = 	2662588;
const uint32 kSoundMusicOffset = 2664746;
const uint32 kSoundSfxOffset = 2667140;
const int kTransparentColor = 15;
// const int kTransparentColor = 195;
const int kSoundControlsTransparentColor = 195;
const uint32 kCreditsBackgroundOffset = 3271454;

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
	MASTER_LEFT_BUTTON,
	MASTER_RIGHT_BUTTON,
	SFX_LEFT_BUTTON,
	SFX_RIGHT_BUTTON,
	MUSIC_LEFT_BUTTON,
	MUSIC_RIGHT_BUTTON,
	NO_BUTTON
};

static const int kCreditsOrder[34] = {
	5, // PROGRAMACION
	 8, // Juan Jose Gil
	 20, // Jose Vicente Pons
	 24, // LuisFer Fernandez
	 22, // Fernando Perez
	 7, // GRAFICOS
	12, // Queral,
	14, // Ana maria polo
	18, // Juan Arocas
	16, // Gost
	26, // Astorga
	28, // Santi Sanz
	2, // Fernando Aparicio
	11,// INTRODUCCION
	12, // Queral,
	26, // Astorga
	28, // Santi Sanz
	9, // MUSICA
	6, // Rufino Acosta
	13, // GUION
	8, // Juan Jose Gil
	19, // DIALOGOS
	4, // Vicent raul arnau,
	8, // Juan Jose Gil,
	21, // PROBADORES
	0, //David Burgos
	10, // Alberto Leon
	1, // Carles Pons
	3, // Roman Pons
	25, // Andres Ruiz,
	27, // Juan Jose Ruiz
	23, // Marilo
	15, // PRODUCCION
	17 // DDM
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
	bool checkMouseClick(int x, int y);
	void showCredits();
	bool selectInventoryItem(int i);
	void loadMenuTexts();
	void cleanUp();
	void drawButtons();
	void readButton(byte *rawData, uint32 offset, byte *outBuffer[2], int w, int h);
	void readButton(Common::File &alfred7, uint32 offset, byte *outBuffer[2], Common::Rect rect);
	MenuButton isButtonClicked(int x, int y);
	Graphics::Screen *_screen = nullptr;
	PelrockEventManager *_events = nullptr;
	ResourceManager *_res = nullptr;
	SoundManager *_sound = nullptr;
	Graphics::ManagedSurface _mainMenu;
	Graphics::ManagedSurface _compositeBuffer;

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

	Common::Rect _masterVolumeLeftRect = Common::Rect(Common::Point(232, 252), 36, 28);
	Common::Rect _masterVolumeRightRect = Common::Rect(Common::Point(268, 252), 31, 28);

	Common::Rect _sfxVolumeLeftRect = Common::Rect(Common::Point(298, 252), 36, 28);
	Common::Rect _sfxVolumeRightRect = Common::Rect(Common::Point(334, 252), 31, 28);

	Common::Rect _musicVolumeLeftRect = Common::Rect(Common::Point(364, 252), 36, 28);
	Common::Rect _musicVolumeRightRect = Common::Rect(Common::Point(400, 252), 31, 28);


	byte *_soundControlArrowLeft[2] = {nullptr};
	byte *_soundControlArrowRight[2] = {nullptr};

	byte *_soundControlMasterIcon = nullptr;
	byte *_soundControlSfxIcon = nullptr;
	byte *_soundControlMusicIcon = nullptr;

	Graphics::ManagedSurface _masterSoundIcon;
	Graphics::ManagedSurface _sfxSoundIcon;
	Graphics::ManagedSurface _musicSoundIcon;

	float currentMasterVolumeScale = 1.0f;
	float currentSfxVolumeScale = 1.0f;
	float currentMusicVolumeScale = 1.0f;

	Common::Array<Common::StringArray> _menuTexts;
	// Temporary
	int _selectedInvIndex = 0;
	int _curInventoryPage = 0;
	Common::StringArray _menuText;
	Common::Array<Common::StringArray> _inventoryDescriptions;
	Common::Array<Common::Point> _inventorySlots;

	bool showButtons = true;
	bool _showSoundOptions = false;
};

} // End of namespace Pelrock
#endif // PELROCK_MENU_H
