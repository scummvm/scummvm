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
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "common/debug.h"
#include "common/events.h"
#include "common/memstream.h"

#include "engines/advancedDetector.h"

#include "saga2/detection.h"
#include "saga2/saga2.h"
#include "saga2/setup.h"
#include "saga2/transit.h"
#include "saga2/player.h"
#include "saga2/tile.h"
#include "saga2/messager.h"
#include "saga2/intrface.h"
#include "saga2/script.h"
#include "saga2/localize.h"
#include "saga2/mainmap.h"
#include "saga2/display.h"
#include "saga2/tower.h"
#include "saga2/tromode.h"
#include "saga2/saveload.h"
#include "saga2/gamerate.h"
#include "saga2/modal.h"
#include "saga2/msgbox.h"
#include "saga2/grabinfo.h"

namespace Saga2 {

/* ===================================================================== *
   Optional Debugging Code
 * ===================================================================== */

extern char            *gameTimeStr;
extern bool             underground;
extern char             commandLineHelp[];
extern hResContext     *tileRes;       // tile resource handle
extern hResContext     *listRes;

/* ===================================================================== *
   Globals
 * ===================================================================== */

// command line options
bool cliWriteStatusF    = false;
bool cliScriptDebug     = false;
bool cliSpeechText      = false;
bool cliDrawInv         = false;
uint32 cliMemory        = 0;

//  Display variables
BackWindow              *mainWindow;            // main window...

//  Global game state
bool                    allPlayerActorsDead = false;
//bool                  graphicsInit = false;   // true if graphics init OK
bool                    checkExit = false;      // true while game running
int                     gameKiller = 0;         // will contain the exception that ends the game

//  Resource files
hResource               *resFile,               // main resource file
                        *objResFile,            // object resource file
                        *auxResFile,            // auxiliary data resource file
                        *scriptResFile,         // script resources
                        *soundResFile,
                        *voiceResFile;          // sound resources

//  Import list from resource file.
ResImportTable          *resImports;

// x location of status lines
uint16      writeStatusFX = 468;
uint16      writeStatusFY = 354;

/* ===================================================================== *
   Locals
 * ===================================================================== */

// game states
static bool             cleanExit = true;
bool                    gameInitialized = false;        // true when game initialized
bool                    fullInitialized = false;
bool                    delayReDraw = false;

/* ===================================================================== *
   Debug
 * ===================================================================== */
// frame counting
uint32                  frames = 0;
static uint32           loops = 0;
static uint32           elapsed = 0;
static uint32           lastGameTime = 0;

// message handlers
static pMessager Status[10];
static pMessager Status2[10];

pMessager ratemess[3];

/* ===================================================================== *
   Prototypes
 * ===================================================================== */

bool readCommandLine(int argc, char *argv[]);
void findProgramDir(char *argv);     // save program home directory

APPFUNC(cmdWindowFunc);                      // main window event handler

//  Exportable prototypes
void SystemEventLoop();

void runPathFinder();

void updateMainDisplay();

const char *getExeFromCommandLine(int argc, char *argv[]);
void WriteStatusF2(int16 line, const char *msg, ...);
bool initUserDialog();
void cleanupUserDialog();
int16 OptionsDialog(bool disableSaveResume = false);

static void mainLoop(bool &cleanExit, int argc, char *argv[]);
void displayUpdate();
void showDebugMessages();

bool initResourceHandles();
bool initDisplayPort();
bool initPanelSystem();
bool initDisplay();
bool initGameMaps();

/********************************************************************/
/*                                                                  */
/* MAIN FUNCTION                                                    */
/*                                                                  */
/********************************************************************/
void termFaultHandler();

void main_saga2() {
	gameInitialized = false;

	mainDisable();
	initCleanup();

	//  parse command-line arguments and store results
//	if (!readCommandLine(argc, argv))
//		abortMain;

	//  call the initialization code
	gameInitialized = initializeGame();
	cleanExit = gameInitialized;

	if (gameInitialized)
		mainLoop(cleanExit, 0, nullptr);

	shutdownGame();
	gameInitialized = false;
}

// ------------------------------------------------------------------------
// Inner chunk of main - this bizarre nesting is required because VC++
// doesn't like  try{} catch(){ } blocks in the same routine as its
// __try{} __except(){} blocks

static void mainLoop(bool &cleanExit_, int argc, char *argv[]) {
	const char *exeFile = getExeFromCommandLine(argc, argv);
	if (displayEnabled())
		displayUpdate();
	checkRestartGame(exeFile);
	fullInitialized = true;
	EventLoop(g_vm->_gameRunning, false);
}

/********************************************************************/
/*                                                                  */
/* INITIALIZATION and CLEANUP CODE                                  */
/*                                                                  */
/********************************************************************/

//
// Note: the bulk of the Initialization & cleanup routines have
//   been moved to TOWERFTA.CPP. This file together with
//   TOWER.CPP automate initialization & cleanup. This is needed
//   to accommodate differences in system startup between
//   the windows & DOS versions
//
//

// ------------------------------------------------------------------------
// Game setup function

bool setupGame() {
	g_vm->_frate = new frameSmoother(kFrameRate, TICKSPERSECOND, gameTime);
	g_vm->_lrate = new frameCounter(TICKSPERSECOND, gameTime);

	return programInit();
}

// ------------------------------------------------------------------------
// Game cleanup function

void cleanupGame() {
	delete g_vm->_frate;
	delete g_vm->_lrate;

	programTerm();
}


/********************************************************************/
/*                                                                  */
/* EVENT LOOP HANDLING                                              */
/*                                                                  */
/********************************************************************/

void processEventLoop(bool updateScreen = true);

//-----------------------------------------------------------------------
//	Main loop

void EventLoop(bool &running, bool) {
	//  Our typical main loop
	while (running && g_vm->_gameRunning)
		processEventLoop(displayEnabled());
}

//-----------------------------------------------------------------------
//	Main event which does everything (including handle user input)

void dumpGBASE(char *msg);

void processEventLoop(bool updateScreen) {

	debugC(1, kDebugEventLoop, "EventLoop: starting event loop");

	if (g_vm->shouldQuit()) {
		//g_vm->_gameRunning=false;
		endGame();
		return;
	}

	debugC(1, kDebugEventLoop, "EventLoop: audio event loop");
	audioEventLoop();

	debugC(1, kDebugEventLoop, "EventLoop: game mode update");
	if (GameMode::_newmodeFlag)
		GameMode::update();

	Common::Event event;
	while (g_vm->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_RBUTTONDOWN:
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONUP:
		case Common::EVENT_MOUSEMOVE:
			g_vm->_toolBase->handleMouse(event, g_system->getMillis());
			break;
		case Common::EVENT_KEYDOWN:
			g_vm->_toolBase->handleKeyStroke(event);
			break;
		case Common::EVENT_QUIT:
			if (verifyUserExit())
				endGame();
			break;
		default:
			break;
		}
	}
	//if(!running) return; // This Is No Tasks Are Done After Saving Game

	debugC(1, kDebugEventLoop, "EventLoop: timer update");
	//  Handle the timer events
	//  REM: Causing code corruption in windows for some reason...
	g_vm->_toolBase->handleTimerTick(gameTime >> 2);

	//  Handle updating of the display.
	debugC(1, kDebugEventLoop, "EventLoop: display update");
	if (!g_vm->checkVideo()) {
		displayUpdate();
	}

	if (allPlayerActorsDead) {
		allPlayerActorsDead = false;
		setLostroMode();
	}
}

void displayUpdate() {
	if (displayEnabled()) { //updateScreen)
		//debugC(1, kDebugEventLoop, "EventLoop: daytime transition update loop");
		dayNightUpdate();
		//debugC(1, kDebugEventLoop, "EventLoop: Game mode handle task");
		GameMode::_modeStackPtr[GameMode::_modeStackCtr - 1]->_handleTask();
		g_vm->_lrate->updateFrameCount();
		loops++;
		elapsed += (gameTime - lastGameTime);
		lastGameTime = gameTime;

		if (g_vm->getGameId() == GID_FTA2) {
			debugC(1, kDebugEventLoop, "EventLoop: Interface indicator updates");
			updateIndicators();
		}

		g_system->updateScreen();
		g_system->delayMillis(10);

		if (delayReDraw)
			reDrawScreen();
		//  Call asynchronous resource loader
		debugC(1, kDebugEventLoop, "EventLoop: resource update");

		audioEventLoop();

		//  Call the asynchronous path finder
		debugC(1, kDebugEventLoop, "EventLoop: pathfinder update");
		runPathFinder();

		showDebugMessages();
	}
}

void showDebugMessages() {
	if (g_vm->_showPosition) {
		TilePoint p = centerActorCoords();
		WriteStatusF2(0, "Position: %d, %d, %d", p.u, p.v, p.z);
	}

	if (g_vm->_showStats) {
		ObjectID objID = g_vm->_mouseInfo->getObjectId();
		GameObject *obj = GameObject::objectAddress(objID);

		if (ProtoObj *p = obj->proto()) {
			WriteStatusF2(1, "%s (%d)", obj->objName(), objID);
			WriteStatusF2(2, "dmg = %d", p->weaponDamage);
			WriteStatusF2(3, "firerate = %d", p->weaponFireRate);
			WriteStatusF2(4, "maximumRange = %d", p->maximumRange);
			WriteStatusF2(5, "dmgAbsorbtion = %d", p->damageAbsorbtion);
			WriteStatusF2(6, "dmgDivider = %d", p->damageDivider);
			WriteStatusF2(7, "defenseBonus = %d", p->defenseBonus);
			WriteStatusF2(8, "maxCharges = %d", p->maxCharges);
			WriteStatusF2(9, "price = %d", p->price);
		}
	}
}

/* ===================================================================== *
   Abbreviated event loop
 * ===================================================================== */

void SystemEventLoop() {
	if (
#ifdef DO_OUTRO_IN_CLEANUP
	    whichOutro == -1 &&
#endif
	    !g_vm->_gameRunning)
		TroModeExternEvent();

	Common::Event event;
	while (g_vm->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONUP:
		case Common::EVENT_KEYDOWN:
		case Common::EVENT_QUIT:
			TroModeExternEvent();
			break;
		default:
			break;
		}
	}

	g_system->updateScreen();
	g_system->delayMillis(10);
}

/********************************************************************/
/*                                                                  */
/* COMMAND LINE PARSING WITHOUT CRASHES                             */
/*                                                                  */
/********************************************************************/

// ------------------------------------------------------------------------
// Determines the EXE file executed from command line info

const char *getExeFromCommandLine(int argc, char *argv[]) {
	if (argv == nullptr)
		return "scummvm";
	return argv[0];
}

// ------------------------------------------------------------------------
// Adds error handling to command line parsing

bool readCommandLine(int argc, char *argv[]) {
	parseCommandLine(argc, argv);

	return true;
}

/********************************************************************/
/*                                                                  */
/* MOUSE EVENT QUEUE                                                */
/*                                                                  */
/********************************************************************/

// ------------------------------------------------------------------------
// clears any queued input (mouse AND keyboard)
void resetInputDevices() {
	Common::Event event;
	while (g_vm->getEventManager()->pollEvent(event));
}

/********************************************************************/
/*                                                                  */
/* RESOURCE MANAGEMENT CODE                                         */
/*                                                                  */
/********************************************************************/

//-----------------------------------------------------------------------
//	Opens a file using simple DOS i/o, allocates a buffer the same size
//	as the file, and loads the file into the buffer

void *LoadFile(char *filename, const char desc[]) {
#if 0
	int             fHandle;                // file handle
	struct stat     fileStat;               // stat structure
	uint8           *buffer;                // allocated buffer

	//  Open the file
	if ((fHandle = open(filename, O_RDONLY)) < 0)
		error("Error opening %s", filename);

	//  Determine size of file
	if (fstat(fHandle, &fileStat) < 0)
		error("Error accessing %s", filename);

	//  Allocate the buffer
	buffer = (uint8 *)mustAlloc(fileStat.st_size, desc);

	//  Read file into buffer
	if (read(fHandle, buffer, fileStat.st_size) < 0)
		error("Error reading %s", filename);

	//  Close file and return
	close(fHandle);
	return buffer;
#endif
	warning("STUB: LoadFile(%s)", filename);
	return nullptr;
}

//-----------------------------------------------------------------------
//	Loads a resource into a buffer and returns a pointer

void *LoadResource(hResContext *con, uint32 id, const char desc[]) {
	int32           size;
	uint8           *buffer;                // allocated buffer

	debugC(3, kDebugResources, "LoadResource(): Loading resource %d (%s, %s)", id, tag2str(id), desc);

	size = con->size(id);
	if (size <= 0 || !con->seek(id)) {
		error("LoadResource(): Error reading resource ID '%s'.", tag2str(id));
	}

	//  Allocate the buffer
	buffer = (uint8 *)malloc(size);
	con->read(buffer, size);
	con->rest();

	return buffer;
}

Common::SeekableReadStream *loadResourceToStream(hResContext *con, uint32 id, const char desc[]) {
	int32           size;
	uint8           *buffer;                // allocated buffer

	debugC(3, kDebugResources, "loadResourceToStream(): Loading resource %d (%s, %s)", id, tag2str(id), desc);

	size = con->size(id);
	if (size <= 0 || !con->seek(id)) {
		warning("loadResourceToStream(): Error reading resource ID '%s'.", tag2str(id));
		return nullptr;
	}

	//  Allocate the buffer
	buffer = (uint8 *)malloc(size);
	con->read(buffer, size);
	con->rest();

	return new Common::MemoryReadStream(buffer, size, DisposeAfterUse::YES);
}

void dumpResource(hResContext *con, uint32 id) {
	int32 size = con->size(id);
	if (size <= 0 || !con->seek(id)) {
		error("dumpResource(): Error reading resource ID '%s'.", tag2str(id));
	}

	byte *buffer = (byte *)malloc(size);
	con->read(buffer, size);
	con->rest();

	Common::DumpFile out;

	Common::Path path(Common::String::format("./dumps/mus%s.dat", tag2strP(id)));

	if (out.open(path, true)) {
		out.write(buffer, size);
		out.flush();
		out.close();
	}

	free(buffer);
}

typedef hResource *pHResource;

inline char drive(char *path) {
	return (path[0] % 32);
}

//-----------------------------------------------------------------------
//	Routine to initialize an arbitrary resource file

static bool openResource(pHResource &hr, const char *fileName) {
	if (hr)
		delete hr;
	hr = nullptr;

	hr = new hResource(fileName);

	while (hr == nullptr || !hr->_valid) {
		if (hr) delete hr;
		hr = nullptr;
		hr = new hResource(fileName);
	}

	if (hr == nullptr || !hr->_valid) {
		error("openResource: Cannot open resource: %s", fileName);
//		return false;
	}
	return true;
}

//-----------------------------------------------------------------------
//	Routine to initialize all the resource files

bool openResources() {
	for (const ADGameFileDescription *desc = g_vm->getFilesDescriptions(); desc->fileName; desc++) {
		bool res = true;

		switch (desc->fileType) {
		case GAME_RESOURCEFILE:
			res = openResource(auxResFile, desc->fileName);
			break;
		case GAME_OBJRESOURCEFILE:
			res = openResource(objResFile, desc->fileName);
			break;
		case GAME_SCRIPTFILE:
			res = openResource(scriptResFile, desc->fileName);
			break;
		case GAME_SOUNDFILE:
			res = openResource(soundResFile, desc->fileName);
			// Dinotopia contains both sound and voices in the same file
			if (g_vm->getGameId() == GID_DINO)
				res = openResource(voiceResFile, desc->fileName);
			break;
		case GAME_IMAGEFILE:
			res = openResource(resFile, desc->fileName);
			break;
		case GAME_VOICEFILE:
			res = openResource(voiceResFile, desc->fileName);
			break;
		default:
			break;
		}

		if (!res)
			return false;
	}

	return true;
}

//-----------------------------------------------------------------------
//	Routine to cleanup all the resource files

void closeResources() {
	delete soundResFile;
	soundResFile = nullptr;
	delete voiceResFile;
	voiceResFile = nullptr;
	delete scriptResFile;
	scriptResFile = nullptr;
	delete auxResFile;
	auxResFile = nullptr;
	delete objResFile;
	objResFile = nullptr;
	delete resFile;
	resFile = nullptr;
}

/********************************************************************/
/*                                                                  */
/* GLOBAL DATA SAVE / RESTORE                                       */
/*                                                                  */
/********************************************************************/

extern int32        objectIndex,
       actorIndex;
extern bool         brotherBandingEnabled,
       centerActorIndicatorEnabled,
       interruptableMotionsPaused,
       objectStatesPaused,
       actorTasksPaused,
       backgroundSimulationPaused;

//-----------------------------------------------------------------------
//	Assign initial values to miscellaneous globals

void initGlobals() {
	objectIndex = 0;
	actorIndex = 0;
	brotherBandingEnabled = true;
	centerActorIndicatorEnabled = false;
	interruptableMotionsPaused = false;
	objectStatesPaused = false;
	g_vm->_act->_actorStatesPaused = false;
	actorTasksPaused = false;
	g_vm->_act->_combatBehaviorEnabled = false;
	backgroundSimulationPaused = false;
}

void saveGlobals(Common::OutSaveFile *outS) {
	debugC(2, kDebugSaveload, "Saving globals");

	outS->write("GLOB", 4);
	CHUNK_BEGIN;
	out->writeUint32LE(objectIndex);
	out->writeUint32LE(actorIndex);
	out->writeUint16LE(brotherBandingEnabled);
	out->writeUint16LE(centerActorIndicatorEnabled);
	out->writeUint16LE(interruptableMotionsPaused);
	out->writeUint16LE(objectStatesPaused);
	out->writeUint16LE(g_vm->_act->_actorStatesPaused);
	out->writeUint16LE(actorTasksPaused);
	out->writeUint16LE(g_vm->_act->_combatBehaviorEnabled);
	out->writeUint16LE(backgroundSimulationPaused);
	CHUNK_END;

	debugC(3, kDebugSaveload, "... objectIndex = %d", objectIndex);
	debugC(3, kDebugSaveload, "... actorIndex = %d", actorIndex);
	debugC(3, kDebugSaveload, "... brotherBandingEnabled = %d", brotherBandingEnabled);
	debugC(3, kDebugSaveload, "... centerActorIndicatorEnabled = %d", centerActorIndicatorEnabled);
	debugC(3, kDebugSaveload, "... interruptableMotionsPaused = %d", interruptableMotionsPaused);
	debugC(3, kDebugSaveload, "... objectStatesPaused = %d", objectStatesPaused);
	debugC(3, kDebugSaveload, "... g_vm->_act->_actorStatesPaused = %d", g_vm->_act->_actorStatesPaused);
	debugC(3, kDebugSaveload, "... actorTasksPaused = %d", actorTasksPaused);
	debugC(3, kDebugSaveload, "... g_vm->_act->_combatBehaviorEnabled = %d", g_vm->_act->_combatBehaviorEnabled);
	debugC(3, kDebugSaveload, "... backgroundSimulationPaused = %d", backgroundSimulationPaused);
}

void loadGlobals(Common::InSaveFile *in) {
	debugC(2, kDebugSaveload, "Loading globals");

	objectIndex = in->readUint32LE();
	actorIndex = in->readUint32LE();
	brotherBandingEnabled = in->readUint16LE();
	centerActorIndicatorEnabled = in->readUint16LE();
	interruptableMotionsPaused = in->readUint16LE();
	objectStatesPaused = in->readUint16LE();
	g_vm->_act->_actorStatesPaused = in->readUint16LE();
	actorTasksPaused = in->readUint16LE();
	g_vm->_act->_combatBehaviorEnabled = in->readUint16LE();
	backgroundSimulationPaused = in->readUint16LE();

	debugC(3, kDebugSaveload, "... objectIndex = %d", objectIndex);
	debugC(3, kDebugSaveload, "... actorIndex = %d", actorIndex);
	debugC(3, kDebugSaveload, "... brotherBandingEnabled = %d", brotherBandingEnabled);
	debugC(3, kDebugSaveload, "... centerActorIndicatorEnabled = %d", centerActorIndicatorEnabled);
	debugC(3, kDebugSaveload, "... interruptableMotionsPaused = %d", interruptableMotionsPaused);
	debugC(3, kDebugSaveload, "... objectStatesPaused = %d", objectStatesPaused);
	debugC(3, kDebugSaveload, "... g_vm->_act->_actorStatesPaused = %d", g_vm->_act->_actorStatesPaused);
	debugC(3, kDebugSaveload, "... actorTasksPaused = %d", actorTasksPaused);
	debugC(3, kDebugSaveload, "... g_vm->_act->_combatBehaviorEnabled = %d", g_vm->_act->_combatBehaviorEnabled);
	debugC(3, kDebugSaveload, "... backgroundSimulationPaused = %d", backgroundSimulationPaused);
}

/********************************************************************/
/*                                                                  */
/* ERROR / MESSAGE HANDLING                                         */
/*                                                                  */
/********************************************************************/

// ------------------------------------------------------------------------
// pops up a window to see if the user really wants to exit

bool verifyUserExit() {
	if (!g_vm->_gameRunning)
		return true;
	if (FTAMessageBox("Are you sure you want to exit", ERROR_YE_BUTTON, ERROR_NO_BUTTON) != 0)
		return true;
	return false;
}

//-----------------------------------------------------------------------
//	Allocate visual messagers

bool initGUIMessagers() {
	initUserDialog();
	for (int i = 0; i < 10; i++) {
		char debItem[16];
		Common::sprintf_s(debItem, "Status%1.1d", i);
		Status[i] = new StatusLineMessager(debItem, i, &g_vm->_mainPort);
		if (Status[i] == nullptr)
			return false;
		Common::sprintf_s(debItem, "Status%2.2d", i + 10);
		Status2[i] = new StatusLineMessager(debItem, i, &g_vm->_mainPort, 20, 21 + (11 * i));
	}
	for (int j = 0; j < 3; j++)
		ratemess[j] = new StatusLineMessager("FrameRates", j, &g_vm->_mainPort, 5, 450 + (11 * j), 500);
	return true;
}

//-----------------------------------------------------------------------
//	cleanup visual messagers

void cleanupGUIMessagers() {
	for (int i = 0; i < 10; i++) {
		if (Status[i]) delete Status[i];
		Status[i] = nullptr;
		if (Status2[i]) delete Status2[i];
		Status2[i] = nullptr;
	}
	cleanupUserDialog();
}

//-----------------------------------------------------------------------
//	Debugging status functions

#ifdef  WriteStatus
void WriteStatusF(int16 line, const char *msg, ...) {
	if (!g_vm->_showStatusMsg)
		return;

	va_list         argptr;
	if (displayEnabled()) {
		va_start(argptr, msg);
		if (line > 9) {
			if (Status2[line - 10])
				Status2[line - 10]->va(msg, argptr);
		} else {
			if (Status[line])
				Status[line]->va(msg, argptr);
		}
		va_end(argptr);
	}
}

void WriteStatusF2(int16 line, const char *msg, ...) {
	if (!g_vm->_showStatusMsg)
		return;

	va_list         argptr;
	if (displayEnabled()) {
		va_start(argptr, msg);
		if (Status2[line])
			Status2[line]->va(msg, argptr);
		va_end(argptr);
	}
}
#else
void WriteStatusF(int16, const char *, ...) {}
void WriteStatusF2(int16, const char *, ...) {}
#endif

//---------------------------------------------------------
// Game performance can be used as a gauge of how much
//   CPU time is available. We'd like to keep the retu

int32 currentGamePerformance() {
	int32 framePer = 100;
	int32 lval = int(g_vm->_lrate->frameStat());
	int32 fval = int(g_vm->_lrate->frameStat(kGRFramesPerSecond));
	if (fval >= kFrameRate && lval > fval) {
		framePer += (50 * ((lval - fval) / fval));
	} else {
		framePer = (100 * g_vm->_frate->frameStat(kGRFramesPerSecond)) / kFrameRate;
	}
	framePer = clamp(10, framePer, 240);
	return framePer;
}


void updateFrameCount() {
	g_vm->_frate->updateFrameCount();
}

int32 eloopsPerSecond = 0;
int32 framesPerSecond = 0;

int32 gamePerformance() {
	if (framesPerSecond < kFrameRate) {
		return (100 * framesPerSecond) / kFrameRate;
	}
	if (framesPerSecond == kFrameRate)
		return 100;
	return 100 + 50 * (eloopsPerSecond - kFrameRate) / kFrameRate;

}


/********************************************************************/
/*                                                                  */
/* APPFUNC FOR MAIN WINDOW                                          */
/*                                                                  */
/********************************************************************/

//-----------------------------------------------------------------------
//	Function to handle miscellaneous events to the window.
//	Any panel events which are not handled by individual panels
//	are sent to this function.

APPFUNC(cmdWindowFunc) {
	int16           key, qual;

	switch (ev.eventType) {
	case kEventKeyDown:
		key = ev.value & 0xffff;
		qual = ev.value >> 16;

		GameMode::_modeStackPtr[GameMode::_modeStackCtr - 1]->_handleKey(key, qual);
		break;

	default:
		break;
	}
}

} // end of namespace Saga2
