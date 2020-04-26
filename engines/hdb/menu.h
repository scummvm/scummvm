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
#include "hdb/ai.h"
#include "hdb/sound.h"

namespace HDB {

#define	CONFIG_MUSICVOL		"music_volume"
#define	CONFIG_SOUNDVOL		"sound_volume"
#define	CONFIG_MSTONE7		"hdb_memory_heap"
#define	CONFIG_MSTONE14		"lua_stack_offset"
#define	CONFIG_MSTONE21		"fmod_mix_timer"
#define	CONFIG_SOUNDCACHE	"sound_cache_max"
#define	CONFIG_GFXCACHE		"gfx_cache_max"
#define	CONFIG_CHEAT		"hypercheat"
#define	CONFIG_VOICES		"voices"

#define	TITLE_DELAY1		2				// time to wait before OOH OOH
#define	TITLE_DELAY2		0.5				// time to spend doing OOH OOH
#define	TITLE_DELAY3		1				// time to wait before ending title

#define centerPic(x)	(g_hdb->_screenWidth / 2 - x->_width / 2)

enum {
	kStarRedX = 70,
	kStarRedY = 20,
	kStarGreenX = 70,
	kStarGreenY = 100,
	kStarBlueX = 70,
	kStarBlueY = 180,

	kOptionSPC = 16,
	kOptionLineSPC = 32,

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

	// Platform-specific Constants
	int _menuX, _menuY;
	int _menuItemWidth;
	int _menuItemHeight;
	int _mResumeY;
	int _mQuitY;
	int _mOptionsY;
	int _mLoadY;
	int _mControlsY;
	int _menuExitXLeft;
	int _menuExitY;
	int _menuExitYTop;
	int _menuVortSaveY;
	int _menuVortSaveX;
	int _mRocketX;
	int _mRocketY;
	int _mRocketYBottom;
	int _mRocketEXHX;
	int _mRocketEXHX2;
	int _mTitleY;
	int _oohOhhX;
	int _oohOhhY;
	int _newGameX;
	int _newGameX2;
	int _modePuzzleY;
	int _modeActionY;
	int _optionsX;
	int _optionsY;
	int _vortSaveX;
	int _vortSaveTextX;
	int _vortSaveY;
	int _saveSlotX;
	int _saveSlotY;
	int _quitX;
	int _quitY;
	int _quitYesX1;
	int _quitYesX2;
	int _quitYesY1;
	int _quitYesY2;
	int _quitNoX1;
	int _quitNoX2;
	int _quitNoY1;
	int _quitNoY2;
	int _controlX;
	int _controlY;
	int _controlUpX;
	int _controlUpY;
	int _controlDownX;
	int _controlDownY;
	int _controlLeftX;
	int _controlLeftY;
	int _controlRightX;
	int _controlRightY;
	int _controlUseX;
	int _controlUseY;
	int _controlWidth;
	int _controlHeight;
	int _assignX;
	int _assignY;
	int _backoutX;
	int _backoutY;
	int _warpBackoutX;
	int _warpBackoutY;
	int _warpX;
	int _warpY;

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
	Picture *_screenshots1gfx, *_screenshots1agfx, *_screenshots2gfx, *_demoPlaqueGfx, *_handangoGfx;

	bool _menuActive, _optionsScrolling, _newgameActive, _sayHDB;
	int	_gamefilesActive, _clickDelay, _saveSlot, _optionsActive, _quitActive, _warpActive;
	int	_optionsScrollX, _optionsXV, _oBannerY;
	int	_nextScreen;

	Picture	*_contArrowUp, *_contArrowDown, *_contArrowLeft, *_contArrowRight, *_contAssign, *_warpPlaque;

	Picture	*_sliderLeft, *_sliderMid, *_sliderRight, *_sliderKnob;
	Picture	*_gCheckEmpty, *_gCheckOn, *_gCheckOff, *_gCheckLeft, *_gCheckRight;

	SoundType _introSong, _titleSong;

	Star _fStars[kMaxStars];

	int _menuKey;
};

} // End of Namespace

#endif // !HDB_SOUND_H
