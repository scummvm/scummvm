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

#include "saga2/std.h"
#include "saga2/rmemfta.h"
#include "saga2/towerwin.h"
#include "saga2/towerfta.h"
#include "saga2/fta.h"
#include "saga2/mainmap.h"
#include "saga2/config.h"
#include "saga2/itevideo.h"
#include "saga2/tromode.h"
#include "saga2/audio.h"
#include "saga2/annoy.h"
#include "saga2/script.h"
#include "saga2/palette.h"
#include "saga2/intrface.h"
#include "saga2/mouseimg.h"
#include "saga2/panel.h"
#include "saga2/floating.h"
#include "saga2/images.h"
#include "saga2/patrol.h"
#include "saga2/weapons.h"
#include "saga2/loadsave.h"
#include "saga2/display.h"
#include "saga2/transit.h"

namespace Saga2 {

/* ===================================================================== *
   FTA initialization & cleanup records
 * ===================================================================== */

int maxInitState = fullyInitialized;

TowerLayer tower[fullyInitialized] = {
	{ nothingInitialized,        &initTowerBase,        &termTowerBase,        cpInternal },
	{ errHandlersInitialized,    &initErrorManagers,    &termErrorManagers,    cpInternal },
	{ delayedErrInitialized,     &initDelayedErrors,    &termDelayedErrors,    cpInternal },
	{ activeErrInitialized,      &initActiveErrors,     &termActiveErrors,     cpInternal },
	{ messagersInitialized,      &initSysMessagers,     &termSysMessagers,     cpInternal },
	{ errLoggersInitialized,     &initErrorLoggers,     &termErrorLoggers,     cpInternal },
	{ breakHandlerInitialized,   &initCtlBreakTrap,     &termCtlBreakTrap,     cpInternal },
	{ configTestInitialized,     &initSystemConfig,     &termTowerBase,        cpInternal },
	{ memoryInitialized,         &initMemPool,          &termMemPool,          cpInsufVirtMemFree },
	{ graphicsSystemInitialized, &initGraphicsSystem,   &termGraphicsSystem,   cpDDrawInitFail },
	{ videoInitialized,          &initVideoPlayer,      &termVideoPlayer,      cpInternal },
	{ introInitialized,          &initPlayIntro,        &termPlayOutro,        cpInternal },
	{ timerInitialized,          &initSystemTimer,      &termSystemTimer,      cpDTimerInitFail },
	{ audioInitialized,          &initAudio,            &termAudio,            cpDSoundInitFail },
	{ SVGAInitialized,           &initGraphics,         &termGraphics,         cpInternal },
	{ resourcesInitialized,      &initResourceFiles,    &termResourceFiles,    cpResFileMissing },
	{ serversInitialized,        &initResourceServers,  &termResourceServers,  cpInternal },
	{ pathFinderInitialized,     &initPathFinders,      &termPathFinders,      cpInternal },
	{ scriptsInitialized,        &initSAGAInterpreter,  &termSAGAInterpreter,  cpInternal },
	{ audStartInitialized,       &initAudioChannels,    &termAudioChannels,    cpInternal },
	{ tileResInitialized,        &initResourceHandles,  &termResourceHandles,  cpInternal },
	{ palettesInitialized,       &initPalettes,         &termPalettes,         cpInternal },
	{ mainWindowInitialized,     &initDisplayPort,      &termDisplayPort,      cpInternal },
	{ panelsInitialized,         &initPanelSystem,      &termPanelSystem,      cpInternal },
	{ mainWindowOpenInitialized, &initMainWindow,       &termMainWindow,       cpInternal },
	{ guiMessInitialized,        &initGUIMessagers,     &termGUIMessagers,     cpInternal },
	{ mouseImageInitialized,     &initMousePointer,     &termMousePointer,     cpNoMouseDriver },
	{ displayInitialized,        &initDisplay,          &termDisplay,          cpInternal },
	{ mapsInitialized,           &initGameMaps,         &termGameMaps,         cpInternal },
	{ patrolsInitialized,        &initRouteData,        &termRouteData,        cpInternal },
	{ spritesInitialized,        &initActorSprites,     &termActorSprites,     cpInternal },
	{ weaponsInitialized,        &initWeaponData,       &termWeaponData,       cpInternal },
	{ magicInitialized,          &initSpellData,        &termSpellData,        cpInternal },
	{ objectSoundFXInitialized,  &initObjectSoundFX,    &termObjectSoundFX,    cpInternal },
	{ prototypesInitialized,     &initObjectPrototypes, &termObjectPrototypes, cpInternal },
	{ gameStateInitialized,      &initDynamicGameData,  &termDynamicGameData,  cpInternal },
	{ gameModeInitialized,       &initGameMode,         &termGameMode,         cpInternal },
	{ gameDisplayEnabled,        &initTop,              &termTop,              cpInternal },
	{ procResEnabled,            &initProcessResources, &termProcessResources, cpInternal },
};

/* ===================================================================== *
   Declarations for FTA initialization & cleanup
 * ===================================================================== */

//#include "saga2/loadmsg.h"
bool initMessagers(void);
bool initGUIMessagers(void);
void cleanupMessagers(void);
void cleanupGUIMessagers(void);
bool initMemPool(void);
void cleanupMemPool(void);
bool openResources(void);
void closeResources(void);
void initServers(void);
void cleanupServers(void);
//void initBackPanel(void);
void initMagic(void);
void cleanupMagic(void);
void cleanupMousePointer(void);
//void mainEnable( void );
//void mainDisable( void );
//void lightsOut( void );
void resetInputDevices(void);
#ifdef _WIN32
//	void localCursorOn( void );
#endif

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

INITIALIZER(initErrorLoggers) {
	initErrorHandlers();
	return TRUE;
}

TERMINATOR(termErrorLoggers) {
	cleanupErrorHandlers();
}


// ------------------------------------------------------------------------

INITIALIZER(initSysMessagers) {
	initMessagers();
	return TRUE;
}

TERMINATOR(termSysMessagers) {
	cleanupMessagers();
}


// ------------------------------------------------------------------------

INITIALIZER(initCtlBreakTrap) {
	initBreakHandler();
	return TRUE;
}

TERMINATOR(termCtlBreakTrap) {
	cleanupBreakHandler();
}


// ------------------------------------------------------------------------

INITIALIZER(initSystemConfig) {
	return TRUE;
}

// uses null cleanup

// ------------------------------------------------------------------------

extern INITIALIZER(initMemPool);

TERMINATOR(termMemPool) {
	cleanupMemPool();                       // deallocate memory buffers
}


// ------------------------------------------------------------------------

extern INITIALIZER(initGraphicsSystem);

TERMINATOR(termGraphicsSystem) {
	cleanupGraphicsSystem();
}


// ------------------------------------------------------------------------

INITIALIZER(initVideoPlayer) {
	initVideo();
	return TRUE;
}

TERMINATOR(termVideoPlayer) {
	cleanupVideo();
}


// ------------------------------------------------------------------------

INITIALIZER(initPlayIntro) {
	setIntroMode();
	return TRUE;
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

TERMINATOR(termAudio) {
	cleanupAudio();
}


// ------------------------------------------------------------------------

extern INITIALIZER(initGraphics);

TERMINATOR(termGraphics) {
	cleanupGraphics();
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
	initServers();
	return TRUE;
}

TERMINATOR(termResourceServers) {
	cleanupServers();                       // cleanup async servers
}


// ------------------------------------------------------------------------

INITIALIZER(initPathFinders) {
	initPathFinder();
	return TRUE;
}

TERMINATOR(termPathFinders) {
	cleanupPathFinder();                    // cleanup pathfinder server
}


// ------------------------------------------------------------------------

INITIALIZER(initSAGAInterpreter) {
	initScripts();
	return TRUE;
}

TERMINATOR(termSAGAInterpreter) {
	cleanupScripts();                       // cleanup SAGA interpreter
}


// ------------------------------------------------------------------------

INITIALIZER(initAudioChannels) {
	startAudio();
	return TRUE;
}

TERMINATOR(termAudioChannels) {
}


// ------------------------------------------------------------------------

INITIALIZER(initResourceHandles) {
	tileRes = resFile->newContext(RES_ID('T', 'I', 'L', 'E'), "tile resources");
	if (!tileRes->valid)
		return FALSE;
	listRes = objResFile->newContext(RES_ID('L', 'I', 'S', 'T'), "list resources");
	if (!listRes->valid)
		return FALSE;
	resImports = (ResImportTable *)LoadResource(listRes, RES_ID('I', 'M', 'P', 'O'), "res imports");
	if (!resImports)
		return FALSE;
	return TRUE;
}

TERMINATOR(termResourceHandles) {
	if (resImports) {
		RDisposePtr(resImports);
		resImports = NULL;
	}
	if (listRes) objResFile->disposeContext(listRes);
	listRes = NULL;
	if (tileRes) resFile->disposeContext(tileRes);
	tileRes = NULL;
}


// ------------------------------------------------------------------------

INITIALIZER(initPalettes) {
	loadPalettes();
	return TRUE;
}

TERMINATOR(termPalettes) {
	cleanupPaletteData();
}


// ------------------------------------------------------------------------

INITIALIZER(initDisplayPort) {
	initBackPanel();
	return TRUE;
}

TERMINATOR(termDisplayPort) {
	if (mainWindow) delete mainWindow;
	mainWindow = NULL;
}


// ------------------------------------------------------------------------

INITIALIZER(initPanelSystem) {
	initPanels(mainPort);
	return TRUE;
}

TERMINATOR(termPanelSystem) {
	cleanupPanels();                        // restore mouse state
}


// ------------------------------------------------------------------------

INITIALIZER(initMainWindow) {
	mainWindow->open();
	return TRUE;
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
	setMouseImage(ArrowImage, 0, 0);
	return pointer.init(640, 480);
}

TERMINATOR(termMousePointer) {
	cleanupMousePointer();
}


// ------------------------------------------------------------------------

INITIALIZER(initDisplay) {
	mainPort.setColor(0);            //  fill screen with color
	//lightsOut();
	//mainPort.fillRect( Rect16( 0, 0, screenWidth, screenHeight ) );

	//  Create a panelList to contain the controls which are
	//  only active when all three brothers are shown
	if ((trioControls = NEW_UI gPanelList(*mainWindow)) == NULL)
		return FALSE;

	//  Create a panelList to contain the controls which are
	//  only active when a single brother is shown
	if ((indivControls = NEW_UI gPanelList(*mainWindow)) == NULL)
		return FALSE;

	//  Since we start in "trio" mode, turn off the "individual"
	//  controls...
	indivControls->enable(FALSE);

	return TRUE;
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
	return TRUE;
}

TERMINATOR(termGameMaps) {
	cleanupMaps();
}


// ------------------------------------------------------------------------

INITIALIZER(initRouteData) {
	initPatrolRoutes();
	return TRUE;
}

TERMINATOR(termRouteData) {
	cleanupPatrolRoutes();
}


// ------------------------------------------------------------------------

INITIALIZER(initActorSprites) {
	initSprites();
	return TRUE;
}

TERMINATOR(termActorSprites) {
	cleanupSprites();
}


// ------------------------------------------------------------------------

INITIALIZER(initWeaponData) {
	initWeapons();
	return TRUE;
}

TERMINATOR(termWeaponData) {
	cleanupWeapons();
}


// ------------------------------------------------------------------------

INITIALIZER(initSpellData) {
	initMagic();
	return TRUE;
}

TERMINATOR(termSpellData) {
	cleanupMagic();
}


// ------------------------------------------------------------------------

INITIALIZER(initObjectSoundFX) {
	initObjectSoundFXTable();
	return TRUE;
}

TERMINATOR(termObjectSoundFX) {
	cleanupObjectSoundFXTable();
}

// ------------------------------------------------------------------------

INITIALIZER(initObjectPrototypes) {
	initPrototypes();
	return TRUE;
}

TERMINATOR(termObjectPrototypes) {
	cleanupPrototypes();
}


// ------------------------------------------------------------------------

INITIALIZER(initDynamicGameData) {
	initGameState();
	return TRUE;
}

TERMINATOR(termDynamicGameData) {
	cleanupGameState();
}


// ------------------------------------------------------------------------

INITIALIZER(initGameMode) {
	GameMode::SetStack(&PlayMode, &TileMode, End_List);
	if (GameMode::newmodeFlag)
		GameMode::update();
	return TRUE;
}

TERMINATOR(termGameMode) {
	GameMode::modeUnStack(0); //Zero Equals Unstack All
	cleanupSystemTasks();
}


// ------------------------------------------------------------------------
void RMemFastCleanup(void);

INITIALIZER(initTop) {
	niceScreenStartup();
	return TRUE;
}

TERMINATOR(termTop) {
	mainDisable();
	RMemFastCleanup();
#ifdef _WIN32
	//pWindow->RestoreDisplay();
#endif
}

} // end of namespace Saga2
