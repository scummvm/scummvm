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

#ifndef ULTIMA4_INTRO_H
#define ULTIMA4_INTRO_H

#include "ultima/ultima4/events/controller.h"
#include "ultima/ultima4/game/menu.h"
#include "ultima/ultima4/core/observer.h"
#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/gfx/imageview.h"
#include "ultima/ultima4/game/textview.h"
#include "ultima/ultima4/map/tileview.h"

namespace Ultima {
namespace Ultima4 {

class IntroObjectState;
class Tile;

/**
 * Binary data loaded from the U4DOS title.exe file.
 */
class IntroBinData {
public:
	const static int INTRO_TEXT_OFFSET;
	const static int INTRO_MAP_OFFSET;
	const static int INTRO_FIXUPDATA_OFFSET;
	const static int INTRO_SCRIPT_TABLE_SIZE;
	const static int INTRO_SCRIPT_TABLE_OFFSET;
	const static int INTRO_BASETILE_TABLE_SIZE;
	const static int INTRO_BASETILE_TABLE_OFFSET;
	const static int BEASTIE1_FRAMES;
	const static int BEASTIE2_FRAMES;
	const static int BEASTIE_FRAME_TABLE_OFFSET;
	const static int BEASTIE1_FRAMES_OFFSET;
	const static int BEASTIE2_FRAMES_OFFSET;

	IntroBinData();
	~IntroBinData();

	bool load();

	Std::vector<MapTile> _introMap;
	unsigned char *_sigData;
	unsigned char *_scriptTable;
	Tile **_baseTileTable;
	unsigned char *_beastie1FrameTable;
	unsigned char *_beastie2FrameTable;
	Std::vector<Common::String> _introText;
	Std::vector<Common::String> _introQuestions;
	Std::vector<Common::String> _introGypsy;

private:
	// disallow assignments, copy contruction
	IntroBinData(const IntroBinData &);
	const IntroBinData &operator=(const IntroBinData &);
};


/**
 * Controls the title animation sequences, including the traditional
 * plotted "Lord British" signature, the pixelized fade-in of the
 * "Ultima IV" game title, as well as the other more simple animated
 * features, followed by the traditional animated map and "Journey
 * Onward" menu, plus the xU4-specific configuration menu.
 *
 * @todo
 * <ul>
 *      <li>make initial menu a Menu too</li>
 *      <li>get rid of mode and switch(mode) statements</li>
 *      <li>get rid global intro instance -- should only need to be accessed from u4.cpp</li>
 * </ul>
 */
class IntroController : public Controller, public Observer<Menu *, MenuEvent &> {
public:
	IntroController();

	bool init();
	bool hasInitiatedNewGame();

	void deleteIntro();
	bool keyPressed(int key);
	unsigned char *getSigData();
	void updateScreen();
	void timerFired();

	void preloadMap();

	void update(Menu *menu, MenuEvent &event);
	void updateConfMenu(MenuEvent &event);
	void updateVideoMenu(MenuEvent &event);
	void updateGfxMenu(MenuEvent &event);
	void updateSoundMenu(MenuEvent &event);
	void updateInputMenu(MenuEvent &event);
	void updateSpeedMenu(MenuEvent &event);
	void updateGameplayMenu(MenuEvent &event);
	void updateInterfaceMenu(MenuEvent &event);

	//
	// Title methods
	//
	void initTitles();
	bool updateTitle();

private:
	void drawMap();
	void drawMapStatic();
	void drawMapAnimated();
	void drawBeasties();
	void drawBeastie(int beast, int vertoffset, int frame);
	void animateTree(const Common::String &frame);
	void drawCard(int pos, int card);
	void drawAbacusBeads(int row, int selectedVirtue, int rejectedVirtue);

	void initQuestionTree();
	bool doQuestion(int answer);
	void initPlayers(SaveGame *saveGame);
	Common::String getQuestion(int v1, int v2);
#ifdef IOS
public:
	void tryTriggerIntroMusic();
#endif
	void initiateNewGame();
	void finishInitiateGame(const Common::String &nameBuffer, SexType sex);
	void startQuestions();
	void showStory();
	void journeyOnward();
	void about();
#ifdef IOS
private:
#endif
	void showText(const Common::String &text);

	void runMenu(Menu *menu, TextView *view, bool withBeasties);

	/**
	 * The states of the intro.
	 */
	enum {
		INTRO_TITLES,                       // displaying the animated intro titles
		INTRO_MAP,                          // displaying the animated intro map
		INTRO_MENU                          // displaying the main menu: journey onward, etc.
	} _mode;

	enum MenuConstants {
		MI_CONF_VIDEO,
		MI_CONF_SOUND,
		MI_CONF_INPUT,
		MI_CONF_SPEED,
		MI_CONF_GAMEPLAY,
		MI_CONF_INTERFACE,
		MI_CONF_01,
		MI_VIDEO_CONF_GFX,
		MI_VIDEO_02,
		MI_VIDEO_03,
		MI_VIDEO_04,
		MI_VIDEO_05,
		MI_VIDEO_06,
		MI_VIDEO_07,
		MI_VIDEO_08,
		MI_GFX_SCHEME,
		MI_GFX_TILE_TRANSPARENCY,
		MI_GFX_TILE_TRANSPARENCY_SHADOW_SIZE,
		MI_GFX_TILE_TRANSPARENCY_SHADOW_OPACITY,
		MI_GFX_RETURN,
		MI_SOUND_01,
		MI_SOUND_02,
		MI_SOUND_03,
		MI_INPUT_01,
		MI_INPUT_02,
		MI_INPUT_03,
		MI_SPEED_01,
		MI_SPEED_02,
		MI_SPEED_03,
		MI_SPEED_04,
		MI_SPEED_05,
		MI_SPEED_06,
		MI_SPEED_07,
		MI_GAMEPLAY_01,
		MI_GAMEPLAY_02,
		MI_GAMEPLAY_03,
		MI_GAMEPLAY_04,
		MI_GAMEPLAY_05,
		MI_GAMEPLAY_06,
		MI_INTERFACE_01,
		MI_INTERFACE_02,
		MI_INTERFACE_03,
		MI_INTERFACE_04,
		MI_INTERFACE_05,
		MI_INTERFACE_06,
		USE_SETTINGS = 0xFE,
		CANCEL = 0xFF
	};

	ImageView _backgroundArea;
	TextView _menuArea;
	TextView _extendedMenuArea;
	TextView _questionArea;
	TileView _mapArea;

	// menus
	Menu _mainMenu;
	Menu _confMenu;
	Menu _videoMenu;
	Menu _gfxMenu;
	Menu _soundMenu;
	Menu _inputMenu;
	Menu _speedMenu;
	Menu _gameplayMenu;
	Menu _interfaceMenu;

	// data loaded in from title.exe
	IntroBinData *binData;

	// additional introduction state data
	Common::String _errorMessage;
	int _answerInd;
	int _questionRound;
	int __questionTree[15];
	int _beastie1Cycle;
	int _beastie2Cycle;
	int _beastieOffset;
	bool _beastiesVisible;
	int _sleepCycles;
	int _scrPos;  /* current position in the script table */
	IntroObjectState *_objectStateTable;

	bool _justInitiatedNewGame;

	//
	// Title defs, structs, methods, and data members
	//
	enum AnimType {
		SIGNATURE,
		AND,
		BAR,
		ORIGIN,
		PRESENT,
		TITLE,
		SUBTITLE,
		MAP
	};

	struct AnimPlot {
		uint8 x, y;
		uint8 r, g, b, a;
	};

	struct AnimElement {
		int _rx, _ry;                         // screen/source x and y
		int _rw, _rh;                         // source width and height
		AnimType _method;                    // render method
		int _animStep;                       // tracks animation position
		int _animStepMax;
		int _timeBase;                       // initial animation time
		int _timeDelay;                      // delay before rendering begins
		int _timeDuration;                   // total animation time
		Image *_srcImage;                    // storage for the source image
		Image *_destImage;                   // storage for the animation frame
		Std::vector <AnimPlot> _plotData;    // plot data
		bool _prescaled;
	};

	void addTitle(int x, int y, int w, int h, AnimType method, int delay, int duration);
	void compactTitle();
	void drawTitle();
	void getTitleSourceData();
	void skipTitles();
	Std::vector<AnimElement> _titles;            // list of title elements
	Std::vector<AnimElement>::iterator _title;   // current title element

	int _transparentIndex;           // palette index for transparency
	RGBA _transparentColor;     // palette color for transparency

	bool _bSkipTitles;
};

extern IntroController *intro;

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
