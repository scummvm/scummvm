/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "sword1.h"

#include "backends/fs/fs.h"

#include "base/plugins.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/timer.h"

#include "memman.h"
#include "resman.h"
#include "objectman.h"
#include "mouse.h"
#include "logic.h"
#include "sound.h"
#include "screen.h"
#include "swordres.h"
#include "menu.h"
#include "music.h"
#include "control.h"

using namespace Sword1;

/* Broken Sword 1 */
static const GameSettings sword1_setting =
	{"sword1", "Broken Sword I", GF_DEFAULT_TO_1X_SCALER};

GameList Engine_SWORD1_gameList() {
	GameList games;
	games.push_back(sword1_setting);
	return games;
}

DetectedGameList Engine_SWORD1_detectGames(const FSList &fslist) {
	DetectedGameList detectedGames;

	// Iterate over all files in the given directory
	for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		const char *gameName = file->displayName().c_str();

		if ((0 == scumm_stricmp("swordres.rif", gameName)) ||
			(0 == scumm_stricmp("cd1.id", gameName)) ||
			(0 == scumm_stricmp("cd2.id", gameName))) {
			// Match found, add to list of candidates, then abort inner loop.
			detectedGames.push_back(sword1_setting);
			break;
		}
	}

	return detectedGames;
}

Engine *Engine_SWORD1_create(GameDetector *detector, OSystem *syst) {
	return new SwordEngine(detector, syst);
}

REGISTER_PLUGIN("Broken Sword", Engine_SWORD1_gameList, Engine_SWORD1_create, Engine_SWORD1_detectGames)

namespace Sword1 {

SystemVars SwordEngine::_systemVars;

void SwordEngine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

SwordEngine::SwordEngine(GameDetector *detector, OSystem *syst)
	: Engine(syst) {

	if (!_mixer->isReady())
		warning("Sound initialization failed");
}

SwordEngine::~SwordEngine() {
}

void SwordEngine::initialize(void) {
	_system->initSize(640, 480);
	debug(5, "Starting memory manager");
	_memMan = new MemMan();
	debug(5, "Starting resource manager");
	_resMan = new ResMan("swordres.rif", _memMan);
	debug(5, "Starting object manager");
	_objectMan = new ObjectMan(_resMan);
	_mixer->setVolume(255);
	_mouse = new Mouse(_system, _resMan, _objectMan);
	_screen = new Screen(_system, _resMan, _objectMan);
	_music = new Music(_system, _mixer);
	_sound = new Sound("", _mixer, _resMan);
	_menu = new Menu(_screen, _mouse);
	_logic = new Logic(_objectMan, _resMan, _screen, _mouse, _sound, _music, _menu, _system, _mixer);
	_mouse->useLogicAndMenu(_logic, _menu);

	uint8 musicVol = (uint8)ConfMan.getInt("music_volume");
	uint8 speechVol = (uint8)ConfMan.getInt("speech_volume");
	uint8 sfxVol = (uint8)ConfMan.getInt("sfx_volume");
	if (!speechVol)
		speechVol = 192;

	_music->setVolume(musicVol, musicVol);      // these routines expect left and right volume,
	_sound->setSpeechVol(speechVol, speechVol); // but our config manager doesn't support it.
	_sound->setSfxVol(sfxVol, sfxVol);

	_systemVars.justRestoredGame = _systemVars.currentCD = 
		_systemVars.gamePaused = 0;
	_systemVars.deathScreenFlag = 3;
	_systemVars.forceRestart = false;
	_systemVars.wantFade = true;

	switch (Common::parseLanguage(ConfMan.get("language"))) {
	case Common::DE_DEU:
		_systemVars.language = BS1_GERMAN;
		break;
	case Common::FR_FRA:
		_systemVars.language = BS1_FRENCH;
		break;
	case Common::IT_ITA:
		_systemVars.language = BS1_ITALIAN;
		break;
	case Common::ES_ESP:
		_systemVars.language = BS1_SPANISH;
		break;
	case Common::PT_BRA:
		_systemVars.language = BS1_PORT;
		break;
	case Common::CZ_CZE:
		_systemVars.language = BS1_CZECH;
		break;
	default:
		_systemVars.language = BS1_ENGLISH;
	}

	_systemVars.showText = ConfMan.getBool("subtitles");
	
	_systemVars.playSpeech = 1;
	_mouseState = 0;

	_logic->initialize();
	_objectMan->initialize();
	_mouse->initialize();
	_control = new Control(_resMan, _objectMan, _system, _mouse, _sound, _music, getSavePath());
}

void SwordEngine::reinitialize(void) {
	_resMan->flush(); // free everything that's currently alloced and opened.
	_memMan->flush(); // Handle with care.

	_logic->initialize();     // now reinitialize these objects as they (may) have locked
	_objectMan->initialize(); // resources which have just been wiped.
	_mouse->initialize();
	_system->warp_mouse(320, 240);
	_systemVars.wantFade = true;
}

void SwordEngine::startPositions(int32 startNumber) {
	// int32 sect;
	Object *compact;

	Logic::_scriptVars[CHANGE_STANCE] = STAND;
	Logic::_scriptVars[GEORGE_CDT_FLAG] = GEO_TLK_TABLE;

	//-------------------------------------------------------------------------------------------------------
	// START 0==intro; 1==without

	if ((startNumber==0)||(startNumber==1))
	{
		if (startNumber==0)
		{
			// Tdebug("Calling fn check cd");
			// FN_check_CD(0,0,1,0,0,0,0,0);	// request CD for sc1 (which happens to be CD-1)
			// Tdebug("Calling fn play sequence");
			_logic->fnPlaySequence(0,0,4,0,0,0,0,0);	// intro
			debug(1, "Setting start number to 1");
			startNumber=1;
		}
	
	
		Logic::_scriptVars[CHANGE_X] = 481;
		Logic::_scriptVars[CHANGE_Y] = 413;
		Logic::_scriptVars[CHANGE_DIR] = DOWN;
		Logic::_scriptVars[CHANGE_PLACE] = FLOOR_1;
	} 
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==2)	// blind_alley
	{
		Logic::_scriptVars[CHANGE_X] = 480;
		Logic::_scriptVars[CHANGE_Y] = 388;
		Logic::_scriptVars[CHANGE_DIR] = DOWN_LEFT;
		Logic::_scriptVars[CHANGE_PLACE] = FLOOR_2;

		_logic->fnAddObject(0,0,LIFTING_KEYS,0,0,0,0,0);
		_logic->fnAddObject(0,0,ROSSO_CARD,0,0,0,0,0);

		Logic::_scriptVars[POCKET_1] = 1;
		Logic::_scriptVars[POCKET_2] = 1;
		Logic::_scriptVars[POCKET_3] = 1;
		Logic::_scriptVars[POCKET_4] = 1;
		Logic::_scriptVars[POCKET_5] = 1;
		Logic::_scriptVars[POCKET_6] = 1;
		Logic::_scriptVars[POCKET_7] = 1;
		Logic::_scriptVars[POCKET_8] = 1;
		Logic::_scriptVars[POCKET_9] = 1;

		Logic::_scriptVars[POCKET_10] = 1;
		Logic::_scriptVars[POCKET_11] = 1;
		Logic::_scriptVars[POCKET_12] = 1;
		Logic::_scriptVars[POCKET_13] = 1;
		Logic::_scriptVars[POCKET_14] = 1;
		Logic::_scriptVars[POCKET_15] = 1;
		Logic::_scriptVars[POCKET_16] = 1;
		Logic::_scriptVars[POCKET_17] = 1;
		Logic::_scriptVars[POCKET_18] = 1;
		Logic::_scriptVars[POCKET_19] = 1;

		Logic::_scriptVars[POCKET_20] = 1;
		Logic::_scriptVars[POCKET_21] = 1;
		Logic::_scriptVars[POCKET_22] = 1;
		Logic::_scriptVars[POCKET_23] = 1;
		Logic::_scriptVars[POCKET_24] = 1;
		Logic::_scriptVars[POCKET_25] = 1;
		Logic::_scriptVars[POCKET_26] = 1;
		Logic::_scriptVars[POCKET_27] = 1;
		Logic::_scriptVars[POCKET_28] = 1;
		Logic::_scriptVars[POCKET_29] = 1;
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==3)	// cafe
	{
		Logic::_scriptVars[CHANGE_X] = 660;
		Logic::_scriptVars[CHANGE_Y] = 368;
		Logic::_scriptVars[CHANGE_DIR] = DOWN_LEFT;
		Logic::_scriptVars[CHANGE_PLACE] = FLOOR_3;
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==4)	// ready to use phone
	{
		Logic::_scriptVars[CHANGE_X] = 463;
		Logic::_scriptVars[CHANGE_Y] = 391;
		Logic::_scriptVars[CHANGE_DIR] = DOWN;
		Logic::_scriptVars[CHANGE_PLACE] = FLOOR_4;

		Logic::_scriptVars[MOUE_TEXT] = 1;		// stop moue from entering ("Freeze...")
		Logic::_scriptVars[MOUE_NICO_FLAG] = 1;			// Moue gave George her number
//		paris_flag=2;					// for Nico's phone script
		Logic::_scriptVars[PARIS_FLAG] = 5;	// to access room8 (cafe_repaired)
		Logic::_scriptVars[NICO_PHONE_FLAG] = 1;	// Nico's number is on envelope
		Logic::_scriptVars[TAILOR_PHONE_FLAG] = 1;	// Todryk's number is on envelope
		Logic::_scriptVars[WORKMAN_GONE_FLAG] = 1;	// Workman not here
		Logic::_scriptVars[ALBERT_INFO_FLAG] = 1;	// Albert has told us the tailor's number (for Nico's phone script)
		Logic::_scriptVars[SEEN_SEWERS_FLAG] = 1;

		Logic::_scriptVars[POCKET_30] = 1;
		Logic::_scriptVars[POCKET_31] = 1;
		Logic::_scriptVars[POCKET_32] = 1;
		Logic::_scriptVars[POCKET_33] = 1;
		Logic::_scriptVars[POCKET_34] = 1;
		Logic::_scriptVars[POCKET_35] = 1;
		Logic::_scriptVars[POCKET_36] = 1;
		Logic::_scriptVars[POCKET_37] = 1;
		Logic::_scriptVars[POCKET_38] = 1;
		Logic::_scriptVars[POCKET_39] = 1;

		Logic::_scriptVars[POCKET_40] = 1;
		Logic::_scriptVars[POCKET_41] = 1;
		Logic::_scriptVars[POCKET_42] = 1;
		Logic::_scriptVars[POCKET_43] = 1;
		Logic::_scriptVars[POCKET_44] = 1;
		Logic::_scriptVars[POCKET_45] = 1;
		Logic::_scriptVars[POCKET_46] = 1;
		Logic::_scriptVars[POCKET_47] = 1;
		Logic::_scriptVars[POCKET_48] = 1;
		Logic::_scriptVars[POCKET_49] = 1;

		Logic::_scriptVars[POCKET_50] = 1;
		Logic::_scriptVars[POCKET_51] = 1;
		Logic::_scriptVars[POCKET_52] = 1;
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==5)	// court_yard
	{
		Logic::_scriptVars[CHANGE_X] = 400;
		Logic::_scriptVars[CHANGE_Y] = 400;
		Logic::_scriptVars[CHANGE_DIR] = DOWN_LEFT;
		Logic::_scriptVars[CHANGE_PLACE] = FLOOR_5;
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==7)	// sewer_two
	{
		Logic::_scriptVars[CHANGE_X] = 520;
		Logic::_scriptVars[CHANGE_Y] = 310;
		Logic::_scriptVars[CHANGE_DIR] = DOWN_LEFT;
		Logic::_scriptVars[CHANGE_PLACE] = FLOOR_7;
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==8)	// cafe_repaired
	{
		Logic::_scriptVars[CHANGE_X] = 481;
		Logic::_scriptVars[CHANGE_Y] = 413;
		Logic::_scriptVars[CHANGE_DIR] = DOWN;
		Logic::_scriptVars[CHANGE_PLACE] = FLOOR_8;
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==11)	// costumier
	{
		Logic::_scriptVars[CHANGE_X] = 264;
		Logic::_scriptVars[CHANGE_Y] = 436;
		Logic::_scriptVars[CHANGE_DIR] = DOWN_RIGHT;
		Logic::_scriptVars[CHANGE_PLACE] = FLOOR_11;

		_logic->fnAddObject(0,0,TISSUE,0,0,0,0,0);
		_logic->fnAddObject(0,0,PHOTOGRAPH,0,0,0,0,0);
	}
	//-------------------------------------------------------------------------------------------------------
	// HOTEL SECTION
	else if (startNumber==12)	// hotel_street
	{
		Logic::_scriptVars[CHANGE_X] = 730;
		Logic::_scriptVars[CHANGE_Y] = 460;
		Logic::_scriptVars[CHANGE_DIR] = LEFT;
		Logic::_scriptVars[CHANGE_PLACE] = FLOOR_12;

		Logic::_scriptVars[PARIS_FLAG] = 6;

		_logic->fnAddObject(0,0,PHOTOGRAPH,0,0,0,0,0);

		Logic::_scriptVars[NICO_ADDRESS_FLAG] = 1;
		Logic::_scriptVars[NICO_PHONE_FLAG] = 1;
		Logic::_scriptVars[COSTUMES_ADDRESS_FLAG] = 1;
		Logic::_scriptVars[HOTEL_ADDRESS_FLAG] = 1;
		Logic::_scriptVars[AEROPORT_ADDRESS_FLAG] = 1;
		Logic::_scriptVars[TAILOR_PHONE_FLAG] = 1;
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==14)	// hotel_corridor
	{
		Logic::_scriptVars[CHANGE_X] = 528;
		Logic::_scriptVars[CHANGE_Y] = 484;
		Logic::_scriptVars[CHANGE_DIR] = UP;
		Logic::_scriptVars[CHANGE_PLACE] = FLOOR_14;

		_logic->fnAddObject(0,0,HOTEL_KEY,0,0,0,0,0);
		_logic->fnAddObject(0,0,MANUSCRIPT,0,0,0,0,0);
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==17)	// hotel_assassin
	{
		Logic::_scriptVars[CHANGE_X] = 714;
		Logic::_scriptVars[CHANGE_Y] = 484;
		Logic::_scriptVars[CHANGE_DIR] = LEFT;
		Logic::_scriptVars[CHANGE_PLACE] = FLOOR_17;
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==18)	// gendarmerie
	{
		Logic::_scriptVars[CHANGE_X] = 446;
		Logic::_scriptVars[CHANGE_Y] = 408;
		Logic::_scriptVars[CHANGE_DIR] = DOWN_LEFT;
		Logic::_scriptVars[CHANGE_PLACE] = FLOOR_18;

		Logic::_scriptVars[PARIS_FLAG] = 5;	// for Moue & Rosso
	}
	//-------------------------------------------------------------------------------------------------------
	// MUSEUM RAID
	else if (startNumber==27)	// museum_street
	{
		Logic::_scriptVars[CHANGE_X] = 300;
		Logic::_scriptVars[CHANGE_Y] = 510;
		Logic::_scriptVars[CHANGE_DIR] = UP_RIGHT;
		Logic::_scriptVars[CHANGE_PLACE] = FLOOR_27;

		Logic::_scriptVars[PARIS_FLAG] = 12;	// for Lobineau in Museum
		Logic::_scriptVars[MANUSCRIPT_ON_TABLE_10_FLAG] = 1;
	}
	//-------------------------------------------------------------------------------------------------------
	// HOSPITAL SECTION
	else if (startNumber==31)	// hospital_street
	{
		Logic::_scriptVars[CHANGE_X] = 400;
		Logic::_scriptVars[CHANGE_Y] = 500;
		Logic::_scriptVars[CHANGE_DIR] = UP_RIGHT;
		Logic::_scriptVars[CHANGE_PLACE] = FLOOR_31;

		Logic::_scriptVars[PARIS_FLAG] = 11;

		_logic->fnAddObject(0,0,PHOTOGRAPH,0,0,0,0,0);
		_logic->fnAddObject(0,0,LAB_PASS,0,0,0,0,0);
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==32)	// hospital_desk (after we've found out where Marquet is)
	{
		Logic::_scriptVars[CHANGE_X] = 405;
		Logic::_scriptVars[CHANGE_Y] = 446;
		Logic::_scriptVars[CHANGE_DIR] = UP_RIGHT;
		Logic::_scriptVars[CHANGE_PLACE] = FLOOR_32;

		Logic::_scriptVars[PARIS_FLAG] = 11;

		Logic::_scriptVars[EVA_TEXT] = 1;		// talked to eva
		Logic::_scriptVars[EVA_MARQUET_FLAG] = 2;
		Logic::_scriptVars[EVA_NURSE_FLAG] = 4;
		Logic::_scriptVars[FOUND_WARD_FLAG] = 1;
		Logic::_scriptVars[CONSULTANT_HERE] = 1;

		compact = (Object*)_objectMan->fetchObject(PLAYER);
		_logic->fnMegaSet(compact,PLAYER,GEORGE_WLK,MEGA_WHITE,0,0,0,0);

		Logic::_scriptVars[GEORGE_CDT_FLAG] = WHT_TLK_TABLE;
		Logic::_scriptVars[GEORGE_TALK_FLAG] = 0;
		Logic::_scriptVars[WHITE_COAT_FLAG] = 1;
		Logic::_scriptVars[GEORGE_ALLOWED_REST_ANIMS] = 0;		// because wearing white coat now
		
		Logic::_scriptVars[GOT_BENOIR_FLAG] = 1;

		_logic->fnAddObject(0,0,PHOTOGRAPH,0,0,0,0,0);
		_logic->fnAddObject(0,0,LAB_PASS,0,0,0,0,0);
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==35)	// hospital_jacques
	{
		Logic::_scriptVars[CHANGE_X] = 640;
		Logic::_scriptVars[CHANGE_Y] = 500;
		Logic::_scriptVars[CHANGE_DIR] = LEFT;
		Logic::_scriptVars[CHANGE_PLACE] = FLOOR_35;

		Logic::_scriptVars[PARIS_FLAG] = 11;

		Logic::_scriptVars[EVA_TEXT] = 1;		// talked to eva
		Logic::_scriptVars[EVA_MARQUET_FLAG] = 2;
		Logic::_scriptVars[EVA_NURSE_FLAG] = 4;
		Logic::_scriptVars[FOUND_WARD_FLAG] = 1;
		Logic::_scriptVars[CONSULTANT_HERE] = 1;

		compact = (Object*)_objectMan->fetchObject(PLAYER);
		_logic->fnMegaSet(compact,PLAYER,GEORGE_WLK,MEGA_WHITE,0,0,0,0);

		Logic::_scriptVars[GEORGE_CDT_FLAG] = WHT_TLK_TABLE;
		Logic::_scriptVars[GEORGE_TALK_FLAG] = 0;
		Logic::_scriptVars[WHITE_COAT_FLAG] = 1;
		Logic::_scriptVars[GEORGE_ALLOWED_REST_ANIMS] = 0;		// because wearing white coat now
		Logic::_scriptVars[DOOR_34_OPEN] = 1;
		
		Logic::_scriptVars[GOT_BENOIR_FLAG] = 2;
		Logic::_scriptVars[HOS_POS_FLAG] = 26;
		Logic::_scriptVars[BENOIR_FLAG] = 24;	// for 'george_enters_ward' script
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==36)	// montfaucon
	{
		Logic::_scriptVars[CHANGE_X] = 300;
		Logic::_scriptVars[CHANGE_Y] = 480;
		Logic::_scriptVars[CHANGE_DIR] = RIGHT;
		Logic::_scriptVars[CHANGE_PLACE] = FLOOR_36;

		_logic->fnAddObject(0,0,LENS,0,0,0,0,0);
		_logic->fnAddObject(0,0,RED_NOSE,0,0,0,0,0);
		_logic->fnAddObject(0,0,LIFTING_KEYS,0,0,0,0,0);
		
		Logic::_scriptVars[MONTFAUCON_CONTROL_FLAG] = 1;
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==37)	// catacomb_sewer
	{
		Logic::_scriptVars[CHANGE_X] = 592;
		Logic::_scriptVars[CHANGE_Y] = 386;
		Logic::_scriptVars[CHANGE_DIR] = RIGHT;
		Logic::_scriptVars[CHANGE_PLACE] = FLOOR_37;

		_logic->fnAddObject(0,0,LIFTING_KEYS,0,0,0,0,0);
		_logic->fnAddObject(0,0,TRIPOD,0,0,0,0,0);
		_logic->fnAddObject(0,0,GEM,0,0,0,0,0);
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==38)	// catacomb_room
	{
		Logic::_scriptVars[CHANGE_X] = 200;
		Logic::_scriptVars[CHANGE_Y] = 390;
		Logic::_scriptVars[CHANGE_DIR] = RIGHT;
		Logic::_scriptVars[CHANGE_PLACE] = FLOOR_38;

		_logic->fnAddObject(0,0,TRIPOD,0,0,0,0,0);
		_logic->fnAddObject(0,0,GEM,0,0,0,0,0);
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==39)	// catacomb_meeting
	{
		Logic::_scriptVars[CHANGE_X] = 636;
		Logic::_scriptVars[CHANGE_Y] = 413;
		Logic::_scriptVars[CHANGE_DIR] = DOWN_LEFT;
		Logic::_scriptVars[CHANGE_PLACE] = FLOOR_39;

		Logic::_scriptVars[MEETING_FLAG] = 3;	// meeting finished

		_logic->fnAddObject(0,0,TRIPOD,0,0,0,0,0);
		_logic->fnAddObject(0,0,GEM,0,0,0,0,0);
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==40)	// excavation_exterior
	{
		Logic::_scriptVars[CHANGE_X] = 648;
		Logic::_scriptVars[CHANGE_Y] = 492;
		Logic::_scriptVars[CHANGE_DIR] = LEFT;
		Logic::_scriptVars[CHANGE_PLACE] = FLOOR_40;

		Logic::_scriptVars[NICO_PHONE_FLAG] = 1;
		Logic::_scriptVars[PARIS_FLAG] = 16;

		_logic->fnAddObject(0,0,PLASTER,0,0,0,0,0);
		_logic->fnAddObject(0,0,POLISHED_CHALICE,0,0,0,0,0);
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==48)	// templar_church
	{
		Logic::_scriptVars[CHANGE_X] = 315;
		Logic::_scriptVars[CHANGE_Y] = 392;
		Logic::_scriptVars[CHANGE_DIR] = DOWN;
		Logic::_scriptVars[CHANGE_PLACE] = FLOOR_48;

		_logic->fnAddObject(0,0,CHALICE,0,0,0,0,0);
		_logic->fnAddObject(0,0,LENS,0,0,0,0,0);

		Logic::_scriptVars[CHALICE_FLAG] = 2;	// from end of Spain1, when George gets chalice
		Logic::_scriptVars[NEJO_TEXT] = 1;		// so priest is there
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==99)	// test text+speech on blank screen (sc99)
	{
		Logic::_scriptVars[CHANGE_PLACE] = FLOOR_99;

		// if (testingText)
		if (1)
		{
//			lastLineNo = testTextSection*65536 + LastTextNumber(testTextSection);
			Logic::_scriptVars[LASTLINENO] = 146*65536 + _objectMan->lastTextNumber(146);	// don't finished until at last line in last section
		}
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==80)	// Paris Map
	{
		Logic::_scriptVars[CHANGE_X] = 645;
		Logic::_scriptVars[CHANGE_Y] = 160;
		Logic::_scriptVars[CHANGE_DIR] = DOWN;
		Logic::_scriptVars[CHANGE_PLACE] = FLOOR_80;

		Logic::_scriptVars[PARIS_FLAG] = 3;	// for Nico's appartment
		Logic::_scriptVars[NICO_CLOWN_FLAG] = 3;

		Logic::_scriptVars[NICO_DOOR_FLAG] = 2;	// for instant access to Nico's appartment

		_logic->fnAddObject(0,0,RED_NOSE,0,0,0,0,0);
		_logic->fnAddObject(0,0,PHOTOGRAPH,0,0,0,0,0);
		_logic->fnAddObject(0,0,PLASTER,0,0,0,0,0);
		_logic->fnAddObject(0,0,LAB_PASS,0,0,0,0,0);	// for hospital

		Logic::_scriptVars[MANUSCRIPT_FLAG] = 1;

		Logic::_scriptVars[NICO_ADDRESS_FLAG] = 1;
		Logic::_scriptVars[NICO_PHONE_FLAG] = 1;
		Logic::_scriptVars[COSTUMES_ADDRESS_FLAG] = 1;
		Logic::_scriptVars[HOTEL_ADDRESS_FLAG] = 1;
		Logic::_scriptVars[MUSEUM_ADDRESS_FLAG] = 1;
		Logic::_scriptVars[HOSPITAL_ADDRESS_FLAG] = 1;
		Logic::_scriptVars[MONTFACN_ADDRESS_FLAG] = 1;
		Logic::_scriptVars[AEROPORT_ADDRESS_FLAG] = 1;
		Logic::_scriptVars[NERVAL_ADDRESS_FLAG] = 1;

		Logic::_scriptVars[IRELAND_MAP_FLAG] = 1;
		Logic::_scriptVars[SPAIN_MAP_FLAG] = 1;
		Logic::_scriptVars[SYRIA_FLAG] = 2;

		Logic::_scriptVars[TAILOR_PHONE_FLAG] = 1;
	}
	//-------------------------------------------------------------------------------------------------------
	// IRELAND
	else if ((startNumber>=19)&&(startNumber<=26))
	{
		Logic::_scriptVars[PARIS_FLAG] = 9;

		// already carrying these objects by the time we reach Ireland...
		_logic->fnAddObject(0,0,RED_NOSE,0,0,0,0,0);
		_logic->fnAddObject(0,0,PHOTOGRAPH,0,0,0,0,0);
		_logic->fnAddObject(0,0,LAB_PASS,0,0,0,0,0);
		_logic->fnAddObject(0,0,LIFTING_KEYS,0,0,0,0,0);
		_logic->fnAddObject(0,0,MATCHBOOK,0,0,0,0,0);
		_logic->fnAddObject(0,0,BUZZER,0,0,0,0,0);
		_logic->fnAddObject(0,0,TISSUE,0,0,0,0,0);

		switch (startNumber)
		{
			//---------------------------
			case 19:	// ireland_street
			{
				Logic::_scriptVars[CHANGE_X] = 256;
				Logic::_scriptVars[CHANGE_Y] = 966;
				Logic::_scriptVars[CHANGE_DIR] = UP_RIGHT;
				Logic::_scriptVars[CHANGE_PLACE] = FLOOR_19;
			}
			break;
			//---------------------------
			case 20:	// macdevitts
			{
				Logic::_scriptVars[CHANGE_X] = 194;
				Logic::_scriptVars[CHANGE_Y] = 417;
				Logic::_scriptVars[CHANGE_DIR] = DOWN_RIGHT;
				Logic::_scriptVars[CHANGE_PLACE] = FLOOR_20;

				Logic::_scriptVars[FARMER_MOVED_FLAG] = 1;
				Logic::_scriptVars[FARMER_SEAN_FLAG] = 5;

				Logic::_scriptVars[PUB_FLAP_FLAG] = 1;
				Logic::_scriptVars[PUB_TRAP_DOOR] = 2;
				Logic::_scriptVars[KNOWS_PEAGRAM_FLAG] = 1;
			}
			break;
			//---------------------------
			case 21:	// pub_cellar
			{
				Logic::_scriptVars[CHANGE_X] = 291;
				Logic::_scriptVars[CHANGE_Y] = 444;
				Logic::_scriptVars[CHANGE_DIR] = DOWN_RIGHT;
				Logic::_scriptVars[CHANGE_PLACE] = FLOOR_21;

				_logic->fnAddObject(0,0,BEER_TOWEL,0,0,0,0,0);
				
				Logic::_scriptVars[FARMER_MOVED_FLAG] = 1;
				Logic::_scriptVars[FLEECY_STUCK] = 1;
				Logic::_scriptVars[LIFTING_KEYS_IN_HOLE_23] = 1;
			}
			break;
			//---------------------------
			case 22:	// castle_gate
			{
				Logic::_scriptVars[CHANGE_X] = 547;
				Logic::_scriptVars[CHANGE_Y] = 500;
				Logic::_scriptVars[CHANGE_DIR] = UP_LEFT;
				Logic::_scriptVars[CHANGE_PLACE] = FLOOR_22;

				Logic::_scriptVars[IRELAND_FLAG] = 4;	// so we can get rid of farmer
			}
			break;
			//---------------------------
			case 23:	// castle_hay_top
			{
				Logic::_scriptVars[CHANGE_X] = 535;
				Logic::_scriptVars[CHANGE_Y] = 510;
				Logic::_scriptVars[CHANGE_DIR] = UP;
				Logic::_scriptVars[CHANGE_PLACE] = FLOOR_23;
			}
			break;
			//---------------------------
			case 24:	// castle_yard
			{
				Logic::_scriptVars[CHANGE_X] = 815;
				Logic::_scriptVars[CHANGE_Y] = 446;
				Logic::_scriptVars[CHANGE_DIR] = DOWN_LEFT;
				Logic::_scriptVars[CHANGE_PLACE] = FLOOR_24;
			}
			break;
			//---------------------------
			case 25:	// castle_dig
			{
				Logic::_scriptVars[CHANGE_X] = 369;
				Logic::_scriptVars[CHANGE_Y] = 492;
				Logic::_scriptVars[CHANGE_DIR] = LEFT;
				Logic::_scriptVars[CHANGE_PLACE] = FLOOR_25;

				_logic->fnAddObject(0,0,BEER_TOWEL,0,0,0,0,0);

				Logic::_scriptVars[BEER_TOWEL_BEEN_WET] = 1;
				Logic::_scriptVars[WET_BEER_TOWEL_TIMER] = 1000;
			}
			break;
			//---------------------------
			case 26:	// cellar_dark
			{
				Logic::_scriptVars[CHANGE_X] = 291;
				Logic::_scriptVars[CHANGE_Y] = 444;
				Logic::_scriptVars[CHANGE_DIR] = DOWN_RIGHT;
				Logic::_scriptVars[CHANGE_PLACE] = FLOOR_26;
			}
			break;
			//---------------------------
		}
	}
	//-------------------------------------------------------------------------------------------------------
	// SYRIA

	else if ((startNumber >= 45) && (startNumber <= 55))
	{
		Logic::_scriptVars[PARIS_FLAG] = 1;

		// already carrying these objects by the time we reach Syria...
		_logic->fnAddObject(0,0,BALL,0,0,0,0,0);
		_logic->fnAddObject(0,0,RED_NOSE,0,0,0,0,0);
		_logic->fnAddObject(0,0,PHOTOGRAPH,0,0,0,0,0);
		_logic->fnAddObject(0,0,LIFTING_KEYS,0,0,0,0,0);
		_logic->fnAddObject(0,0,MATCHBOOK,0,0,0,0,0);
		_logic->fnAddObject(0,0,BUZZER,0,0,0,0,0);
		_logic->fnAddObject(0,0,TISSUE,0,0,0,0,0);

		Logic::_scriptVars[CHANGE_STANCE] = STAND;		// use STAND for all starts
		
		switch (startNumber)
		{
			//---------------------------
			case 45:	// syria_stall
			{
				Logic::_scriptVars[CHANGE_X] = 410;
				Logic::_scriptVars[CHANGE_Y] = 490;
				Logic::_scriptVars[CHANGE_DIR] = DOWN_RIGHT;
				Logic::_scriptVars[CHANGE_PLACE] = FLOOR_45;
			}
			break;
			//---------------------------
			case 47:	// syria_carpet
			{
				Logic::_scriptVars[CHANGE_X] = 225;
				Logic::_scriptVars[CHANGE_Y] = 775;
				Logic::_scriptVars[CHANGE_DIR] = RIGHT;
				Logic::_scriptVars[CHANGE_PLACE] = FLOOR_47;
			}
			break;
			//---------------------------
			case 49:	// syria_club
			{
				Logic::_scriptVars[CHANGE_X] = 438;
				Logic::_scriptVars[CHANGE_Y] = 400;
				Logic::_scriptVars[CHANGE_DIR] = DOWN_RIGHT;
				Logic::_scriptVars[CHANGE_PLACE] = FLOOR_49;

				_logic->fnAddObject(0,0,TOILET_BRUSH,0,0,0,0,0);
			}
			break;
			//---------------------------
			case 50:	// syria_toilet
			{
				Logic::_scriptVars[CHANGE_X] = 313;
				Logic::_scriptVars[CHANGE_Y] = 440;
				Logic::_scriptVars[CHANGE_DIR] = DOWN_RIGHT;
				Logic::_scriptVars[CHANGE_PLACE] = FLOOR_50;

				_logic->fnAddObject(0,0,TOILET_KEY,0,0,0,0,0);
			}
			break;
			//---------------------------
			case 53:	// bull_head pan
			{
				Logic::_scriptVars[CHANGE_PLACE] = FLOOR_53;

				_logic->fnAddObject(0,0,TOWEL_CUT,0,0,0,0,0);
			}
			break;
			//---------------------------
			case 54:	// bull_head
			{
				Logic::_scriptVars[CHANGE_X] = 680;
				Logic::_scriptVars[CHANGE_Y] = 425;
				Logic::_scriptVars[CHANGE_DIR] = DOWN_LEFT;
				Logic::_scriptVars[CHANGE_PLACE] = FLOOR_54;

				_logic->fnAddObject(0,0,TOWEL_CUT,0,0,0,0,0);
			}
			break;
			//---------------------------
			case 55:	// bull_secret
			{
				Logic::_scriptVars[CHANGE_X] = 825;
				Logic::_scriptVars[CHANGE_Y] = 373;
				Logic::_scriptVars[CHANGE_DIR] = DOWN_LEFT;
				Logic::_scriptVars[CHANGE_PLACE] = FLOOR_55;
			}
			break;
			//---------------------------
			default:
			{
				error("Can't start in location %d", startNumber);
			}
			break;
			//---------------------------
		}
	}
	//-------------------------------------------------------------------------------------------------------
	// SPAIN 1 or 2

	else if ( ((startNumber >= 56) && (startNumber <= 62)) || 
		  ((startNumber >=956) && (startNumber <= 962)) )
	{
		Logic::_scriptVars[PARIS_FLAG] = 1;

		if (startNumber > 900)	// SPAIN 2
		{
			startNumber -= 900;	// set to correct start location
			Logic::_scriptVars[SPAIN_VISIT] = 2;	// set my own script flag to indicate 2nd visit
		}
		else
			Logic::_scriptVars[SPAIN_VISIT] = 1;	// set my own script flag to indicate 1st visit

		// already carrying these objects by the time we reach Spain...
		_logic->fnAddObject(0,0,RED_NOSE,0,0,0,0,0);
		_logic->fnAddObject(0,0,PHOTOGRAPH,0,0,0,0,0);
		_logic->fnAddObject(0,0,LAB_PASS,0,0,0,0,0);
		_logic->fnAddObject(0,0,LIFTING_KEYS,0,0,0,0,0);
		_logic->fnAddObject(0,0,BUZZER,0,0,0,0,0);
		_logic->fnAddObject(0,0,TISSUE,0,0,0,0,0);
		_logic->fnAddObject(0,0,BALL,0,0,0,0,0);
		_logic->fnAddObject(0,0,MATCHBOOK,0,0,0,0,0);

		if (Logic::_scriptVars[SPAIN_VISIT] == 1)	// 1st visit
			_logic->fnAddObject(0,0,PRESSURE_GAUGE,0,0,0,0,0);
		else						// 2nd visit
			_logic->fnAddObject(0,0,POLISHED_CHALICE,0,0,0,0,0);

		switch (startNumber)
		{
			//---------------------------
			case 56:	// Countess' room
			{
				Logic::_scriptVars[CHANGE_X] = 572;
				Logic::_scriptVars[CHANGE_Y] = 443;
				Logic::_scriptVars[CHANGE_DIR] = LEFT;
				Logic::_scriptVars[CHANGE_PLACE] = FLOOR_56;
			}
			break;
			//---------------------------
			case 57:	// Spain Drive
			{
				Logic::_scriptVars[CHANGE_X] = 1630;
				Logic::_scriptVars[CHANGE_Y] = 460;
				Logic::_scriptVars[CHANGE_DIR] = DOWN_LEFT;
				Logic::_scriptVars[CHANGE_PLACE] = FLOOR_57;
			}
			break;
			//---------------------------
			case 58:	// Mausoleum Exterior
			{
				Logic::_scriptVars[CHANGE_X] = SC58_PATH_X;
				Logic::_scriptVars[CHANGE_Y] = SC58_PATH_Y;
				Logic::_scriptVars[CHANGE_DIR] = UP_RIGHT;
				Logic::_scriptVars[CHANGE_PLACE] = FLOOR_58;
			}
			break;
			//---------------------------
			case 59:	// Mausoleum Interior
			{
				Logic::_scriptVars[CHANGE_X] = 750;
				Logic::_scriptVars[CHANGE_Y] = 455;
				Logic::_scriptVars[CHANGE_DIR] = LEFT;
				Logic::_scriptVars[CHANGE_PLACE] = FLOOR_59;
			}
			break;
			//---------------------------
			case 60:	// Spain Reception
			{
				Logic::_scriptVars[CHANGE_X] = 750;
				Logic::_scriptVars[CHANGE_Y] = 475;
				Logic::_scriptVars[CHANGE_DIR] = DOWN_LEFT;
				Logic::_scriptVars[CHANGE_PLACE] = FLOOR_60;
			}
			break;
			//---------------------------
			case 61:	// Spain Well
			{
				Logic::_scriptVars[CHANGE_X] = 400;
				Logic::_scriptVars[CHANGE_Y] = 345;
				Logic::_scriptVars[CHANGE_DIR] = DOWN;
				Logic::_scriptVars[CHANGE_PLACE] = LEFT_FLOOR_61;

				_logic->fnAddObject(0,0,STONE_KEY,0,0,0,0,0);
				_logic->fnAddObject(0,0,MIRROR,0,0,0,0,0);
			}
			break;
			//---------------------------
			case 62:	// CHESS PUZZLE
			{
				Logic::_scriptVars[CHANGE_PLACE] = FLOOR_62;  // so it doesn't crash!!
				
				_logic->_scriptVars[TOP_MENU_DISABLED] = 1; // because it's irrelevant to the chess puzzle
				_logic->_scriptVars[GEORGE_ALLOWED_REST_ANIMS] = 0; // because it's irrelevant to the chess puzzle

				compact = (Object*)_objectMan->fetchObject(PLAYER);
				_logic->fnNoSprite(compact, PLAYER, 0, 0, 0, 0, 0, 0);
			}
			break;
			//---------------------------
		}
	}
	//-------------------------------------------------------------------------------------------------------
	// NIGHT TRAIN

	else if ((startNumber >= 63) && (startNumber <= 70))
	{
		Logic::_scriptVars[PARIS_FLAG] = 18;

		switch (startNumber)
		{
			//---------------------------
			case 63:	// train_one
			{
				Logic::_scriptVars[CHANGE_X] = 710;
				Logic::_scriptVars[CHANGE_Y] = 450;
				Logic::_scriptVars[CHANGE_DIR] = LEFT;
				Logic::_scriptVars[CHANGE_PLACE] = FLOOR_63;
				Logic::_scriptVars[DOOR_SC65_FLAG] = 2;
				Logic::_scriptVars[DOOR_ONE_63_OPEN] = 0;
				Logic::_scriptVars[DOOR_65_OPEN] = 1;
				Logic::_scriptVars[VAIL_TEXT] = 1;
			}
			break;
			//---------------------------
			case 65:	// compt_one
			{
				Logic::_scriptVars[CHANGE_X] = 460;
				Logic::_scriptVars[CHANGE_Y] = 430;
				Logic::_scriptVars[CHANGE_DIR] = DOWN;
				Logic::_scriptVars[CHANGE_PLACE] = FLOOR_65;
			}
			//---------------------------
			case 66:	// compt_two
			{
				Logic::_scriptVars[CHANGE_X] = 460;
				Logic::_scriptVars[CHANGE_Y] = 430;
				Logic::_scriptVars[CHANGE_DIR] = DOWN;
				Logic::_scriptVars[CHANGE_PLACE] = FLOOR_66;
			
				Logic::_scriptVars[WINDOW_66_OPEN] = 1;
			}
			break;
			//---------------------------
			case 67:	// compt_three
			{
				Logic::_scriptVars[CHANGE_X] = 460;
				Logic::_scriptVars[CHANGE_Y] = 430;
				Logic::_scriptVars[CHANGE_DIR] = DOWN;
				Logic::_scriptVars[CHANGE_PLACE] = FLOOR_67;
			}
			break;
			//---------------------------
			case 69:	// train_guard
			{
				Logic::_scriptVars[CHANGE_X] = 310;
				Logic::_scriptVars[CHANGE_Y] = 430;
				Logic::_scriptVars[CHANGE_DIR] = DOWN;
				Logic::_scriptVars[CHANGE_PLACE] = FLOOR_69;
			}
			break;
			//---------------------------
			default:
			{
				error("Can't start in location %d", startNumber);
			}
			break;
			//---------------------------
		}
	}
	//-------------------------------------------------------------------------------------------------------
	// SCOTLAND

	else if ((startNumber >= 71) && (startNumber <= 73))
	{
		Logic::_scriptVars[PARIS_FLAG] = 1;

		// already carrying these objects by the time we reach Scotland...
		_logic->fnAddObject(0,0,RED_NOSE,0,0,0,0,0);
		_logic->fnAddObject(0,0,PHOTOGRAPH,0,0,0,0,0);
		_logic->fnAddObject(0,0,LAB_PASS,0,0,0,0,0);
		_logic->fnAddObject(0,0,LIFTING_KEYS,0,0,0,0,0);
		_logic->fnAddObject(0,0,BUZZER,0,0,0,0,0);

		switch (startNumber)
		{
			//---------------------------
			case 71:	// churchyard
			{
				Logic::_scriptVars[CHANGE_X] = 1538;
				Logic::_scriptVars[CHANGE_Y] = 444;
				Logic::_scriptVars[CHANGE_DIR] = LEFT;
				Logic::_scriptVars[CHANGE_PLACE] = RIGHT_FLOOR_71;

				Logic::_scriptVars[NICO_SCOT_SCREEN] = 71;
				Logic::_scriptVars[NICO_POSITION_71] = 1;
			}
			break;
			//---------------------------
			case 72:	// church_tower
			{
				Logic::_scriptVars[CHANGE_X] = 150;
				Logic::_scriptVars[CHANGE_Y] = 503;
				Logic::_scriptVars[CHANGE_DIR] = RIGHT;
				Logic::_scriptVars[CHANGE_PLACE] = FLOOR_72;

				Logic::_scriptVars[NICO_SCOT_SCREEN] = 72;
			}
			break;
			//---------------------------
			case 73:	// crypt
			{
				Logic::_scriptVars[CHANGE_X] = 250;
				Logic::_scriptVars[CHANGE_Y] = 390;
				Logic::_scriptVars[CHANGE_DIR] = DOWN_RIGHT;
				Logic::_scriptVars[CHANGE_PLACE] = FLOOR_73;

				Logic::_scriptVars[NICO_SCOT_SCREEN] = 73;
				Logic::_scriptVars[NICO_POSITION_73] = 1;
			}
			break;
			//---------------------------	
		}
	}
	else
	{
		error("Can't start in location %d", startNumber);
	}

	compact = (Object*)_objectMan->fetchObject(PLAYER);
	_logic->fnEnterSection(compact, PLAYER, startNumber, 0, 0, 0, 0, 0);	// (automatically opens the compact resource for that section)
	_systemVars.deathScreenFlag = 0;
	_systemVars.wantFade = true;
}

void SwordEngine::checkCdFiles(void) { // check if we're running from cd, hdd or what...
	File test;

	_systemVars.playSpeech = true;
	if (test.open("SPEECH1.CLU")) {
		test.close();
		if (test.open("SPEECH2.CLU")) {
			// both files exist, assume running from HDD and everything's fine.
			test.close();
			_systemVars.runningFromCd = false;
			_systemVars.playSpeech = true;
			return ;
		} else
			error("SPEECH2.CLU not found.\nPlease copy the SPEECH.CLU from CD2 and rename it to SPEECH2.CLU");
	} else { // speech1.clu & speech2.clu not present. are we running from cd?
		if (test.open("cd1.id")) {
			_systemVars.runningFromCd = true;
			_systemVars.currentCD = 1;
			test.close();
		} else if (test.open("cd2.id")) {
			_systemVars.runningFromCd = true;
			_systemVars.currentCD = 2;
			test.close();
		} else
			error("Unable to find files.\nPlease read the instructions again");
	}
}

void SwordEngine::go(void) {
	
	initialize();
	checkCdFiles();

	uint8 startPos = ConfMan.getInt("boot_param");
	if (startPos)
		startPositions(startPos);
	else {
		if (_control->savegamesExist()) {
			_systemVars.deathScreenFlag = 3;
			if (_control->runPanel() == CONTROL_GAME_RESTORED)
				_control->doRestore();
			else
				startPositions(0);
		} else // no savegames, start new game.
			startPositions(0);
	}
	_systemVars.deathScreenFlag = 0;

	do {
		uint8 action = mainLoop();

		// the mainloop was left, we have to reinitialize.
		reinitialize();
		if (action == CONTROL_GAME_RESTORED)
			_control->doRestore();
		else if (action == CONTROL_RESTART_GAME)
			startPositions(1);
		_systemVars.forceRestart = false;
		_systemVars.deathScreenFlag = 0;
	} while (true);
}

void SwordEngine::checkCd(void) {
	uint8 needCd = _cdList[Logic::_scriptVars[NEW_SCREEN]];
	if (_systemVars.runningFromCd) { // are we running from cd?
		if (needCd == 0) { // needCd == 0 means we can use either CD1 or CD2.
			if (_systemVars.currentCD == 0) {
				_systemVars.currentCD = 1; // if there is no CD currently inserted, ask for CD1.
				_control->askForCd();
			} // else: there is already a cd inserted and we don't care if it's cd1 or cd2.
		} else if (needCd != _systemVars.currentCD) { // we need a different CD than the one in drive.
			_music->startMusic(0, 0); // 
			_sound->closeCowSystem(); // close music and sound files before changing CDs
			_systemVars.currentCD = needCd; // askForCd will ask the player to insert _systemVars.currentCd,
			_control->askForCd();		    // so it has to be updated before calling it.
		}
	} else {		// we're running from HDD, we don't have to care about music files and Sound will take care of
		if (needCd) // switching sound.clu files on Sound::newScreen by itself, so there's nothing to be done.
			_systemVars.currentCD = needCd;
		else if (_systemVars.currentCD == 0)
			_systemVars.currentCD = 1;
	}
}

uint8 SwordEngine::mainLoop(void) {
	uint8 retCode = 0;
	_keyPressed = 0;

	while (retCode == 0) {
		// do we need the section45-hack from sword.c here?
		checkCd();

		_screen->newScreen(Logic::_scriptVars[NEW_SCREEN]);
		_logic->newScreen(Logic::_scriptVars[NEW_SCREEN]);
		_sound->newScreen(Logic::_scriptVars[NEW_SCREEN]);
		Logic::_scriptVars[SCREEN] = Logic::_scriptVars[NEW_SCREEN];
		
		do {
			uint32 newTime;
			bool scrollFrameShown = false;

			uint32 frameTime = _system->get_msecs();
			_logic->engine();
			_logic->updateScreenParams(); // sets scrolling

			_screen->draw();
			_mouse->animate();

			_sound->engine();
			_menu->refresh(MENU_TOP);
			_menu->refresh(MENU_BOT);

			newTime = _system->get_msecs();
			if (newTime - frameTime < 1000 / FRAME_RATE) {
				scrollFrameShown = _screen->showScrollFrame();
				int32 restDelay = (1000 / (FRAME_RATE * 2)) - (_system->get_msecs() - frameTime);
				if (restDelay > 0)
					delay((uint)restDelay);
			}

			newTime = _system->get_msecs();
			if ((newTime - frameTime < 1000 / FRAME_RATE) || (!scrollFrameShown))
				_screen->updateScreen();

			int32 frameDelay = (1000 / FRAME_RATE) - (_system->get_msecs() - frameTime);
			if (frameDelay > 0)
				delay((uint)frameDelay);
			else
				delay(0);

			_mouse->engine( _mouseX, _mouseY, _mouseState);
			_mouseState = 0;

			if (_systemVars.forceRestart)
				retCode = CONTROL_RESTART_GAME;
			else if (((_keyPressed == 63) && (Logic::_scriptVars[MOUSE_STATUS] & 1)) || (_systemVars.deathScreenFlag)) {
				retCode = _control->runPanel();
				if (!retCode)
					_screen->fullRefresh();
			}
			_keyPressed = 0;

			// do something smart here to implement pausing the game. If we even want that, that is.
		} while ((Logic::_scriptVars[SCREEN] == Logic::_scriptVars[NEW_SCREEN]) && (retCode == 0));

		if ((retCode == 0) && (Logic::_scriptVars[SCREEN] != 53) && _systemVars.wantFade) {
			_screen->fadeDownPalette();
			while (_screen->stillFading()) {
				_screen->updateScreen();
				delay(1000/12);
			}
		}

		_sound->quitScreen();
		_screen->quitScreen(); // close graphic resources
		_objectMan->closeSection(Logic::_scriptVars[SCREEN]); // close the section that PLAYER has just left, if it's empty now
	}
	return retCode;
}

void SwordEngine::delay(uint amount) { //copied and mutilated from sky.cpp

	OSystem::Event event;

	uint32 start = _system->get_msecs();
	uint32 cur = start;

	do {
		while (_system->poll_event(&event)) {
			switch (event.event_code) {
			case OSystem::EVENT_KEYDOWN:

				// Make sure backspace works right (this fixes a small issue on OS X)
				if (event.kbd.keycode == 8)
					_keyPressed = 8;
				else
					_keyPressed = (uint8)event.kbd.ascii;
				break;
			case OSystem::EVENT_MOUSEMOVE:
				_mouseX = event.mouse.x;
				_mouseY = event.mouse.y;
				break;
			case OSystem::EVENT_LBUTTONDOWN:
				_mouseState |= BS1L_BUTTON_DOWN;
#ifdef _WIN32_WCE
				_mouseX = event.mouse.x;
				_mouseY = event.mouse.y;
#endif
				break;
			case OSystem::EVENT_RBUTTONDOWN:
				_mouseState |= BS1R_BUTTON_DOWN;
#ifdef _WIN32_WCE
				_mouseX = event.mouse.x;
				_mouseY = event.mouse.y;
#endif
				break;
			case OSystem::EVENT_LBUTTONUP:
				_mouseState |= BS1L_BUTTON_UP;
				break;
			case OSystem::EVENT_RBUTTONUP:
				_mouseState |= BS1R_BUTTON_UP;
				break;
			case OSystem::EVENT_QUIT:
				_system->quit();
				break;
			default:
				break;
			}
		}

		if (amount == 0)
			break;

		{
			uint this_delay = 20; // 1?
#ifdef _WIN32_WCE
			this_delay = 10;
#endif
			if (this_delay > amount)
				this_delay = amount;
			_system->delay_msecs(this_delay);
		}
		cur = _system->get_msecs();
	} while (cur < start + amount);
}

} // End of namespace Sword1
