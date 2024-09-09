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

#ifndef ULTIMA4_CONTROLLERS_INTRO_CONTROLLER_H
#define ULTIMA4_CONTROLLERS_INTRO_CONTROLLER_H

#include "ultima/ultima4/controllers/controller.h"
#include "ultima/ultima4/core/observer.h"
#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/views/menu.h"
#include "ultima/ultima4/views/textview.h"
#include "ultima/ultima4/views/imageview.h"
#include "ultima/ultima4/views/tileview.h"
#include "ultima/shared/core/file.h"

namespace Ultima {
namespace Ultima4 {

class IntroObjectState;
class Tile;

/**
 * Binary data loaded from the U4DOS title.exe file.
 */
class IntroBinData {
private:
	// disallow assignments, copy construction
	IntroBinData(const IntroBinData &);
	const IntroBinData &operator=(const IntroBinData &);
	void openFile(Shared::File &f, const Common::String &name);
public:
	IntroBinData();
	~IntroBinData();

	bool load();

	Std::vector<MapTile> _introMap;
	byte *_sigData;
	byte *_scriptTable;
	Tile **_baseTileTable;
	byte *_beastie1FrameTable;
	byte *_beastie2FrameTable;
	Std::vector<Common::String> _introText;
	Std::vector<Common::String> _introQuestions;
	Std::vector<Common::String> _introGypsy;
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
	~IntroController() override;

	/**
	 * Initializes intro state and loads in introduction graphics, text
	 * and map data from title.exe.
	 */
	bool init();
	bool hasInitiatedNewGame();

	/**
	 * Frees up data not needed after introduction.
	 */
	void deleteIntro();

	/**
	 * Handles keystrokes during the introduction.
	 */
	bool keyPressed(int key) override;

	/**
	 * Mouse button was pressed
	 */
	bool mousePressed(const Common::Point &mousePos) override;

	byte *getSigData();

	/**
	 * Paints the screen.
	 */
	void updateScreen();

	/**
	 * Timer callback for the intro sequence.  Handles animating the intro
	 * map, the beasties, etc..
	 */
	void timerFired() override;

	/**
	 * Preload map tiles
	 */
	void preloadMap();

	/**
	 * Update the screen when an observed menu is reset or has an item
	 * activated.
	 * TODO: Reduce duped code.
	 */
	void update(Menu *menu, MenuEvent &event) override;
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
	/**
	 * Initialize the title elements
	 */
	void initTitles();

	/**
	 * Update the title element, drawing the appropriate frame of animation
	 */
	bool updateTitle();

private:
	/**
	 * Draws the small map on the intro screen.
	 */
	void drawMap();
	void drawMapStatic();
	void drawMapAnimated();

	/**
	 * Draws the animated beasts in the upper corners of the screen.
	 */
	void drawBeasties();

	/**
	 * Animates the "beasties".  The animate intro image is made up frames
	 * for the two creatures in the top left and top right corners of the
	 * screen.  This function draws the frame for the given beastie on the
	 * screen.  vertoffset is used lower the creatures down from the top
	 * of the screen.
	 */
	void drawBeastie(int beast, int vertoffset, int frame);

	/**
	 * Animates the moongate in the tree intro image.  There are two
	 * overlays in the part of the image normally covered by the text.  If
	 * the frame parameter is "moongate", the moongate overlay is painted
	 * over the image.  If frame is "items", the second overlay is
	 * painted: the circle without the moongate, but with a small white
	 * dot representing the anhk and history book.
	 */
	void animateTree(const Common::String &frame);

	/**
	 * Draws the cards in the character creation sequence with the gypsy.
	 */
	void drawCard(int pos, int card);

	/**
	 * Draws the beads in the abacus during the character creation sequence
	 */
	void drawAbacusBeads(int row, int selectedVirtue, int rejectedVirtue);

	/**
	 * Initializes the question tree.  The tree starts off with the first
	 * eight entries set to the numbers 0-7 in a random order.
	 */
	void initQuestionTree();

	/**
	 * Updates the question tree with the given answer, and advances to
	 * the next round.
	 * @return true if all questions have been answered, false otherwise
	 */
	bool doQuestion(int answer);

	/**
	 * Build the initial avatar player record from the answers to the
	 * gypsy's questions.
	 */
	void initPlayers(SaveGame *saveGame);

	/**
	 * Get the text for the question giving a choice between virtue v1 and
	 * virtue v2 (zero based virtue index, starting at honesty).
	 */
	Common::String getQuestion(int v1, int v2);
#ifdef IOS_ULTIMA4
public:
	/**
	 * Try to put the intro music back at just the correct moment on iOS;
	 * don't play it at the very beginning.
	 */
	void tryTriggerIntroMusic();
#endif

	/**
	 * Initiate a new savegame by reading the name, sex, then presenting a
	 * series of questions to determine the class of the new character.
	 */
	void initiateNewGame();
	void finishInitiateGame(const Common::String &nameBuffer, SexType sex);

	/**
	 * Starts the gypsys questioning that eventually determines the new
	 * characters class.
	 */
	void startQuestions();
	void showStory();

	/**
	 * Starts the game.
	 */
	void journeyOnward();

	/**
	 * Shows an about box.
	 */
	void about();
#ifdef IOS_ULTIMA4
private:
#endif
	/**
	 * Shows text in the question area.
	 */
	void showText(const Common::String &text);

	/**
	 * Run a menu and return when the menu has been closed.  Screen
	 * updates are handled by observing the menu.
	 */
	void runMenu(Menu *menu, TextView *view, bool withBeasties);

	/**
	 * The states of the intro.
	 */
	enum {
		INTRO_TITLES,                       // displaying the animated intro titles
		INTRO_MAP,                          // displaying the animated intro map
		INTRO_MENU,                         // displaying the main menu: journey onward, etc.
		INTRO_ABOUT
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
		MI_VIDEO_07,
		MI_VIDEO_08,
		MI_GFX_SCHEME,
		MI_GFX_TILE_TRANSPARENCY,
		MI_GFX_TILE_TRANSPARENCY_SHADOW_SIZE,
		MI_GFX_TILE_TRANSPARENCY_SHADOW_OPACITY,
		MI_GFX_RETURN,
		MI_SOUND_03,
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
	Image *_mapScreen;

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
	IntroBinData *_binData;

	// additional introduction state data
	Common::String _errorMessage;
	int _answerInd;
	int _questionRound;
	int _questionTree[15];
	int _beastie1Cycle;
	int _beastie2Cycle;
	int _beastieOffset;
	bool _beastiesVisible;
	int _sleepCycles;
	int _scrPos;  /* current position in the script table */
	IntroObjectState *_objectStateTable;

	bool _justInitiatedNewGame;
	Common::String _profileName;
	bool _useProfile;

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
		void shufflePlotData();

		int _rx, _ry;                        // screen/source x and y
		int _rw, _rh;                        // source width and height
		AnimType _method;                    // render method
		int _animStep;                       // tracks animation position
		int _animStepMax;
		int _timeBase;                       // initial animation time
		uint32 _timeDelay;                   // delay before rendering begins
		int _timeDuration;                   // total animation time
		Image *_srcImage;                    // storage for the source image
		Image *_destImage;                   // storage for the animation frame
		Std::vector <AnimPlot> _plotData;    // plot data
		bool _prescaled;
	};

	/**
	 * Add the intro element to the element list
	 */
	void addTitle(int x, int y, int w, int h, AnimType method, uint32 delay, int duration);

	/**
	 * The title element has finished drawing all frames, so delete, remove,
	 * or free data that is no longer needed
	 */
	void compactTitle();

	/**
	 * Scale the animation canvas, then draw it to the screen
	 */
	void drawTitle();

	/**
	 * Get the source data for title elements that have already been initialized
	 */
	void getTitleSourceData();

	/**
	 * skip the remaining titles
	 */
	void skipTitles();

	Std::vector<AnimElement> _titles;            // list of title elements
	Std::vector<AnimElement>::iterator _title;   // current title element

	int _transparentIndex;           // palette index for transparency
	RGBA _transparentColor;     // palette color for transparency

	bool _bSkipTitles;

	// Temporary place-holder for settings changes
	SettingsData _settingsChanged;
};

extern IntroController *g_intro;

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
