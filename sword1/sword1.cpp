/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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

/* Broken Sword 1 */
static const GameSettings sword1_setting =
	{"sword1", "Broken Sword I", GF_DEFAULT_TO_1X_SCALER};

GameList Engine_SWORD1_gameList() {
	GameList games;
	games.push_back(sword1_setting);
	return games;
}

GameList Engine_SWORD1_detectGames(const FSList &fslist) {
	GameList detectedGames;

	// Iterate over all files in the given directory
	for (FSList::ConstIterator file = fslist.begin(); file != fslist.end(); ++file) {
		const char *gameName = file->displayName().c_str();

		if (0 == scumm_stricmp("swordres.rif", gameName)) {
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

SystemVars SwordEngine::_systemVars;

void SwordEngine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

extern uint16 _debugLevel;

SwordEngine::SwordEngine(GameDetector *detector, OSystem *syst)
	: Engine(syst) {

	_detector = detector;
	_system = syst;
	_debugLevel = ConfMan.getInt("debuglevel");
	
	if (!_mixer->bindToSystem(syst))
		warning("Sound initialization failed");
}

SwordEngine::~SwordEngine() {
}

void SwordEngine::initialize(void) {
	_system->init_size(640, 480);
	debug(5, "Starting memory manager");
	_memMan = new MemMan();
	debug(5, "Starting resource manager");
	_resMan = new ResMan("swordres.rif", _memMan);
	debug(5, "Starting object manager");
	_objectMan = new ObjectMan(_resMan);
	_mixer->setVolume(255);
	_mouse = new SwordMouse(_system, _resMan, _objectMan);
	_screen = new SwordScreen(_system, _resMan, _objectMan);
	_music = new SwordMusic(_system, _mixer);
	_sound = new SwordSound("", _mixer, _resMan);
	_menu = new SwordMenu(_screen, _mouse);
	_logic = new SwordLogic(_objectMan, _resMan, _screen, _mouse, _sound, _music, _menu);
	_mouse->useLogicAndMenu(_logic, _menu);

	_systemVars.justRestoredGame = _systemVars.currentCD = 
		_systemVars.gamePaused = _systemVars.saveGameFlag = 
		_systemVars.deathScreenFlag = _systemVars.currentMusic = 0;
	_systemVars.snrStatus = 0;
	_systemVars.rate = 8;

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
	// TODO add czech option
	default:
		_systemVars.language = BS1_ENGLISH;
	}

	_systemVars.showText = ConfMan.getBool("subtitles");
	
	_systemVars.playSpeech = 1;
	startPositions(ConfMan.getInt("boot_param"));
	_mouseState = 0;
}

void SwordEngine::startPositions(int32 startNumber) {
	// int32 sect;
	BsObject *compact;

	SwordLogic::_scriptVars[CHANGE_STANCE] = STAND;
	SwordLogic::_scriptVars[GEORGE_CDT_FLAG] = GEO_TLK_TABLE;

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
	
	
		SwordLogic::_scriptVars[CHANGE_X] = 481;
		SwordLogic::_scriptVars[CHANGE_Y] = 413;
		SwordLogic::_scriptVars[CHANGE_DIR] = DOWN;
		SwordLogic::_scriptVars[CHANGE_PLACE] = FLOOR_1;
	} 
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==2)	// blind_alley
	{
		SwordLogic::_scriptVars[CHANGE_X] = 480;
		SwordLogic::_scriptVars[CHANGE_Y] = 388;
		SwordLogic::_scriptVars[CHANGE_DIR] = DOWN_LEFT;
		SwordLogic::_scriptVars[CHANGE_PLACE] = FLOOR_2;

		_logic->fnAddObject(0,0,LIFTING_KEYS,0,0,0,0,0);
		_logic->fnAddObject(0,0,ROSSO_CARD,0,0,0,0,0);

		SwordLogic::_scriptVars[POCKET_1] = 1;
		SwordLogic::_scriptVars[POCKET_2] = 1;
		SwordLogic::_scriptVars[POCKET_3] = 1;
		SwordLogic::_scriptVars[POCKET_4] = 1;
		SwordLogic::_scriptVars[POCKET_5] = 1;
		SwordLogic::_scriptVars[POCKET_6] = 1;
		SwordLogic::_scriptVars[POCKET_7] = 1;
		SwordLogic::_scriptVars[POCKET_8] = 1;
		SwordLogic::_scriptVars[POCKET_9] = 1;

		SwordLogic::_scriptVars[POCKET_10] = 1;
		SwordLogic::_scriptVars[POCKET_11] = 1;
		SwordLogic::_scriptVars[POCKET_12] = 1;
		SwordLogic::_scriptVars[POCKET_13] = 1;
		SwordLogic::_scriptVars[POCKET_14] = 1;
		SwordLogic::_scriptVars[POCKET_15] = 1;
		SwordLogic::_scriptVars[POCKET_16] = 1;
		SwordLogic::_scriptVars[POCKET_17] = 1;
		SwordLogic::_scriptVars[POCKET_18] = 1;
		SwordLogic::_scriptVars[POCKET_19] = 1;

		SwordLogic::_scriptVars[POCKET_20] = 1;
		SwordLogic::_scriptVars[POCKET_21] = 1;
		SwordLogic::_scriptVars[POCKET_22] = 1;
		SwordLogic::_scriptVars[POCKET_23] = 1;
		SwordLogic::_scriptVars[POCKET_24] = 1;
		SwordLogic::_scriptVars[POCKET_25] = 1;
		SwordLogic::_scriptVars[POCKET_26] = 1;
		SwordLogic::_scriptVars[POCKET_27] = 1;
		SwordLogic::_scriptVars[POCKET_28] = 1;
		SwordLogic::_scriptVars[POCKET_29] = 1;
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==3)	// cafe
	{
		SwordLogic::_scriptVars[CHANGE_X] = 660;
		SwordLogic::_scriptVars[CHANGE_Y] = 368;
		SwordLogic::_scriptVars[CHANGE_DIR] = DOWN_LEFT;
		SwordLogic::_scriptVars[CHANGE_PLACE] = FLOOR_3;
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==4)	// ready to use phone
	{
		SwordLogic::_scriptVars[CHANGE_X] = 463;
		SwordLogic::_scriptVars[CHANGE_Y] = 391;
		SwordLogic::_scriptVars[CHANGE_DIR] = DOWN;
		SwordLogic::_scriptVars[CHANGE_PLACE] = FLOOR_4;

		SwordLogic::_scriptVars[MOUE_TEXT] = 1;		// stop moue from entering ("Freeze...")
		SwordLogic::_scriptVars[MOUE_NICO_FLAG] = 1;			// Moue gave George her number
//		paris_flag=2;					// for Nico's phone script
		SwordLogic::_scriptVars[PARIS_FLAG] = 5;	// to access room8 (cafe_repaired)
		SwordLogic::_scriptVars[NICO_PHONE_FLAG] = 1;	// Nico's number is on envelope
		SwordLogic::_scriptVars[TAILOR_PHONE_FLAG] = 1;	// Todryk's number is on envelope
		SwordLogic::_scriptVars[WORKMAN_GONE_FLAG] = 1;	// Workman not here
		SwordLogic::_scriptVars[ALBERT_INFO_FLAG] = 1;	// Albert has told us the tailor's number (for Nico's phone script)
		SwordLogic::_scriptVars[SEEN_SEWERS_FLAG] = 1;

		SwordLogic::_scriptVars[POCKET_30] = 1;
		SwordLogic::_scriptVars[POCKET_31] = 1;
		SwordLogic::_scriptVars[POCKET_32] = 1;
		SwordLogic::_scriptVars[POCKET_33] = 1;
		SwordLogic::_scriptVars[POCKET_34] = 1;
		SwordLogic::_scriptVars[POCKET_35] = 1;
		SwordLogic::_scriptVars[POCKET_36] = 1;
		SwordLogic::_scriptVars[POCKET_37] = 1;
		SwordLogic::_scriptVars[POCKET_38] = 1;
		SwordLogic::_scriptVars[POCKET_39] = 1;

		SwordLogic::_scriptVars[POCKET_40] = 1;
		SwordLogic::_scriptVars[POCKET_41] = 1;
		SwordLogic::_scriptVars[POCKET_42] = 1;
		SwordLogic::_scriptVars[POCKET_43] = 1;
		SwordLogic::_scriptVars[POCKET_44] = 1;
		SwordLogic::_scriptVars[POCKET_45] = 1;
		SwordLogic::_scriptVars[POCKET_46] = 1;
		SwordLogic::_scriptVars[POCKET_47] = 1;
		SwordLogic::_scriptVars[POCKET_48] = 1;
		SwordLogic::_scriptVars[POCKET_49] = 1;

		SwordLogic::_scriptVars[POCKET_50] = 1;
		SwordLogic::_scriptVars[POCKET_51] = 1;
		SwordLogic::_scriptVars[POCKET_52] = 1;
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==5)	// court_yard
	{
		SwordLogic::_scriptVars[CHANGE_X] = 400;
		SwordLogic::_scriptVars[CHANGE_Y] = 400;
		SwordLogic::_scriptVars[CHANGE_DIR] = DOWN_LEFT;
		SwordLogic::_scriptVars[CHANGE_PLACE] = FLOOR_5;
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==7)	// sewer_two
	{
		SwordLogic::_scriptVars[CHANGE_X] = 520;
		SwordLogic::_scriptVars[CHANGE_Y] = 310;
		SwordLogic::_scriptVars[CHANGE_DIR] = DOWN_LEFT;
		SwordLogic::_scriptVars[CHANGE_PLACE] = FLOOR_7;
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==8)	// cafe_repaired
	{
		SwordLogic::_scriptVars[CHANGE_X] = 481;
		SwordLogic::_scriptVars[CHANGE_Y] = 413;
		SwordLogic::_scriptVars[CHANGE_DIR] = DOWN;
		SwordLogic::_scriptVars[CHANGE_PLACE] = FLOOR_8;
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==11)	// costumier
	{
		SwordLogic::_scriptVars[CHANGE_X] = 264;
		SwordLogic::_scriptVars[CHANGE_Y] = 436;
		SwordLogic::_scriptVars[CHANGE_DIR] = DOWN_RIGHT;
		SwordLogic::_scriptVars[CHANGE_PLACE] = FLOOR_11;

		_logic->fnAddObject(0,0,TISSUE,0,0,0,0,0);
		_logic->fnAddObject(0,0,PHOTOGRAPH,0,0,0,0,0);
	}
	//-------------------------------------------------------------------------------------------------------
	// HOTEL SECTION
	else if (startNumber==12)	// hotel_street
	{
		SwordLogic::_scriptVars[CHANGE_X] = 730;
		SwordLogic::_scriptVars[CHANGE_Y] = 460;
		SwordLogic::_scriptVars[CHANGE_DIR] = LEFT;
		SwordLogic::_scriptVars[CHANGE_PLACE] = FLOOR_12;

		SwordLogic::_scriptVars[PARIS_FLAG] = 6;

		_logic->fnAddObject(0,0,PHOTOGRAPH,0,0,0,0,0);

		SwordLogic::_scriptVars[NICO_ADDRESS_FLAG] = 1;
		SwordLogic::_scriptVars[NICO_PHONE_FLAG] = 1;
		SwordLogic::_scriptVars[COSTUMES_ADDRESS_FLAG] = 1;
		SwordLogic::_scriptVars[HOTEL_ADDRESS_FLAG] = 1;
		SwordLogic::_scriptVars[AEROPORT_ADDRESS_FLAG] = 1;
		SwordLogic::_scriptVars[TAILOR_PHONE_FLAG] = 1;
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==14)	// hotel_corridor
	{
		SwordLogic::_scriptVars[CHANGE_X] = 528;
		SwordLogic::_scriptVars[CHANGE_Y] = 484;
		SwordLogic::_scriptVars[CHANGE_DIR] = UP;
		SwordLogic::_scriptVars[CHANGE_PLACE] = FLOOR_14;

		_logic->fnAddObject(0,0,HOTEL_KEY,0,0,0,0,0);
		_logic->fnAddObject(0,0,MANUSCRIPT,0,0,0,0,0);
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==17)	// hotel_assassin
	{
		SwordLogic::_scriptVars[CHANGE_X] = 714;
		SwordLogic::_scriptVars[CHANGE_Y] = 484;
		SwordLogic::_scriptVars[CHANGE_DIR] = LEFT;
		SwordLogic::_scriptVars[CHANGE_PLACE] = FLOOR_17;
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==18)	// gendarmerie
	{
		SwordLogic::_scriptVars[CHANGE_X] = 446;
		SwordLogic::_scriptVars[CHANGE_Y] = 408;
		SwordLogic::_scriptVars[CHANGE_DIR] = DOWN_LEFT;
		SwordLogic::_scriptVars[CHANGE_PLACE] = FLOOR_18;

		SwordLogic::_scriptVars[PARIS_FLAG] = 5;	// for Moue & Rosso
	}
	//-------------------------------------------------------------------------------------------------------
	// MUSEUM RAID
	else if (startNumber==27)	// museum_street
	{
		SwordLogic::_scriptVars[CHANGE_X] = 300;
		SwordLogic::_scriptVars[CHANGE_Y] = 510;
		SwordLogic::_scriptVars[CHANGE_DIR] = UP_RIGHT;
		SwordLogic::_scriptVars[CHANGE_PLACE] = FLOOR_27;

		SwordLogic::_scriptVars[PARIS_FLAG] = 12;	// for Lobineau in Museum
		SwordLogic::_scriptVars[MANUSCRIPT_ON_TABLE_10_FLAG] = 1;
	}
	//-------------------------------------------------------------------------------------------------------
	// HOSPITAL SECTION
	else if (startNumber==31)	// hospital_street
	{
		SwordLogic::_scriptVars[CHANGE_X] = 400;
		SwordLogic::_scriptVars[CHANGE_Y] = 500;
		SwordLogic::_scriptVars[CHANGE_DIR] = UP_RIGHT;
		SwordLogic::_scriptVars[CHANGE_PLACE] = FLOOR_31;

		SwordLogic::_scriptVars[PARIS_FLAG] = 11;

		_logic->fnAddObject(0,0,PHOTOGRAPH,0,0,0,0,0);
		_logic->fnAddObject(0,0,LAB_PASS,0,0,0,0,0);
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==32)	// hospital_desk (after we've found out where Marquet is)
	{
		SwordLogic::_scriptVars[CHANGE_X] = 405;
		SwordLogic::_scriptVars[CHANGE_Y] = 446;
		SwordLogic::_scriptVars[CHANGE_DIR] = UP_RIGHT;
		SwordLogic::_scriptVars[CHANGE_PLACE] = FLOOR_32;

		SwordLogic::_scriptVars[PARIS_FLAG] = 11;

		SwordLogic::_scriptVars[EVA_TEXT] = 1;		// talked to eva
		SwordLogic::_scriptVars[EVA_MARQUET_FLAG] = 2;
		SwordLogic::_scriptVars[EVA_NURSE_FLAG] = 4;
		SwordLogic::_scriptVars[FOUND_WARD_FLAG] = 1;
		SwordLogic::_scriptVars[CONSULTANT_HERE] = 1;

		compact = (BsObject*)_objectMan->fetchObject(PLAYER);
		_logic->fnMegaSet(compact,PLAYER,GEORGE_WLK,MEGA_WHITE,0,0,0,0);

		SwordLogic::_scriptVars[GEORGE_CDT_FLAG] = WHT_TLK_TABLE;
		SwordLogic::_scriptVars[GEORGE_TALK_FLAG] = 0;
		SwordLogic::_scriptVars[WHITE_COAT_FLAG] = 1;
		SwordLogic::_scriptVars[GEORGE_ALLOWED_REST_ANIMS] = 0;		// because wearing white coat now
		
		SwordLogic::_scriptVars[GOT_BENOIR_FLAG] = 1;

		_logic->fnAddObject(0,0,PHOTOGRAPH,0,0,0,0,0);
		_logic->fnAddObject(0,0,LAB_PASS,0,0,0,0,0);
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==35)	// hospital_jacques
	{
		SwordLogic::_scriptVars[CHANGE_X] = 640;
		SwordLogic::_scriptVars[CHANGE_Y] = 500;
		SwordLogic::_scriptVars[CHANGE_DIR] = LEFT;
		SwordLogic::_scriptVars[CHANGE_PLACE] = FLOOR_35;

		SwordLogic::_scriptVars[PARIS_FLAG] = 11;

		SwordLogic::_scriptVars[EVA_TEXT] = 1;		// talked to eva
		SwordLogic::_scriptVars[EVA_MARQUET_FLAG] = 2;
		SwordLogic::_scriptVars[EVA_NURSE_FLAG] = 4;
		SwordLogic::_scriptVars[FOUND_WARD_FLAG] = 1;
		SwordLogic::_scriptVars[CONSULTANT_HERE] = 1;

		compact = (BsObject*)_objectMan->fetchObject(PLAYER);
		_logic->fnMegaSet(compact,PLAYER,GEORGE_WLK,MEGA_WHITE,0,0,0,0);

		SwordLogic::_scriptVars[GEORGE_CDT_FLAG] = WHT_TLK_TABLE;
		SwordLogic::_scriptVars[GEORGE_TALK_FLAG] = 0;
		SwordLogic::_scriptVars[WHITE_COAT_FLAG] = 1;
		SwordLogic::_scriptVars[GEORGE_ALLOWED_REST_ANIMS] = 0;		// because wearing white coat now
		SwordLogic::_scriptVars[DOOR_34_OPEN] = 1;
		
		SwordLogic::_scriptVars[GOT_BENOIR_FLAG] = 2;
		SwordLogic::_scriptVars[HOS_POS_FLAG] = 26;
		SwordLogic::_scriptVars[BENOIR_FLAG] = 24;	// for 'george_enters_ward' script
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==36)	// montfaucon
	{
		SwordLogic::_scriptVars[CHANGE_X] = 300;
		SwordLogic::_scriptVars[CHANGE_Y] = 480;
		SwordLogic::_scriptVars[CHANGE_DIR] = RIGHT;
		SwordLogic::_scriptVars[CHANGE_PLACE] = FLOOR_36;

		_logic->fnAddObject(0,0,LENS,0,0,0,0,0);
		_logic->fnAddObject(0,0,RED_NOSE,0,0,0,0,0);
		_logic->fnAddObject(0,0,LIFTING_KEYS,0,0,0,0,0);
		
		SwordLogic::_scriptVars[MONTFAUCON_CONTROL_FLAG] = 1;
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==37)	// catacomb_sewer
	{
		SwordLogic::_scriptVars[CHANGE_X] = 592;
		SwordLogic::_scriptVars[CHANGE_Y] = 386;
		SwordLogic::_scriptVars[CHANGE_DIR] = RIGHT;
		SwordLogic::_scriptVars[CHANGE_PLACE] = FLOOR_37;

		_logic->fnAddObject(0,0,LIFTING_KEYS,0,0,0,0,0);
		_logic->fnAddObject(0,0,TRIPOD,0,0,0,0,0);
		_logic->fnAddObject(0,0,GEM,0,0,0,0,0);
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==38)	// catacomb_room
	{
		SwordLogic::_scriptVars[CHANGE_X] = 200;
		SwordLogic::_scriptVars[CHANGE_Y] = 390;
		SwordLogic::_scriptVars[CHANGE_DIR] = RIGHT;
		SwordLogic::_scriptVars[CHANGE_PLACE] = FLOOR_38;

		_logic->fnAddObject(0,0,TRIPOD,0,0,0,0,0);
		_logic->fnAddObject(0,0,GEM,0,0,0,0,0);
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==39)	// catacomb_meeting
	{
		SwordLogic::_scriptVars[CHANGE_X] = 636;
		SwordLogic::_scriptVars[CHANGE_Y] = 413;
		SwordLogic::_scriptVars[CHANGE_DIR] = DOWN_LEFT;
		SwordLogic::_scriptVars[CHANGE_PLACE] = FLOOR_39;

		SwordLogic::_scriptVars[MEETING_FLAG] = 3;	// meeting finished

		_logic->fnAddObject(0,0,TRIPOD,0,0,0,0,0);
		_logic->fnAddObject(0,0,GEM,0,0,0,0,0);
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==40)	// excavation_exterior
	{
		SwordLogic::_scriptVars[CHANGE_X] = 648;
		SwordLogic::_scriptVars[CHANGE_Y] = 492;
		SwordLogic::_scriptVars[CHANGE_DIR] = LEFT;
		SwordLogic::_scriptVars[CHANGE_PLACE] = FLOOR_40;

		SwordLogic::_scriptVars[NICO_PHONE_FLAG] = 1;
		SwordLogic::_scriptVars[PARIS_FLAG] = 16;

		_logic->fnAddObject(0,0,PLASTER,0,0,0,0,0);
		_logic->fnAddObject(0,0,POLISHED_CHALICE,0,0,0,0,0);
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==48)	// templar_church
	{
		SwordLogic::_scriptVars[CHANGE_X] = 315;
		SwordLogic::_scriptVars[CHANGE_Y] = 392;
		SwordLogic::_scriptVars[CHANGE_DIR] = DOWN;
		SwordLogic::_scriptVars[CHANGE_PLACE] = FLOOR_48;

		_logic->fnAddObject(0,0,CHALICE,0,0,0,0,0);
		_logic->fnAddObject(0,0,LENS,0,0,0,0,0);

		SwordLogic::_scriptVars[CHALICE_FLAG] = 2;	// from end of Spain1, when George gets chalice
		SwordLogic::_scriptVars[NEJO_TEXT] = 1;		// so priest is there
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==99)	// test text+speech on blank screen (sc99)
	{
		SwordLogic::_scriptVars[CHANGE_PLACE] = FLOOR_99;

		// if (testingText)
		if (1)
		{
//			lastLineNo = testTextSection*65536 + LastTextNumber(testTextSection);
			SwordLogic::_scriptVars[LASTLINENO] = 146*65536 + _objectMan->lastTextNumber(146);	// don't finished until at last line in last section
		}
	}
	//-------------------------------------------------------------------------------------------------------
	else if (startNumber==80)	// Paris Map
	{
		SwordLogic::_scriptVars[CHANGE_X] = 645;
		SwordLogic::_scriptVars[CHANGE_Y] = 160;
		SwordLogic::_scriptVars[CHANGE_DIR] = DOWN;
		SwordLogic::_scriptVars[CHANGE_PLACE] = FLOOR_80;

		SwordLogic::_scriptVars[PARIS_FLAG] = 3;	// for Nico's appartment
		SwordLogic::_scriptVars[NICO_CLOWN_FLAG] = 3;

		SwordLogic::_scriptVars[NICO_DOOR_FLAG] = 2;	// for instant access to Nico's appartment

		_logic->fnAddObject(0,0,RED_NOSE,0,0,0,0,0);
		_logic->fnAddObject(0,0,PHOTOGRAPH,0,0,0,0,0);
		_logic->fnAddObject(0,0,PLASTER,0,0,0,0,0);
		_logic->fnAddObject(0,0,LAB_PASS,0,0,0,0,0);	// for hospital

		SwordLogic::_scriptVars[MANUSCRIPT_FLAG] = 1;

		SwordLogic::_scriptVars[NICO_ADDRESS_FLAG] = 1;
		SwordLogic::_scriptVars[NICO_PHONE_FLAG] = 1;
		SwordLogic::_scriptVars[COSTUMES_ADDRESS_FLAG] = 1;
		SwordLogic::_scriptVars[HOTEL_ADDRESS_FLAG] = 1;
		SwordLogic::_scriptVars[MUSEUM_ADDRESS_FLAG] = 1;
		SwordLogic::_scriptVars[HOSPITAL_ADDRESS_FLAG] = 1;
		SwordLogic::_scriptVars[MONTFACN_ADDRESS_FLAG] = 1;
		SwordLogic::_scriptVars[AEROPORT_ADDRESS_FLAG] = 1;
		SwordLogic::_scriptVars[NERVAL_ADDRESS_FLAG] = 1;

		SwordLogic::_scriptVars[IRELAND_MAP_FLAG] = 1;
		SwordLogic::_scriptVars[SPAIN_MAP_FLAG] = 1;
		SwordLogic::_scriptVars[SYRIA_FLAG] = 2;

		SwordLogic::_scriptVars[TAILOR_PHONE_FLAG] = 1;
	}
	else
	{
		error("Can't start in location %d", startNumber);
	}


	compact = (BsObject*)_objectMan->fetchObject(PLAYER);
	_logic->fnEnterSection(compact, PLAYER, startNumber, 0, 0, 0, 0, 0);	// (automatically opens the compact resource for that section)
}

void SwordEngine::go(void) {
	
	initialize();
	// check if we have savegames. if we do, show control panel, else start intro.
	do {
        mainLoop();
		// mainLoop was left, show control panel
	} while (true);
}

void SwordEngine::mainLoop(void) {
	uint32 newTime, frameTime;
	do {
		// do we need the section45-hack from sword.c here?
		// todo: ensure right cd is inserted
		_sound->newScreen(SwordLogic::_scriptVars[NEW_SCREEN]);
		_screen->newScreen(SwordLogic::_scriptVars[NEW_SCREEN]);
		_logic->newScreen(SwordLogic::_scriptVars[NEW_SCREEN]);
		SwordLogic::_scriptVars[SCREEN] = SwordLogic::_scriptVars[NEW_SCREEN];
		//		 let swordSound start room sfx
		do {
			_music->stream();
			frameTime = _system->get_msecs();
			_systemVars.saveGameFlag = 0;
			_logic->engine();
			_logic->updateScreenParams(); // sets scrolling

			_screen->recreate();
			_screen->spritesAndParallax();
			_mouse->animate();

			newTime = _system->get_msecs();
			/*if ((newTime - frameTime < 50) && (!SwordLogic::_scriptVars[NEW_PALETTE])) {
				RenderScreenGDK();
				BlitMenusGDK();
				BlitMousePm();
				if (newTime - frameTime < 40)
					_system->delay_msecs(40 - (newTime - frameTime));
				FlipScreens();
			}*/

			_sound->engine();
			_screen->updateScreen();
		//-
			_menu->refresh(MENU_TOP);
			_menu->refresh(MENU_BOT);

			newTime = _system->get_msecs();
			if (newTime - frameTime < 80)
				delay(80 - (newTime - frameTime));
			else
				delay(0);

			/*FlipScreens(); this is done in SwordScreen::updateScreen() now.
			if (SwordLogic::_scriptVars[NEW_PALETTE]) {
				SwordLogic::_scriptVars[NEW_PALETTE] = 0;
				startFadePaletteUp();
			}*/

			_mouse->engine( _mouseX, _mouseY, _mouseState);
			_mouseState = 0;
			// do something smart here to implement pausing the game. If we even want that, that is.
		} while ((SwordLogic::_scriptVars[SCREEN] == SwordLogic::_scriptVars[NEW_SCREEN]) &&
			(_systemVars.saveGameFlag < 2));	// change screen

		if (SwordLogic::_scriptVars[SCREEN] != 53) // we don't fade down after syria pan (53).
			_screen->fadeDownPalette();
		while (_screen->stillFading()) {
			_music->stream();
			_screen->updateScreen();
			delay(1000/12);
			// todo: fade sfx?
		}

		_screen->quitScreen(); // close graphic resources
		_objectMan->closeSection(SwordLogic::_scriptVars[SCREEN]); // close the section that PLAYER has just left, if it's empty now
        // todo: stop sfx, clear sfx queue, free sfx memory
	} while (_systemVars.saveGameFlag < 2);
}

void SwordEngine::delay(uint amount) { //copied and mutilated from sky.cpp

	OSystem::Event event;

	uint32 start = _system->get_msecs();
	uint32 cur = start;
	uint16 _key_pressed = 0;	//reset

	do {
		while (_system->poll_event(&event)) {
			switch (event.event_code) {
			case OSystem::EVENT_KEYDOWN:

				// Make sure backspace works right (this fixes a small issue on OS X)
				if (event.kbd.keycode == 8)
					_key_pressed = 8;
				else
					_key_pressed = (byte)event.kbd.ascii;
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
