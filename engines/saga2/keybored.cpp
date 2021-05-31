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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL // FIXME: Remove

#include "saga2/std.h"
#include "saga2/rmemfta.h"
#include "saga2/tilemode.h"
#include "saga2/calender.h"

#include "saga2/tile.h"
#include "saga2/setup.h"
#include "saga2/grabinfo.h"
#include "saga2/motion.h"
#include "saga2/actor.h"
#include "saga2/modal.h"
#include "saga2/transit.h"
#include "saga2/player.h"
#include "saga2/annoy.h"
#include "saga2/intrface.h"

#include "saga2/cmisc.h"
#include "saga2/button.h"

#define TEST1           1           //  enable test code
#define TEST2           1
#define TEST3           1           //  loading/saving
#define CHEATMOVE       1           // For moving with keypad in 8 directions

#if TEST3
#include "saga2/loadsave.h"
#endif

#include "saga2/imagcach.h"


#include "saga2/assign.h"

#ifdef ALEXS
#include "saga2/automap.h"
#include "saga2/uidialog.h"

uint8   temp        = 0;
uint16  secNum      = 0;

extern  uint16  mapSizeU, mapSizeV;
uint16  sumNum;
TilePoint tilePos;

#endif

#ifdef WINKLUDGE
#include "saga2/automap.h"
#endif

namespace Saga2 {

extern int16        speechButtonCount;      // count of speech buttons
extern ObjectID     viewCenterObject;
extern ObjectID         pickedObject;           // which object picked by mouse
#if TEST1
extern ObjectID         pickedActor;
#endif

//-----------------------------------------------------------------------
//	Handle keystrokes for main mode.

extern int16 stageType;
extern int16 shopType;
void sentenceGenerator(char *sentence);
void createTestPalette();//For Key Testing 'f'
void deleteSpeech(ObjectID id);         // voice sound sample ID
#if DEBUG
void soundTest1(void);
void soundTest2(void);
void voiceTest1(void);
void voiceTest2(void);
#endif
void toggleMusic(void);

void RShowMem(void);

extern APPFUNCV(videoTest1);
extern APPFUNCV(videoTest2);

extern PlayerActor playerList[];    //  Master list of all PlayerActors

void queueActorSpeech(GameObject            *obj,
                      char                *text,
                      int                 count,
                      int32               sampleID,
                      int                 flags
                     );          // voice sound sample ID

void cheatMove(int16 key);


//-----------------------------------------------------------------------

void TileModeHandleKey(int16 key, int16 /* qual */) {
#if DINO
	ReadyContainerView *tradeNathanCV,
	                   *tradeOtherCV;
#endif

	TilePoint Pos, ActorTP;
	Actor   *a = getCenterActor();
	Location l(a->getLocation(), a->IDParent());

	GameObject *object = (GameObject *)getCenterActor();
#ifdef FTA
	static int Object = 2;
#endif

	//This is for moving center actor in cardinal directions
	//by amount specified by loc const int moveDist and using keypad
	//without num lock on

#if CHEATMOVE
	cheatMove(key);
#endif
	//  If there is currently a speech balloon up, and the
	//  speech balloon has embedded buttons, then disallow
	//  user input -- except that for now we will still allow
	//  the special 'quit' key.
	if (speechButtonCount > 0) {
		if (key != 0x1b && key != 'b') return;
	}

	//-----------------------------------------------------------------------

	switch (tolower(key)) {

	case 'q':
	case 0x1b:
		gameRunning = FALSE;
		break;

	case 'j':
		if (a) MotionTask::jump(*a);
		break;

	case 'l':
		RShowMem();
		break;

#ifdef WINKLUDGE
	case 'm':
		toggleMusic();
		break;
	case '\\':
		openAutoMap();
		break;
#endif

	case 'e':
		a->takeMana(manaIDRed, -20);
		a->takeMana(manaIDOrange, -20);
		a->takeMana(manaIDYellow, -20);
		a->takeMana(manaIDGreen, -20);
		a->takeMana(manaIDBlue, -20);
		a->takeMana(manaIDViolet, -20);
		a->effectiveStats.spellcraft = 3;
		break;

#ifdef __WATCOMC__
	case 'm':
		toggleMusic();
		break;
#endif

#if TEST1
	case 'v':
		if (viewCenterObject == getCenterActorID()) {
			if (pickedActor != Nothing) viewCenterObject = pickedActor;
		} else
			viewCenterObject = getCenterActorID();
		break;
#endif


#if TEST3
	case '5':
		saveGameState(0, "game 1");
		break;

	case '6':
		saveGameState(1, "game 2");
		break;

	case '%':
		cleanupGameState();
		loadSavedGameState(0);
		break;

	case '^':
		cleanupGameState();
		loadSavedGameState(1);
		break;
#endif

#ifdef FTA
	case 'a':
		if (++Object < 8) {
			GameObject *obj1 = GameObject::objectAddress(Object);
			ActorTP = a->getLocation();
			obj1->move(ActorTP);
		} else
			Object = 2;
		break;

#endif

#if DINO
#ifdef GENE
	case '~':
		WriteStatusF(2, "Switching to Trice");
		GameMode::SetStack(&PlayMode, &TriceMode, End_List);
		GameMode::update();
		break;
#endif
#ifdef FRANKC
	case '~':
		buttonList[tradeButton]->enable(TRUE);
		buttonList[tradeBackButton]->enable(TRUE);
		buttonList[tradeButton]->invalidate();
		buttonList[tradeBackButton]->invalidate();
		tradeNathanCV = new ReadyContainerView(*playControls,
		                                       Rect16(259, 360,
		                                               iconOriginX * 2 + iconWidth * 1 + iconSpacingY * (1 - 1),
		                                               iconOriginY + (iconOriginY * 1) + (1 * iconHeight)),
		                                       NULL, 0,
		                                       Point16(iconOriginX, iconOriginY),
		                                       Point16(iconSpacingX, iconSpacingY),
		                                       1,
		                                       1,
		                                       1,
		                                       0);

		tradeOtherCV = new ReadyContainerView(*playControls,
		                                      Rect16(300, 360,
		                                              iconOriginX * 2 + iconWidth * 1 + iconSpacingY * (1 - 1),
		                                              iconOriginY + (iconOriginY * 1) + (1 * iconHeight)),
		                                      NULL, 0,
		                                      Point16(iconOriginX, iconOriginY),
		                                      Point16(iconSpacingX, iconSpacingY),
		                                      1,
		                                      1,
		                                      1,
		                                      0);

		tradeNathanCV->setContainer(GameObject::objectAddress(0x8001));
		tradeNathanCV->draw();

		tradeOtherCV->setContainer(GameObject::objectAddress(0x8002));
		tradeOtherCV->draw();
		break;
#endif
#endif

#ifdef FRANKC
		/* Took Out For Milestone
		    case 'y':
		        tilePickPos.u += 100;
		        tilePickPos.v += 100;
		        MotionTask::walkToDirect((Actor &)*GameObject::objectAddress( 32817 ) , tilePickPos, FALSE );
		        break;
		    case 'z':

		        Target subTarget;
		        subTarget.setTargetObject( a->thisID() );
		        ((Actor *)GameObject::objectAddress( 32791 ))->setupTarget( subTarget ,
		                                      Actor::huntToBeNear,
		                                      FALSE,
		                                      Actor::priorityMedium );
		        break;
		*/
#endif

#ifdef DAVIDR
	case 't':
		extern bool showTile;
		showTile = !showTile;
		break;
#endif


	// basically for changing the light level

	case 'b':
		calender.hour += 1;

		if (calender.hour > 24) {
			calender.hour = 0;
		}

		break;

	case 'w':
		calender.hour = 9;
		calender.days++;
		dayNightUpdate();

		break;


#ifdef ALEXS
		int16 FileDialog(int16);
		int16 OptionsDialog(void);
		int16 openScroll(uint16);
		extern char bookText[];
		extern uint8 weight;
		extern uint8 encum;
		extern uint16   writeStatusFX;
		extern uint16   writeStatusFY;
		extern Point16 manaStarSizes[];
		extern Point16 manaStarSec[];
		extern Point16 manaStarCentXY;
		extern gCompImage *weightEncum;

	case 'd':
		openAutoMap();
		//FileDialog( 0 );
		break;

	case 'o':
		OptionsDialog();
		break;

	case 'u':
		//userDialog( "title", " This is a message", "btn 1", "btn 2", NULL );
		break;

	case 'f':
		if (pickedActor != Nothing)
			new ((Actor *)GameObject::objectAddress(pickedActor)) HuntToBeNearActorAssignment(SpecificActorTarget(getCenterActor()), 16, TRUE);
		break;

	case '[':
		//writeStatusFX = 0;
		//writeStatusFY = 0;
		//mapSizeU--;
		sumNum--;
		break;

	case ']':
		//writeStatusFX = 468;
		//mapSizeU++;
		//sumNum++;
		playerList[ 0 ].skillAdvance(eBrawn, 200);
		break;

	case '=':
		writeStatusFY = 200;
		break;

	case '`':
		//StatusLine->setLine( "Goblins, and Ogres, and Giants, oh my!" );
		playerList[ 0 ].skillAdvance(eBrawn, 1);
		break;


	case 'h':

		//( ( Actor * )GameObject::objectAddress( 32768 ) )->effectiveStats.redMana -= 10;
		//( ( Actor * )GameObject::objectAddress( 32768 ) )->effectiveStats.vitality -= 10;
		//( ( Actor * )GameObject::objectAddress( 32768 ) )->effectiveStats.brawn -= 10;
		playerList[ 0 ].setTestBrawn();
		break;

	case 'y':
		playerList[ 0 ].recBrawnSome();
		break;

	case 'c':
		playerList[ 0 ].decBrawnSome();
		break;


#endif

#ifdef JEFFL
	case 'z':
		soundTest1();
		break;
	case 'y':
		soundTest2();
		break;
	case 'r':
		voiceTest1();
		break;
	case 't':
		voiceTest2();
		break;
#endif

#ifdef EVANO
#if DEBUG
		int16 OptionsDialog(void);
	case 'o':
		OptionsDialog();
		break;

	case 'z':
		soundTest1();
		break;
	case 'y':
		soundTest2();
		break;
	case 'r':
		voiceTest1();
		break;
	case 't':
		voiceTest2();
		break;
#endif
#endif

	}
}

} // end of namespace Saga2
