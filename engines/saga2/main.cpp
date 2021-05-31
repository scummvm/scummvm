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

#include "common/debug.h"

#include "saga2/std.h"

#include "saga2/rmemfta.h"
#include "saga2/fta.h"

#include "saga2/setup.h"
#include "saga2/transit.h"
#include "saga2/player.h"
#include "saga2/calender.h"
#include "saga2/tile.h"
#include "saga2/messager.h"
#include "saga2/intrface.h"
#include "saga2/script.h"
#include "saga2/localize.h"
#include "saga2/mainmap.h"
#include "saga2/display.h"
#include "saga2/tower.h"
#include "saga2/tromode.h"
#include "saga2/loadmsg.h"
#include "saga2/ioerrors.h"
#include "saga2/loadsave.h"
#include "saga2/gamerate.h"
#include "saga2/msgbox.h"
#include "saga2/script.h"

namespace Saga2 {

/* ===================================================================== *
   Optional Debugging Code
 * ===================================================================== */

// enable the following to display event loop processing
#define DEBUG_LOOP 0

/* ===================================================================== *
   Constants
 * ===================================================================== */

const   uint32  gameTimeID  = MKTAG('T', 'I', 'M', 'E');

/* ===================================================================== *
   Imports
 * ===================================================================== */

extern WindowDecoration autoMapDecorations[];
extern gToolBase        G_BASE;
extern configuration    globalConfig;
extern char             *gameTimeStr;
extern bool             underground;
extern char             commandLineHelp[];
extern hResContext        *tileRes;       // tile resource handle
extern hResContext        *listRes;

/* ===================================================================== *
   Globals
 * ===================================================================== */

// command line options
bool cliWriteStatusF    = false;
bool cliScriptDebug     = false;
bool cliSpeechText      = false;
bool cliDrawInv         = false;
uint32 cliMemory        = 0;

//  User-interface variables
gMouseState             mouseState;

//  Display variables
gDisplayPort            mainPort;               // default rendering port
gMousePointer           pointer(mainPort);   // the actual pointer
BackWindow              *mainWindow;            // main window...

//  Memory allocation heap
long                    memorySize = 8000000L;

//  Global game state
bool                    gameRunning = true;     // true while game running
bool                    allPlayerActorsDead = false;
//bool                  graphicsInit = false;   // true if graphics init OK
bool                    checkExit = false;      // true while game running
int                     gameKiller = 0;         // will contain the exception that ends the game

//  Resource files
hResource               *resFile,               // main resource file
                        *objResFile,            // object resource file
                        *auxResFile,            // auxillary data resource file
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

// main heap
static uint8            *heapMemory;


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

#if 1
frameSmoother frate(frameRate, TICKSPERSECOND, gameTime);
//frameSmoother lrate(frameRate,TICKSPERSECOND,gameTime);
frameCounter lrate(TICKSPERSECOND, gameTime);
frameCounter irate(TICKSPERSECOND, gameTime);
#else
frameCounter frate(TICKSPERSECOND, gameTime);
frameCounter lrate(TICKSPERSECOND, gameTime);
frameCounter irate(TICKSPERSECOND, gameTime);
#endif

/* ===================================================================== *
   Prototypes
 * ===================================================================== */

bool readCommandLine(int argc, char *argv[]);
void findProgramDir(char *argv);     // save program home directory

APPFUNC(cmdWindowFunc);                      // main window event handler

//  Exportable prototypes
void EventLoop(bool &running, bool modal);           // handles input and distributes
void SystemEventLoop(void);

void ClearMessageQueue(void);
void SystemMouseHandler(void);
void SystemKeyHandler(short, short);
void waitForVideoFile(char *fileName);

void memtest(void);
void runPathFinder(void);

bool setupGame(void);

void mainEnable(void);
void mainDisable(void);
void lightsOut(void);

void cleanupGame(void);                  // auto-cleanup function
void RShowMem(void);
void parseCommandLine(int argc, char *argv[]);
char *getExeFromCommandLine(int argc, char *argv[]);
void updateMouse(void);
void WriteStatusF2(int16 line, const char *msg, ...);
bool initUserDialog(void);
void cleanupUserDialog(void);
int16 OptionsDialog(bool disableSaveResume = false);

static void mainLoop(bool &cleanExit, int argc, char *argv[]);
void displayUpdate(void);
//EO//#if DEBUG
void updatePerfStats(void);
//EO//#endif

/********************************************************************/
/*                                                                  */
/* MAIN FUNCTION                                                    */
/*                                                                  */
/********************************************************************/
void termFaultHandler(void);

MAIN_RETURN_TYPE main_saga2() {
	gameInitialized = false;

	mainDisable();
	initCleanup();

	//  parse command-line arguments and store results
//	if (!readCommandLine(argc, argv))
//		abortMain;

	//  call the initialization code
	gameInitialized = initializeGame();
	cleanExit = gameInitialized;

	if (gameInitialized) {
		mainLoop(cleanExit, 0, NULL);
	}

	shutdownGame();
	gameInitialized = false;

	if (cleanExit)
		exitMain;
	else
		abortMain;
}

// ------------------------------------------------------------------------
// Inner chunk of main - this bizzare nesting is required because VC++
// doesn't like  try{} catch(){ } blocks in the same routine as its
// __try{} __except(){} blocks
void updateActiveRegions(void);

static void mainLoop(bool &cleanExit, int argc, char *argv[]) {
	char *exeFile = getExeFromCommandLine(argc, argv);
	if (displayEnabled())
		displayUpdate();
	checkRestartGame(exeFile);
	fullInitialized = true;
	EventLoop(gameRunning, false);
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
//   to accomodate differences in system startup between
//   the windows & DOS versions
//
//

// ------------------------------------------------------------------------
// Game setup function

bool setupGame(void) {
	return programInit();
}

// ------------------------------------------------------------------------
// Game cleanup function

void cleanupGame(void) {
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
	while (running && gameRunning)
		processEventLoop(displayEnabled());
}

//-----------------------------------------------------------------------
//	Main event which does everything (including handle user input)

void dumpGBASE(char *msg);

void processEventLoop(bool updateScreen) {

	int         key, qual;

	debugC(1, kDebugEventLoop, "EventLoop: starting event loop");
	irate.updateFrameCount();

	debugC(1, kDebugEventLoop, "EventLoop: checking user abort");
	breakEventLoop();
	if (checkExit && verifyUserExit()) {
		//gameRunning=false;
		endGame();
		return;
	}

	debugC(1, kDebugEventLoop, "EventLoop: handle win messages");
	if (handlingMessages())
		return;

	debugC(1, kDebugEventLoop, "EventLoop: check for game suspend");
	if (gameSuspended())
		return;

	debugC(1, kDebugEventLoop, "EventLoop: audio event loop");
	audioEventLoop();

	debugC(1, kDebugEventLoop, "EventLoop: game mode update");
	if (GameMode::newmodeFlag)
		GameMode::update();

	debugC(1, kDebugEventLoop, "EventLoop: mouse update");
	updateMouse();

	debugC(1, kDebugEventLoop, "EventLoop: keyboard update");
	if (ReadKeyboard(key, qual)) {
		G_BASE.handleKeyStroke(key, qual);
	}

	//if(!running) return; // This Is No Tasks Are Done After Saving Game

	debugC(1, kDebugEventLoop, "EventLoop: timer update");
	//  Handle the timer events
	//  REM: Causing code corruption in windows for some reason...
	G_BASE.handleTimerTick(gameTime >> 2);

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

void displayUpdate(void) {
	if (displayEnabled()) { //updateScreen)
		//debugC(1, kDebugEventLoop, "EventLoop: daytime transition update loop");
		dayNightUpdate();
		//debugC(1, kDebugEventLoop, "EventLoop: Game mode handle task");
		GameMode::modeStackPtr[GameMode::modeStackCtr - 1]->handleTask();
		lrate.updateFrameCount();
		loops++;
		elapsed += (gameTime - lastGameTime);
		lastGameTime = gameTime;


		debugC(1, kDebugEventLoop, "EventLoop: Interface indicator updates");
		updateIndicators();

		debugC(1, kDebugEventLoop, "EventLoop: OS specific display routines");
		displayEventLoop();
		if (delayReDraw)
			reDrawScreen();
		//  Call asynchronous resource loader
		debugC(1, kDebugEventLoop, "EventLoop: resource update");
		loadAsyncResources();

		audioEventLoop();

		//  Call the asynchronous path finder
		debugC(1, kDebugEventLoop, "EventLoop: pathfinder update");
		runPathFinder();

		//  Hows the game running?
		debugC(1, kDebugEventLoop, "EventLoop: updating stats");
		updatePerfStats();

	}
}

/* ===================================================================== *
   Abbreviated event loop
 * ===================================================================== */

void SystemEventLoop(void) {
	int         key, qual;

	//make sure the user hasn't quit on us
	breakEventLoop();
	if (
#ifdef DO_OUTRO_IN_CLEANUP
	    whichOutro == -1 &&
#endif
	    !gameRunning)
		TroModeExternEvent();

	// check for messages
	if (handlingMessages())
		return;

	// check mouse
	SystemMouseHandler();

	// check keyboard
	if (ReadKeyboard(key, qual)) {
		SystemKeyHandler(key, qual);
	}

	displayEventLoop();
}

/********************************************************************/
/*                                                                  */
/* COMMAND LINE PARSING WITHOUT CRASHES                             */
/*                                                                  */
/********************************************************************/

// ------------------------------------------------------------------------
// Determines the EXE file executed from command line info

char *getExeFromCommandLine(int argc, char *argv[]) {
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
// Mouse handling

gMouseState     prevState;
MouseExtState   mouseQueue[ 64 ];

int16           queueIn = 0,
                queueOut = 0;

inline int BUMP(int x) {
	return (x + 1) & 63;
}

void ClearMessageQueue(void) {
	while (handlingMessages());
}

// ------------------------------------------------------------------------
// clears any queued input (mouse AND keyboard)

void resetInputDevices(void) {
	int         key, qual;
	ClearMessageQueue();
	while (ReadKeyboard(key, qual)) ;
	PollMouse();
	Forbid();                       // shut of tasking while we check the queue
	while (queueOut != queueIn)
		queueOut = BUMP(queueOut);
	Permit();                       // turn on tasking again
}

//-----------------------------------------------------------------------

void updateMouse(void) {
	Point16 moveto = Point16(-1, -1);

	PollMouse();

	//  Empty the mouse queue of mouse events, and feed them
	//  to the panel system.
	Forbid();                       // shut of tasking while we check the queue
	while (queueOut != queueIn) {
		int16   queueIndex = queueOut;

		queueOut = BUMP(queueOut);
		Permit();                   // turn on tasking while handling message
		moveto = mouseQueue[ queueIndex ].st.pos;
		G_BASE.handleMouse(mouseQueue[ queueIndex ].st, mouseQueue[ queueIndex ].time);
		Forbid();                   // turn off tasking again
	}
	Permit();                       // turn on tasking again
	//if ( moveto.x>-1 )
	//  pointer.move( moveto );
}


//-----------------------------------------------------------------------
//	Function to poll the mouse pointer when engine is busy, so that
//	we can properly detect a double-click.

void PollMouse(void) {
	mousePoll();
}

void AddMouseEvent(const gMouseState &mouseState) {
	//  Read the current mouse position.
	if (memcmp(&mouseState, &prevState, sizeof mouseState) == 0)
		return;

	if (displayEnabled())
		pointer.move(mouseState.pos);
	//else
	//  TroModeExternEvent();

	//  only add a new entry into the queue if the mouse buttons
	//  changed state.

	Forbid();                           //  Turn off tasking while adding event
	if (BUMP(queueIn) != queueOut) {
		prevState = mouseState;
		mouseQueue[ queueIn ].st = mouseState;
		mouseQueue[ queueIn ].time = ReadTimer();
		queueIn = BUMP(queueIn);
	}
	Permit();                           //  Re-enable tasking
}

// ------------------------------------------------------------------------
// keystroke code

void SystemKeyHandler(short, short) {
	TroModeExternEvent();
}

// ------------------------------------------------------------------------
// abbreviated mouse handler


void SystemMouseHandler(void) {
	PollMouse();

	//  Empty the mouse queue of mouse events, and feed them
	//  to the panel system.
	Forbid();                       // shut of tasking while we check the queue
	while (queueOut != queueIn) {
		int16   queueIndex = queueOut;

		queueOut = BUMP(queueOut);
		gMouseState &gms = mouseQueue[ queueIndex ].st;
		if (gms.left || gms.right)
			TroModeExternEvent();
	}
	Permit();                       // turn on tasking again
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
	uint32          idString[ 2 ];
	int32           size;
	uint8           *buffer;                // allocated buffer

	idString[ 0 ] = id;
	idString[ 1 ] = 0;
	debugC(3, kDebugResources, "Loading resource %d (%s, %s)", id, tag2str(id), desc);

	size = con->size(id);
	if (size <= 0 || !con->seek(id)) {
		error("Error reading resource ID '%s'.", &idString);
	}

	//  Allocate the buffer
	buffer = (uint8 *)malloc(size);
	con->read(buffer, size);
	con->rest();

	return buffer;
}

//-----------------------------------------------------------------------
//	Loads a resource into a relocatable buffer and returns a handle

RHANDLE LoadResourceToHandle(hResContext *con, uint32 id, const char desc[]) {
	uint32          idString[ 2 ];
	int32           size;
	RHANDLE         buffer;             // allocated buffer

	idString[ 0 ] = id;
	idString[ 1 ] = 0;

	size = con->size(id);
	if (size <= 0 || !con->seek(id)) {
		error("Error reading resource ID '%s'.", &idString);
	}

	//  Allocate the buffer
	buffer = mustAllocHandle(size, desc);
	con->read(*buffer, size);
	con->rest();

	return buffer;
}

typedef hResource *pHResource;

inline char drive(char *path) {
	return (path[0] % 32);
}

//-----------------------------------------------------------------------
//	Routine to initialize an arbitrary resource file

static bool openResource(
    pHResource &hr,      // resource to initialize
    char *basePath,      // path to data file
    char *defaultPath,   // backup path
    char *fileName,      // file name & extension
    char *description) {
	if (hr) delete hr;
	hr = NULL;

	hr = NEW_PRES hResource(fileName, defaultPath, description);

	while (hr == NULL || !hr->_valid) {
		if (hr) delete hr;
		hr = NULL;
		hr = NEW_PRES hResource(fileName, defaultPath, description);
	}

	if (hr == NULL || !hr->_valid) {
		error("openResource: Cannot open resource: %s, %s", fileName, description);
//		return false;
	}
	return true;
}

//-----------------------------------------------------------------------
//	Routine to initialize all the resource files

bool openResources(void) {

	if (
	    openResource(resFile,           globalConfig.imageResfilePath,
	                 "..\\resfile\\",  IMAGE_RESFILE,
	                 "Imagery resource file")      &&

	    openResource(objResFile,        globalConfig.mainResfilePath,
	                 "..\\resfile\\",  OBJECT_RESFILE,
	                 "Object resource file")      &&

	    openResource(auxResFile,        globalConfig.dataResfilePath,
	                 "..\\resfile\\",  AUX_RESFILE,
	                 "Data resource file")      &&

	    openResource(scriptResFile,     globalConfig.scriptResfilePath,
	                 "..\\scripts\\",  SCRIPT_RESFILE,
	                 "Script resource file")      &&
	    openResource(voiceResFile,      globalConfig.voiceResfilePath,
	                 "..\\sound\\",    VOICE_RESFILE,
	                 "Voice resource file")       &&
	    openResource(soundResFile,      globalConfig.soundResfilePath,
	                 "..\\sound\\",    SOUND_RESFILE,
	                 "Sound resource file")) {
		return true;
	}
	return false;

}

void testOpenImage() {
		hResContext     *decRes;

		decRes = resFile->newContext(MKTAG('A', 'M', 'A', 'P'), "Automap Resources");
		//checkAlloc(summaryData = LoadResource(decRes,
		//									  MKTAG('S', 'U', 'M', currentMapNum),
		//									  "summary data"));

		WindowDecoration *dec = &autoMapDecorations[0];
		dec->image = LoadResource(decRes, MKTAG('M', 'A', 'P', 0), "MAP0");
		//dec->image = ImageCache.requestImage(decRes, MKTAG('M', 'A', 'P', 0) | MKTAG('B', 'R', 'D', dec->imageNumber));
		Point16 pos(0, 0);
		drawCompressedImage(mainPort, pos, dec->image);
}

void testScripts() {
	scriptCallFrame scf;
	//for (int i = 1; i < 100; ++i)
	//	runScript(i, scf);
	runScript(1, scf);
}

void testTileRendering() {
	tileRes = resFile->newContext(MKTAG('T', 'I', 'L', 'E'), "tile resources");
	listRes = objResFile->newContext(MKTAG('L', 'I', 'S', 'T'), "list resources");
	resImports = (ResImportTable *)LoadResource(listRes, MKTAG('I', 'M', 'P', 'O'), "res imports");

	initMaps();
	initTileCyclingStates();

//	uint8 *img = nullptr;
//	TileInfo *ti;
//	for (int i = 0; i <= 65535; ++i) {
//		ti = TileInfo::tileAddress(i, &img);
//		if (ti) {
//			debugC(3, kDebugTiles, "terrainHeight = %d, height = %d", ti->attrs.terrainHeight, ti->attrs.height);
//			Common::hexdump(img, ti->attrs.height);
//		}
//	}

	cleanupTileCyclingStates();
	cleanupMaps();
}

//-----------------------------------------------------------------------
//	Routine to cleanup all the resource files

void closeResources(void) {
	if (soundResFile)  delete soundResFile;
	soundResFile = NULL;
	if (voiceResFile)  delete voiceResFile;
	voiceResFile = NULL;
	if (scriptResFile) delete scriptResFile;
	scriptResFile = NULL;
	if (auxResFile)    delete auxResFile;
	auxResFile = NULL;
	if (objResFile)    delete objResFile;
	objResFile = NULL;
	if (resFile)       delete resFile;
	resFile = NULL;
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
       actorStatesPaused,
       actorTasksPaused,
       combatBehaviorEnabled,
       backgroundSimulationPaused;

//	This structure is used archiving any globals which will need to be saved
//	in a save game file.

struct GlobalsArchive {
	int32           objectIndex,
	                actorIndex;
	bool            brotherBandingEnabled,
	                centerActorIndicatorEnabled,
	                interruptableMotionsPaused,
	                objectStatesPaused,
	                actorStatesPaused,
	                actorTasksPaused,
	                combatBehaviorEnabled,
	                backgroundSimulationPaused;
};

//-----------------------------------------------------------------------
//	Assign initial values to miscellaneous globals

void initGlobals(void) {
	objectIndex = 0;
	actorIndex = 0;
	brotherBandingEnabled = true;
	centerActorIndicatorEnabled = false;
	interruptableMotionsPaused = false;
	objectStatesPaused = false;
	actorStatesPaused = false;
	actorTasksPaused = false;
	combatBehaviorEnabled = false;
	backgroundSimulationPaused = false;
}

//-----------------------------------------------------------------------
//	Store miscellaneous globals in a save file

void saveGlobals(SaveFileConstructor &saveGame) {
	GlobalsArchive  archive;

	archive.objectIndex                 = objectIndex;
	archive.actorIndex                  = actorIndex;
	archive.brotherBandingEnabled       = brotherBandingEnabled;
	archive.centerActorIndicatorEnabled = centerActorIndicatorEnabled;
	archive.interruptableMotionsPaused  = interruptableMotionsPaused;
	archive.objectStatesPaused          = objectStatesPaused;
	archive.actorStatesPaused           = actorStatesPaused;
	archive.actorTasksPaused            = actorTasksPaused;
	archive.combatBehaviorEnabled       = combatBehaviorEnabled;
	archive.backgroundSimulationPaused  = backgroundSimulationPaused;

	saveGame.writeChunk(
	    MakeID('G', 'L', 'O', 'B'),
	    &archive,
	    sizeof(archive));
}

//-----------------------------------------------------------------------
//	Restore miscellaneouse globals from a save file

void loadGlobals(SaveFileReader &saveGame) {
	GlobalsArchive  archive;

	saveGame.read(&archive, sizeof(archive));

	objectIndex                 = archive.objectIndex;
	actorIndex                  = archive.actorIndex;
	brotherBandingEnabled       = archive.brotherBandingEnabled;
	centerActorIndicatorEnabled = archive.centerActorIndicatorEnabled;
	interruptableMotionsPaused  = archive.interruptableMotionsPaused;
	objectStatesPaused          = archive.objectStatesPaused;
	actorStatesPaused           = archive.actorStatesPaused;
	actorTasksPaused            = archive.actorTasksPaused;
	combatBehaviorEnabled       = archive.combatBehaviorEnabled;
	backgroundSimulationPaused  = archive.backgroundSimulationPaused;
}

/********************************************************************/
/*                                                                  */
/* ERROR / MESSAGE HANDLING                                         */
/*                                                                  */
/********************************************************************/

// ------------------------------------------------------------------------
// pops up a window to see if the user really wants to exit

bool verifyUserExit(void) {
	if (!gameRunning)
		return true;
	if (FTAMessageBox("Are you sure you want to exit", ERROR_YE_BUTTON, ERROR_NO_BUTTON) != 0)
		return true;
	return false;
}

//-----------------------------------------------------------------------
//	Allocate visual messagers

bool initGUIMessagers(void) {
	initUserDialog();
	for (int i = 0; i < 10; i++) {
		char debItem[16];
		sprintf(debItem, "Status%1.1d", i);
		Status[i] = NEW_MSGR StatusLineMessager(debItem, i, &mainPort);
		if (Status[i] == NULL)
			return false;
		sprintf(debItem, "Status%2.2d", i + 10);
		Status2[i] = NEW_MSGR StatusLineMessager(debItem, i, &mainPort, 468, 21 + (11 * i));
	}
	for (int j = 0; j < 3; j++)
		ratemess[j] = NEW_MSGR StatusLineMessager("FrameRates", j, &mainPort, 5, 450 + (11 * j), 500);
	return true;
}

//-----------------------------------------------------------------------
//	cleanup visual messagers

void cleanupGUIMessagers(void) {
	for (int i = 0; i < 10; i++) {
		if (Status[i]) delete Status[i];
		Status[i] = NULL;
		if (Status2[i]) delete Status2[i];
		Status2[i] = NULL;
	}
	cleanupUserDialog();
}

//-----------------------------------------------------------------------
//	Debugging status functions

#ifdef  WriteStatus
void WriteStatusF(int16 line, const char *msg, ...) {
	va_list         argptr;
	int             cnt;
	if (displayEnabled()) {
		va_start(argptr, msg);
		if (line > 9) {
			if (Status2[line - 10])
				cnt = Status2[line - 10]->va(msg, argptr);
		} else {
			if (Status[line])
				cnt = Status[line]->va(msg, argptr);
		}
		va_end(argptr);
	}
}

void WriteStatusF2(int16 line, const char *msg, ...) {
	va_list         argptr;
	int             cnt;
	if (displayEnabled()) {
		va_start(argptr, msg);
		if (Status2[line])
			cnt = Status2[line]->va(msg, argptr);
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

int32 currentGamePerformance(void) {
	int32 framePer = 100;
	int32 lval = int(lrate.frameStat());
	int32 fval = int(lrate.frameStat(grFramesPerSecond));
	if (fval >= frameRate && lval > fval) {
		framePer += (50 * ((lval - fval) / fval));
	} else {
		framePer = (100 * frate.frameStat(grFramesPerSecond)) / frameRate;
	}
	framePer = clamp(10, framePer, 240);
	return framePer;
}


void updatePerfStats(void) {
	char bigmess[512];

	frate.whatDoYouKnow(bigmess);
	(*ratemess[0])("Display: %s", bigmess);
	lrate.whatDoYouKnow(bigmess);
	(*ratemess[1])("Enginge: %s", bigmess);
	irate.whatDoYouKnow(bigmess);
	(*ratemess[2])("Message: %s", bigmess);
}

void updateFrameCount(void) {
	frate.updateFrameCount();
}

int32 eloopsPerSecond = 0;
int32 framesPerSecond = 0;

void updatePerfStats(void);
void oldUpdatePerfStats(void) {
	static uint32   prevMem = 0;
	static float f, lastF = 0.0;
	static float l, lastL = 0.0;
	if (elapsed > 0) {
		f = ((float)frames * (float)TICKSPERSECOND) / (float)elapsed;
		f = f + lastF / 2;
		l = ((float)loops * (float)TICKSPERSECOND) / (float)elapsed;
		l = l + lastL / 2;
		if (elapsed > 10 * TICKSPERSECOND) {
			elapsed = 0;
			frames = 0;
			loops = 0;
			lastF = f;
			lastL = l;
		}
	}
	eloopsPerSecond = int(l);
	framesPerSecond = int(f);
}


int32 gamePerformance(void) {
	if (framesPerSecond < frameRate) {
		return (100 * framesPerSecond) / frameRate;
	}
	if (framesPerSecond == frameRate)
		return 100;
	return 100 + 50 * (eloopsPerSecond - frameRate) / frameRate;

}


/********************************************************************/
/*                                                                  */
/* APPFUNC FOR MAIN WINDOW                                          */
/*                                                                  */
/********************************************************************/

//-----------------------------------------------------------------------
//	Function to handle miscellanous events to the window.
//	Any panel events which are not handled by individual panels
//	are sent to this function.

APPFUNC(cmdWindowFunc) {
	int16           key, qual;

	switch (ev.eventType) {
	case gEventKeyDown:
		key = ev.value & 0xffff;
		qual = ev.value >> 16;

		GameMode::modeStackPtr[GameMode::modeStackCtr - 1]->handleKey(key, qual);
		break;
	}
}

/********************************************************************/
/*                                                                  */
/* MEMORY MANAGEMENT CODE                                           */
/*                                                                  */
/********************************************************************/

/* ===================================================================== *
   Functions to initialize the memory manager.
 * ===================================================================== */

//-----------------------------------------------------------------------
//	Initialize memory manager

bool initMemPool(void) {
	uint32 take = pickHeapSize(memorySize);
	memorySize = take;
	if (NULL == (heapMemory = (uint8 *)malloc(take)))
		return false;
	//initMemHandler();
	return true;
}

//-----------------------------------------------------------------------
//	De-initialize memory manager

void cleanupMemPool(void) {
	//clearMemHandler();
	if (heapMemory) {
		free(heapMemory);
		heapMemory = nullptr;
	}
}

//-----------------------------------------------------------------------
//	Allocates memory, or throws exception if allocation fails.

void *mustAlloc(uint32 size, const char desc[]) {
	void            *ptr;

	ptr = malloc(size);
	//  REM: Before we give up completely, try unloading some things...
	if (ptr == NULL)
		error("Local heap allocation size %d bytes failed.", size);
	return ptr;
}

//-----------------------------------------------------------------------
//	Allocates relocatable memory, or throws exception if allocation fails.

RHANDLE mustAllocHandle(uint32 size, const char desc[]) {
	void            **ptr;

	ptr = (void **)malloc(size);
	//  REM: Before we give up completely, try unloading some things...
	if (ptr == NULL)
		error("Local handle allocation size %d bytes failed.", size);
	return ptr;
}


/********************************************************************/
/*                                                                  */
/* HOOK FOR CTRL-BREAK HANDLER                                      */
/*                                                                  */
/********************************************************************/

extern "C" {

	void breakEventKludge(void) {
		if (verifyUserExit())
			endGame();
		//gameRunning=false;
	}

}

} // end of namespace Saga2
