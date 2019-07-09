/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef HDB_MENU_H
#define HDB_MENU_H

namespace HDB {

#define STARS_MONKEYSTONE_7			0xfe257d	// magic value in the config file for the unlocking of the Monkeystone secret #7
#define STARS_MONKEYSTONE_7_FAKE	0x11887e	// fake value that means it hasn't been unlocked
#define STARS_MONKEYSTONE_14		0x3341fe	// <same> for the Monkeystone #14
#define STARS_MONKEYSTONE_14_FAKE	0x1cefd0	// fake value that means it hasn't been unlocked
#define STARS_MONKEYSTONE_21		0x77ace3	// <same> for the Monkeystone #21
#define STARS_MONKEYSTONE_21_FAKE	0x3548fe	// fake value that means it hasn't been unlocked

enum {
	kMenuX = 48 * 8,
	kMenuY = 80,

	kMenuItemWidth = 88,
	kMenuItemHeight = 40,

	kMResumeY =	48 * 4,
	kMQuitY = 48 * 3,
	kMOptionsY = 48 * 2,
	kMLoadY = 48 * 1,
	kMControlsY = 270,

	kMenuExitY = kScreenHeight - 80,
	kMenuExitXLeft = 60,
	kMenuExitYTop = 48,
	kMenuVortSaveY = 220,
	kMenuVortSaveX = 76,

	kMRocketX = 18 * 8,
	kMRocketY = 48,
	kMRocketYBottom = 28 * 8,
	kMRocketEXHX = 6 * 8 - 4,
	kMRocketEXHX2 = 11 * 8 - 4,
	kMTitleY = 34 * 8,
	kOohOhhX = 256,
	kOohOhhY = 224,

	kNewGameX = 220,
	kNewGameX2 = kNewGameX + 50,

	kModePuzzleY = 130,
	kModeActionY = 220,

	kOptionsX = 8 * 26,
	kOptionsY = 8 * 10,

	kVortSaveX = 200 - 44,
	kVortSaveTextX = kVortSaveX + 40,
	kVortSaveTextY = 72,
	kSaveSlotX = 200,
	kSaveSlotY = 80,

	kQuitX = 0,
	kQuitY = 0,
	kQuitYesX1 = 30 * 8,
	kQuitYesX2 = 38 * 8 + 4,
	kQuitYesY1 = 50 * 8,
	kQuitYesY2 = 53 * 8 + 4,
	kQuitNoX1 = 41 * 8,
	kQuitNoX2 = 49 * 8 + 4,
	kQuitNoY1 = 50 * 8,
	kQuitNoY2 = 53 * 8 + 4,

	kControlX = 60,
	kControlY = 40,
	kControlUpX = 276 + kControlX,
	kControlUpY = 114 + kControlY,
	kControlDownX = 276 + kControlX,
	kControlDownY = 129 + kControlY,
	kControlLeftX = 396 + kControlX,
	kControlLeftY = 114 + kControlY,
	kControlRightX = 396 + kControlX,
	kControlRightY = 129 + kControlY,
	kControlUseX = 330 + kControlX,
	kControlUseY = 209 + kControlY,
	kControlWidth = 32,
	kControlHeight = 16,

	kAssignX = kControlX + 20 * 8,
	kAssignY = kControlY + 150,

	kBackoutX = 0,
	kBackoutY = kScreenHeight - 32,

	kWarpBackoutX = kBackoutX,
	kWarpBackoutY = kBackoutY,

	kWarpX = 214,
	kWarpY = 160,

	kScreenFade = 512,
	kNebulaCount = 7,
	kMaxStars = 10
};

struct Star {
	int x, y, speed, anim, delay;

	Star() : x(0), y(0), speed(0), anim(0), delay(0) {}
};

class Menu {
public:

	bool init();

	void startTitle();
	void startMenu();
	void freeMenu();

	void fillSavegameSlots();

	int _starWarp;
	int _titleCycle;
	uint32 _titleDelay;
	bool _titleActive;
	SoundType _resumeSong;	// the song that was playing before entering the Options screen

	Picture *_ohhOhhGfx;
	Picture *_titleScreen, *_titleLogo, *_hdbLogoScreen, *_menuBackoutGfx, *_controlButtonGfx, *_controlsGfx, *_menuBackspaceGfx;

	int _rocketY, _rocketYVel, _rocketEx;	// Rocket Vars
	Picture	*_rocketEx1, *_rocketEx2, *_rocketMain, *_rocketSecond;
	int	_rocketX;

	int	_nebulaX, _nebulaY, _nebulaYVel, _nebulaWhich;
	Picture *_nebulaGfx[kNebulaCount];

	Picture	*_newGfx, *_loadGfx, *_optionsGfx, *_quitGfx, *_resumeGfx, *_slotGfx;
	Picture	*_modePuzzleGfx, *_modeActionGfx, *_modeLoadGfx, *_modeSaveGfx, *_quitScreen;
	Tile *_vortexian[3];
	Picture	*_star[3], *_warpGfx;
	uint32 _quitTimer;
	Picture	*_starRedGfx[2], *_starGreenGfx[2], *_starBlueGfx[2], *_versionGfx;

	bool _menuActive, _optionsScrolling, _newgameActive, _sayHDB;
	int	_gamefilesActive, _clickDelay, _saveSlot, _optionsActive, _quitActive, _warpActive;
	int	_optionsScrollX, _optionsXV, _oBannerY;
	int	_nextScreen, _loadOrSave;

	Common::KeyCode _keyAssignUp, _keyAssignDown, _keyAssignLeft, _keyAssignRight, _keyAssignUse;
	Picture	*_contArrowUp, *_contArrowDown, *_contArrowLeft, *_contArrowRight, *_contAssign, *_warpPlaque;
//	void	ShowControl(int x, int y, short key);

	bool _waitingForKey;
	int	_whichKey;

	Picture	*_sliderLeft, *_sliderMid, *_sliderRight, *_sliderKnob;
	Picture	*_gCheckEmpty, *_gCheckOn, *_gCheckOff, *_gCheckLeft, *_gCheckRight;

	SoundType _introSong, _titleSong;

	Star _fStars[kMaxStars];
};

} // End of Namespace

#endif // !HDB_SOUND_H
