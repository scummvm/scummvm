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

#include "common/events.h"

namespace HDB {

#define	CONFIG_MUSICVOL		"music_volume"
#define	CONFIG_SOUNDVOL		"sound_volume"
#define	CONFIG_MSTONE7		"hdb_memory_heap"
#define	CONFIG_MSTONE14		"lua_stack_offset"
#define	CONFIG_MSTONE21		"fmod_mix_timer"
#define	CONFIG_SOUNDCACHE	"sound_cache_max"
#define	CONFIG_GFXCACHE		"gfx_cache_max"
#define	CONFIG_CHEAT		"hypercheat"

#define	CONFIG_KEY_UP		"keyup"
#define	CONFIG_KEY_DOWN		"keydown"
#define	CONFIG_KEY_LEFT		"keyleft"
#define	CONFIG_KEY_RIGHT	"keyright"
#define	CONFIG_KEY_USE		"keyuse"
#define	CONFIG_VOICES		"voices"

#define	TITLE_DELAY1		2				// time to wait before OOH OOH
#define	TITLE_DELAY2		0.5				// time to spend doing OOH OOH
#define	TITLE_DELAY3		1				// time to wait before ending title

#define centerPic(x)	(kScreenWidth / 2 - x->_width / 2)

enum {
	kStarRedX = 70,
	kStarRedY = 20,
	kStarGreenX = 70,
	kStarGreenY = 100,
	kStarBlueX = 70,
	kStarBlueY = 180,

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
	kVortSaveY = 72,
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
	kMaxStars = 10,

	kOptionSPC = 16,
	kOptionLineSPC = 32
};

struct Star {
	int x, y, speed, anim, delay;

	Star() : x(0), y(0), speed(0), anim(0), delay(0) {}
};

class Menu {
public:

	Menu();
	~Menu();

	bool init();

	void readConfig();
	void writeConfig();

	bool startTitle();
	void drawTitle();
	void startMenu();
	void changeToMenu();	// Changing from Intro to Menu
	void drawMenu();
	void freeMenu();

	void processInput(int x, int y);	// this is where the items are clicked!
	void controlsInput(int x, int y);	// take mouse input and pass through to menu
	void controlsDraw();
	void drawNebula();
	void drawRocketAndSelections();		// draw the background stuff
	void drawSlider(int x, int y, int offset);
	void drawToggle(int x, int y, bool flag);
	void drawWarpScreen();
	void saveSong(SoundType song) {
		_resumeSong = song;
	}

	void fillSavegameSlots();

	void setMenuKey(int status) {
		_menuKey = status;
	}
	int getMenuKey() {
		return _menuKey;
	}

	Save _saveGames[kNumSaveSlots + 1];

	int _starWarp;
	int _titleCycle;
	uint32 _titleDelay;
	bool _titleActive;
	SoundType _resumeSong;	// the song that was playing before entering the Options screen

	Picture *_oohOohGfx;
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

	int _menuKey;
};

} // End of Namespace

#endif // !HDB_SOUND_H
