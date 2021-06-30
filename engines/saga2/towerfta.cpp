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

#include "saga2/saga2.h"
#include "saga2/towerwin.h"
#include "saga2/towerfta.h"
#include "saga2/fta.h"
#include "saga2/mainmap.h"
#include "saga2/tromode.h"
#include "saga2/script.h"
#include "saga2/vpal.h"
#include "saga2/palette.h"
#include "saga2/intrface.h"
#include "saga2/mouseimg.h"
#include "saga2/patrol.h"
#include "saga2/weapons.h"
#include "saga2/loadsave.h"
#include "saga2/display.h"
#include "saga2/tile.h"

namespace Saga2 {

/* ===================================================================== *
   FTA initialization & cleanup records
 * ===================================================================== */

int maxInitState = fullyInitialized;

TowerLayer tower[fullyInitialized] = {
	{ nothingInitialized,        &initTowerBase,        &termTowerBase },
	{ errHandlersInitialized,    &initErrorManagers,    &termErrorManagers },
	{ delayedErrInitialized,     &initDelayedErrors,    &termDelayedErrors },
	{ activeErrInitialized,      &initActiveErrors,     &termActiveErrors },
	{ configTestInitialized,     &initSystemConfig,     &termTowerBase },
	{ memoryInitialized,         &initMemPool,          &termMemPool },
	{ introInitialized,          &initPlayIntro,        &termPlayOutro },
	{ timerInitialized,          &initSystemTimer,      &termSystemTimer },
	{ audioInitialized,          &initAudio,            &termAudio},
	{ resourcesInitialized,      &initResourceFiles,    &termResourceFiles },
	{ serversInitialized,        &initResourceServers,  &termResourceServers },
	{ pathFinderInitialized,     &initPathFinders,      &termPathFinders },
	{ scriptsInitialized,        &initSAGAInterpreter,  &termSAGAInterpreter },
	{ audStartInitialized,       &initAudioChannels,    &termAudioChannels },
	{ tileResInitialized,        &initResourceHandles,  &termResourceHandles },
	{ palettesInitialized,       &initPalettes,         &termPalettes },
	{ mainWindowInitialized,     &initDisplayPort,      &termDisplayPort },
	{ panelsInitialized,         &initPanelSystem,      &termPanelSystem },
	{ mainWindowOpenInitialized, &initMainWindow,       &termMainWindow },
	{ guiMessInitialized,        &initGUIMessagers,     &termGUIMessagers },
	{ mouseImageInitialized,     &initMousePointer,     &termMousePointer },
	{ displayInitialized,        &initDisplay,          &termDisplay },
	{ mapsInitialized,           &initGameMaps,         &termGameMaps },
	{ patrolsInitialized,        &initRouteData,        &termRouteData },
	{ spritesInitialized,        &initActorSprites,     &termActorSprites },
	{ weaponsInitialized,        &initWeaponData,       &termWeaponData },
	{ magicInitialized,          &initSpellData,        &termSpellData },
	{ objectSoundFXInitialized,  &initObjectSoundFX,    &termObjectSoundFX },
	{ prototypesInitialized,     &initObjectPrototypes, &termObjectPrototypes },
	{ gameStateInitialized,      &initDynamicGameData,  &termDynamicGameData },
	{ gameModeInitialized,       &initGameMode,         &termGameMode },
	{ gameDisplayEnabled,        &initTop,              &termTop },
	{ procResEnabled,            &initProcessResources, &termProcessResources }
};

/* ===================================================================== *
   Declarations for FTA initialization & cleanup
 * ===================================================================== */

//#include "saga2/loadmsg.h"
bool initGUIMessagers(void);
void cleanupMessagers(void);
void cleanupGUIMessagers(void);
bool initMemPool(void);
void cleanupMemPool(void);
bool openResources(void);
void closeResources(void);
void initServers(void);
void cleanupServers(void);
void initMagic(void);
void cleanupMagic(void);
void cleanupMousePointer(void);
void resetInputDevices(void);

extern hResContext      *tileRes;       // tile resource handle
extern hResContext      *listRes;
extern ResImportTable   *resImports;
extern BackWindow       *mainWindow;            // main window...
extern gPanelList       *trioControls;
extern gPanelList       *indivControls;

/* ===================================================================== *
   Actual FTA initialization & cleanup pairs
 * ===================================================================== */


// ------------------------------------------------------------------------

INITIALIZER(initSystemConfig) {
	return true;
}

// uses null cleanup

// ------------------------------------------------------------------------

extern INITIALIZER(initMemPool);

TERMINATOR(termMemPool) {
	cleanupMemPool();                       // deallocate memory buffers
}


// ------------------------------------------------------------------------

INITIALIZER(initPlayIntro) {
	setIntroMode();
	return true;
}

TERMINATOR(termPlayOutro) {
	setOutroMode();
}


// ------------------------------------------------------------------------

extern INITIALIZER(initSystemTimer);

TERMINATOR(termSystemTimer) {
	cleanupSystemTimer();
}

// ------------------------------------------------------------------------

extern INITIALIZER(initAudio);

TERMINATOR (termAudio)
{
	cleanupAudio();
}

// ------------------------------------------------------------------------

INITIALIZER(initResourceFiles) {
	return openResources();
}

TERMINATOR(termResourceFiles) {
	closeResources();
}


// ------------------------------------------------------------------------

INITIALIZER(initResourceServers) {
	return true;
}

TERMINATOR(termResourceServers) {
}


// ------------------------------------------------------------------------

INITIALIZER(initPathFinders) {
	initPathFinder();
	return true;
}

TERMINATOR(termPathFinders) {
	cleanupPathFinder();                    // cleanup pathfinder server
}


// ------------------------------------------------------------------------

INITIALIZER(initSAGAInterpreter) {
	initScripts();
	return true;
}

TERMINATOR(termSAGAInterpreter) {
	cleanupScripts();                       // cleanup SAGA interpreter
}


// ------------------------------------------------------------------------

INITIALIZER(initAudioChannels) {
	startAudio();
	return true;
}

TERMINATOR(termAudioChannels) {
}


// ------------------------------------------------------------------------

INITIALIZER(initResourceHandles) {
	tileRes = resFile->newContext(MKTAG('T', 'I', 'L', 'E'), "tile resources");
	if (!tileRes->_valid)
		return false;
	listRes = objResFile->newContext(MKTAG('L', 'I', 'S', 'T'), "list resources");
	if (!listRes->_valid)
		return false;
	resImports = (ResImportTable *)LoadResource(listRes, MKTAG('I', 'M', 'P', 'O'), "res imports");
	if (!resImports)
		return false;
	return true;
}

TERMINATOR(termResourceHandles) {
	if (resImports) {
		free(resImports);
		resImports = nullptr;
	}
	if (listRes) objResFile->disposeContext(listRes);
	listRes = nullptr;
	if (tileRes) resFile->disposeContext(tileRes);
	tileRes = nullptr;
}


// ------------------------------------------------------------------------

INITIALIZER(initPalettes) {
	loadPalettes();
	return true;
}

TERMINATOR(termPalettes) {
	cleanupPaletteData();
}


// ------------------------------------------------------------------------

INITIALIZER(initDisplayPort) {
	initBackPanel();
	return true;
}

TERMINATOR(termDisplayPort) {
	if (mainWindow) delete mainWindow;
	mainWindow = nullptr;
}


// ------------------------------------------------------------------------

INITIALIZER(initPanelSystem) {
	initPanels(g_vm->_mainPort);
	if (g_vm->_mainPort.map == nullptr) {
		gPixelMap *tmap = new gPixelMap;
		tmap->size = Point16(screenWidth, screenHeight);
		tmap->data = new uint8[tmap->bytes()];
		g_vm->_mainPort.setMap(tmap);
	}
	return true;
}

TERMINATOR(termPanelSystem) {
	cleanupPanels();                        // restore mouse state
}


// ------------------------------------------------------------------------

INITIALIZER(initMainWindow) {
	mainWindow->open();
	return true;
}

TERMINATOR(termMainWindow) {
}


// ------------------------------------------------------------------------

extern INITIALIZER(initGUIMessagers);

TERMINATOR(termGUIMessagers) {
	cleanupGUIMessagers();
}


// ------------------------------------------------------------------------

INITIALIZER(initMousePointer) {
	//pointer.hide();
	setMouseImage(kMouseArrowImage, 0, 0);
	return pointer.init(640, 480);
}

TERMINATOR(termMousePointer) {
	cleanupMousePointer();
}


// ------------------------------------------------------------------------

INITIALIZER(initDisplay) {
	g_vm->_mainPort.setColor(0);            //  fill screen with color
	drawPage = &g_vm->_mainPort.protoPage;
	g_vm->_mainPort.setDisplayPage(drawPage);
	//lightsOut();
	//g_vm->_mainPort.fillRect( Rect16( 0, 0, screenWidth, screenHeight ) );

	//  Create a panelList to contain the controls which are
	//  only active when all three brothers are shown
	if ((trioControls = new gPanelList(*mainWindow)) == nullptr)
		return false;

	//  Create a panelList to contain the controls which are
	//  only active when a single brother is shown
	if ((indivControls = new gPanelList(*mainWindow)) == nullptr)
		return false;

	//  Since we start in "trio" mode, turn off the "individual"
	//  controls...
	indivControls->enable(false);

	return true;
}

TERMINATOR(termDisplay) {
	if (trioControls) {
		delete trioControls;
		trioControls = NULL;
	}
	if (indivControls) {
		delete indivControls;
		indivControls = NULL;
	}
}


// ------------------------------------------------------------------------

INITIALIZER(initGameMaps) {
	initMaps();
	return true;
}

TERMINATOR(termGameMaps) {
	cleanupMaps();
}


// ------------------------------------------------------------------------

INITIALIZER(initRouteData) {
	initPatrolRoutes();
	return true;
}

TERMINATOR(termRouteData) {
	cleanupPatrolRoutes();
}


// ------------------------------------------------------------------------

INITIALIZER(initActorSprites) {
	initSprites();
	return true;
}

TERMINATOR(termActorSprites) {
	cleanupSprites();
}


// ------------------------------------------------------------------------

INITIALIZER(initWeaponData) {
	initWeapons();
	return true;
}

TERMINATOR(termWeaponData) {
	cleanupWeapons();
}


// ------------------------------------------------------------------------

INITIALIZER(initSpellData) {
	initMagic();
	return true;
}

TERMINATOR(termSpellData) {
	cleanupMagic();
}


// ------------------------------------------------------------------------

INITIALIZER(initObjectSoundFX) {
	initObjectSoundFXTable();
	return true;
}

TERMINATOR(termObjectSoundFX) {
	cleanupObjectSoundFXTable();
}

// ------------------------------------------------------------------------

INITIALIZER(initObjectPrototypes) {
	initPrototypes();
	return true;
}

TERMINATOR(termObjectPrototypes) {
	cleanupPrototypes();
}


// ------------------------------------------------------------------------

INITIALIZER(initDynamicGameData) {
	initGameState();
	return true;
}

TERMINATOR(termDynamicGameData) {
	cleanupGameState();
}


// ------------------------------------------------------------------------

INITIALIZER(initGameMode) {
	GameMode *gameModes[] = {&PlayMode, &TileMode};
	GameMode::SetStack(gameModes, 2);
	if (GameMode::newmodeFlag)
		GameMode::update();
	return true;
}

TERMINATOR(termGameMode) {
	GameMode::modeUnStack(0); //Zero Equals Unstack All
	cleanupSystemTasks();
}


// ------------------------------------------------------------------------
void RMemFastCleanup(void);

INITIALIZER(initTop) {
	niceScreenStartup();
	return true;
}

TERMINATOR(termTop) {
	mainDisable();
}

} // end of namespace Saga2
