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

#include "engines/util.h"
#include "gui/saveload.h"
#include "common/translation.h"
#include "sherlock/scalpel/scalpel.h"
#include "sherlock/scalpel/scalpel_fixed_text.h"
#include "sherlock/scalpel/scalpel_map.h"
#include "sherlock/scalpel/scalpel_people.h"
#include "sherlock/scalpel/scalpel_scene.h"
#include "sherlock/scalpel/tsage/logo.h"
#include "sherlock/sherlock.h"
#include "sherlock/music.h"
#include "sherlock/animation.h"
// for 3DO
#include "sherlock/scalpel/3do/movie_decoder.h"

namespace Sherlock {

namespace Scalpel {

#define PROLOGUE_NAMES_COUNT 6

// The following are a list of filenames played in the prologue that have
// special effects associated with them at specific frames
static const char *const PROLOGUE_NAMES[PROLOGUE_NAMES_COUNT] = {
	"subway1", "subway2", "finale2", "suicid", "coff3", "coff4"
};

static const int PROLOGUE_FRAMES[6][9] = {
	{ 4, 26, 54, 72, 92, 134, FRAMES_END },
	{ 2, 80, 95, 117, 166, FRAMES_END },
	{ 1, FRAMES_END },
	{ 42, FRAMES_END },
	{ FRAMES_END },
	{ FRAMES_END }
};

#define TITLE_NAMES_COUNT 7

// Title animations file list
static const char *const TITLE_NAMES[TITLE_NAMES_COUNT] = {
	"27pro1", "14note", "coff1", "coff2", "coff3", "coff4", "14kick"
};

static const int TITLE_FRAMES[7][9] = {
	{ 29, 131, FRAMES_END },
	{ 55, 80, 95, 117, 166, FRAMES_END },
	{ 15, FRAMES_END },
	{ 4, 37, 92, FRAMES_END },
	{ 2, 43, FRAMES_END },
	{ 2, FRAMES_END },
	{ 10, 50, FRAMES_END }
};

#define NUM_PLACES 100

static const int MAP_X[NUM_PLACES] = {
	0, 368, 0, 219, 0, 282, 0, 43, 0, 0, 396, 408, 0, 0, 0, 568, 37, 325,
	28, 0, 263, 36, 148, 469, 342, 143, 443, 229, 298, 0, 157, 260, 432,
	174, 0, 351, 0, 528, 0, 136, 0, 0, 0, 555, 165, 0, 506, 0, 0, 344, 0, 0
};
static const int MAP_Y[NUM_PLACES] = {
	0, 147, 0, 166, 0, 109, 0, 61, 0, 0, 264, 70, 0, 0, 0, 266, 341, 30, 275,
	0, 294, 146, 311, 230, 184, 268, 133, 94, 207, 0, 142, 142, 330, 255, 0,
	37, 0, 70, 0, 116, 0, 0, 0, 50, 21, 0, 303, 0, 0, 229, 0, 0
};

static const int MAP_TRANSLATE[NUM_PLACES] = {
	0, 0, 0, 1, 0, 2, 0, 3, 4, 0, 4, 6, 0, 0, 0, 8, 9, 10, 11, 0, 12, 13, 14, 7,
	15, 16, 17, 18, 19, 0, 20, 21, 22, 23, 0, 24, 0, 25, 0, 26, 0, 0, 0, 27,
	28, 0, 29, 0, 0, 30, 0
};

static const byte MAP_SEQUENCES[3][MAX_FRAME] = {
	{ 1, 1, 2, 3, 4, 0 },		// Overview Still
	{ 5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 0 },
	{ 5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 0 }
};

#define MAX_PEOPLE 66

struct PeopleData {
	const char *portrait;
	const char *name;
	byte stillSequences[MAX_TALK_SEQUENCES];
	byte talkSequences[MAX_TALK_SEQUENCES];
};

const PeopleData PEOPLE_DATA[MAX_PEOPLE] = {
	{ "HOLM", "Sherlock Holmes", { 1, 0, 0 }, { 1, 0, 0 } },
	{ "WATS", "Dr. Watson", { 6, 0, 0 }, { 5, 5, 6, 7, 8, 7, 8, 6, 0, 0 } },
	{ "LEST", "Inspector Lestrade", { 4, 0, 0 }, { 2, 0, 0 } },
	{ "CON1", "Constable O'Brien", { 2, 0, 0 }, { 1, 0, 0 } },
	{ "CON2", "Constable Lewis", { 2, 0, 0 }, { 1, 0, 0 } },
	{ "SHEI", "Sheila Parker", { 2, 0, 0 }, { 2, 3, 0, 0 } },
	{ "HENR", "Henry Carruthers", { 3, 0, 0 }, { 3, 0, 0 } },
	{ "LESL", "Lesley", { 9, 0, 0 }, { 1, 2, 3, 2, 1, 2, 3, 0, 0 } },
	{ "USH1", "An Usher", { 13, 0, 0 }, { 13, 14, 0, 0 } },
	{ "USH2", "An Usher", { 2, 0, 0 }, { 2, 0, 0 } },
	{ "FRED", "Fredrick Epstein", { 4, 0, 0 }, { 1, 2, 3, 4, 3, 4, 3, 2, 0, 0 } },
	{ "WORT", "Mrs. Worthington", { 9, 0, 0 }, { 8, 0, 0 } },
	{ "COAC", "The Coach", { 2, 0, 0 }, { 1, 2, 3, 4, 5, 4, 3, 2, 0, 0 } },
	{ "PLAY", "A Player", { 8, 0, 0 }, { 7, 8, 0, 0 } },
	{ "WBOY", "Tim", { 13, 0, 0 }, { 12, 13, 0, 0 } },
	{ "JAME", "James Sanders", { 6, 0, 0 }, { 3, 4, 0, 0 } },
	{ "BELL", "Belle", { 1, 0, 0 }, { 4, 5, 0, 0 } },
	{ "GIRL", "Cleaning Girl", { 20, 0, 0 }, { 14, 15, 16, 17, 18, 19, 20, 20, 20, 0, 0 } },
	{ "EPST", "Fredrick Epstein", { 17, 0, 0 }, { 16, 17, 18, 18, 18, 17, 17, 0, 0 } },
	{ "WIGG", "Wiggins", { 3, 0, 0 }, { 2, 3, 0, 0 } },
	{ "PAUL", "Paul", { 2, 0, 0 }, { 1, 2, 0, 0 } },
	{ "BART", "The Bartender", { 1, 0, 0 }, { 1, 0, 0 } },
	{ "DIRT", "A Dirty Drunk", { 1, 0, 0 }, { 1, 0, 0 } },
	{ "SHOU", "A Shouting Drunk", { 1, 0, 0 }, { 1, 0, 0 } },
	{ "STAG", "A Staggering Drunk", { 1, 0, 0 }, { 1, 0, 0 } },
	{ "BOUN", "The Bouncer", { 1, 0, 0 }, { 1, 0, 0 } },
	{ "SAND", "James Sanders", { 6, 0, 0 }, { 5, 6, 0, 0 } },
	{ "CORO", "The Coroner", { 6, 0, 0 }, { 4, 5, 0, 0 } },
	{ "EQUE", "Reginald Snipes", { 1, 0, 0 }, { 1, 0, 0 } },
	{ "GEOR", "George Blackwood", { 1, 0, 0 }, { 1, 0, 0 } },
	{ "LARS", "Lars", { 7, 0, 0 }, { 5, 6, 0, 0 } },
	{ "PARK", "Sheila Parker", { 1, 0, 0 }, { 1, 0, 0 } },
	{ "CHEM", "The Chemist", { 8, 0, 0 }, { 8, 9, 0, 0 } },
	{ "GREG", "Inspector Gregson", { 6, 0, 0 }, { 5, 6, 0, 0 } },
	{ "LAWY", "Jacob Farthington", { 1, 0, 0 }, { 1, 0, 0 } },
	{ "MYCR", "Mycroft", { 1, 0, 0 }, { 1, 0, 0 } },
	{ "SHER", "Old Sherman", { 7, 0, 0 }, { 7, 8, 0, 0 } },
	{ "CHMB", "Richard", { 1, 0, 0 }, { 1, 0, 0 } },
	{ "BARM", "The Barman", { 1, 0, 0 }, { 1, 0, 0 } },
	{ "DAND", "A Dandy Player", { 1, 0, 0 }, { 1, 0, 0 } },
	{ "ROUG", "A Rough-looking Player", { 1, 0, 0 }, { 1, 0, 0 } },
	{ "SPEC", "A Spectator", { 1, 0, 0 }, { 1, 0, 0 } },
	{ "HUNT", "Robert Hunt", { 1, 0, 0 }, { 1, 0, 0 } },
	{ "VIOL", "Violet", { 3, 0, 0 }, { 3, 4, 0, 0 } },
	{ "PETT", "Pettigrew", { 1, 0, 0 }, { 1, 0, 0 } },
	{ "APPL", "Augie", { 8, 0, 0 }, { 14, 15, 0, 0 } },
	{ "ANNA", "Anna Carroway", { 16, 0, 0 }, { 3, 4, 5, 6, 0, 0 } },
	{ "GUAR", "A Guard", { 1, 0, 0 }, { 4, 5, 6, 0, 0 } },
	{ "ANTO", "Antonio Caruso", { 8, 0, 0 }, { 7, 8, 0, 0 } },
	{ "TOBY", "Toby the Dog", { 1, 0, 0 }, { 1, 0, 0 } },
	{ "KING", "Simon Kingsley", { 13, 0, 0 }, { 13, 14, 0, 0 } },
	{ "ALFR", "Alfred", { 2, 0, 0 }, { 2, 3, 0, 0 } },
	{ "LADY", "Lady Brumwell", { 1, 0, 0 }, { 3, 4, 0, 0 } },
	{ "ROSA", "Madame Rosa", { 1, 0, 0 }, { 1, 30, 0, 0 } },
	{ "LADB", "Lady Brumwell", { 1, 0, 0 }, { 3, 4, 0, 0 } },
	{ "MOOR", "Joseph Moorehead", { 1, 0, 0 }, { 1, 0, 0 } },
	{ "BEAL", "Mrs. Beale", { 5, 0, 0 }, { 14, 15, 16, 17, 18, 19, 20, 0, 0 } },
	{ "LION", "Felix", { 1, 0, 0 }, { 1, 0, 0 } },
	{ "HOLL", "Hollingston", { 1, 0, 0 }, { 1, 0, 0 } },
	{ "CALL", "Constable Callaghan", { 1, 0, 0 }, { 1, 0, 0 } },
	{ "JERE", "Sergeant Duncan", { 2, 0, 0 }, { 1, 1, 2, 2, 0, 0 } },
	{ "LORD", "Lord Brumwell", { 1, 0, 0 }, { 9, 10, 0, 0 } },
	{ "NIGE", "Nigel Jaimeson", { 1, 0, 0 }, { 1, 2, 0, 138, 3, 4, 0, 138, 0, 0 } },
	{ "JONA", "Jonas", { 1, 0, 0 }, { 1, 8, 0, 0 } },
	{ "DUGA", "Constable Dugan", { 1, 0, 0 }, { 1, 0, 0 } },
	{ "INSP", "Inspector Lestrade", { 4, 0, 0 }, { 2, 0, 0 } }
};

uint INFO_BLACK;
uint BORDER_COLOR;
uint COMMAND_BACKGROUND;
uint BUTTON_BACKGROUND;
uint TALK_FOREGROUND;
uint TALK_NULL;
uint BUTTON_TOP;
uint BUTTON_MIDDLE;
uint BUTTON_BOTTOM;
uint COMMAND_FOREGROUND;
uint COMMAND_HIGHLIGHTED;
uint COMMAND_NULL;
uint INFO_FOREGROUND;
uint INFO_BACKGROUND;
uint INV_FOREGROUND;
uint INV_BACKGROUND;
uint PEN_COLOR;

/*----------------------------------------------------------------*/

#define FROM_RGB(r, g, b) pixelFormatRGB565.RGBToColor(r, g, b)

ScalpelEngine::ScalpelEngine(OSystem *syst, const SherlockGameDescription *gameDesc) :
		SherlockEngine(syst, gameDesc) {
	_darts = nullptr;
	_mapResult = 0;

	if (getPlatform() == Common::kPlatform3DO) {
		const Graphics::PixelFormat pixelFormatRGB565 = Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
		INFO_BLACK = FROM_RGB(0, 0, 0);
		BORDER_COLOR = FROM_RGB(0x6d, 0x38, 0x10);
		COMMAND_BACKGROUND = FROM_RGB(0x38, 0x38, 0xce);
		BUTTON_BACKGROUND = FROM_RGB(0x95, 0x5d, 0x24);
		TALK_FOREGROUND = FROM_RGB(0xff, 0x55, 0x55);
		TALK_NULL = FROM_RGB(0xce, 0xc6, 0xc2);
		BUTTON_TOP = FROM_RGB(0xbe, 0x85, 0x3c);
		BUTTON_MIDDLE = FROM_RGB(0x9d, 0x40, 0);
		BUTTON_BOTTOM = FROM_RGB(0x69, 0x24, 0);
		COMMAND_FOREGROUND = FROM_RGB(0xFF, 0xFF, 0xFF);
		COMMAND_HIGHLIGHTED = FROM_RGB(0x55, 0xff, 0x55);
		COMMAND_NULL = FROM_RGB(0x69, 0x24, 0);
		INFO_FOREGROUND = FROM_RGB(0x55, 0xff, 0xff);
		INFO_BACKGROUND = FROM_RGB(0, 0, 0x48);
		INV_FOREGROUND = FROM_RGB(0xff, 0xff, 0x55);
		INV_BACKGROUND = FROM_RGB(0, 0, 0x48);
		PEN_COLOR = FROM_RGB(0x50, 0x18, 0);
	} else {
		INFO_BLACK = 1;
		BORDER_COLOR = 237;
		COMMAND_BACKGROUND = 4;
		BUTTON_BACKGROUND = 235;
		TALK_FOREGROUND = 12;
		TALK_NULL = 16;
		BUTTON_TOP = 233;
		BUTTON_MIDDLE = 244;
		BUTTON_BOTTOM = 248;
		COMMAND_FOREGROUND = 15;
		COMMAND_HIGHLIGHTED = 10;
		COMMAND_NULL = 248;
		INFO_FOREGROUND = 11;
		INFO_BACKGROUND = 1;
		INV_FOREGROUND = 14;
		INV_BACKGROUND = 1;
		PEN_COLOR = 250;
	}
}

ScalpelEngine::~ScalpelEngine() {
	delete _darts;
}

void ScalpelEngine::initialize() {
	// 3DO actually uses RGB555, but some platforms of ours only support RGB565, so we use that

	if (getPlatform() == Common::kPlatform3DO) {
		const Graphics::PixelFormat pixelFormatRGB565 = Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
		// 320x200 16-bit RGB565 for 3DO support
		initGraphics(320, 200, false, &pixelFormatRGB565);
	} else {
		// 320x200 palettized
		initGraphics(320, 200, false);
	}

	// Let the base engine intialize
	SherlockEngine::initialize();

	_darts = new Darts(this);

	_flags.resize(100 * 8);
	_flags[3] = true;		// Turn on Alley
	_flags[39] = true;		// Turn on Baker Street

	if (!isDemo()) {
		// Load the map co-ordinates for each scene and sequence data
		ScalpelMap &map = *(ScalpelMap *)_map;
		map.loadPoints(NUM_PLACES, &MAP_X[0], &MAP_Y[0], &MAP_TRANSLATE[0]);
		map.loadSequences(3, &MAP_SEQUENCES[0][0]);
		map._oldCharPoint = BAKER_ST_EXTERIOR;
	}

	// Load the inventory
	loadInventory();

	// Set up list of people
	for (int idx = 0; idx < MAX_PEOPLE; ++idx)
		_people->_characters.push_back(PersonData(PEOPLE_DATA[idx].name, PEOPLE_DATA[idx].portrait,
			PEOPLE_DATA[idx].stillSequences, PEOPLE_DATA[idx].talkSequences));

	_animation->setPrologueNames(&PROLOGUE_NAMES[0], PROLOGUE_NAMES_COUNT);
	_animation->setPrologueFrames(&PROLOGUE_FRAMES[0][0], 6, 9);

	_animation->setTitleNames(&TITLE_NAMES[0], TITLE_NAMES_COUNT);
	_animation->setTitleFrames(&TITLE_FRAMES[0][0], 7, 9);

	// Starting scene
	if (isDemo() && _interactiveFl)
		_scene->_goToScene = 3;
	else
		_scene->_goToScene = 4;
}

void ScalpelEngine::showOpening() {
	bool finished = true;

	if (isDemo() && _interactiveFl)
		return;

	_events->setFrameRate(60);

	if (getPlatform() == Common::kPlatform3DO) {
		show3DOSplash();

		finished = showCityCutscene3DO();
		if (finished)
			finished = showAlleyCutscene3DO();
		if (finished)
			finished = showStreetCutscene3DO();
		if (finished)
			showOfficeCutscene3DO();

		_events->clearEvents();
		_music->stopMusic();
	} else {
		TsAGE::Logo::show(this);

		finished = showCityCutscene();
		if (finished)
			finished = showAlleyCutscene();
		if (finished)
			finished = showStreetCutscene();
		if (finished)
			showOfficeCutscene();

		_events->clearEvents();
		_music->stopMusic();
	}

	_events->setFrameRate(GAME_FRAME_RATE);
}

bool ScalpelEngine::showCityCutscene() {
	byte greyPalette[PALETTE_SIZE];
	byte palette[PALETTE_SIZE];

	// Demo fades from black into grey and then fades from grey into the scene
	Common::fill(&greyPalette[0], &greyPalette[PALETTE_SIZE], 142);
	_screen->fadeIn((const byte *)greyPalette, 3);

	_music->loadSong("prolog1");
	_animation->_gfxLibraryFilename = "title.lib";
	_animation->_soundLibraryFilename = "title.snd";
	bool finished = _animation->play("26open1", true, 1, 255, true, 2);

	if (finished) {
		ImageFile titleImages_LondonNovember("title2.vgs", true);
		_screen->_backBuffer1.blitFrom(*_screen);
		_screen->_backBuffer2.blitFrom(*_screen);

		Common::Point londonPosition;

		if ((titleImages_LondonNovember[0]._width == 302) && (titleImages_LondonNovember[0]._height == 39)) {
			// Spanish
			londonPosition = Common::Point(9, 8);
		} else {
			// English (German uses the same English graphics), width 272, height 37
			// In the German version this is placed differently, check against German floppy version TODO
			londonPosition = Common::Point(30, 50);
		}

		// London, England
		_screen->_backBuffer1.transBlitFrom(titleImages_LondonNovember[0], londonPosition);
		_screen->randomTransition();
		finished = _events->delay(1000, true);

		// November, 1888
		if (finished) {
			_screen->_backBuffer1.transBlitFrom(titleImages_LondonNovember[1], Common::Point(100, 100));
			_screen->randomTransition();
			finished = _events->delay(5000, true);
		}

		// Transition out the title
		_screen->_backBuffer1.blitFrom(_screen->_backBuffer2);
		_screen->randomTransition();
	}

	if (finished)
		finished = _animation->play("26open2", true, 1, 0, false, 2);

	if (finished) {
		ImageFile titleImages_SherlockHolmesTitle("title.vgs", true);
		_screen->_backBuffer1.blitFrom(*_screen);
		_screen->_backBuffer2.blitFrom(*_screen);

		Common::Point lostFilesPosition;
		Common::Point sherlockHolmesPosition;
		Common::Point copyrightPosition;

		if ((titleImages_SherlockHolmesTitle[0]._width == 306) && (titleImages_SherlockHolmesTitle[0]._height == 39)) {
			// Spanish
			lostFilesPosition = Common::Point(5, 5);
			sherlockHolmesPosition = Common::Point(24, 40);
			copyrightPosition = Common::Point(3, 190);
		} else {
			// English (German uses the same English graphics), width 208, height 39
			lostFilesPosition = Common::Point(75, 6);
			sherlockHolmesPosition = Common::Point(34, 21);
			copyrightPosition = Common::Point(4, 190);
		}

		// The Lost Files of
		_screen->_backBuffer1.transBlitFrom(titleImages_SherlockHolmesTitle[0], lostFilesPosition);
		// Sherlock Holmes
		_screen->_backBuffer1.transBlitFrom(titleImages_SherlockHolmesTitle[1], sherlockHolmesPosition);
		// copyright
		_screen->_backBuffer1.transBlitFrom(titleImages_SherlockHolmesTitle[2], copyrightPosition);

		_screen->verticalTransition();
		finished = _events->delay(4000, true);

		if (finished) {
			_screen->_backBuffer1.blitFrom(_screen->_backBuffer2);
			_screen->randomTransition();
			finished = _events->delay(2000);
		}

		if (finished) {
			_screen->getPalette(palette);
			_screen->fadeToBlack(2);
		}

		if (finished) {
			// In the alley...
			Common::Point alleyPosition;

			if ((titleImages_SherlockHolmesTitle[3]._width == 105) && (titleImages_SherlockHolmesTitle[3]._height == 16)) {
				// German
				alleyPosition = Common::Point(72, 50);
			} else if ((titleImages_SherlockHolmesTitle[3]._width == 166) && (titleImages_SherlockHolmesTitle[3]._height == 36)) {
				// Spanish
				alleyPosition = Common::Point(71, 50);
			} else {
				// English, width 175, height 38
				alleyPosition = Common::Point(72, 51);
			}
			_screen->transBlitFrom(titleImages_SherlockHolmesTitle[3], alleyPosition);
			_screen->fadeIn(palette, 3);

			// Wait until the track got looped and the first few notes were played
			finished = _music->waitUntilMSec(4300, 21300, 0, 2500); // ticks 0x104 / ticks 0x500
		}
	}

	_animation->_gfxLibraryFilename = "";
	_animation->_soundLibraryFilename = "";
	return finished;
}

bool ScalpelEngine::showAlleyCutscene() {
	byte palette[PALETTE_SIZE];
	_music->loadSong("prolog2");

	_animation->_gfxLibraryFilename = "TITLE.LIB";
	_animation->_soundLibraryFilename = "TITLE.SND";

	// Fade "In The Alley..." text to black
	_screen->fadeToBlack(2);

	bool finished = _animation->play("27PRO1", true, 1, 3, true, 2);
	if (finished) {
		_screen->getPalette(palette);
		_screen->fadeToBlack(2);

		// wait until second lower main note
		finished = _music->waitUntilMSec(26800, 0xFFFFFFFF, 0, 1000); // ticks 0x64A
	}

	if (finished) {
		_screen->setPalette(palette);
		finished = _animation->play("27PRO2", true, 1, 0, false, 2);
	}

	if (finished) {
		showLBV("scream.lbv");

		// wait until first "scream" in music happened
		finished = _music->waitUntilMSec(45800, 0xFFFFFFFF, 0, 6000); // ticks 0xABE
	}

	if (finished) {
		// quick fade out 
		_screen->fadeToBlack(1);

		// wait until after third "scream" in music happened
		finished = _music->waitUntilMSec(49000, 0xFFFFFFFF, 0, 2000); // ticks 0xB80
	}

	if (finished)
		finished = _animation->play("27PRO3", true, 1, 0, true, 2);

	if (finished) {
		_screen->getPalette(palette);
		_screen->fadeToBlack(2);
	}

	if (finished) {
		ImageFile titleImages_EarlyTheFollowingMorning("title3.vgs", true);
		// "Early the following morning on Baker Street..."
		Common::Point earlyTheFollowingMorningPosition;

		if ((titleImages_EarlyTheFollowingMorning[0]._width == 164) && (titleImages_EarlyTheFollowingMorning[0]._height == 19)) {
			// German
			earlyTheFollowingMorningPosition = Common::Point(35, 50);
		} else if ((titleImages_EarlyTheFollowingMorning[0]._width == 171) && (titleImages_EarlyTheFollowingMorning[0]._height == 32)) {
			// Spanish
			earlyTheFollowingMorningPosition = Common::Point(35, 50);
		} else {
			// English, width 218, height 31
			earlyTheFollowingMorningPosition = Common::Point(35, 52);
		}

		_screen->transBlitFrom(titleImages_EarlyTheFollowingMorning[0], earlyTheFollowingMorningPosition);

		// fast fade-in
		_screen->fadeIn(palette, 1);

		// wait for music to end and wait an additional 2.5 seconds
		finished = _music->waitUntilMSec(0xFFFFFFFF, 0xFFFFFFFF, 2500, 3000);
	}

	_animation->_gfxLibraryFilename = "";
	_animation->_soundLibraryFilename = "";
	return finished;
}

bool ScalpelEngine::showStreetCutscene() {
	_animation->_gfxLibraryFilename = "TITLE.LIB";
	_animation->_soundLibraryFilename = "TITLE.SND";

	_music->loadSong("prolog3");

	// wait a bit
	bool finished = _events->delay(500);

	if (finished) {
		// fade out "Early the following morning..."
		_screen->fadeToBlack(2);

		// wait for music a bit
		finished = _music->waitUntilMSec(3800, 0xFFFFFFFF, 0, 1000); // ticks 0xE4
	}

	if (finished)
		finished = _animation->play("14KICK", true, 1, 3, true, 2);

	// Constable animation plays slower than speed 2
	// If we play it with speed 2, music gets obviously out of sync
	if (finished)
		finished = _animation->play("14NOTE", true, 1, 0, false, 3);

	// Fade to black
	if (finished)
		_screen->fadeToBlack(1);

	_animation->_gfxLibraryFilename = "";
	_animation->_soundLibraryFilename = "";
	return finished;
}

bool ScalpelEngine::showOfficeCutscene() {
	_music->loadSong("prolog4");
	_animation->_gfxLibraryFilename = "TITLE2.LIB";
	_animation->_soundLibraryFilename = "TITLE.SND";

	bool finished = _animation->play("COFF1", true, 1, 3, true, 3);
	if (finished)
		finished = _animation->play("COFF2", true, 1, 0, false, 3);
	if (finished) {
		showLBV("note.lbv");

		if (_sound->_voices) {
			finished = _sound->playSound("NOTE1", WAIT_KBD_OR_FINISH);
			if (finished)
				finished = _sound->playSound("NOTE2", WAIT_KBD_OR_FINISH);
			if (finished)
				finished = _sound->playSound("NOTE3", WAIT_KBD_OR_FINISH);
			if (finished)
				finished = _sound->playSound("NOTE4", WAIT_KBD_OR_FINISH);
		} else
			finished = _events->delay(19000);

		if (finished) {
			_events->clearEvents();
			finished = _events->delay(500);
		}
	}

	if (finished)
		finished = _animation->play("COFF3", true, 1, 0, true, 3);

	if (finished)
		finished = _animation->play("COFF4", true, 1, 0, false, 3);

	if (finished)
		finished = scrollCredits();

	if (finished)
		_screen->fadeToBlack(3);

	_animation->_gfxLibraryFilename = "";
	_animation->_soundLibraryFilename = "";
	return finished;
}

bool ScalpelEngine::scrollCredits() {
	// Load the images for displaying credit text
	Common::SeekableReadStream *stream = _res->load("credits.vgs", "title.lib");
	ImageFile creditsImages(*stream);

	// Demo fades slowly from the scene into credits palette
	_screen->fadeIn(creditsImages._palette, 3);

	delete stream;

	// Save a copy of the screen background for use in drawing each credit frame
	_screen->_backBuffer1.blitFrom(*_screen);

	// Loop for showing the credits
	for(int idx = 0; idx < 600 && !_events->kbHit() && !shouldQuit(); ++idx) {
		// Copy the entire screen background before writing text
		_screen->blitFrom(_screen->_backBuffer1);

		// Write the text appropriate for the next frame
		if (idx < 400)
			_screen->transBlitFrom(creditsImages[0], Common::Point(10, 200 - idx), false, 0);
		if (idx > 200)
			_screen->transBlitFrom(creditsImages[1], Common::Point(10, 400 - idx), false, 0);

		// Don't show credit text on the top and bottom ten rows of the screen
		_screen->blitFrom(_screen->_backBuffer1, Common::Point(0, 0), Common::Rect(0, 0, _screen->w(), 10));
		_screen->blitFrom(_screen->_backBuffer1, Common::Point(0, _screen->h() - 10),
			Common::Rect(0, _screen->h() - 10, _screen->w(), _screen->h()));

		_events->delay(100);
	}

	return true;
}

// 3DO variant
bool ScalpelEngine::show3DOSplash() {
	// 3DO EA Splash screen
	ImageFile3DO titleImage_3DOSplash("3DOSplash.cel", kImageFile3DOType_Cel);

	_screen->transBlitFrom(titleImage_3DOSplash[0]._frame, Common::Point(0, -20));
	bool finished = _events->delay(3000, true);

	if (finished) {
		_screen->clear();
		finished = _events->delay(500, true);
	}

	if (finished) {
		// EA logo movie
		Scalpel3DOMoviePlay("EAlogo.stream", Common::Point(20, 0));
	}

	// Always clear screen
	_screen->clear();
	return finished;
}

bool ScalpelEngine::showCityCutscene3DO() {
	_animation->_soundLibraryFilename = "TITLE.SND";

	_screen->clear();
	bool finished = _events->delay(2500, true);

	// rain.aiff seems to be playing in an endless loop until
	// sherlock logo fades away TODO

	if (finished) {
		finished = _events->delay(2500, true);

		// Play intro music
		_music->loadSong("prolog");

		// Fade screen to grey
		_screen->_backBuffer1.fill(0xCE59); // RGB565: 25, 50, 25 (grey)
		_screen->fadeIntoScreen3DO(2);
	}

	if (finished) {
		finished = _music->waitUntilMSec(3400, 0, 0, 3400);
	}

	if (finished) {
		_screen->_backBuffer1.fill(0); // fill backbuffer with black to avoid issues during fade from white
		finished = _animation->play3DO("26open1", true, 1, true, 2);
	}

	if (finished) {
		_screen->_backBuffer1.blitFrom(*_screen); // save into backbuffer 1, used for fade
		_screen->_backBuffer2.blitFrom(*_screen); // save into backbuffer 2, for restoring later

		// "London, England"
		ImageFile3DO titleImage_London("title2a.cel", kImageFile3DOType_Cel);
		_screen->_backBuffer1.transBlitFrom(titleImage_London[0]._frame, Common::Point(30, 50));

		_screen->fadeIntoScreen3DO(1);
		finished = _events->delay(1500, true);

		if (finished) {
			// "November, 1888"
			ImageFile3DO titleImage_November("title2b.cel", kImageFile3DOType_Cel);
			_screen->_backBuffer1.transBlitFrom(titleImage_November[0]._frame, Common::Point(100, 100));

			_screen->fadeIntoScreen3DO(1);
			finished = _music->waitUntilMSec(14700, 0, 0, 5000);
		}

		if (finished) {
			// Restore screen
			_screen->blitFrom(_screen->_backBuffer2);
		}
	}

	if (finished)
		finished = _animation->play3DO("26open2", true, 1, false, 2);

	if (finished) {
		_screen->_backBuffer1.blitFrom(*_screen); // save into backbuffer 1, used for fade

		// "Sherlock Holmes" (title)
		ImageFile3DO titleImage_SherlockHolmesTitle("title1ab.cel", kImageFile3DOType_Cel);
		_screen->_backBuffer1.transBlitFrom(titleImage_SherlockHolmesTitle[0]._frame, Common::Point(34, 5));

		// Blend in
		_screen->fadeIntoScreen3DO(2);
		finished = _events->delay(500, true);

		// Title should fade in, Copyright should be displayed a bit after that
		if (finished) {
			ImageFile3DO titleImage_Copyright("title1c.cel", kImageFile3DOType_Cel);

			_screen->transBlitFrom(titleImage_Copyright[0]._frame, Common::Point(20, 190));
			finished = _events->delay(3500, true);
		}
	}

	if (finished)
		finished = _music->waitUntilMSec(33600, 0, 0, 2000);

	if (finished) {
		// Fade to black
		_screen->_backBuffer1.clear();
		_screen->fadeIntoScreen3DO(3);
	}

	if (finished) {
		// "In the alley behind the Regency Theatre..."
		ImageFile3DO titleImage_InTheAlley("title1d.cel", kImageFile3DOType_Cel);
		_screen->_backBuffer1.transBlitFrom(titleImage_InTheAlley[0]._frame, Common::Point(72, 51));

		// Fade in
		_screen->fadeIntoScreen3DO(4);
		finished = _music->waitUntilMSec(39900, 0, 0, 2500);

		// Fade out
		_screen->_backBuffer1.clear();
		_screen->fadeIntoScreen3DO(4);
	}
	return finished;
}

bool ScalpelEngine::showAlleyCutscene3DO() {
	bool finished = _music->waitUntilMSec(43500, 0, 0, 1000);

	if (finished)
		finished = _animation->play3DO("27PRO1", true, 1, false, 2);

	if (finished) {
		// Fade out...
		_screen->_backBuffer1.clear();
		_screen->fadeIntoScreen3DO(3);

		finished = _music->waitUntilMSec(67100, 0, 0, 1000); // 66700
	}

	if (finished)
		finished = _animation->play3DO("27PRO2", true, 1, false, 2);

	if (finished)
		finished = _music->waitUntilMSec(76000, 0, 0, 1000);

	if (finished) {
		// Show screaming victim
		ImageFile3DO titleImage_ScreamingVictim("scream.cel", kImageFile3DOType_Cel);

		_screen->clear();
		_screen->transBlitFrom(titleImage_ScreamingVictim[0]._frame, Common::Point(0, 0));

		// Play "scream.aiff"
		if (_sound->_voices)
			_sound->playSound("prologue/sounds/scream.aiff", WAIT_RETURN_IMMEDIATELY, 100);

		finished = _music->waitUntilMSec(81600, 0, 0, 6000);
	}

	if (finished) {
		// Fade out
		_screen->_backBuffer1.clear();
		_screen->fadeIntoScreen3DO(5);

		finished = _music->waitUntilMSec(84400, 0, 0, 2000);
	}

	if (finished)
		finished = _animation->play3DO("27PRO3", true, 1, false, 2);

	if (finished) {
		// Fade out
		_screen->_backBuffer1.clear();
		_screen->fadeIntoScreen3DO(5);
	}

	if (finished) {
		// "Early the following morning on Baker Street..."
		ImageFile3DO titleImage_EarlyTheFollowingMorning("title3.cel", kImageFile3DOType_Cel);
		_screen->_backBuffer1.transBlitFrom(titleImage_EarlyTheFollowingMorning[0]._frame, Common::Point(35, 51));

		// Fade in
		_screen->fadeIntoScreen3DO(4);
		finished = _music->waitUntilMSec(96700, 0, 0, 3000);
	}

	return finished;
}

bool ScalpelEngine::showStreetCutscene3DO() {
	bool finished = true;

	if (finished) {
		// fade out "Early the following morning..."
		_screen->_backBuffer1.clear();
		_screen->fadeIntoScreen3DO(4);

		// wait for music a bit
		finished = _music->waitUntilMSec(100300, 0, 0, 1000);
	}

	if (finished)
		finished = _animation->play3DO("14KICK", true, 1, false, 2);

	// note: part of the constable is sticking to the door during the following
	//       animation, when he walks away. This is a bug of course, but it actually happened on 3DO!
	//       I'm not sure if it happens because the door is pure black (0, 0, 0) and it's because
	//       of transparency - or if the animation itself is bad. We will definitely have to adjust
	//       the animation data to fix it.
	if (finished)
		finished = _animation->play3DO("14NOTE", true, 1, false, 3);

	if (finished) {
		// Fade out
		_screen->_backBuffer1.clear();
		_screen->fadeIntoScreen3DO(4);
	}

	return finished;
}

bool ScalpelEngine::showOfficeCutscene3DO() {
	bool finished = _music->waitUntilMSec(151000, 0, 0, 1000);

	if (finished)
		finished = _animation->play3DO("COFF1", true, 1, false, 3);

	if (finished)
		finished = _animation->play3DO("COFF2", true, 1, false, 3);

	if (finished)
		finished = _music->waitUntilMSec(182400, 0, 0, 1000);

	if (finished) {
		// Show the note
		ImageFile3DO titleImage_CoffeeNote("note.cel", kImageFile3DOType_Cel);

		_screen->clear();
		_screen->transBlitFrom(titleImage_CoffeeNote[0]._frame, Common::Point(0, 0));

		if (_sound->_voices) {
			finished = _sound->playSound("prologue/sounds/note.aiff", WAIT_KBD_OR_FINISH);
		} else
			finished = _events->delay(19000);

		if (finished)
			finished = _music->waitUntilMSec(218800, 0, 0, 1000);

		// Fade out
		_screen->clear();
	}

	if (finished)
		finished = _music->waitUntilMSec(222200, 0, 0, 1000);

	if (finished)
		finished = _animation->play3DO("COFF3", true, 1, false, 3);

	if (finished)
		finished = _animation->play3DO("COFF4", true, 1, false, 3);

	if (finished) {
		finished = _music->waitUntilMSec(244500, 0, 0, 2000);

		// TODO: Brighten the image, possibly by doing a partial fade
		// to white.

		_screen->_backBuffer1.blitFrom(*_screen);

		for (int nr = 1; finished && nr <= 4; nr++) {
			char filename[15];
			sprintf(filename, "credits%d.cel", nr);
			ImageFile3DO *creditsImage = new ImageFile3DO(filename, kImageFile3DOType_Cel);
			ImageFrame *creditsFrame = &(*creditsImage)[0];
			for (int i = 0; finished && i < 200 + creditsFrame->_height; i++) {
				_screen->blitFrom(_screen->_backBuffer1);
				_screen->transBlitFrom(creditsFrame->_frame, Common::Point((320 - creditsFrame->_width) / 2, 200 - i));
				if (!_events->delay(70, true))
					finished = false;
			}
			delete creditsImage;
		}
	}

	return finished;
}

void ScalpelEngine::loadInventory() {
	ScalpelFixedText &fixedText = *(ScalpelFixedText *)_fixedText;
	Inventory &inv = *_inventory;

	Common::String fixedText_Message    = fixedText.getText(kFixedText_InitInventory_Message);
	Common::String fixedText_HolmesCard = fixedText.getText(kFixedText_InitInventory_HolmesCard);
	Common::String fixedText_Tickets    = fixedText.getText(kFixedText_InitInventory_Tickets);
	Common::String fixedText_CuffLink   = fixedText.getText(kFixedText_InitInventory_CuffLink);
	Common::String fixedText_WireHook   = fixedText.getText(kFixedText_InitInventory_WireHook);
	Common::String fixedText_Note       = fixedText.getText(kFixedText_InitInventory_Note);
	Common::String fixedText_OpenWatch  = fixedText.getText(kFixedText_InitInventory_OpenWatch);
	Common::String fixedText_Paper      = fixedText.getText(kFixedText_InitInventory_Paper);
	Common::String fixedText_Letter     = fixedText.getText(kFixedText_InitInventory_Letter);
	Common::String fixedText_Tarot      = fixedText.getText(kFixedText_InitInventory_Tarot);
	Common::String fixedText_OrnateKey  = fixedText.getText(kFixedText_InitInventory_OrnateKey);
	Common::String fixedText_PawnTicket = fixedText.getText(kFixedText_InitInventory_PawnTicket);

	// Initial inventory
	inv._holdings = 2;
	inv.push_back(InventoryItem(0,     "Message", fixedText_Message,    "_ITEM03A"));
	inv.push_back(InventoryItem(0, "Holmes Card", fixedText_HolmesCard, "_ITEM07A"));

	// Hidden items
	inv.push_back(InventoryItem(95,  "Tickets",     fixedText_Tickets,    "_ITEM10A"));
	inv.push_back(InventoryItem(138, "Cuff Link",   fixedText_CuffLink,   "_ITEM04A"));
	inv.push_back(InventoryItem(138, "Wire Hook",   fixedText_WireHook,   "_ITEM06A"));
	inv.push_back(InventoryItem(150, "Note",        fixedText_Note,       "_ITEM13A"));
	inv.push_back(InventoryItem(481, "Open Watch",  fixedText_OpenWatch,  "_ITEM62A"));
	inv.push_back(InventoryItem(481, "Paper",       fixedText_Paper,      "_ITEM44A"));
	inv.push_back(InventoryItem(532, "Letter",      fixedText_Letter,     "_ITEM68A"));
	inv.push_back(InventoryItem(544, "Tarot",       fixedText_Tarot,      "_ITEM71A"));
	inv.push_back(InventoryItem(544, "Ornate Key",  fixedText_OrnateKey,  "_ITEM70A"));
	inv.push_back(InventoryItem(586, "Pawn ticket", fixedText_PawnTicket, "_ITEM16A"));
}

void ScalpelEngine::showLBV(const Common::String &filename) {
	Common::SeekableReadStream *stream = _res->load(filename, "title.lib");
	ImageFile images(*stream);
	delete stream;

	_screen->setPalette(images._palette);
	_screen->_backBuffer1.blitFrom(images[0]);
	_screen->verticalTransition();
}

void ScalpelEngine::startScene() {
	if (_scene->_goToScene == OVERHEAD_MAP || _scene->_goToScene == OVERHEAD_MAP2) {
		// Show the map
		if (_music->_musicOn && _music->loadSong(100))
			_music->startSong();

		_scene->_goToScene = _map->show();

		_music->freeSong();
		_people->_savedPos = Common::Point(-1, -1);
		_people->_savedPos._facing = -1;
	}

	// Some rooms are prologue cutscenes, rather than normal game scenes. These are:
	//  2: Blackwood's capture
	// 52: Rescuing Anna
	// 53: Moorehead's death / subway train
	// 55: Fade out and exit
	// 70: Brumwell suicide
	switch (_scene->_goToScene) {
	case BLACKWOOD_CAPTURE:
	case RESCUE_ANNA:
	case MOOREHEAD_DEATH:
	case BRUMWELL_SUICIDE:
		if (_music->_musicOn && _music->loadSong(_scene->_goToScene))
			_music->startSong();

		switch (_scene->_goToScene) {
		case BLACKWOOD_CAPTURE:
			// Blackwood's capture
			_res->addToCache("final2.vda", "epilogue.lib");
			_res->addToCache("final2.vdx", "epilogue.lib");
			_animation->play("final1", false, 1, 3, true, 4);
			_animation->play("final2", false, 1, 0, false, 4);
			break;

		case RESCUE_ANNA:
			// Rescuing Anna
			_res->addToCache("finalr2.vda", "epilogue.lib");
			_res->addToCache("finalr2.vdx", "epilogue.lib");
			_res->addToCache("finale1.vda", "epilogue.lib");
			_res->addToCache("finale1.vdx", "epilogue.lib");
			_res->addToCache("finale2.vda", "epilogue.lib");
			_res->addToCache("finale2.vdx", "epilogue.lib");
			_res->addToCache("finale3.vda", "epilogue.lib");
			_res->addToCache("finale3.vdx", "epilogue.lib");
			_res->addToCache("finale4.vda", "EPILOG2.lib");
			_res->addToCache("finale4.vdx", "EPILOG2.lib");

			_animation->play("finalr1", false, 1, 3, true, 4);
			_animation->play("finalr2", false, 1, 0, false, 4);

			if (!_res->isInCache("finale2.vda")) {
				// Finale file isn't cached
				_res->addToCache("finale2.vda", "epilogue.lib");
				_res->addToCache("finale2.vdx", "epilogue.lib");
				_res->addToCache("finale3.vda", "epilogue.lib");
				_res->addToCache("finale3.vdx", "epilogue.lib");
				_res->addToCache("finale4.vda", "EPILOG2.lib");
				_res->addToCache("finale4.vdx", "EPILOG2.lib");
			}

			_animation->play("finale1", false, 1, 0, false, 4);
			_animation->play("finale2", false, 1, 0, false, 4);
			_animation->play("finale3", false, 1, 0, false, 4);

			_useEpilogue2 = true;
			_animation->play("finale4", false, 1, 0, false, 4);
			_useEpilogue2 = false;
			break;

		case MOOREHEAD_DEATH:
			// Moorehead's death / subway train
			_res->addToCache("SUBWAY2.vda", "epilogue.lib");
			_res->addToCache("SUBWAY2.vdx", "epilogue.lib");
			_res->addToCache("SUBWAY3.vda", "epilogue.lib");
			_res->addToCache("SUBWAY3.vdx", "epilogue.lib");

			_animation->play("SUBWAY1", false, 1, 3, true, 4);
			_animation->play("SUBWAY2", false, 1, 0, false, 4);
			_animation->play("SUBWAY3", false, 1, 0, false, 4);

			// Set fading to direct fade temporary so the transition goes quickly.
			_scene->_tempFadeStyle = _screen->_fadeStyle ? 257 : 256;
			_screen->_fadeStyle = false;
			break;

		case BRUMWELL_SUICIDE:
			// Brumwell suicide
			_animation->play("suicid", false, 1, 3, true, 4);
			break;
		default:
			break;
		}

		// Except for the Moorehead Murder scene, fade to black first
		if (_scene->_goToScene != MOOREHEAD_DEATH) {
			_events->wait(40);
			_screen->fadeToBlack(3);
		}

		switch (_scene->_goToScene) {
		case 52:
			_scene->_goToScene = LAWYER_OFFICE;		// Go to the Lawyer's Office
			_map->_bigPos = Common::Point(0, 0);	// Overland scroll position
			_map->_overPos = Common::Point(22900 - 600, 9400 + 900);	// Overland position
			_map->_oldCharPoint = LAWYER_OFFICE;
			break;

		case 53:
			_scene->_goToScene = STATION;			// Go to St. Pancras Station
			_map->_bigPos = Common::Point(0, 0);	// Overland scroll position
			_map->_overPos = Common::Point(32500 - 600, 3000 + 900);	// Overland position
			_map->_oldCharPoint = STATION;
			break;

		default:
			_scene->_goToScene = BAKER_STREET;		// Back to Baker st.
			_map->_bigPos = Common::Point(0, 0);	// Overland scroll position
			_map->_overPos = Common::Point(14500 - 600, 8400 + 900);	// Overland position
			_map->_oldCharPoint = BAKER_STREET;
			break;
		}

		// Free any song from the previous scene
		_music->freeSong();
		break;

	case EXIT_GAME:
		// Exit game
		_screen->fadeToBlack(3);
		quitGame();
		return;

	default:
		break;
	}

	_events->setCursor(ARROW);

	if (_scene->_goToScene == 99) {
		// Darts Board minigame
		_darts->playDarts();
		_mapResult = _scene->_goToScene = PUB_INTERIOR;
	}

	_mapResult = _scene->_goToScene;
}

void ScalpelEngine::eraseBrumwellMirror() {
	Common::Point pt((*_people)[HOLMES]._position.x / FIXED_INT_MULTIPLIER, (*_people)[HOLMES]._position.y / FIXED_INT_MULTIPLIER);

	// If player is in range of the mirror, then restore background from the secondary back buffer
	if (Common::Rect(70, 100, 200, 200).contains(pt)) {
		_screen->_backBuffer1.blitFrom(_screen->_backBuffer2, Common::Point(137, 18),
			Common::Rect(137, 18, 184, 74));
	}
}

void ScalpelEngine::doBrumwellMirror() {
	People &people = *_people;
	Person &player = people[HOLMES];

	Common::Point pt((*_people)[HOLMES]._position.x / FIXED_INT_MULTIPLIER, (*_people)[HOLMES]._position.y / FIXED_INT_MULTIPLIER);
	int frameNum = player._walkSequences[player._sequenceNumber][player._frameNumber] +
		player._walkSequences[player._sequenceNumber][0] - 2;

	switch ((*_people)[HOLMES]._sequenceNumber) {
	case WALK_DOWN:
		frameNum -= 7;
		break;
	case WALK_UP:
		frameNum += 7;
		break;
	case WALK_DOWNRIGHT:
		frameNum += 7;
		break;
	case WALK_UPRIGHT:
		frameNum -= 7;
		break;
	case WALK_DOWNLEFT:
		frameNum += 7;
		break;
	case WALK_UPLEFT:
		frameNum -= 7;
		break;
	case STOP_DOWN:
		frameNum -= 10;
		break;
	case STOP_UP:
		frameNum += 11;
		break;
	case STOP_DOWNRIGHT:
		frameNum -= 15;
		break;
	case STOP_DOWNLEFT:
		frameNum -= 15;
		break;
	case STOP_UPRIGHT:
	case STOP_UPLEFT:
		frameNum += 15;
		if (frameNum == 55)
			frameNum = 54;
		break;
	default:
		break;
	}

	if (Common::Rect(80, 100, 145, 138).contains(pt)) {
		// Get the frame of Sherlock to draw
		ImageFrame &imageFrame = (*people[HOLMES]._images)[frameNum];

		// Draw the mirror image of Holmes
		bool flipped = people[HOLMES]._sequenceNumber == WALK_LEFT || people[HOLMES]._sequenceNumber == STOP_LEFT
			|| people[HOLMES]._sequenceNumber == WALK_UPRIGHT || people[HOLMES]._sequenceNumber == STOP_UPRIGHT
			|| people[HOLMES]._sequenceNumber == WALK_DOWNLEFT || people[HOLMES]._sequenceNumber == STOP_DOWNLEFT;
		_screen->_backBuffer1.transBlitFrom(imageFrame, pt + Common::Point(38, -imageFrame._frame.h - 25), flipped);

		// Redraw the mirror borders to prevent the drawn image of Holmes from appearing outside of the mirror
		_screen->_backBuffer1.blitFrom(_screen->_backBuffer2, Common::Point(114, 18),
			Common::Rect(114, 18, 137, 114));
		_screen->_backBuffer1.blitFrom(_screen->_backBuffer2, Common::Point(137, 70),
			Common::Rect(137, 70, 142, 114));
		_screen->_backBuffer1.blitFrom(_screen->_backBuffer2, Common::Point(142, 71),
			Common::Rect(142, 71, 159, 114));
		_screen->_backBuffer1.blitFrom(_screen->_backBuffer2, Common::Point(159, 72),
			Common::Rect(159, 72, 170, 116));
		_screen->_backBuffer1.blitFrom(_screen->_backBuffer2, Common::Point(170, 73),
			Common::Rect(170, 73, 184, 114));
		_screen->_backBuffer1.blitFrom(_screen->_backBuffer2, Common::Point(184, 18),
			Common::Rect(184, 18, 212, 114));
	}
}

void ScalpelEngine::flushBrumwellMirror() {
	Common::Point pt((*_people)[HOLMES]._position.x / FIXED_INT_MULTIPLIER, (*_people)[HOLMES]._position.y / FIXED_INT_MULTIPLIER);

	// If player is in range of the mirror, then draw the entire mirror area to the screen
	if (Common::Rect(70, 100, 200, 200).contains(pt))
		_screen->slamArea(137, 18, 47, 56);
}


void ScalpelEngine::showScummVMSaveDialog() {
	GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);

	int slot = dialog->runModalWithCurrentTarget();
	if (slot >= 0) {
		Common::String desc = dialog->getResultString();

		saveGameState(slot, desc);
	}
}

void ScalpelEngine::showScummVMRestoreDialog() {
	GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);

	int slot = dialog->runModalWithCurrentTarget();
	if (slot >= 0) {
		loadGameState(slot);
	}
}

} // End of namespace Scalpel

} // End of namespace Sherlock
