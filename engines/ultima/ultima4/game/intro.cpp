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

#include "ultima/ultima4/ultima4.h"
#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/game/intro.h"
#include "ultima/ultima4/core/debug.h"
#include "ultima/ultima4/core/error.h"
#include "ultima/ultima4/events/event.h"
#include "ultima/ultima4/gfx/imagemgr.h"
#include "ultima/ultima4/game/menu.h"
#include "ultima/ultima4/sound/music.h"
#include "ultima/ultima4/sound/sound.h"
#include "ultima/ultima4/game/player.h"
#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/ultima4/core/settings.h"
#include "ultima/ultima4/map/shrine.h"
#include "ultima/ultima4/map/tileset.h"
#include "ultima/ultima4/map/tilemap.h"
#include "ultima/ultima4/filesys/u4file.h"
#include "ultima/ultima4/core/utils.h"
#include "common/savefile.h"
#include "common/system.h"

namespace Ultima {
namespace Ultima4 {

using namespace std;

extern bool quit;
Common::String profileName;
bool useProfile;

IntroController *intro = NULL;

#define INTRO_MAP_HEIGHT 5
#define INTRO_MAP_WIDTH 19
#define INTRO_TEXT_X 0
#define INTRO_TEXT_Y 19
#define INTRO_TEXT_WIDTH 40
#define INTRO_TEXT_HEIGHT 6

#define GYP_PLACES_FIRST 0
#define GYP_PLACES_TWOMORE 1
#define GYP_PLACES_LAST 2
#define GYP_UPON_TABLE 3
#define GYP_SEGUE1 13
#define GYP_SEGUE2 14

class IntroObjectState {
public:
	IntroObjectState() : x(0), y(0), tile(0) {}
	int x, y;
	MapTile tile; /* base tile + tile frame */
};

/* temporary place-holder for settings changes */
SettingsData settingsChanged;

const int IntroBinData::INTRO_TEXT_OFFSET = 17445 - 1;  // (start at zero)
const int IntroBinData::INTRO_MAP_OFFSET = 30339;
const int IntroBinData::INTRO_FIXUPDATA_OFFSET = 29806;
const int IntroBinData::INTRO_SCRIPT_TABLE_SIZE = 548;
const int IntroBinData::INTRO_SCRIPT_TABLE_OFFSET = 30434;
const int IntroBinData::INTRO_BASETILE_TABLE_SIZE = 15;
const int IntroBinData::INTRO_BASETILE_TABLE_OFFSET = 16584;
const int IntroBinData::BEASTIE1_FRAMES = 0x80;
const int IntroBinData::BEASTIE2_FRAMES = 0x40;
const int IntroBinData::BEASTIE_FRAME_TABLE_OFFSET = 0x7380;
const int IntroBinData::BEASTIE1_FRAMES_OFFSET = 0;
const int IntroBinData::BEASTIE2_FRAMES_OFFSET = 0x78;

IntroBinData::IntroBinData() :
	_sigData(NULL),
	_scriptTable(NULL),
	_baseTileTable(NULL),
	_beastie1FrameTable(NULL),
	_beastie2FrameTable(NULL) {
}

IntroBinData::~IntroBinData() {
	if (_sigData)
		delete [] _sigData;
	if (_scriptTable)
		delete [] _scriptTable;
	if (_baseTileTable)
		delete [] _baseTileTable;
	if (_beastie1FrameTable)
		delete [] _beastie1FrameTable;
	if (_beastie2FrameTable)
		delete [] _beastie2FrameTable;

	_introQuestions.clear();
	_introText.clear();
	_introGypsy.clear();
}

bool IntroBinData::load() {
	int i;

	U4FILE *title = u4fopen("title.exe");
	if (!title)
		return false;

	_introQuestions = u4read_stringtable(title, INTRO_TEXT_OFFSET, 28);
	_introText = u4read_stringtable(title, -1, 24);
	_introGypsy = u4read_stringtable(title, -1, 15);

	/* clean up stray newlines at end of strings */
	for (i = 0; i < 15; i++)
		trim(_introGypsy[i]);

	if (_sigData)
		delete _sigData;
	_sigData = new unsigned char[533];
	u4fseek(title, INTRO_FIXUPDATA_OFFSET, SEEK_SET);
	u4fread(_sigData, 1, 533, title);

	u4fseek(title, INTRO_MAP_OFFSET, SEEK_SET);
	_introMap.clear();
	_introMap.resize(INTRO_MAP_WIDTH * INTRO_MAP_HEIGHT);
	for (i = 0; i < INTRO_MAP_HEIGHT * INTRO_MAP_WIDTH; i++)
		_introMap[i] = TileMap::get("base")->translate(u4fgetc(title));

	u4fseek(title, INTRO_SCRIPT_TABLE_OFFSET, SEEK_SET);
	_scriptTable = new unsigned char[INTRO_SCRIPT_TABLE_SIZE];
	for (i = 0; i < INTRO_SCRIPT_TABLE_SIZE; i++)
		_scriptTable[i] = u4fgetc(title);

	u4fseek(title, INTRO_BASETILE_TABLE_OFFSET, SEEK_SET);
	_baseTileTable = new Tile*[INTRO_BASETILE_TABLE_SIZE];
	for (i = 0; i < INTRO_BASETILE_TABLE_SIZE; i++) {
		MapTile tile = TileMap::get("base")->translate(u4fgetc(title));
		_baseTileTable[i] = Tileset::get("base")->get(tile._id);
	}

	/* --------------------------
	   load beastie frame table 1
	   -------------------------- */
	_beastie1FrameTable = new unsigned char[BEASTIE1_FRAMES];
	u4fseek(title, BEASTIE_FRAME_TABLE_OFFSET + BEASTIE1_FRAMES_OFFSET, SEEK_SET);
	for (i = 0; i < BEASTIE1_FRAMES; i++) {
		_beastie1FrameTable[i] = u4fgetc(title);
	}

	/* --------------------------
	   load beastie frame table 2
	   -------------------------- */
	_beastie2FrameTable = new unsigned char[BEASTIE2_FRAMES];
	u4fseek(title, BEASTIE_FRAME_TABLE_OFFSET + BEASTIE2_FRAMES_OFFSET, SEEK_SET);
	for (i = 0; i < BEASTIE2_FRAMES; i++) {
		_beastie2FrameTable[i] = u4fgetc(title);
	}

	u4fclose(title);

	return true;
}

IntroController::IntroController() :
	Controller(1),
	_backgroundArea(),
	_menuArea(1 * CHAR_WIDTH, 13 * CHAR_HEIGHT, 38, 11),
	_extendedMenuArea(2 * CHAR_WIDTH, 10 * CHAR_HEIGHT, 36, 13),
	_questionArea(INTRO_TEXT_X * CHAR_WIDTH, INTRO_TEXT_Y * CHAR_HEIGHT, INTRO_TEXT_WIDTH, INTRO_TEXT_HEIGHT),
	_mapArea(BORDER_WIDTH, (TILE_HEIGHT * 6) + BORDER_HEIGHT, INTRO_MAP_WIDTH, INTRO_MAP_HEIGHT, "base"),
	_binData(NULL),
	_titles(),                   // element list
	_title(_titles.begin()),      // element iterator
	_transparentIndex(13),       // palette index for transparency
	_transparentColor(),         // palette color for transparency
	_bSkipTitles(false) {
	// initialize menus
	_confMenu.setTitle("XU4 Configuration:", 0, 0);
	_confMenu.add(MI_CONF_VIDEO,               "\010 Video Options",              2,  2,/*'v'*/  2);
	_confMenu.add(MI_CONF_SOUND,               "\010 Sound Options",              2,  3,/*'s'*/  2);
	_confMenu.add(MI_CONF_INPUT,               "\010 Input Options",              2,  4,/*'i'*/  2);
	_confMenu.add(MI_CONF_SPEED,               "\010 Speed Options",              2,  5,/*'p'*/  3);
	_confMenu.add(MI_CONF_01, new BoolMenuItem("Game Enhancements         %s",    2,  7,/*'e'*/  5, &settingsChanged._enhancements));
	_confMenu.add(MI_CONF_GAMEPLAY,            "\010 Enhanced Gameplay Options",  2,  9,/*'g'*/ 11);
	_confMenu.add(MI_CONF_INTERFACE,           "\010 Enhanced Interface Options", 2, 10,/*'n'*/ 12);
	_confMenu.add(CANCEL,                      "\017 Main Menu",                  2, 12,/*'m'*/  2);
	_confMenu.addShortcutKey(CANCEL, ' ');
	_confMenu.setClosesMenu(CANCEL);

	/* set the default visibility of the two enhancement menus */
	_confMenu.getItemById(MI_CONF_GAMEPLAY)->setVisible(settings._enhancements);
	_confMenu.getItemById(MI_CONF_INTERFACE)->setVisible(settings._enhancements);

	_videoMenu.setTitle("Video Options:", 0, 0);
	_videoMenu.add(MI_VIDEO_CONF_GFX,                        "\010 Game Graphics Options",  2,  2,/*'g'*/  2);
	_videoMenu.add(MI_VIDEO_04,         new IntMenuItem("Scale                x%d", 2,  4,/*'s'*/  0, reinterpret_cast<int *>(&settingsChanged._scale), 1, 5, 1));
	_videoMenu.add(MI_VIDEO_05, (new BoolMenuItem("Mode                 %s",  2,  5,/*'m'*/  0, &settingsChanged._fullscreen))->setValueStrings("Fullscreen", "Window"));
	_videoMenu.add(MI_VIDEO_06,             new StringMenuItem("Filter               %s",  2,  6,/*'f'*/  0, &settingsChanged._filter, screenGetFilterNames()));
	_videoMenu.add(MI_VIDEO_08,         new IntMenuItem("Gamma                %s",  2,  7,/*'a'*/  1, &settingsChanged._gamma, 50, 150, 10, MENU_OUTPUT_GAMMA));
	_videoMenu.add(USE_SETTINGS,                   "\010 Use These Settings",  2, 11,/*'u'*/  2);
	_videoMenu.add(CANCEL,                         "\010 Cancel",              2, 12,/*'c'*/  2);
	_videoMenu.addShortcutKey(CANCEL, ' ');
	_videoMenu.setClosesMenu(USE_SETTINGS);
	_videoMenu.setClosesMenu(CANCEL);

	_gfxMenu.setTitle("Game Graphics Options", 0, 0);
	_gfxMenu.add(MI_GFX_SCHEME, new StringMenuItem("Graphics Scheme    %s", 2, 2, /*'G'*/ 0, &settingsChanged._videoType, imageMgr->getSetNames()));
	_gfxMenu.add(MI_GFX_TILE_TRANSPARENCY, new BoolMenuItem("Transparency Hack  %s", 2, 4, /*'t'*/ 0, &settingsChanged._enhancementsOptions._u4TileTransparencyHack));
	_gfxMenu.add(MI_GFX_TILE_TRANSPARENCY_SHADOW_SIZE, new IntMenuItem("  Shadow Size:     %d", 2, 5, /*'s'*/ 9, &settingsChanged._enhancementsOptions._u4TrileTransparencyHackShadowBreadth, 0, 16, 1));
	_gfxMenu.add(MI_GFX_TILE_TRANSPARENCY_SHADOW_OPACITY, new IntMenuItem("  Shadow Opacity:  %d", 2, 6, /*'o'*/ 9, &settingsChanged._enhancementsOptions._u4TileTransparencyHackPixelShadowOpacity, 8, 256, 8));
	_gfxMenu.add(MI_VIDEO_02,               new StringMenuItem("Gem Layout         %s",  2,  8,/*'e'*/  1, &settingsChanged._gemLayout, screenGetGemLayoutNames()));
	_gfxMenu.add(MI_VIDEO_03,           new StringMenuItem("Line Of Sight      %s",  2,  9,/*'l'*/  0, &settingsChanged._lineOfSight, screenGetLineOfSightStyles()));
	_gfxMenu.add(MI_VIDEO_07,           new BoolMenuItem("Screen Shaking     %s",  2, 10,/*'k'*/ 8, &settingsChanged._screenShakes));
	_gfxMenu.add(MI_GFX_RETURN,               "\010 Return to Video Options",              2,  12,/*'r'*/  2);
	_gfxMenu.setClosesMenu(MI_GFX_RETURN);


	_soundMenu.setTitle("Sound Options:", 0, 0);
	_soundMenu.add(MI_SOUND_01,  new IntMenuItem("Music Volume         %s", 2,  2,/*'m'*/  0, &settingsChanged._musicVol, 0, MAX_VOLUME, 1, MENU_OUTPUT_VOLUME));
	_soundMenu.add(MI_SOUND_02,  new IntMenuItem("Sound Effect Volume  %s", 2,  3,/*'s'*/  0, &settingsChanged._soundVol, 0, MAX_VOLUME, 1, MENU_OUTPUT_VOLUME));
	_soundMenu.add(MI_SOUND_03, new BoolMenuItem("Fading               %s", 2,  4,/*'f'*/  0, &settingsChanged._volumeFades));
	_soundMenu.add(USE_SETTINGS,                 "\010 Use These Settings", 2, 11,/*'u'*/  2);
	_soundMenu.add(CANCEL,                       "\010 Cancel",             2, 12,/*'c'*/  2);
	_soundMenu.addShortcutKey(CANCEL, ' ');
	_soundMenu.setClosesMenu(USE_SETTINGS);
	_soundMenu.setClosesMenu(CANCEL);

	_inputMenu.setTitle("Keyboard Options:", 0, 0);
	_inputMenu.add(MI_INPUT_01,  new IntMenuItem("Repeat Delay        %4d msec", 2,  2,/*'d'*/  7, &settingsChanged._keydelay, 100, MAX_KEY_DELAY, 100));
	_inputMenu.add(MI_INPUT_02,  new IntMenuItem("Repeat Interval     %4d msec", 2,  3,/*'i'*/  7, &settingsChanged._keyinterval, 10, MAX_KEY_INTERVAL, 10));
	/* "Mouse Options:" is drawn in the updateInputMenu() function */
	_inputMenu.add(MI_INPUT_03, new BoolMenuItem("Mouse                %s",      2,  7,/*'m'*/  0, &settingsChanged._mouseOptions.enabled));
	_inputMenu.add(USE_SETTINGS,                 "\010 Use These Settings",      2, 11,/*'u'*/  2);
	_inputMenu.add(CANCEL,                       "\010 Cancel",                  2, 12,/*'c'*/  2);
	_inputMenu.addShortcutKey(CANCEL, ' ');
	_inputMenu.setClosesMenu(USE_SETTINGS);
	_inputMenu.setClosesMenu(CANCEL);

	_speedMenu.setTitle("Speed Options:", 0, 0);
	_speedMenu.add(MI_SPEED_01, new IntMenuItem("Game Cycles per Second    %3d",      2,  2,/*'g'*/  0, &settingsChanged._gameCyclesPerSecond, 1, MAX_CYCLES_PER_SECOND, 1));
	_speedMenu.add(MI_SPEED_02, new IntMenuItem("Battle Speed              %3d",      2,  3,/*'b'*/  0, &settingsChanged._battleSpeed, 1, MAX_BATTLE_SPEED, 1));
	_speedMenu.add(MI_SPEED_03, new IntMenuItem("Spell Effect Length       %s",       2,  4,/*'p'*/  1, &settingsChanged._spellEffectSpeed, 1, MAX_SPELL_EFFECT_SPEED, 1, MENU_OUTPUT_SPELL));
	_speedMenu.add(MI_SPEED_04, new IntMenuItem("Camping Length            %3d sec",  2,  5,/*'m'*/  2, &settingsChanged._campTime, 1, MAX_CAMP_TIME, 1));
	_speedMenu.add(MI_SPEED_05, new IntMenuItem("Inn Rest Length           %3d sec",  2,  6,/*'i'*/  0, &settingsChanged._innTime, 1, MAX_INN_TIME, 1));
	_speedMenu.add(MI_SPEED_06, new IntMenuItem("Shrine Meditation Length  %3d sec",  2,  7,/*'s'*/  0, &settingsChanged._shrineTime, 1, MAX_SHRINE_TIME, 1));
	_speedMenu.add(MI_SPEED_07, new IntMenuItem("Screen Shake Interval     %3d msec", 2,  8,/*'r'*/  2, &settingsChanged._shakeInterval, MIN_SHAKE_INTERVAL, MAX_SHAKE_INTERVAL, 10));
	_speedMenu.add(USE_SETTINGS,                "\010 Use These Settings",            2, 11,/*'u'*/  2);
	_speedMenu.add(CANCEL,                      "\010 Cancel",                        2, 12,/*'c'*/  2);
	_speedMenu.addShortcutKey(CANCEL, ' ');
	_speedMenu.setClosesMenu(USE_SETTINGS);
	_speedMenu.setClosesMenu(CANCEL);

	/* move the BATTLE DIFFICULTY, DEBUG, and AUTOMATIC ACTIONS settings to "enhancementsOptions" */
	_gameplayMenu.setTitle("Enhanced Gameplay Options:", 0, 0);
	_gameplayMenu.add(MI_GAMEPLAY_01, new StringMenuItem("Battle Difficulty          %s", 2,  2,/*'b'*/  0, &settingsChanged._battleDiff, settings.getBattleDiffs()));
	_gameplayMenu.add(MI_GAMEPLAY_02,   new BoolMenuItem("Fixed Chest Traps          %s", 2,  3,/*'t'*/ 12, &settingsChanged._enhancementsOptions._c64chestTraps));
	_gameplayMenu.add(MI_GAMEPLAY_03,   new BoolMenuItem("Gazer Spawns Insects       %s", 2,  4,/*'g'*/  0, &settingsChanged._enhancementsOptions._gazerSpawnsInsects));
	_gameplayMenu.add(MI_GAMEPLAY_04,   new BoolMenuItem("Gem View Shows Objects     %s", 2,  5,/*'e'*/  1, &settingsChanged._enhancementsOptions._peerShowsObjects));
	_gameplayMenu.add(MI_GAMEPLAY_05,   new BoolMenuItem("Slime Divides              %s", 2,  6,/*'s'*/  0, &settingsChanged._enhancementsOptions._slimeDivides));
	_gameplayMenu.add(MI_GAMEPLAY_06,   new BoolMenuItem("Debug Mode (Cheats)        %s", 2,  8,/*'d'*/  0, &settingsChanged._debug));
	_gameplayMenu.add(USE_SETTINGS,                      "\010 Use These Settings",       2, 11,/*'u'*/  2);
	_gameplayMenu.add(CANCEL,                            "\010 Cancel",                   2, 12,/*'c'*/  2);
	_gameplayMenu.addShortcutKey(CANCEL, ' ');
	_gameplayMenu.setClosesMenu(USE_SETTINGS);
	_gameplayMenu.setClosesMenu(CANCEL);

	_interfaceMenu.setTitle("Enhanced Interface Options:", 0, 0);
	_interfaceMenu.add(MI_INTERFACE_01, new BoolMenuItem("Automatic Actions          %s", 2,  2,/*'a'*/  0, &settingsChanged._shortcutCommands));
	/* "(Open, Jimmy, etc.)" */
	_interfaceMenu.add(MI_INTERFACE_02, new BoolMenuItem("Set Active Player          %s", 2,  4,/*'p'*/ 11, &settingsChanged._enhancementsOptions._activePlayer));
	_interfaceMenu.add(MI_INTERFACE_03, new BoolMenuItem("Smart 'Enter' Key          %s", 2,  5,/*'e'*/  7, &settingsChanged._enhancementsOptions._smartEnterKey));
	_interfaceMenu.add(MI_INTERFACE_04, new BoolMenuItem("Text Colorization          %s", 2,  6,/*'t'*/  0, &settingsChanged._enhancementsOptions._textColorization));
	_interfaceMenu.add(MI_INTERFACE_05, new BoolMenuItem("Ultima V Shrines           %s", 2,  7,/*'s'*/  9, &settingsChanged._enhancementsOptions._u5shrines));
	_interfaceMenu.add(MI_INTERFACE_06, new BoolMenuItem("Ultima V Spell Mixing      %s", 2,  8,/*'m'*/ 15, &settingsChanged._enhancementsOptions._u5spellMixing));
	_interfaceMenu.add(USE_SETTINGS,                     "\010 Use These Settings",       2, 11,/*'u'*/  2);
	_interfaceMenu.add(CANCEL,                           "\010 Cancel",                   2, 12,/*'c'*/  2);
	_interfaceMenu.addShortcutKey(CANCEL, ' ');
	_interfaceMenu.setClosesMenu(USE_SETTINGS);
	_interfaceMenu.setClosesMenu(CANCEL);
}

/**
 * Initializes intro state and loads in introduction graphics, text
 * and map data from title.exe.
 */
bool IntroController::init() {

	_justInitiatedNewGame = false;

	// sigData is referenced during Titles initialization
	_binData = new IntroBinData();
	_binData->load();

	if (_bSkipTitles) {
		// the init() method is called again from within the
		// game via ALT-Q, so return to the menu
		//
#ifndef IOS
		_mode = INTRO_MENU;
#else
		mode = INTRO_MAP;
#endif
		_beastiesVisible = true;
		_beastieOffset = 0;
		musicMgr->intro();
	} else {
		// initialize the titles
		initTitles();
		_mode = INTRO_TITLES;
		_beastiesVisible = false;
		_beastieOffset = -32;
	}

	_beastie1Cycle = 0;
	_beastie2Cycle = 0;

	_sleepCycles = 0;
	_scrPos = 0;
	_objectStateTable = new IntroObjectState[IntroBinData::INTRO_BASETILE_TABLE_SIZE];

	_backgroundArea.reinit();
	_menuArea.reinit();
	_extendedMenuArea.reinit();
	_questionArea.reinit();
	_mapArea.reinit();

	// only update the screen if we are returning from game mode
	if (_bSkipTitles)
		updateScreen();

	return true;
}

bool IntroController::hasInitiatedNewGame() {
	return this->_justInitiatedNewGame;
}

/**
 * Frees up data not needed after introduction.
 */
void IntroController::deleteIntro() {
	delete _binData;
	_binData = NULL;

	delete [] _objectStateTable;
	_objectStateTable = NULL;

	imageMgr->freeIntroBackgrounds();
}

unsigned char *IntroController::getSigData() {
	ASSERT(_binData->_sigData != NULL, "intro sig data not loaded");
	return _binData->_sigData;
}

/**
 * Handles keystrokes during the introduction.
 */
bool IntroController::keyPressed(int key) {
	bool valid = true;

	switch (_mode) {

	case INTRO_TITLES:
		// the user pressed a key to abort the sequence
		skipTitles();
		break;

	case INTRO_MAP:
		_mode = INTRO_MENU;
		updateScreen();
		break;

	case INTRO_MENU:
		switch (key) {
		case 'i':
			_errorMessage.clear();
			initiateNewGame();
			break;
		case 'j':
			journeyOnward();
			break;
		case 'r':
			_errorMessage.clear();
			_mode = INTRO_MAP;
			updateScreen();
			break;
		case 'c': {
			_errorMessage.clear();
			// Make a copy of our settings so we can change them
			settingsChanged = settings;
			screenDisableCursor();
			runMenu(&_confMenu, &_extendedMenuArea, true);
			screenEnableCursor();
			updateScreen();
			break;
		}
		case 'a':
			_errorMessage.clear();
			about();
			break;
		case 'q':
			EventHandler::end();
			quit = true;
			break;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			musicMgr->introSwitch(key - '0');
			break;
		default:
			valid = false;
			break;
		}
		break;

	default:
		ASSERT(0, "key handler called in wrong mode");
		return true;
	}

	return valid || KeyHandler::defaultHandler(key, NULL);
}

/**
 * Draws the small map on the intro screen.
 */
void IntroController::drawMap() {
	if (0 && _sleepCycles > 0) {
		drawMapStatic();
		drawMapAnimated();
		_sleepCycles--;
	} else {
		unsigned char commandNibble;
		unsigned char dataNibble;

		do {
			commandNibble = _binData->_scriptTable[_scrPos] >> 4;

			switch (commandNibble) {
			/* 0-4 = set object position and tile frame */
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
				/* ----------------------------------------------------------
				   Set object position and tile frame
				   Format: yi [t(3); x(5)]
				   i = table index
				   x = x coordinate (5 least significant bits of second byte)
				   y = y coordinate
				   t = tile frame (3 most significant bits of second byte)
				   ---------------------------------------------------------- */
				dataNibble = _binData->_scriptTable[_scrPos] & 0xf;
				_objectStateTable[dataNibble].x = _binData->_scriptTable[_scrPos + 1] & 0x1f;
				_objectStateTable[dataNibble].y = commandNibble;

				// See if the tile id needs to be recalculated
				if ((_binData->_scriptTable[_scrPos + 1] >> 5) >= _binData->_baseTileTable[dataNibble]->getFrames()) {
					int frame = (_binData->_scriptTable[_scrPos + 1] >> 5) - _binData->_baseTileTable[dataNibble]->getFrames();
					_objectStateTable[dataNibble].tile = MapTile(_binData->_baseTileTable[dataNibble]->getId() + 1);
					_objectStateTable[dataNibble].tile._frame = frame;
				} else {
					_objectStateTable[dataNibble].tile = MapTile(_binData->_baseTileTable[dataNibble]->getId());
					_objectStateTable[dataNibble].tile._frame = (_binData->_scriptTable[_scrPos + 1] >> 5);
				}

				_scrPos += 2;
				break;
			case 7:
				/* ---------------
				   Delete object
				   Format: 7i
				   i = table index
				   --------------- */
				dataNibble = _binData->_scriptTable[_scrPos] & 0xf;
				_objectStateTable[dataNibble].tile = 0;
				_scrPos++;
				break;
			case 8:
				/* ----------------------------------------------
				   Redraw intro map and objects, then go to sleep
				   Format: 8c
				   c = cycles to sleep
				   ---------------------------------------------- */
				drawMapStatic();
				drawMapAnimated();

				/* set sleep cycles */
				_sleepCycles = _binData->_scriptTable[_scrPos] & 0xf;
				_scrPos++;
				break;
			case 0xf:
				/* -------------------------------------
				   Jump to the start of the script table
				   Format: f?
				   ? = doesn't matter
				   ------------------------------------- */
				_scrPos = 0;
				break;
			default:
				/* invalid command */
				_scrPos++;
				break;
			}

		} while (commandNibble != 8);
	}
}

void IntroController::drawMapStatic() {
	int x, y;

	// draw unmodified map
	for (y = 0; y < INTRO_MAP_HEIGHT; y++)
		for (x = 0; x < INTRO_MAP_WIDTH; x++)
			_mapArea.drawTile(_binData->_introMap[x + (y * INTRO_MAP_WIDTH)], false, x, y);
}

void IntroController::drawMapAnimated() {
	int i;

	// draw animated objects
	for (i = 0; i < IntroBinData::INTRO_BASETILE_TABLE_SIZE; i++)
		if (_objectStateTable[i].tile != 0) {
			Std::vector<MapTile> tiles;
			tiles.push_back(_objectStateTable[i].tile);
			tiles.push_back(_binData->_introMap[_objectStateTable[i].x + (_objectStateTable[i].y * INTRO_MAP_WIDTH)]);
			_mapArea.drawTile(tiles, false, _objectStateTable[i].x, _objectStateTable[i].y);
		}
}

/**
 * Draws the animated beasts in the upper corners of the screen.
 */
void IntroController::drawBeasties() {
	drawBeastie(0, _beastieOffset, _binData->_beastie1FrameTable[_beastie1Cycle]);
	drawBeastie(1, _beastieOffset, _binData->_beastie2FrameTable[_beastie2Cycle]);
	if (_beastieOffset < 0)
		_beastieOffset++;
}

/**
 * Animates the "beasties".  The animate intro image is made up frames
 * for the two creatures in the top left and top right corners of the
 * screen.  This function draws the frame for the given beastie on the
 * screen.  vertoffset is used lower the creatures down from the top
 * of the screen.
 */
void IntroController::drawBeastie(int beast, int vertoffset, int frame) {
	char buffer[128];
	int destx;

	ASSERT(beast == 0 || beast == 1, "invalid beast: %d", beast);

	sprintf(buffer, "beast%dframe%02d", beast, frame);

	destx = beast ? (320 - 48) : 0;
	_backgroundArea.draw(buffer, destx, vertoffset);
}

/**
 * Animates the moongate in the tree intro image.  There are two
 * overlays in the part of the image normally covered by the text.  If
 * the frame parameter is "moongate", the moongate overlay is painted
 * over the image.  If frame is "items", the second overlay is
 * painted: the circle without the moongate, but with a small white
 * dot representing the anhk and history book.
 */
void IntroController::animateTree(const Common::String &frame) {
	_backgroundArea.draw(frame, 72, 68);
}

/**
 * Draws the cards in the character creation sequence with the gypsy.
 */
void IntroController::drawCard(int pos, int card) {
	static const char *cardNames[] = {
		"honestycard", "compassioncard", "valorcard", "justicecard",
		"sacrificecard", "honorcard", "spiritualitycard", "humilitycard"
	};

	ASSERT(pos == 0 || pos == 1, "invalid pos: %d", pos);
	ASSERT(card < 8, "invalid card: %d", card);

	_backgroundArea.draw(cardNames[card], pos ? 218 : 12, 12);
}

/**
 * Draws the beads in the abacus during the character creation sequence
 */
void IntroController::drawAbacusBeads(int row, int selectedVirtue, int rejectedVirtue) {
	ASSERT(row >= 0 && row < 7, "invalid row: %d", row);
	ASSERT(selectedVirtue < 8 && selectedVirtue >= 0, "invalid virtue: %d", selectedVirtue);
	ASSERT(rejectedVirtue < 8 && rejectedVirtue >= 0, "invalid virtue: %d", rejectedVirtue);

	_backgroundArea.draw("whitebead", 128 + (selectedVirtue * 9), 24 + (row * 15));
	_backgroundArea.draw("blackbead", 128 + (rejectedVirtue * 9), 24 + (row * 15));
}

/**
 * Paints the screen.
 */
void IntroController::updateScreen() {
	screenHideCursor();

	switch (_mode) {
	case INTRO_MAP:
		_backgroundArea.draw(BKGD_INTRO);
		drawMap();
		drawBeasties();
		// display the profile name if a local profile is being used
		if (useProfile)
			screenTextAt(40 - profileName.size(), 24, "%s", profileName.c_str());
		break;

	case INTRO_MENU:
		// draw the extended background for all option screens
		_backgroundArea.draw(BKGD_INTRO);
		_backgroundArea.draw(BKGD_OPTIONS_BTM, 0, 120);

		// if there is an error message to display, show it
		if (!_errorMessage.empty()) {
			_menuArea.textAt(6, 5, "%s", _errorMessage.c_str());
			drawBeasties();
			g_screen->update();
			// wait for a couple seconds
			EventHandler::wait_msecs(2000);
			// clear the screen again
			_errorMessage.clear();
			_backgroundArea.draw(BKGD_INTRO);
			_backgroundArea.draw(BKGD_OPTIONS_BTM, 0, 120);
		}

		_menuArea.textAt(1,  1, "In another world, in a time to come.");
		_menuArea.textAt(14, 3, "Options:");
		_menuArea.textAt(10, 5, "%s", _menuArea.colorizeString("Return to the view", FG_YELLOW, 0, 1).c_str());
		_menuArea.textAt(10, 6, "%s", _menuArea.colorizeString("Journey Onward",     FG_YELLOW, 0, 1).c_str());
		_menuArea.textAt(10, 7, "%s", _menuArea.colorizeString("Initiate New Game",  FG_YELLOW, 0, 1).c_str());
		_menuArea.textAt(10, 8, "%s", _menuArea.colorizeString("Configure",          FG_YELLOW, 0, 1).c_str());
		_menuArea.textAt(10, 9, "%s", _menuArea.colorizeString("About",              FG_YELLOW, 0, 1).c_str());
		drawBeasties();

		// draw the cursor last
		screenSetCursorPos(24, 16);
		screenShowCursor();
		break;

	default:
		ASSERT(0, "bad mode in updateScreen");
	}

	screenUpdateCursor();
	g_screen->update();
}

/**
 * Initiate a new savegame by reading the name, sex, then presenting a
 * series of questions to determine the class of the new character.
 */
void IntroController::initiateNewGame() {
	// disable the screen cursor because a text cursor will now be used
	screenDisableCursor();

	// draw the extended background for all option screens
	_backgroundArea.draw(BKGD_INTRO);
	_backgroundArea.draw(BKGD_OPTIONS_BTM, 0, 120);

	// display name prompt and read name from keyboard
	_menuArea.textAt(3, 3, "By what name shalt thou be known");
	_menuArea.textAt(3, 4, "in this world and time?");

	// enable the text cursor after setting it's initial position
	// this will avoid drawing in undesirable areas like 0,0
	_menuArea.setCursorPos(11, 7, false);
	_menuArea.setCursorFollowsText(true);
	_menuArea.enableCursor();

	drawBeasties();
	g_screen->update();

	Common::String nameBuffer = ReadStringController::get(12, &_menuArea);
	if (nameBuffer.empty()) {
		// the user didn't enter a name
		_menuArea.disableCursor();
		screenEnableCursor();
		updateScreen();
		return;
	}

	// draw the extended background for all option screens
	_backgroundArea.draw(BKGD_INTRO);
	_backgroundArea.draw(BKGD_OPTIONS_BTM, 0, 120);

	// display sex prompt and read sex from keyboard
	_menuArea.textAt(3, 3, "Art thou Male or Female?");

	// the cursor is already enabled, just change its position
	_menuArea.setCursorPos(28, 3, true);

	drawBeasties();

	SexType sex;
	int sexChoice = ReadChoiceController::get("mf");
	if (sexChoice == 'm')
		sex = SEX_MALE;
	else
		sex = SEX_FEMALE;

	finishInitiateGame(nameBuffer, sex);
}

void IntroController::finishInitiateGame(const Common::String &nameBuffer, SexType sex) {
#ifdef IOS
	mode = INTRO_MENU; // ensure we are now in the menu mode, (i.e., stop drawing the map).
#endif
	// no more text entry, so disable the text cursor
	_menuArea.disableCursor();

	// show the lead up story
	showStory();

	// ask questions that determine character class
	startQuestions();

	// write out save game an segue into game
	SaveGame saveGame;
	SaveGamePlayerRecord avatar;

	Common::OutSaveFile *saveGameFile = g_system->getSavefileManager()->openForSaving(PARTY_SAV_BASE_FILENAME);
	if (!saveGameFile) {
		_questionArea.disableCursor();
		_errorMessage = "Unable to create save game!";
		updateScreen();
		return;
	}

	avatar.init();
	strcpy(avatar.name, nameBuffer.c_str());
	avatar._sex = sex;
	saveGame.init(&avatar);
	screenHideCursor();
	initPlayers(&saveGame);
	saveGame._food = 30000;
	saveGame._gold = 200;
	saveGame._reagents[REAG_GINSENG] = 3;
	saveGame._reagents[REAG_GARLIC] = 4;
	saveGame._torches = 2;

	Common::Serializer ser(nullptr, saveGameFile);
	saveGame.synchronize(ser);

	saveGameFile->finalize();
	delete saveGameFile;

	saveGameFile = g_system->getSavefileManager()->openForSaving(MONSTERS_SAV_BASE_FILENAME);
	if (saveGameFile) {
		Common::Serializer ser2(nullptr, saveGameFile);
		SaveGameMonsterRecord::synchronize(nullptr, ser2);
		delete saveGameFile;
	}
	_justInitiatedNewGame = true;

	// show the text thats segues into the main game
	showText(_binData->_introGypsy[GYP_SEGUE1]);
#ifdef IOS
	U4IOS::switchU4IntroControllerToContinueButton();
#endif
	ReadChoiceController pauseController("");
	eventHandler->pushController(&pauseController);
	pauseController.waitFor();

	showText(_binData->_introGypsy[GYP_SEGUE2]);

	eventHandler->pushController(&pauseController);
	pauseController.waitFor();

	// done: exit intro and let game begin
	_questionArea.disableCursor();
	EventHandler::setControllerDone();

	return;
}

void IntroController::showStory() {
	ReadChoiceController pauseController("");

	_beastiesVisible = false;

	_questionArea.setCursorFollowsText(true);

	for (int storyInd = 0; storyInd < 24; storyInd++) {
		if (storyInd == 0)
			_backgroundArea.draw(BKGD_TREE);
		else if (storyInd == 3)
			animateTree("moongate");
		else if (storyInd == 5)
			animateTree("items");
		else if (storyInd == 6)
			_backgroundArea.draw(BKGD_PORTAL);
		else if (storyInd == 11)
			_backgroundArea.draw(BKGD_TREE);
		else if (storyInd == 15)
			_backgroundArea.draw(BKGD_OUTSIDE);
		else if (storyInd == 17)
			_backgroundArea.draw(BKGD_INSIDE);
		else if (storyInd == 20)
			_backgroundArea.draw(BKGD_WAGON);
		else if (storyInd == 21)
			_backgroundArea.draw(BKGD_GYPSY);
		else if (storyInd == 23)
			_backgroundArea.draw(BKGD_ABACUS);

		showText(_binData->_introText[storyInd]);

		eventHandler->pushController(&pauseController);
		// enable the cursor here to avoid drawing in undesirable locations
		_questionArea.enableCursor();
		pauseController.waitFor();
	}
}

/**
 * Starts the gypsys questioning that eventually determines the new
 * characters class.
 */
void IntroController::startQuestions() {
	ReadChoiceController pauseController("");
	ReadChoiceController questionController("ab");

	_questionRound = 0;
	initQuestionTree();

	while (1) {
		// draw the abacus background, if necessary
		if (_questionRound == 0)
			_backgroundArea.draw(BKGD_ABACUS);

		// draw the cards and show the lead up text
		drawCard(0, _questionTree[_questionRound * 2]);
		drawCard(1, _questionTree[_questionRound * 2 + 1]);

		_questionArea.clear();
		_questionArea.textAt(0, 0, "%s", _binData->_introGypsy[_questionRound == 0 ? GYP_PLACES_FIRST : (_questionRound == 6 ? GYP_PLACES_LAST : GYP_PLACES_TWOMORE)].c_str());
		_questionArea.textAt(0, 1, "%s", _binData->_introGypsy[GYP_UPON_TABLE].c_str());
		_questionArea.textAt(0, 2, "%s and %s.  She says",
		                     _binData->_introGypsy[_questionTree[_questionRound * 2] + 4].c_str(),
		                     _binData->_introGypsy[_questionTree[_questionRound * 2 + 1] + 4].c_str());
		_questionArea.textAt(0, 3, "\"Consider this:\"");

#ifdef IOS
		U4IOS::switchU4IntroControllerToContinueButton();
#endif
		// wait for a key
		eventHandler->pushController(&pauseController);
		pauseController.waitFor();

		screenEnableCursor();
		// show the question to choose between virtues
		showText(getQuestion(_questionTree[_questionRound * 2], _questionTree[_questionRound * 2 + 1]));

#ifdef IOS
		U4IOS::switchU4IntroControllerToABButtons();
#endif
		// wait for an answer
		eventHandler->pushController(&questionController);
		int choice = questionController.waitFor();

		// update the question tree
		if (doQuestion(choice == 'a' ? 0 : 1)) {
			return;
		}
	}
}

/**
 * Get the text for the question giving a choice between virtue v1 and
 * virtue v2 (zero based virtue index, starting at honesty).
 */
Common::String IntroController::getQuestion(int v1, int v2) {
	int i = 0;
	int d = 7;

	ASSERT(v1 < v2, "first virtue must be smaller (v1 = %d, v2 = %d)", v1, v2);

	while (v1 > 0) {
		i += d;
		d--;
		v1--;
		v2--;
	}

	ASSERT((i + v2 - 1) < 28, "calculation failed");

	return _binData->_introQuestions[i + v2 - 1];
}

/**
 * Starts the game.
 */
void IntroController::journeyOnward() {
	Common::InSaveFile *saveGameFile;
	bool validSave = false;

	/*
	 * ensure a party.sav file exists, otherwise require user to
	 * initiate game
	 */
	saveGameFile = g_system->getSavefileManager()->openForLoading(PARTY_SAV_BASE_FILENAME);
	if (saveGameFile) {
		SaveGame *saveGame = new SaveGame;

		// Make sure there are players in party.sav --
		// In the Ultima Collection CD, party.sav exists, but does
		// not contain valid info to journey onward
		Common::Serializer ser(saveGameFile, nullptr);
		saveGame->synchronize(ser);

		if (saveGame->_members > 0)
			validSave = true;
		delete saveGame;
		delete saveGameFile;
	}

	if (!validSave) {
		_errorMessage = "Initiate a new game first!";
		updateScreen();
		g_screen->update();
		return;
	}

	EventHandler::setControllerDone();
}

/**
 * Shows an about box.
 */
void IntroController::about() {
#ifdef TODO
	// draw the extended background for all option screens
	backgroundArea.draw(BKGD_INTRO);
	backgroundArea.draw(BKGD_OPTIONS_BTM, 0, 120);

	screenHideCursor();
	menuArea.textAt(14, 1, "XU4 %s", VERSION);
	menuArea.textAt(1, 3, "xu4 is free software; you can redist-");
	menuArea.textAt(1, 4, "ribute it and/or modify it under the");
	menuArea.textAt(1, 5, "terms of the GNU GPL as published by");
	menuArea.textAt(1, 6, "the FSF.  See COPYING.");
	menuArea.textAt(4, 8, "Copyright \011 2002-2006, xu4 Team");
	menuArea.textAt(4, 9, "Copyright \011 1987, Lord British");
	drawBeasties();

	ReadChoiceController::get("");

	screenShowCursor();
	updateScreen();
#endif
}

/**
 * Shows text in the question area.
 */
void IntroController::showText(const Common::String &text) {
	Common::String current = text;
	int lineNo = 0;

	_questionArea.clear();

	unsigned long pos = current.find("\n");
	while (pos < current.size()) {
		_questionArea.textAt(0, lineNo++, "%s", current.substr(0, pos).c_str());
		current = current.substr(pos + 1);
		pos = current.find("\n");
	}

	/* write the last line (possibly only line) */
	_questionArea.textAt(0, lineNo++, "%s", current.substr(0, pos).c_str());
}

/**
 * Run a menu and return when the menu has been closed.  Screen
 * updates are handled by observing the menu.
 */
void IntroController::runMenu(Menu *menu, TextView *view, bool withBeasties) {
	menu->addObserver(this);
	menu->reset();

	// if the menu has an extended height, fill the menu background, otherwise reset the display
	menu->show(view);
	if (withBeasties)
		drawBeasties();

	MenuController menuController(menu, view);
	eventHandler->pushController(&menuController);
	menuController.waitFor();

	// enable the cursor here, after the menu has been established
	view->enableCursor();
	menu->deleteObserver(this);
	view->disableCursor();
}

/**
 * Timer callback for the intro sequence.  Handles animating the intro
 * map, the beasties, etc..
 */
void IntroController::timerFired() {
	screenCycle();
	screenUpdateCursor();

	if (_mode == INTRO_TITLES)
		if (updateTitle() == false) {
			// setup the map screen
			_mode = INTRO_MAP;
			_beastiesVisible = true;
			musicMgr->intro();
			updateScreen();
		}

	if (_mode == INTRO_MAP)
		drawMap();

	if (_beastiesVisible)
		drawBeasties();

	/*
	 * refresh the screen only if the timer queue is empty --
	 * i.e. drop a frame if another timer event is about to be fired
	 */
	if (EventHandler::timerQueueEmpty())
		g_screen->update();

	if (xu4_random(2) && ++_beastie1Cycle >= IntroBinData::BEASTIE1_FRAMES)
		_beastie1Cycle = 0;
	if (xu4_random(2) && ++_beastie2Cycle >= IntroBinData::BEASTIE2_FRAMES)
		_beastie2Cycle = 0;
}

/**
 * Update the screen when an observed menu is reset or has an item
 * activated.
 * TODO, reduce duped code.
 */
void IntroController::update(Menu *menu, MenuEvent &event) {
	if (menu == &_confMenu)
		updateConfMenu(event);
	else if (menu == &_videoMenu)
		updateVideoMenu(event);
	else if (menu == &_gfxMenu)
		updateGfxMenu(event);
	else if (menu == &_soundMenu)
		updateSoundMenu(event);
	else if (menu == &_inputMenu)
		updateInputMenu(event);
	else if (menu == &_speedMenu)
		updateSpeedMenu(event);
	else if (menu == &_gameplayMenu)
		updateGameplayMenu(event);
	else if (menu == &_interfaceMenu)
		updateInterfaceMenu(event);

	// beasties are always visible on the menus
	drawBeasties();
}

void IntroController::updateConfMenu(MenuEvent &event) {
	if (event.getType() == MenuEvent::ACTIVATE ||
	        event.getType() == MenuEvent::INCREMENT ||
	        event.getType() == MenuEvent::DECREMENT) {

		// show or hide game enhancement options if enhancements are enabled/disabled
		_confMenu.getItemById(MI_CONF_GAMEPLAY)->setVisible(settingsChanged._enhancements);
		_confMenu.getItemById(MI_CONF_INTERFACE)->setVisible(settingsChanged._enhancements);

		// save settings
		settings.setData(settingsChanged);
		settings.write();

		switch (event.getMenuItem()->getId()) {
		case MI_CONF_VIDEO:
			runMenu(&_videoMenu, &_extendedMenuArea, true);
			break;
		case MI_VIDEO_CONF_GFX:
			runMenu(&_gfxMenu, &_extendedMenuArea, true);
			break;
		case MI_CONF_SOUND:
			runMenu(&_soundMenu, &_extendedMenuArea, true);
			break;
		case MI_CONF_INPUT:
			runMenu(&_inputMenu, &_extendedMenuArea, true);
			break;
		case MI_CONF_SPEED:
			runMenu(&_speedMenu, &_extendedMenuArea, true);
			break;
		case MI_CONF_GAMEPLAY:
			runMenu(&_gameplayMenu, &_extendedMenuArea, true);
			break;
		case MI_CONF_INTERFACE:
			runMenu(&_interfaceMenu, &_extendedMenuArea, true);
			break;
		case CANCEL:
			// discard settings
			settingsChanged = settings;
			break;
		default:
			break;
		}
	}

	// draw the extended background for all option screens
	_backgroundArea.draw(BKGD_OPTIONS_TOP, 0, 0);
	_backgroundArea.draw(BKGD_OPTIONS_BTM, 0, 120);
}

void IntroController::updateVideoMenu(MenuEvent &event) {
	if (event.getType() == MenuEvent::ACTIVATE ||
	        event.getType() == MenuEvent::INCREMENT ||
	        event.getType() == MenuEvent::DECREMENT) {

		switch (event.getMenuItem()->getId()) {
		case USE_SETTINGS:
			/* save settings (if necessary) */
			if (settings != settingsChanged) {
				settings.setData(settingsChanged);
				settings.write();

				/* FIXME: resize images, etc. */
				screenReInit();

				// go back to menu mode
				_mode = INTRO_MENU;
			}
			break;
		case MI_VIDEO_CONF_GFX:
			runMenu(&_gfxMenu, &_extendedMenuArea, true);
			break;
		case CANCEL:
			// discard settings
			settingsChanged = settings;
			break;
		default:
			break;
		}
	}

	// draw the extended background for all option screens
	_backgroundArea.draw(BKGD_OPTIONS_TOP, 0, 0);
	_backgroundArea.draw(BKGD_OPTIONS_BTM, 0, 120);
}

void IntroController::updateGfxMenu(MenuEvent &event) {
	if (event.getType() == MenuEvent::ACTIVATE ||
	        event.getType() == MenuEvent::INCREMENT ||
	        event.getType() == MenuEvent::DECREMENT) {


		switch (event.getMenuItem()->getId()) {
		case MI_GFX_RETURN:
			runMenu(&_videoMenu, &_extendedMenuArea, true);
			break;
		default:
			break;
		}
	}

	// draw the extended background for all option screens
	_backgroundArea.draw(BKGD_OPTIONS_TOP, 0, 0);
	_backgroundArea.draw(BKGD_OPTIONS_BTM, 0, 120);
}

void IntroController::updateSoundMenu(MenuEvent &event) {
	if (event.getType() == MenuEvent::ACTIVATE ||
	        event.getType() == MenuEvent::INCREMENT ||
	        event.getType() == MenuEvent::DECREMENT) {

		switch (event.getMenuItem()->getId()) {
		case MI_SOUND_01:
			musicMgr->setMusicVolume(settingsChanged._musicVol);
			break;
		case MI_SOUND_02:
			musicMgr->setSoundVolume(settingsChanged._soundVol);
			soundPlay(SOUND_FLEE);
			break;
		case USE_SETTINGS:
			// save settings
			settings.setData(settingsChanged);
			settings.write();
			musicMgr->intro();
			break;
		case CANCEL:
			musicMgr->setMusicVolume(settings._musicVol);
			musicMgr->setSoundVolume(settings._soundVol);
			// discard settings
			settingsChanged = settings;
			break;
		default:
			break;
		}
	}

	// draw the extended background for all option screens
	_backgroundArea.draw(BKGD_OPTIONS_TOP, 0, 0);
	_backgroundArea.draw(BKGD_OPTIONS_BTM, 0, 120);
}

void IntroController::updateInputMenu(MenuEvent &event) {
	if (event.getType() == MenuEvent::ACTIVATE ||
	        event.getType() == MenuEvent::INCREMENT ||
	        event.getType() == MenuEvent::DECREMENT) {

		switch (event.getMenuItem()->getId()) {
		case USE_SETTINGS:
			// save settings
			settings.setData(settingsChanged);
			settings.write();

			// re-initialize keyboard
			KeyHandler::setKeyRepeat(settingsChanged._keydelay, settingsChanged._keyinterval);

#ifdef SLACK_ON_SDL_AGNOSTICISM
			if (settings.mouseOptions.enabled) {
				SDL_ShowCursor(SDL_ENABLE);
			} else {
				SDL_ShowCursor(SDL_DISABLE);
			}
#endif

			break;
		case CANCEL:
			// discard settings
			settingsChanged = settings;
			break;
		default:
			break;
		}
	}

	// draw the extended background for all option screens
	_backgroundArea.draw(BKGD_OPTIONS_TOP, 0, 0);
	_backgroundArea.draw(BKGD_OPTIONS_BTM, 0, 120);

	// after drawing the menu, extra menu text can be added here
	_extendedMenuArea.textAt(0, 5, "Mouse Options:");
}

void IntroController::updateSpeedMenu(MenuEvent &event) {
	if (event.getType() == MenuEvent::ACTIVATE ||
	        event.getType() == MenuEvent::INCREMENT ||
	        event.getType() == MenuEvent::DECREMENT) {

		switch (event.getMenuItem()->getId()) {
		case USE_SETTINGS:
			// save settings
			settings.setData(settingsChanged);
			settings.write();

			// re-initialize events
			eventTimerGranularity = (1000 / settings._gameCyclesPerSecond);
			eventHandler->getTimer()->reset(eventTimerGranularity);

			break;
		case CANCEL:
			// discard settings
			settingsChanged = settings;
			break;
		default:
			break;
		}
	}

	// draw the extended background for all option screens
	_backgroundArea.draw(BKGD_OPTIONS_TOP, 0, 0);
	_backgroundArea.draw(BKGD_OPTIONS_BTM, 0, 120);
}

void IntroController::updateGameplayMenu(MenuEvent &event) {
	if (event.getType() == MenuEvent::ACTIVATE ||
	        event.getType() == MenuEvent::INCREMENT ||
	        event.getType() == MenuEvent::DECREMENT) {

		switch (event.getMenuItem()->getId()) {
		case USE_SETTINGS:
			// save settings
			settings.setData(settingsChanged);
			settings.write();
			break;
		case CANCEL:
			// discard settings
			settingsChanged = settings;
			break;
		default:
			break;
		}
	}

	// draw the extended background for all option screens
	_backgroundArea.draw(BKGD_OPTIONS_TOP, 0, 0);
	_backgroundArea.draw(BKGD_OPTIONS_BTM, 0, 120);
}

void IntroController::updateInterfaceMenu(MenuEvent &event) {
	if (event.getType() == MenuEvent::ACTIVATE ||
	        event.getType() == MenuEvent::INCREMENT ||
	        event.getType() == MenuEvent::DECREMENT) {

		switch (event.getMenuItem()->getId()) {
		case USE_SETTINGS:
			// save settings
			settings.setData(settingsChanged);
			settings.write();
			break;
		case CANCEL:
			// discard settings
			settingsChanged = settings;
			break;
		default:
			break;
		}
	}

	// draw the extended background for all option screens
	_backgroundArea.draw(BKGD_OPTIONS_TOP, 0, 0);
	_backgroundArea.draw(BKGD_OPTIONS_BTM, 0, 120);

	// after drawing the menu, extra menu text can be added here
	_extendedMenuArea.textAt(2, 3, "  (Open, Jimmy, etc.)");
}

/**
 * Initializes the question tree.  The tree starts off with the first
 * eight entries set to the numbers 0-7 in a random order.
 */
void IntroController::initQuestionTree() {
	int i, tmp, r;

	for (i = 0; i < 8; i++)
		_questionTree[i] = i;

	for (i = 0; i < 8; i++) {
		r = xu4_random(8);
		tmp = _questionTree[r];
		_questionTree[r] = _questionTree[i];
		_questionTree[i] = tmp;
	}
	_answerInd = 8;

	if (_questionTree[0] > _questionTree[1]) {
		tmp = _questionTree[0];
		_questionTree[0] = _questionTree[1];
		_questionTree[1] = tmp;
	}

}

/**
 * Updates the question tree with the given answer, and advances to
 * the next round.
 * @return true if all questions have been answered, false otherwise
 */
bool IntroController::doQuestion(int answer) {
	if (!answer)
		_questionTree[_answerInd] = _questionTree[_questionRound * 2];
	else
		_questionTree[_answerInd] = _questionTree[_questionRound * 2 + 1];

	drawAbacusBeads(_questionRound, _questionTree[_answerInd],
	                _questionTree[_questionRound * 2 + ((answer) ? 0 : 1)]);

	_answerInd++;
	_questionRound++;

	if (_questionRound > 6)
		return true;

	if (_questionTree[_questionRound * 2] > _questionTree[_questionRound * 2 + 1]) {
		int tmp = _questionTree[_questionRound * 2];
		_questionTree[_questionRound * 2] = _questionTree[_questionRound * 2 + 1];
		_questionTree[_questionRound * 2 + 1] = tmp;
	}

	return false;
}

/**
 * Build the initial avatar player record from the answers to the
 * gypsy's questions.
 */
void IntroController::initPlayers(SaveGame *saveGame) {
	int i, p;
	static const struct {
		WeaponType weapon;
		ArmorType armor;
		int level, xp, x, y;
	} initValuesForClass[] = {
		{ WEAP_STAFF,  ARMR_CLOTH,   2, 125, 231, 136 }, /* CLASS_MAGE */
		{ WEAP_SLING,  ARMR_CLOTH,   3, 240,  83, 105 }, /* CLASS_BARD */
		{ WEAP_AXE,    ARMR_LEATHER, 3, 205,  35, 221 }, /* CLASS_FIGHTER */
		{ WEAP_DAGGER, ARMR_CLOTH,   2, 175,  59,  44 }, /* CLASS_DRUID */
		{ WEAP_MACE,   ARMR_LEATHER, 2, 110, 158,  21 }, /* CLASS_TINKER */
		{ WEAP_SWORD,  ARMR_CHAIN,   3, 325, 105, 183 }, /* CLASS_PALADIN */
		{ WEAP_SWORD,  ARMR_LEATHER, 2, 150,  23, 129 }, /* CLASS_RANGER */
		{ WEAP_STAFF,  ARMR_CLOTH,   1,   5, 186, 171 }  /* CLASS_SHEPHERD */
	};
	static const struct {
		const char *name;
		int str, dex, intel;
		SexType sex;
	} initValuesForNpcClass[] = {
		{ "Mariah",    9, 12, 20, SEX_FEMALE }, /* CLASS_MAGE */
		{ "Iolo",     16, 19, 13, SEX_MALE },   /* CLASS_BARD */
		{ "Geoffrey", 20, 15, 11, SEX_MALE },   /* CLASS_FIGHTER */
		{ "Jaana",    17, 16, 13, SEX_FEMALE }, /* CLASS_DRUID */
		{ "Julia",    15, 16, 12, SEX_FEMALE }, /* CLASS_TINKER */
		{ "Dupre",    17, 14, 17, SEX_MALE },   /* CLASS_PALADIN */
		{ "Shamino",  16, 15, 15, SEX_MALE },   /* CLASS_RANGER */
		{ "Katrina",  11, 12, 10, SEX_FEMALE }  /* CLASS_SHEPHERD */
	};

	saveGame->_players[0]._class = static_cast<ClassType>(_questionTree[14]);

	ASSERT(saveGame->_players[0]._class < 8, "bad class: %d", saveGame->_players[0]._class);

	saveGame->_players[0]._weapon = initValuesForClass[saveGame->_players[0]._class].weapon;
	saveGame->_players[0].armor = initValuesForClass[saveGame->_players[0]._class].armor;
	saveGame->_players[0]._xp = initValuesForClass[saveGame->_players[0]._class].xp;
	saveGame->_x = initValuesForClass[saveGame->_players[0]._class].x;
	saveGame->_y = initValuesForClass[saveGame->_players[0]._class].y;

	saveGame->_players[0]._str = 15;
	saveGame->_players[0]._dex = 15;
	saveGame->_players[0]._intel = 15;

	for (i = 0; i < VIRT_MAX; i++)
		saveGame->_karma[i] = 50;

	for (i = 8; i < 15; i++) {
		saveGame->_karma[_questionTree[i]] += 5;
		switch (_questionTree[i]) {
		case VIRT_HONESTY:
			saveGame->_players[0]._intel += 3;
			break;
		case VIRT_COMPASSION:
			saveGame->_players[0]._dex += 3;
			break;
		case VIRT_VALOR:
			saveGame->_players[0]._str += 3;
			break;
		case VIRT_JUSTICE:
			saveGame->_players[0]._intel++;
			saveGame->_players[0]._dex++;
			break;
		case VIRT_SACRIFICE:
			saveGame->_players[0]._dex++;
			saveGame->_players[0]._str++;
			break;
		case VIRT_HONOR:
			saveGame->_players[0]._intel++;
			saveGame->_players[0]._str++;
			break;
		case VIRT_SPIRITUALITY:
			saveGame->_players[0]._intel++;
			saveGame->_players[0]._dex++;
			saveGame->_players[0]._str++;
			break;
		case VIRT_HUMILITY:
			/* no stats for you! */
			break;
		}
	}

	PartyMember player(NULL, &saveGame->_players[0]);
	saveGame->_players[0]._hp = saveGame->_players[0]._hpMax = player.getMaxLevel() * 100;
	saveGame->_players[0]._mp = player.getMaxMp();

	p = 1;
	for (i = 0; i < VIRT_MAX; i++) {
		player = PartyMember(NULL, &saveGame->_players[i]);

		/* Initial setup for party members that aren't in your group yet... */
		if (i != saveGame->_players[0]._class) {
			saveGame->_players[p]._class = static_cast<ClassType>(i);
			saveGame->_players[p]._xp = initValuesForClass[i].xp;
			saveGame->_players[p]._str = initValuesForNpcClass[i].str;
			saveGame->_players[p]._dex = initValuesForNpcClass[i].dex;
			saveGame->_players[p]._intel = initValuesForNpcClass[i].intel;
			saveGame->_players[p]._weapon = initValuesForClass[i].weapon;
			saveGame->_players[p].armor = initValuesForClass[i].armor;
			strcpy(saveGame->_players[p].name, initValuesForNpcClass[i].name);
			saveGame->_players[p]._sex = initValuesForNpcClass[i].sex;
			saveGame->_players[p]._hp = saveGame->_players[p]._hpMax = initValuesForClass[i].level * 100;
			saveGame->_players[p]._mp = player.getMaxMp();
			p++;
		}
	}
}


/**
 * Preload map tiles
 */
void IntroController::preloadMap() {
	int x, y, i;

	// draw unmodified map
	for (y = 0; y < INTRO_MAP_HEIGHT; y++)
		for (x = 0; x < INTRO_MAP_WIDTH; x++)
			_mapArea.loadTile(_binData->_introMap[x + (y * INTRO_MAP_WIDTH)]);

	// draw animated objects
	for (i = 0; i < IntroBinData::INTRO_BASETILE_TABLE_SIZE; i++) {
		if (_objectStateTable[i].tile != 0)
			_mapArea.loadTile(_objectStateTable[i].tile);
	}
}


//
// Initialize the title elements
//
void IntroController::initTitles() {
	// add the intro elements
	//          x,  y,   w,  h, method,  delay, duration
	//
	addTitle(97,  0, 130, 16, SIGNATURE,   1000, 3000);     // "Lord British"
	addTitle(148, 17,  24,  4, AND,         1000,  100);    // "and"
	addTitle(84, 31, 152,  1, BAR,         1000,  500);     // <bar>
	addTitle(86, 21, 148,  9, ORIGIN,      1000,  100);     // "Origin Systems, Inc."
	addTitle(133, 33,  54,  5, PRESENT,        0,  100);    // "present"
	addTitle(59, 33, 202, 46, TITLE,       1000, 5000);     // "Ultima IV"
	addTitle(40, 80, 240, 13, SUBTITLE,    1000,  100);     // "Quest of the Avatar"
	addTitle(0, 96, 320, 96, MAP,         1000,  100);      // the map

	// get the source data for the elements
	getTitleSourceData();

	// reset the iterator
	_title = _titles.begin();

	// speed up the timer while the intro titles are displayed
	eventHandler->getTimer()->reset(settings._titleSpeedOther);
}


//
// Add the intro element to the element list
//
void IntroController::addTitle(int x, int y, int w, int h, AnimType method, int delay, int duration) {
	AnimElement data = {
		x, y,               // source x and y
		w, h,               // source width and height
		method,             // render method
		0,                  // animStep
		0,                  // animStepMax
		0,                  // timeBase
		delay,              // delay before rendering begins
		duration,           // total animation time
		NULL,               // storage for the source image
		NULL,               // storage for the animation frame
		Std::vector<AnimPlot>(),
		false
	};             // prescaled
	_titles.push_back(data);
}


//
// Get the source data for title elements
// that have already been initialized
//
void IntroController::getTitleSourceData() {
	unsigned int r, g, b, a;        // color values
	unsigned char *srcData;         // plot data
	const int BLUE[16] = {
		255, 250, 226, 226, 210, 194, 161, 161,
		129,  97,  97,  64,  64,  32,  32,   0
	};

	// The BKGD_INTRO image is assumed to have not been
	// loaded yet.  The unscaled version will be loaded
	// here, and elements of the image will be stored
	// individually.  Afterward, the BKGD_INTRO image
	// will be scaled appropriately.
	ImageInfo *info = imageMgr->get(BKGD_INTRO, true);
	if (!info) {
		errorFatal("ERROR 1007: Unable to load the image \"%s\".\t\n\nIs %s installed?\n\nVisit the XU4 website for additional information.\n\thttp://xu4.sourceforge.net/", BKGD_INTRO, settings._game.c_str());
	}

	if (info->_width / info->_prescale != 320 || info->_height / info->_prescale != 200) {
		// the image appears to have been scaled already
		errorWarning("ERROR 1008: The title image (\"%s\") has been scaled too early!\t\n\nVisit the XU4 website for additional information.\n\thttp://xu4.sourceforge.net/", BKGD_INTRO);
	}

	// get the transparent color
	_transparentColor = info->_image->getPaletteColor(_transparentIndex);

	// turn alpha off, if necessary
	bool alpha = info->_image->isAlphaOn();
	info->_image->alphaOff();

	// for each element, get the source data
	for (unsigned i = 0; i < _titles.size(); i++) {
		if ((_titles[i]._method != SIGNATURE) && (_titles[i]._method != BAR)) {
			// create a place to store the source image
			_titles[i]._srcImage = Image::create(
				_titles[i]._rw * info->_prescale,
				_titles[i]._rh * info->_prescale,
			    info->_image->isIndexed() && _titles[i]._method != MAP,
				Image::HARDWARE);
			if (_titles[i]._srcImage->isIndexed())
				_titles[i]._srcImage->setPaletteFromImage(info->_image);

			// get the source image
			info->_image->drawSubRectOn(
			    _titles[i]._srcImage,
			    0,
			    0,
			    _titles[i]._rx * info->_prescale,
			    _titles[i]._ry * info->_prescale,
			    _titles[i]._rw * info->_prescale,
			    _titles[i]._rh * info->_prescale);
		}

		// after getting the srcImage
		switch (_titles[i]._method) {
		case SIGNATURE: {
			// PLOT: "Lord British"
			srcData = intro->getSigData();
			RGBA color = info->_image->setColor(0, 255, 255);    // cyan for EGA
			int x = 0, y = 0;

			while (srcData[_titles[i]._animStepMax] != 0) {
				x = srcData[_titles[i]._animStepMax] - 0x4C;
				y = 0xC0 - srcData[_titles[i]._animStepMax + 1];

				if (settings._videoType != "EGA") {
					// yellow gradient
					color = info->_image->setColor(255, (y == 2 ? 250 : 255), BLUE[y - 1]);
				}
				AnimPlot plot = {
					(uint8)x,
					(uint8)y,
					(uint8)color.r,
					(uint8)color.g,
					(uint8)color.b,
					255
				};
				_titles[i]._plotData.push_back(plot);
				_titles[i]._animStepMax += 2;
			}
			_titles[i]._animStepMax = _titles[i]._plotData.size();
			break;
		}

		case BAR: {
			_titles[i]._animStepMax = _titles[i]._rw;  // image width
			break;
		}

		case TITLE: {
			for (int y = 0; y < _titles[i]._rh; y++) {
				for (int x = 0; x < _titles[i]._rw ; x++) {
					_titles[i]._srcImage->getPixel(x * info->_prescale, y * info->_prescale, r, g, b, a);
					if (r || g || b) {
						AnimPlot plot = { (uint8)(x + 1), (uint8)(y + 1), (uint8)r, (uint8)g, (uint8)b, (uint8)a };
						_titles[i]._plotData.push_back(plot);
					}
				}
			}
			_titles[i]._animStepMax = _titles[i]._plotData.size();
			break;
		}

		case MAP: {
			// fill the map area with the transparent color
			_titles[i]._srcImage->fillRect(
			    8, 8, 304, 80,
			    _transparentColor.r,
			    _transparentColor.g,
			    _transparentColor.b);

			Image *scaled;      // the scaled and filtered image
			scaled = screenScale(_titles[i]._srcImage, settings._scale / info->_prescale, 1, 1);
			if (_transparentIndex >= 0)
				scaled->setTransparentIndex(_transparentIndex);

			_titles[i]._prescaled = true;
			delete _titles[i]._srcImage;
			_titles[i]._srcImage = scaled;

			_titles[i]._animStepMax = 20;
			break;
		}

		default: {
			_titles[i]._animStepMax = _titles[i]._rh ;  // image height
			break;
		}
		}

		// permanently disable alpha
		if (_titles[i]._srcImage)
			_titles[i]._srcImage->alphaOff();

		bool indexed = info->_image->isIndexed() && _titles[i]._method != MAP;
		// create the initial animation frame
		_titles[i]._destImage = Image::create(
			2 + (_titles[i]._prescaled ? SCALED(_titles[i]._rw) : _titles[i]._rw) * info->_prescale ,
		    2 + (_titles[i]._prescaled ? SCALED(_titles[i]._rh) : _titles[i]._rh) * info->_prescale,
		    indexed,
			Image::HARDWARE);
		if (indexed)
			_titles[i]._destImage->setPaletteFromImage(info->_image);
	}

	// turn alpha back on
	if (alpha) {
		info->_image->alphaOn();
	}

	// scale the original image now
	Image *scaled = screenScale(info->_image,
	                            settings._scale / info->_prescale,
	                            info->_image->isIndexed(),
	                            1);
	delete info->_image;
	info->_image = scaled;
}



#ifdef SLACK_ON_SDL_AGNOSTICISM
int getTicks() {
	return SDL_GetTicks();
}
#elif !defined(IOS)
static int ticks = 0;
int getTicks() {
	ticks += 1000;
	return ticks;
}
#endif

//
// Update the title element, drawing the appropriate frame of animation
//
bool IntroController::updateTitle() {
#ifdef IOS
	static bool firstTime = true;
	if (firstTime) {
		firstTime = false;
		startTicks();
	}
#endif

	int animStepTarget = 0;

	int timeCurrent = getTicks();
	float timePercent = 0;

	if (_title->_animStep == 0 && !_bSkipTitles) {
		if (_title->_timeBase == 0) {
			// reset the base time
			_title->_timeBase = timeCurrent;
		}
		if (_title == _titles.begin()) {
			// clear the screen
			Image *screen = imageMgr->get("screen")->_image;
			screen->fillRect(0, 0, screen->width(), screen->height(), 0, 0, 0);
		}
		if (_title->_method == TITLE) {
			// assume this is the first frame of "Ultima IV" and begin sound
			soundPlay(SOUND_TITLE_FADE);
		}
	}

	// abort after processing all elements
	if (_title == _titles.end()) {
		return false;
	}

	// delay the drawing of this phase
	if ((timeCurrent - _title->_timeBase) < _title->_timeDelay) {
		return true;
	}

	// determine how much of the animation should have been drawn up until now
	timePercent = float(timeCurrent - _title->_timeBase - _title->_timeDelay) / _title->_timeDuration;
	if (timePercent > 1 || _bSkipTitles)
		timePercent = 1;
	animStepTarget = int(_title->_animStepMax * timePercent);

	// perform the animation
	switch (_title->_method) {
	case SIGNATURE: {
		while (animStepTarget > _title->_animStep) {
			// blit the pixel-pair to the src surface
			_title->_destImage->fillRect(
			    _title->_plotData[_title->_animStep].x,
			    _title->_plotData[_title->_animStep].y,
			    2,
			    1,
			    _title->_plotData[_title->_animStep].r,
			    _title->_plotData[_title->_animStep].g,
			    _title->_plotData[_title->_animStep].b);
			_title->_animStep++;
		}
		break;
	}

	case BAR: {
		RGBA color;
		while (animStepTarget > _title->_animStep) {
			_title->_animStep++;
			color = _title->_destImage->setColor(128, 0, 0); // dark red for the underline

			// blit bar to the canvas
			_title->_destImage->fillRect(
			    1,
			    1,
			    _title->_animStep,
			    1,
			    color.r,
			    color.g,
			    color.b);
		}
		break;
	}

	case AND: {
		// blit the entire src to the canvas
		_title->_srcImage->drawOn(_title->_destImage, 1, 1);
		_title->_animStep = _title->_animStepMax;
		break;
	}

	case ORIGIN: {
		if (_bSkipTitles)
			_title->_animStep = _title->_animStepMax;
		else {
			_title->_animStep++;
			_title->_timeDelay = getTicks() - _title->_timeBase + 100;
		}

		// blit src to the canvas one row at a time, bottom up
		_title->_srcImage->drawSubRectOn(
		    _title->_destImage,
		    1,
		    _title->_destImage->height() - 1 - _title->_animStep,
		    0,
		    0,
		    _title->_srcImage->width(),
		    _title->_animStep);
		break;
	}

	case PRESENT: {
		if (_bSkipTitles)
			_title->_animStep = _title->_animStepMax;
		else {
			_title->_animStep++;
			_title->_timeDelay = getTicks() - _title->_timeBase + 100;
		}

		// blit src to the canvas one row at a time, top down
		_title->_srcImage->drawSubRectOn(
		    _title->_destImage,
		    1,
		    1,
		    0,
		    _title->_srcImage->height() - _title->_animStep,
		    _title->_srcImage->width(),
		    _title->_animStep);
		break;
	}

	case TITLE: {
		// blit src to the canvas in a random pixelized manner
		_title->_animStep = animStepTarget;
#ifdef TODO
		random_shuffle(title->plotData.begin(), title->plotData.end());
#endif
		_title->_destImage->fillRect(1, 1, _title->_rw, _title->_rh, 0, 0, 0);

		// @TODO: animStepTarget (for this loop) should not exceed
		// half of animStepMax.  If so, instead draw the entire
		// image, and then black out the necessary pixels.
		// this should speed the loop up at the end
		for (int i = 0; i < animStepTarget; ++i) {
			_title->_destImage->putPixel(
			    _title->_plotData[i].x,
			    _title->_plotData[i].y,
			    _title->_plotData[i].r,
			    _title->_plotData[i].g,
			    _title->_plotData[i].b,
			    _title->_plotData[i].a);
		}

		// cover the "present" area with the transparent color
		_title->_destImage->fillRect(
		    75, 1, 54, 5,
		    _transparentColor.r,
		    _transparentColor.g,
		    _transparentColor.b);
		break;
	}

	case SUBTITLE: {
		if (_bSkipTitles)
			_title->_animStep = _title->_animStepMax;
		else {
			_title->_animStep++;
			_title->_timeDelay = getTicks() - _title->_timeBase + 100;
		}

		// blit src to the canvas one row at a time, center out
		int y = int(_title->_rh / 2) - _title->_animStep + 1;
		_title->_srcImage->drawSubRectOn(
		    _title->_destImage,
		    1,
		    y + 1,
		    0,
		    y,
		    _title->_srcImage->width(),
		    1 + ((_title->_animStep - 1) * 2));
		break;
	}

	case MAP: {
		if (_bSkipTitles)
			_title->_animStep = _title->_animStepMax;
		else {
			_title->_animStep++;
			_title->_timeDelay = getTicks() - _title->_timeBase + 100;
		}

		int step = (_title->_animStep == _title->_animStepMax ? _title->_animStepMax - 1 : _title->_animStep);

		// blit src to the canvas one row at a time, center out
		_title->_srcImage->drawSubRectOn(
		    _title->_destImage,
		    SCALED(153 - (step * 8)),
		    SCALED(1),
		    0,
		    0,
		    SCALED((step + 1) * 8),
		    SCALED(_title->_srcImage->height()));
		_title->_srcImage->drawSubRectOn(
		    _title->_destImage,
		    SCALED(161),
		    SCALED(1),
		    SCALED(312 - (step * 8)),
		    0,
		    SCALED((step + 1) * 8),
		    SCALED(_title->_srcImage->height()));


		// create a destimage for the map tiles
		int newtime = getTicks();
		if (newtime > _title->_timeDuration + 250 / 4) {
			// grab the map from the screen
			Image *screen = imageMgr->get("screen")->_image;

			// draw the updated map display
			intro->drawMapStatic();

			screen->drawSubRectOn(
			    _title->_srcImage,
			    SCALED(8),
			    SCALED(8),
			    SCALED(8),
			    SCALED(13 * 8),
			    SCALED(38 * 8),
			    SCALED(10 * 8));

			_title->_timeDuration = newtime + 250 / 4;
		}

		_title->_srcImage->drawSubRectOn(
		    _title->_destImage,
		    SCALED(161 - (step * 8)),
		    SCALED(9),
		    SCALED(160 - (step * 8)),
		    SCALED(8),
		    SCALED((step * 2) * 8),
		    SCALED((10 * 8)));

		break;
	}
	}

	// draw the titles
	drawTitle();

	// if the animation for this title has completed,
	// move on to the next title
	if (_title->_animStep >= _title->_animStepMax) {
		// free memory that is no longer needed
		compactTitle();
		_title++;

		if (_title == _titles.end()) {
			// reset the timer to the pre-titles granularity
			eventHandler->getTimer()->reset(eventTimerGranularity);

			// make sure the titles only appear when the app first loads
			_bSkipTitles = true;

			return false;
		}

		if (_title->_method == TITLE) {
			// assume this is "Ultima IV" and pre-load sound
//            soundLoad(SOUND_TITLE_FADE);
			eventHandler->getTimer()->reset(settings._titleSpeedRandom);
		} else if (_title->_method == MAP) {
			eventHandler->getTimer()->reset(settings._titleSpeedOther);
		} else {
			eventHandler->getTimer()->reset(settings._titleSpeedOther);
		}
	}

	return true;
}


//
// The title element has finished drawing all frames, so
// delete, remove, or free data that is no longer needed
//
void IntroController::compactTitle() {
	if (_title->_srcImage) {
		delete _title->_srcImage;
		_title->_srcImage = NULL;
	}
	_title->_plotData.clear();
}


//
// Scale the animation canvas, then draw it to the screen
//
void IntroController::drawTitle() {
	Image *scaled;      // the scaled and filtered image

	// blit the scaled and filtered surface to the screen
	if (_title->_prescaled)
		scaled = _title->_destImage;
	else
		scaled = screenScale(_title->_destImage, settings._scale, 1, 1);

	scaled->setTransparentIndex(_transparentIndex);
	scaled->drawSubRect(
	    SCALED(_title->_rx),    // dest x, y
	    SCALED(_title->_ry),
	    SCALED(1),              // src x, y, w, h
	    SCALED(1),
	    SCALED(_title->_rw),
	    SCALED(_title->_rh));

	if (!_title->_prescaled) {
		delete scaled;
		scaled = NULL;
	}
}


//
// skip the remaining titles
//
void IntroController::skipTitles() {
	_bSkipTitles = true;
	soundStop();
}

#ifdef IOS
// Try to put the intro music back at just the correct moment on iOS;
// don't play it at the very beginning.
void IntroController::tryTriggerIntroMusic() {
	if (mode == INTRO_MAP)
		musicMgr->intro();
}
#endif

} // End of namespace Ultima4
} // End of namespace Ultima
