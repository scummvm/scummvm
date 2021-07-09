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

#include "saga2/saga2.h"
#include "saga2/loadsave.h"
#include "saga2/savefile.h"
#include "saga2/objects.h"
#include "saga2/tile.h"
#include "saga2/script.h"
#include "saga2/motion.h"
#include "saga2/task.h"
#include "saga2/speech.h"
#include "saga2/timers.h"
#include "saga2/sensor.h"
#include "saga2/band.h"
#include "saga2/mission.h"
#include "saga2/tilemode.h"
#include "saga2/magic.h"
#include "saga2/intrface.h"
#include "saga2/vpal.h"
#include "saga2/palette.h"
#include "saga2/contain.h"
#include "saga2/imagcach.h"

namespace Saga2 {

/* ===================================================================== *
   Functions
 * ===================================================================== */

//	The following resources are included in the save file
//		GLOB -  miscellaneous globals
//		TIME -  game timer
//		CALE -  game calender
//		WRLD -  worlds
//		ACTR -  actors
//		OBJS -  objects
//		BAND -  actor bands
//		PLYR -  player actors
//		CNTR -  center actor ID and view object ID
//		TAGS -  active item instance state arrays
//		CYCL -  tile cycling states
//		SDTA -  SAGA data segment
//		SAGA -  SAGA threads
//		MOTN -  motion tasks
//		TSTK -  actor task stacks
//		TASK -  actor tasks
//		TACT -  tile activity tasks
//		SPCH -  speech tasks
//		AREG -  active regions
//		TIMR -  actor timers
//		SENS -  actor sensors
//		ACNT -  temporary actor count array
//		MISS -  missions
//		FACT -  faction tallies
//		TMST -  TileMode state
//		SPEL -  Active Spell List
//		AMAP -  auto map data
//		UIST -  user interface state
//		PALE -  palette state
//		CONT -  container nodes

//----------------------------------------------------------------------
//	Load initial game state

void initGameState(void) {
	pauseTimer();

	initGlobals();
	initTimer();
	initCalender();
	initWorlds();
	initActors();
	initObjects();
	initBands();
	initPlayerActors();
	initCenterActor();
	initActiveItemStates();
	initTileCyclingStates();
	initSAGADataSeg();
	initSAGAThreads();
	initMotionTasks();
	initTaskStacks();
	initTasks();
	initTileTasks();
	initSpeechTasks();
	initActiveRegions();
	initTimers();
	initSensors();
	initTempActorCount();
	initMissions();
	initFactionTallies();
	initTileModeState();
	initSpellState();
	initAutoMap();
	initUIState();
	initPaletteState();
	initContainerNodes();

	resumeTimer();
}

//----------------------------------------------------------------------
//	Save the current game state

Common::Error saveGameState(int16 saveNo, char *saveName) {
	pauseTimer();

	debugC(1, kDebugSaveload, "Saving game");

	Common::OutSaveFile *out = g_vm->getSaveFileManager()->openForSaving(getSaveFileName(saveNo), false);
	if (!out)
		return Common::kCreatingFileFailed;

	SaveFileHeader header;
	header.gameID = gameID;
	header.saveName = saveName;

	header.write(out);

	saveGlobals(out);
	saveTimer(out);
	saveCalender(out);
	saveWorlds(out);
	saveActors(out);
	saveObjects(out);
	saveBands(out);
	savePlayerActors(out);
	saveCenterActor(out);
	saveActiveItemStates(out);
	saveTileCyclingStates(out);
	saveSAGADataSeg(out);
	saveSAGAThreads(out);
	saveMotionTasks(out);

#if 0
	saveTaskStacks(saveGame);
	saveTasks(saveGame);
	saveTileTasks(saveGame);
	saveSpeechTasks(saveGame);
	saveActiveRegions(saveGame);
	saveTimers(saveGame);
	saveSensors(saveGame);
	saveTempActorCount(saveGame);
	saveMissions(saveGame);
	saveFactionTallies(saveGame);
	saveTileModeState(saveGame);
	saveSpellState(saveGame);
	saveAutoMap(saveGame);
	saveUIState(saveGame);
	savePaletteState(saveGame);
	saveContainerNodes(saveGame);
#endif

	out->finalize();

	delete out;

	resumeTimer();

	return Common::kNoError;
}

//----------------------------------------------------------------------
//	Load a previously saved game state

void loadSavedGameState(int16 saveNo) {
	enum {
		loadGlobalsFlag             = (1 << 0),
		loadTimerFlag               = (1 << 1),
		loadCalenderFlag            = (1 << 2),
		loadWorldsFlag              = (1 << 3),
		loadActorsFlag              = (1 << 4),
		loadObjectsFlag             = (1 << 5),
		loadBandsFlag               = (1 << 6),
		loadPlayerActorsFlag        = (1 << 7),
		loadCenterActorFlag         = (1 << 8),
		loadActiveItemStatesFlag    = (1 << 9),
		loadTileCyclingStatesFlag   = (1 << 10),
		loadSAGADataSegFlag         = (1 << 11),
		loadSAGAThreadsFlag         = (1 << 12),
		loadMotionTasksFlag         = (1 << 13),
		loadTaskStacksFlag          = (1 << 14),
		loadTasksFlag               = (1 << 15),
		loadTileTasksFlag           = (1 << 16),
		loadSpeechTasksFlag         = (1 << 17),
		loadActiveRegionsFlag       = (1 << 18),
		loadTimersFlag              = (1 << 19),
		loadSensorsFlag             = (1 << 20),
		loadTempActorCountFlag      = (1 << 21),
		loadMissionsFlag            = (1 << 22),
		loadFactionTalliesFlag      = (1 << 23),
		loadTileModeStateFlag       = (1 << 24),
		loadSpellStateFlag          = (1 << 25),
		loadAutoMapFlag             = (1 << 26),
		loadUIStateFlag             = (1 << 27),
		loadPaletteStateFlag        = (1 << 28),
		loadContainerNodesFlag      = (1 << 29)
	};

	uint32  loadFlags = 0;

	pauseTimer();

	Common::InSaveFile *in = g_vm->getSaveFileManager()->openForLoading(getSaveFileName(saveNo));
	//SaveFileReader  saveGame(saveNo);
	ChunkID         id;
	int32           chunkSize;
	bool            notEOF;

	notEOF = firstChunk(in, id, chunkSize);
	while (notEOF) {
		switch (id) {
		case MKTAG('G', 'L', 'O', 'B'):
			loadGlobals(in);
			loadFlags |= loadGlobalsFlag;
			break;

		case MKTAG('T', 'I', 'M', 'E'):
			loadTimer(in);
			loadFlags |= loadTimerFlag;
			break;

		case MKTAG('C', 'A', 'L', 'E'):
			loadCalender(in);
			loadFlags |= loadCalenderFlag;
			break;

		case MKTAG('W', 'R', 'L', 'D'):
			loadWorlds(in);
			loadFlags |= loadWorldsFlag;
			break;

		case MKTAG('A', 'C', 'T', 'R'):
			loadActors(in);
			loadFlags |= loadActorsFlag;
			break;

		case MKTAG('O', 'B', 'J', 'S'):
			loadObjects(in);
			loadFlags |= loadObjectsFlag;
			break;

		case MKTAG('B', 'A', 'N', 'D'):
			if (loadFlags & loadActorsFlag) {
				loadBands(in, chunkSize);
				loadFlags |= loadBandsFlag;
			} else
				error("Bands loaded prematurely");
			break;

		case MKTAG('P', 'L', 'Y', 'R'):
			if (loadFlags & loadBandsFlag) {
				loadPlayerActors(in);
				loadFlags |= loadPlayerActorsFlag;
			} else
				error("PlayerActors loaded prematurely");
			break;

		case MKTAG('C', 'N', 'T', 'R'):
			loadCenterActor(in);
			loadFlags |= loadCenterActorFlag;
			break;

		case MKTAG('T', 'A', 'G', 'S'):
			loadActiveItemStates(in);
			loadFlags |= loadActiveItemStatesFlag;
			break;

		case MKTAG('C', 'Y', 'C', 'L'):
			loadTileCyclingStates(in);
			loadFlags |= loadTileCyclingStatesFlag;
			break;

		case MKTAG('S', 'D', 'T', 'A'):
			loadSAGADataSeg(in);
			loadFlags |= loadSAGADataSegFlag;
			break;

		case MKTAG('S', 'A', 'G', 'A'):
			loadSAGAThreads(in, chunkSize);
			loadFlags |= loadSAGAThreadsFlag;
			break;

		case MKTAG('M', 'O', 'T', 'N'):
			if (!(~loadFlags & (loadActorsFlag | loadObjectsFlag))) {
				loadMotionTasks(in, chunkSize);
				loadFlags |= loadMotionTasksFlag;
			} else
				error("MotionTasks loaded prematurely");
			break;
#if 0

		case MKTAG('T', 'S', 'T', 'K'):
			if (loadFlags & loadActorsFlag) {
				loadTaskStacks(saveGame);
				loadFlags |= loadTaskStacksFlag;
			} else
				error("TaskStacks loaded prematurely");
			break;

		case MKTAG('T', 'A', 'S', 'K'):
			if (loadFlags & loadTaskStacksFlag) {
				loadTasks(saveGame);
				loadFlags |= loadTasksFlag;
			} else
				error("Tasks loaded prematurely");
			break;

		case MKTAG('T', 'A', 'C', 'T'):
			if (loadFlags & loadWorldsFlag) {
				loadTileTasks(saveGame);
				loadFlags |= loadTileTasksFlag;
			} else
				error("TileActivityTasks loaded prematurely");
			break;

		case MKTAG('S', 'P', 'C', 'H'):
			if (!(~loadFlags & (loadActorsFlag | loadObjectsFlag))) {
				loadSpeechTasks(saveGame);
				loadFlags |= loadSpeechTasksFlag;
			} else
				error("SpeechTasks loaded prematurely");
			break;

		case MKTAG('A', 'R', 'E', 'G'):
			loadActiveRegions(saveGame);
			loadFlags |= loadActiveRegionsFlag;
			break;

		case MKTAG('T', 'I', 'M', 'R'):
			if (loadFlags & loadActorsFlag) {
				loadTimers(saveGame);
				loadFlags |= loadTimersFlag;
			} else
				error("Timers loaded prematurely");
			break;

		case MKTAG('S', 'E', 'N', 'S'):
			if (loadFlags & loadActorsFlag) {
				loadSensors(saveGame);
				loadFlags |= loadSensorsFlag;
			} else
				error("Sensors loaded prematurely");
			break;

		case MKTAG('A', 'C', 'N', 'T'):
			loadTempActorCount(saveGame);
			loadFlags |= loadTempActorCountFlag;
			break;

		case MKTAG('M', 'I', 'S', 'S'):
			loadMissions(saveGame);
			loadFlags |= loadMissionsFlag;
			break;

		case MKTAG('F', 'A', 'C', 'T'):
			loadFactionTallies(saveGame);
			loadFlags |= loadFactionTalliesFlag;
			break;

		case MKTAG('T', 'M', 'S', 'T'):
			if (loadFlags & loadActorsFlag) {
				loadTileModeState(saveGame);
				loadFlags |= loadTileModeStateFlag;
			} else
				error("TileMode state loaded prematurely");
			break;

		case MKTAG('S', 'P', 'E', 'L'):
			loadSpellState(saveGame);
			loadFlags |= loadSpellStateFlag;
			break;

		case MKTAG('A', 'M', 'A', 'P'):
			if (loadFlags & loadWorldsFlag) {
				loadAutoMap(saveGame);
				loadFlags |= loadAutoMapFlag;
			} else
				error("Auto map loaded prematurely");
			break;

		case MKTAG('U', 'I', 'S', 'T'):
			if (loadFlags & loadPlayerActorsFlag) {
				loadUIState(saveGame);
				loadFlags |= loadUIStateFlag;
			} else
				error("UI state loaded prematurely");
			break;

		case MKTAG('P', 'A', 'L', 'E'):
			loadPaletteState(saveGame);
			loadFlags |= loadPaletteStateFlag;
			break;

		case MKTAG('C', 'O', 'N', 'T'):
			if (loadFlags & loadObjectsFlag) {
				loadContainerNodes(saveGame);
				loadFlags |= loadContainerNodesFlag;
			} else
				error("ContainerNodes loaded prematurely");
			break;
#endif
		}

		notEOF = nextChunk(in, id, chunkSize);
	}

	delete in;

	if (!(loadFlags & loadGlobalsFlag))
		error("Globals not loaded");

	if (!(loadFlags & loadTimerFlag))
		error("Timer not loaded");

	if (!(loadFlags & loadCalenderFlag))
		error("Game calender not loaded");

	if (!(loadFlags & loadWorldsFlag))
		error("Worlds not loaded");

	if (!(loadFlags & loadObjectsFlag))
		error("Objects not loaded");

	if (!(loadFlags & loadActorsFlag))
		error("Actors not loaded");

	if (!(loadFlags & loadPlayerActorsFlag))
		error("Player actors not loaded");

	if (!(loadFlags & loadCenterActorFlag))
		error("Center actor not loaded");

	if (!(loadFlags & loadActiveItemStatesFlag))
		error("Active item states not loaded");

	if (!(loadFlags & loadTileCyclingStatesFlag))
		error("Tile cycling states not loaded");

	if (!(loadFlags & loadSAGADataSegFlag))
		error("SAGA data segment not loaded");

	if (!(loadFlags & loadSAGAThreadsFlag))
		error("SAGA threads not loaded");

	if (!(loadFlags & loadActiveRegionsFlag))
		error("Active Regions not loaded");


	resumeTimer();
}

//----------------------------------------------------------------------
//	Cleanup the game state

void cleanupGameState(void) {
	cleanupContainerNodes();
	cleanupPaletteState();
	cleanupUIState();
	cleanupAutoMap();
	cleanupSpellState();
	cleanupTileModeState();
	cleanupFactionTallies();
	cleanupMissions();
	cleanupTempActorCount();
	cleanupSensors();
	cleanupTimers();
	cleanupActiveRegions();
	cleanupSpeechTasks();
	cleanupTileTasks();
	cleanupTasks();
	cleanupTaskStacks();
	cleanupMotionTasks();
	cleanupSAGAThreads();
	cleanupSAGADataSeg();
	cleanupTileCyclingStates();
	cleanupActiveItemStates();
	cleanupCenterActor();
	cleanupPlayerActors();
	cleanupBands();
	cleanupObjects();
	cleanupActors();
	cleanupWorlds();
	cleanupAudio();
	cleanupTimer();
	cleanupGlobals();
}

//#define DEBUG_FILETIME

#ifdef DEBUG_FILETIME
#include <time.h>
#endif

void checkRestartGame(const char *exeName) {
#if 0
	char saveRestart[260];
	getSaveFileName(999, saveRestart);
	if (!fileExists(saveRestart) ||
	        (getFileDate(exeName) > getFileDate(saveRestart)))
		saveGameState(999, saveRestart);
#endif

	warning("STUB: checkRestartGame()");
}


void loadRestartGame(void) {
	loadSavedGameState(999);
}

} // end of namespace Saga2
