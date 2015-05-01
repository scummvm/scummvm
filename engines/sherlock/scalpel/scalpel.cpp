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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "sherlock/scalpel/scalpel.h"
#include "sherlock/sherlock.h"

namespace Sherlock {

namespace Scalpel {

#define NUM_PLACES 100
const int MAP_X[NUM_PLACES] = { 
	0, 368, 0, 219, 0, 282, 0, 43, 0, 0, 396, 408, 0, 0, 0, 568, 37, 325, 
	28, 0, 263, 36, 148, 469, 342, 143, 443, 229, 298, 0, 157, 260, 432, 
	174, 0, 351, 0, 528, 0, 136, 0, 0, 0, 555, 165, 0, 506, 0, 0, 344, 0, 0 
};
const int MAP_Y[NUM_PLACES] = { 
	0, 147, 0, 166, 0, 109, 0, 61, 0, 0, 264, 70, 0, 0, 0, 266, 341, 30, 275,
	0, 294, 146, 311, 230, 184, 268, 133, 94, 207, 0, 142, 142, 330, 255, 0, 
	37, 0, 70, 0, 116, 0, 0, 0, 50, 21, 0, 303, 0, 0, 229, 0, 0 
};

const int MAP_TRANSLATE[NUM_PLACES] = {
	0, 0, 0, 1, 0, 2, 0, 3, 4, 0, 4, 6, 0, 0, 0, 8, 9, 10, 11, 0, 12, 13, 14, 7,
	15, 16, 17, 18, 19, 0, 20, 21, 22, 23, 0, 24, 0, 25, 0, 26, 0, 0, 0, 27,
	28, 0, 29, 0, 0, 30, 0 
};

const byte MAP_SEQUENCES[3][MAX_FRAME] = {
	{ 1, 1, 2, 3, 4, 0 },     // Overview Still
	{ 5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 0 },
	{ 5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 0 }
};

#define MAX_PEOPLE 66

const byte STILL_SEQUENCES[MAX_PEOPLE][MAX_TALK_SEQUENCES] = {
	{ 1, 0, 0 },	// Sherlock Holmes
	{ 6, 0, 0 },	// Dr. Watson
	{ 4, 0, 0 },	// Inspector Lestrade
	{ 2, 0, 0 },	// Constable #1
	{ 2, 0, 0 },	// Constable #2
	{ 2, 0, 0 },	// Sheila Parker
	{ 3, 0, 0 },	// Henry Carruthers
	{ 9, 0, 0 },	// Lesly (flower girl)
	{ 13, 0, 0 },	// Usher #1
	{ 2, 0, 0 },	// Usher #2
	{ 4, 0, 0 },	// Fredrick Epstein
	{ 9, 0, 0 },	// Mrs.Worthington
	{ 2, 0, 0 },	// Coach
	{ 8, 0, 0 },	// Player
	{ 13, 0, 0 },	// Waterboy
	{ 6, 0, 0 },	// James Sanders
	{ 1, 0, 0 },	// Belle (perfumerie)
	{ 20, 0, 0 },	// Cleaning Girl (perfumerie)
	{ 17, 0, 0 },	// Epstien in the Opera Balcony
	{ 3, 0, 0 },	// Wiggins
	{ 2, 0, 0 },	// Paul (Brumwell/Carroway)
	{ 1, 0, 0 },	// Bartender
	{ 1, 0, 0 },	// Dirty Drunk
	{ 1, 0, 0 },	// Shouting Drunk
	{ 1, 0, 0 },	// Staggering Drunk
	{ 1, 0, 0 },	// Bouncer
	{ 6, 0, 0 },	// James Sanders - At Home
	{ 6, 0, 0 },	// The Coroner
	{ 1, 0, 0 },	// The Equestrian Shop Keeper
	{ 1, 0, 0 },	// George Blackwood
	{ 7, 0, 0 },	// Lars
	{ 1, 0, 0 },	// Sheila Parker
	{ 8, 0, 0 },	// Chemist
	{ 6, 0, 0 },	// Inspector Gregson
	{ 1, 0, 0 },	// Lawyer
	{ 1, 0, 0 },	// Mycroft
	{ 7, 0, 0 },	// Old Sherman
	{ 1, 0, 0 },	// Stock Boy in Chemist Shop
	{ 1, 0, 0 },	// Barman
	{ 1, 0, 0 },	// Dandy Player
	{ 1, 0, 0 },	// Rough-looking Player
	{ 1, 0, 0 },	// Spectator
	{ 1, 0, 0 },	// Robert Hunt
	{ 3, 0, 0 },	// Violet Secretary
	{ 1, 0, 0 },	// Pettigrew
	{ 8, 0, 0 },	// Augie (apple seller)
	{ 16, 0, 0 },	// Anna Carroway
	{ 1, 0, 0 },	// Guard
	{ 8, 0, 0 },	// Antonio Caruso
	{ 1, 0, 0 },	// Toby the Dog
	{ 13, 0, 0 },	// Simon Kingsley
	{ 2, 0, 0 },	// Alfred Tobacco Clerk
	{ 1, 0, 0 },	// Lady Brumwell
	{ 1, 0, 0 },	// Madame Rosa
	{ 1, 0, 0 },	// Lady Brumwell
	{ 1, 0, 0 },	// Joseph Moorehead
	{ 5, 0, 0 },	// Mrs. Beale
	{ 1, 0, 0 },	// Felix the Lion
	{ 1, 0, 0 },	// Hollingston
	{ 1, 0, 0 },	// Constable Callaghan
	{ 2, 0, 0 },	// Sergeant Jeremy Duncan
	{ 1, 0, 0 },	// Lord Brumwell
	{ 1, 0, 0 },	// Nigel Jameson
	{ 1, 0, 0 },	// Jonas (newspaper seller)
	{ 1, 0, 0 },	// Constable Dugan
	{ 4, 0, 0 }		// Inspector Lestrade (Yard)
};

byte TALK_SEQUENCES[MAX_PEOPLE][MAX_TALK_SEQUENCES] = {
	{ 1, 0, 0 },					// Sherlock Holmes
	{ 5, 5, 6, 7, 8, 7, 8, 6, 0, 0 },	// Dr. Watson
	{ 2, 0, 0 },					// Inspector Lestrade
	{ 1, 0, 0 },					// Constable #1
	{ 1, 0, 0 },					// Constable #2
	{ 2, 3, 0, 0 },					// Sheila Parker
	{ 3, 0, 0 },					// Henry Carruthers
	{ 1, 2, 3, 2, 1, 2, 3, 0, 0 },	// Lesly (flower girl)
	{ 13, 14, 0, 0 },				// Usher #1
	{ 2, 0, 0 },					// Usher #2
	{ 1, 2, 3, 4, 3, 4, 3, 2, 0, 0 },	// Fredrick Epstein
	{ 8, 0, 0 },						// Mrs.Worthington
	{ 1, 2, 3, 4, 5, 4, 3, 2, 0, 0 },	// Coach
	{ 7, 8, 0, 0 },						// Player
	{ 12, 13, 0, 0 },					// Waterboy
	{ 3, 4, 0, 0 },						// James Sanders
	{ 4, 5, 0, 0 },						// Belle (perfumerie)
	{ 14, 15, 16, 17, 18, 19, 20, 20, 20, 0, 0 },	// Cleaning Girl (perfumerie)
	{ 16, 17, 18, 18, 18, 17, 17, 0, 0 },	// Epstien in the Opera Balcony
	{ 2, 3, 0, 0 },					// Wiggins
	{ 1, 2, 0, 0 },					// Paul (Brumwell/Carroway)
	{ 1, 0, 0 },					// Bartender
	{ 1, 0, 0 },					// Dirty Drunk
	{ 1, 0, 0 },					// Shouting Drunk
	{ 1, 0, 0 },					// Staggering Drunk
	{ 1, 0, 0 },					// Bouncer
	{ 5, 6, 0, 0 },					// James Sanders - At Home
	{ 4, 5, 0, 0 },					// The Coroner
	{ 1, 0, 0 },					// The Equestrian Shop Keeper
	{ 1, 0, 0 },					// George Blackwood
	{ 5, 6, 0, 0 },					// Lars
	{ 1, 0, 0 },					// Sheila Parker
	{ 8, 9, 0, 0 },					// Chemist
	{ 5, 6, 0, 0 },					// Inspector Gregson
	{ 1, 0, 0 },					// Lawyer
	{ 1, 0, 0 },					// Mycroft
	{ 7, 8, 0, 0 },					// Old Sherman
	{ 1, 0, 0 },					// Stock Boy in Chemist Shop
	{ 1, 0, 0 },					// Barman
	{ 1, 0, 0 },					// Dandy Player
	{ 1, 0, 0 },					// Rough-looking Player
	{ 1, 0, 0 },					// Spectator
	{ 1, 0, 0 },					// Robert Hunt
	{ 3, 4, 0, 0 },					// Violet Secretary
	{ 1, 0, 0 },					// Pettigrew
	{ 14, 15, 0, 0 },				// Augie (apple seller)
	{ 3, 4, 5, 6, 0, 0 },			// Anna Carroway
	{ 4, 5, 6, 0, 0 },				// Guard
	{ 7, 8, 0, 0 },					// Antonio Caruso
	{ 1, 0, 0 },					// Toby the Dog
	{ 13, 14, 0, 0 },				// Simon Kingsley
	{ 2, 3, 0, 0 },					// Alfred Tobacco Clerk
	{ 3, 4, 0, 0 },					// Lady Brumwell
	{ 1, 30, 0, 0 },				// Madame Rosa
	{ 3, 4, 0, 0 },					// Lady Brumwell
	{ 1, 0, 0 },					// Joseph Moorehead
	{ 14, 15, 16, 17, 18, 19, 20, 0, 0 },	// Mrs. Beale
	{ 1, 0, 0 },					// Felix the Lion
	{ 1, 0, 0 },					// Hollingston
	{ 1, 0, 0 },					// Constable Callaghan
	{ 1, 1, 2, 2, 0, 0 },			// Sergeant Jeremy Duncan
	{ 9, 10, 0, 0 },				// Lord Brumwell
	{ 1, 2, 0, 138, 3, 4, 0, 138, 0, 0 },	// Nigel Jameson
	{ 1, 8, 0, 0 },					// Jonas (newspaper seller)
	{ 1, 0, 0 },					// Constable Dugan
	{ 2, 0, 0 }						// Inspector Lestrade (Yard)
};

/*----------------------------------------------------------------*/

ScalpelEngine::ScalpelEngine(OSystem *syst, const SherlockGameDescription *gameDesc) :
		SherlockEngine(syst, gameDesc) {
	_darts = nullptr;
	_mapResult = 0;
}

ScalpelEngine::~ScalpelEngine() {
	delete _darts;
}

/**
 * Game initialization
 */
void ScalpelEngine::initialize() {
	SherlockEngine::initialize();

	_darts = new Darts(this);

	_flags.resize(100 * 8);
	_flags[3] = true;		// Turn on Alley
	_flags[39] = true;		// Turn on Baker Street

	// Load the map co-ordinates for each scene and sequence data
	_map->loadPoints(NUM_PLACES, &MAP_X[0], &MAP_Y[0], &MAP_TRANSLATE[0]);
	_map->loadSequences(3, &MAP_SEQUENCES[0][0]);

	// Load the inventory
	loadInventory();

	// Set up constants used by the talk system
	_talk->setSequences(&TALK_SEQUENCES[0][0], &STILL_SEQUENCES[0][0], MAX_PEOPLE);

	// Starting scene
	_scene->_goToScene = 4;
}

/**
 * Show the opening sequence
 */
void ScalpelEngine::showOpening() {
	if (!showCityCutscene())
		return;
	if (!showAlleyCutscene())
		return;
	if (!showStreetCutscene())
		return;
	if (!showOfficeCutscene())
		return;

	_events->clearEvents();
	_sound->stopMusic();
}

bool ScalpelEngine::showCityCutscene() {
	byte palette[PALETTE_SIZE];
	
	_sound->playMusic("prolog1.mus");
	_titleOverride = "title.lib";
	_soundOverride = "title.snd";
	bool finished = _animation->playPrologue("26open1", 1, 255, true, 2);

	if (finished) {
		ImageFile titleImages("title2.vgs", true);
		_screen->_backBuffer1.copyFrom(*_screen);
		_screen->_backBuffer2.copyFrom(*_screen);

		// London, England
		_screen->_backBuffer1.transBlitFrom(titleImages[0], Common::Point(10, 11));
		_screen->randomTransition();
		finished = _events->delay(1000, true);

		// November, 1888
		if (finished) {
			_screen->_backBuffer1.transBlitFrom(titleImages[1], Common::Point(101, 102));
			_screen->randomTransition();
			finished = _events->delay(5000, true);
		}

		// Transition out the title
		_screen->_backBuffer1.blitFrom(_screen->_backBuffer2);
		_screen->randomTransition();
	}

	if (finished)
		finished = _animation->playPrologue("26open2", 1, 0, false, 2);

	if (finished) {
		ImageFile titleImages("title.vgs", true);
		_screen->_backBuffer1.copyFrom(*_screen);
		_screen->_backBuffer2.copyFrom(*_screen);
		
		// The Lost Files of
		_screen->_backBuffer1.transBlitFrom(titleImages[0], Common::Point(75, 6));
		// Sherlock Holmes
		_screen->_backBuffer1.transBlitFrom(titleImages[1], Common::Point(34, 21));
		// copyright
		_screen->_backBuffer1.transBlitFrom(titleImages[2], Common::Point(4, 190));

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
			_screen->transBlitFrom(titleImages[3], Common::Point(72, 51));
			_screen->fadeIn(palette, 3);
			finished = _events->delay(3000, true);
		}
	}

	_titleOverride = "";
	_soundOverride = "";
	return finished;
}

bool ScalpelEngine::showAlleyCutscene() {
	byte palette[PALETTE_SIZE];
	_sound->playMusic("prolog2.mus");

	_titleOverride = "TITLE.LIB";
	_soundOverride = "TITLE.SND";

	bool finished = _animation->playPrologue("27PRO1", 1, 3, true, 2);
	if (finished)
		finished = _animation->playPrologue("27PRO2", 1, 0, false, 2);

	if (finished) {
		ImageFile screamImages("SCREAM.LBV", false);
		_screen->_backBuffer1.transBlitFrom(screamImages[0], Common::Point(0, 0));
		_screen->_backBuffer2.blitFrom(_screen->_backBuffer1);
		finished = _events->delay(6000);
	}

	if (finished)
		finished = _animation->playPrologue("27PRO3", 1, 0, true, 2);

	if(finished) {
		_screen->getPalette(palette);
		_screen->fadeToBlack(2);
	}

	if(finished) {
		ImageFile titleImages("title3.vgs", true);
		// "Early the following morning on Baker Street..."
		_screen->_backBuffer1.transBlitFrom(titleImages[0], Common::Point(35, 51), false, 0);
		_screen->fadeIn(palette, 3);
		finished = _events->delay(1000);
	}

	_titleOverride = "";
	_soundOverride = "";
	return finished;
}

bool ScalpelEngine::showStreetCutscene() {
	_titleOverride = "TITLE.LIB";
	_soundOverride = "TITLE.SND";

	_sound->playMusic("PROLOG3.MUS");

	bool finished = _animation->playPrologue("14KICK", 1, 3, true, 2);

	if (finished)
		finished = _animation->playPrologue("14NOTE", 1, 0, false, 2);

	_titleOverride = "";
	_soundOverride = "";
	return finished;
}

bool ScalpelEngine::scrollCredits() {
	_titleOverride = "TITLE.LIB";
	ImageFile creditsImages("credits.vgs", true);

	_screen->_backBuffer1.copyFrom(*_screen->_backBuffer);

	for(int i = 0; i < 600 && !_events->kbHit(); i++) {
		_screen->_backBuffer1.copyFrom(*_screen->_backBuffer);
		if (i < 200)
			_screen->transBlitFrom(creditsImages[0], Common::Point(10, -i), false, 0); 
		if (i > 0 && i < 400)
			_screen->transBlitFrom(creditsImages[1], Common::Point(10, 200 - i), false, 0);
		if (i > 200)
			_screen->transBlitFrom(creditsImages[2], Common::Point(10, 400 - i), false, 0);

		warning("TODO: Use VideoBlockMove");
//		videoblockmove(SCREEN, BACKBUFFER, 0, 0, 320, 10);
//		videoblockmove(SCREEN, BACKBUFFER, 0, 190, 320, 10);

		_events->delay(100);
	}

	_titleOverride = "";
	return true;
}

bool ScalpelEngine::showOfficeCutscene() {
	_sound->playMusic("PROLOG4.MUS");
	_titleOverride = "TITLE2.LIB";
	_soundOverride = "TITLE.SND";

	bool finished = _animation->playPrologue("COFF1", 1, 3, true, 3);
	if (finished)
		finished = _animation->playPrologue("COFF2", 1, 0, false, 3);
	if (finished) {
		warning("TODO: ShowLBV(""NOTE.LBV"");");
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

		_events->clearEvents();
		finished = _events->delay(500);
	}

	if (finished)
		finished = _animation->playPrologue("COFF3", 1, 0, true, 3);

	if (finished)
		finished = _animation->playPrologue("COFF4", 1, 0, false, 3);

	if (finished)
		finished = scrollCredits();

	if (finished)
		_screen->fadeToBlack(3);

	_titleOverride = "";
	_soundOverride = "";
	return finished;
}

void ScalpelEngine::loadInventory() {
	Inventory &inv = *_inventory;

	// Initial inventory
	inv._holdings = 2;
	inv.push_back(InventoryItem(0, "Message", "A message requesting help", "_ITEM03A"));
	inv.push_back(InventoryItem(0, "Holmes Card", "A number of business cards", "_ITEM07A"));
	
	// Potential items
	inv.push_back(InventoryItem(95, "Tickets", "Opera Tickets", "_ITEM10A"));
	inv.push_back(InventoryItem(138, "Cuff Link", "Cuff Link", "_ITEM04A"));
	inv.push_back(InventoryItem(138, "Wire Hook", "Wire Hook", "_ITEM06A"));
	inv.push_back(InventoryItem(150, "Note", "Note", "_ITEM13A"));
	inv.push_back(InventoryItem(481, "Open Watch", "An open pocket watch", "_ITEM62A"));
	inv.push_back(InventoryItem(481, "Paper", "A piece of paper with numbers on it", "_ITEM44A"));
	inv.push_back(InventoryItem(532, "Letter", "A letter folded many times", "_ITEM68A"));
	inv.push_back(InventoryItem(544, "Tarot", "Tarot Cards", "_ITEM71A"));
	inv.push_back(InventoryItem(544, "Ornate Key", "An ornate key", "_ITEM70A"));
	inv.push_back(InventoryItem(586, "Pawn ticket", "A pawn ticket", "_ITEM16A"));
};


/**
 * Starting a scene within the game
 */
void ScalpelEngine::startScene() {
	if (_scene->_goToScene == 100 || _scene->_goToScene == 98) {
		// Chessboard selection
		if (_sound->_musicOn) {
			if (_sound->loadSong(100)) {
				if (_sound->_music)
					_sound->startSong();
			}
		}

		_scene->_goToScene = _map->show();

		_sound->freeSong();
		_people->_hSavedPos = Common::Point(-1, -1);
		_people->_hSavedFacing = -1;
	}

	// Some rooms are prologue cutscenes, rather than normal game scenes. These are:
	//  2: Blackwood's capture
	// 52: Rescuing Anna
	// 53: Moorehead's death / subway train
	// 55: Fade out and exit 
	// 70: Brumwell suicide
	switch (_scene->_goToScene) {
	case 2:
	case 52:
	case 53:
	case 70:
		if (_sound->_musicOn && _sound->loadSong(_scene->_goToScene)) {
			if (_sound->_music)
				_sound->startSong();
		}

		switch (_scene->_goToScene) {
		case 2:
			// Blackwood's capture
			_res->addToCache("final2.vda", "epilogue.lib");
			_res->addToCache("final2.vdx", "epilogue.lib");
			_animation->playPrologue("final1", 1, 3, true, 4);
			_animation->playPrologue("final2", 1, 0, false, 4);
			break;

		case 52:
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

			_animation->playPrologue("finalr1", 1, 3, true, 4);
			_animation->playPrologue("finalr2", 1, 0, false, 4);

			if (!_res->isInCache("finale2.vda")) {
				// Finale file isn't cached
				_res->addToCache("finale2.vda", "epilogue.lib");
				_res->addToCache("finale2.vdx", "epilogue.lib");
				_res->addToCache("finale3.vda", "epilogue.lib");
				_res->addToCache("finale3.vdx", "epilogue.lib");
				_res->addToCache("finale4.vda", "EPILOG2.lib");
				_res->addToCache("finale4.vdx", "EPILOG2.lib");
			}

			_animation->playPrologue("finale1", 1, 0, false, 4);
			_animation->playPrologue("finale2", 1, 0, false, 4);
			_animation->playPrologue("finale3", 1, 0, false, 4);

			_useEpilogue2 = true;
			_animation->playPrologue("finale4", 1, 0, false, 4);
			_useEpilogue2 = false;
			break;

		case 53:
			// Moorehead's death / subway train
			_res->addToCache("SUBWAY2.vda", "epilogue.lib");
			_res->addToCache("SUBWAY2.vdx", "epilogue.lib");
			_res->addToCache("SUBWAY3.vda", "epilogue.lib");
			_res->addToCache("SUBWAY3.vdx", "epilogue.lib");

			_animation->playPrologue("SUBWAY1", 1, 3, true, 4);
			_animation->playPrologue("SUBWAY2", 1, 0, false, 4);
			_animation->playPrologue("SUBWAY3", 1, 0, false, 4);

			// Set fading to direct fade temporary so the transition goes quickly.
			_scene->_tempFadeStyle = _screen->_fadeStyle ? 257 : 256;
			_screen->_fadeStyle = false;
			break;

		case 70:
			// Brumwell suicide
			_animation->playPrologue("suicid", 1, 3, true, 4);
			break;
		default:
			break;
		}

		// Except for the Moorehead Murder scene, fade to black first
		if (_scene->_goToScene != 53) {
			_events->wait(40);
			_screen->fadeToBlack(3);
		}

		switch (_scene->_goToScene) {
		case 52:
			_scene->_goToScene = 27;			// Go to the Lawyer's Office
			_map->_bigPos = Common::Point(0, 0);	// Overland scroll position
			_map->_overPos = Common::Point(22900 - 600, 9400 + 900);	// Overland position
			_map->_oldCharPoint = 27;
			break;

		case 53:
			_scene->_goToScene = 17;			// Go to St. Pancras Station
			_map->_bigPos = Common::Point(0, 0);	// Overland scroll position
			_map->_overPos = Common::Point(32500 - 600, 3000 + 900);	// Overland position
			_map->_oldCharPoint = 17;
			break;

		default:
			_scene->_goToScene = 4;			// Back to Baker st.
			_map->_bigPos = Common::Point(0, 0);	// Overland scroll position
			_map->_overPos = Common::Point(14500 - 600, 8400 + 900);	// Overland position
			_map->_oldCharPoint = 4;
			break;
		}

		// Free any song from the previous scene
		_sound->freeSong();
		break;

	case 55:
		// Exit game
		_screen->fadeToBlack(3);
		quitGame();
		return;

	default:
		break;
	}

	_events->loadCursors("rmouse.vgs");
	_events->setCursor(ARROW);

	if (_scene->_goToScene == 99) {
		// Darts Board minigame
		_darts->playDarts();
		_mapResult = _scene->_goToScene = 19;	// Go back to the bar
	}

	_mapResult = _scene->_goToScene;
}

/**
 * Takes care of clearing the mirror in scene 12, in case anything drew over it
 */
void ScalpelEngine::eraseMirror12() {
	Common::Point pt((*_people)[AL]._position.x / 100, (*_people)[AL]._position.y / 100);

	// If player is in range of the mirror, then restore background from the secondary back buffer
	if (Common::Rect(70, 100, 200, 200).contains(pt)) {
		_screen->_backBuffer1.blitFrom(_screen->_backBuffer2, Common::Point(137, 18),
			Common::Rect(137, 18, 184, 74));
	}
}

/**
 * Takes care of drawing Holme's reflection onto the mirror in scene 12
 */
void ScalpelEngine::doMirror12() {
	People &people = *_people;
	Common::Point pt((*_people)[AL]._position.x / 100, (*_people)[AL]._position.y / 100);
	int frameNum = (*people[AL]._sequences)[people[AL]._sequenceNumber][people[AL]._frameNumber] + 
		(*people[AL]._sequences)[people[AL]._sequenceNumber][0] - 2;

	switch ((*_people)[AL]._sequenceNumber) {
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
		ImageFrame &imageFrame = (*people[AL]._images)[frameNum];

		// Draw the mirror image of Holmes
		bool flipped = people[AL]._sequenceNumber == WALK_LEFT || people[AL]._sequenceNumber == STOP_LEFT
			|| people[AL]._sequenceNumber == WALK_UPRIGHT || people[AL]._sequenceNumber == STOP_UPRIGHT
			|| people[AL]._sequenceNumber == WALK_DOWNLEFT || people[AL]._sequenceNumber == STOP_DOWNLEFT;
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

/**
 * This clears the mirror  in scene 12 in case anything messed draw over it
 */
void ScalpelEngine::flushMirror12() {
	Common::Point pt((*_people)[AL]._position.x / 100, (*_people)[AL]._position.y / 100);

	// If player is in range of the mirror, then draw the entire mirror area to the screen
	if (Common::Rect(70, 100, 200, 200).contains(pt))
		_screen->slamArea(137, 18, 47, 56);
}

} // End of namespace Scalpel

} // End of namespace Scalpel
