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
 * $URL$
 * $Id$
 *
 */

#include "common/endian.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/config-manager.h"
#include "common/stream.h"

#include "graphics/cursorman.h"

#include "base/plugins.h"
#include "base/version.h"

#include "sound/mididrv.h"
#include "sound/mixer.h"
#include "sound/audiocd.h"

#include "tinsel/actors.h"
#include "tinsel/background.h"
#include "tinsel/config.h"
#include "tinsel/cursor.h"
#include "tinsel/dw.h"
#include "tinsel/events.h"
#include "tinsel/faders.h"
#include "tinsel/film.h"
#include "tinsel/handle.h"
#include "tinsel/heapmem.h"			// MemoryInit
#include "tinsel/inventory.h"
#include "tinsel/music.h"
#include "tinsel/object.h"
#include "tinsel/pid.h"
#include "tinsel/polygons.h"
#include "tinsel/savescn.h"
#include "tinsel/scn.h"
#include "tinsel/serializer.h"
#include "tinsel/sound.h"
#include "tinsel/strres.h"
#include "tinsel/timers.h"
#include "tinsel/tinsel.h"

namespace Tinsel {

//----------------- EXTERNAL FUNCTIONS ---------------------

// In BG.CPP
extern void SetDoFadeIn(bool tf);
extern void DropBackground(void);

// In CURSOR.CPP
extern void CursorProcess(CORO_PARAM, const void *);

// In INVENTORY.CPP
extern void InventoryProcess(CORO_PARAM, const void *);

// In SCENE.CPP
extern void PrimeBackground();
extern void NewScene(SCNHANDLE scene, int entry);
extern SCNHANDLE GetSceneHandle(void);

// In TIMER.CPP
extern void FettleTimers(void);
extern void RebootTimers(void);

//----------------- FORWARD DECLARATIONS  ---------------------
void SetNewScene(SCNHANDLE scene, int entrance, int transition);

//----------------- GLOBAL GLOBAL DATA --------------------

bool bRestart = false;
bool bHasRestarted = false;

#ifdef DEBUG
bool bFast;		// set to make it go ludicrously fast
#endif

//----------------- LOCAL GLOBAL DATA --------------------

struct Scene {
	SCNHANDLE scene;	// Memory handle for scene
	int	entry;		// Entrance number
	int	trans;		// Transition - not yet used
};

static Scene NextScene = { 0, 0, 0 };
static Scene HookScene = { 0, 0, 0 };
static Scene DelayedScene = { 0, 0, 0 };

static bool bHookSuspend = false;

static PROCESS *pMouseProcess = 0;
static PROCESS *pKeyboardProcess = 0;

// Stack of pending mouse button events
Common::List<Common::EventType> mouseButtons;

// Stack of pending keypresses
Common::List<Common::Event> keypresses;

//----------------- LOCAL PROCEDURES --------------------

/**
 * Process to handle keypresses
 */
void KeyboardProcess(CORO_PARAM, const void *) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);
	while (true) {
		if (keypresses.empty()) {
			// allow scheduling
			CORO_SLEEP(1);
			continue;
		}

		// Get the next keyboard event off the stack
		Common::Event evt = *keypresses.begin();
		keypresses.erase(keypresses.begin());

		// Switch for special keys 
		switch (evt.kbd.keycode) {
		// Drag action
		case Common::KEYCODE_LALT:
		case Common::KEYCODE_RALT:
			if (evt.type == Common::EVENT_KEYDOWN) {
				if (!bSwapButtons)
					ProcessButEvent(BE_RDSTART);
				else
					ProcessButEvent(BE_LDSTART);
			} else {
				if (!bSwapButtons)
					ProcessButEvent(BE_LDEND);
				else
					ProcessButEvent(BE_RDEND);
			}
			continue;

		case Common::KEYCODE_LCTRL:
		case Common::KEYCODE_RCTRL:
			if (evt.type == Common::EVENT_KEYDOWN) {
				ProcessKeyEvent(LOOK_KEY);
			} else {
				// Control key release
			}
			continue;

		default:
			break;
		}

		// At this point only key down events need processing
		if (evt.type == Common::EVENT_KEYUP)
			continue;

		if (_vm->_keyHandler != NULL) 
			// Keyboard is hooked, so pass it on to that handler first
			if (!_vm->_keyHandler(evt.kbd))
				continue;

		switch (evt.kbd.keycode) {
		/*** SPACE = WALKTO ***/
		case Common::KEYCODE_SPACE:
			ProcessKeyEvent(WALKTO_KEY);
			continue;

		/*** RETURN = ACTION ***/
		case Common::KEYCODE_RETURN:
		case Common::KEYCODE_KP_ENTER:
			ProcessKeyEvent(ACTION_KEY);
			continue;

		/*** l = LOOK ***/
		case Common::KEYCODE_l:		// LOOK
			ProcessKeyEvent(LOOK_KEY);
			continue;

		case Common::KEYCODE_ESCAPE:
			// WORKAROUND: Check if any of the starting logo screens are active, and if so
			// manually skip to the title screen, allowing them to be bypassed
			{
				int sceneOffset = (_vm->getFeatures() & GF_SCNFILES) ? 1 : 0;
				int sceneNumber = (GetSceneHandle() >> SCNHANDLE_SHIFT) - sceneOffset;
#if 0	// FIXME: Disabled this code for now, as it doesn't work as it should (see bug #2078922).
				if ((g_language == TXT_GERMAN) && 
					((sceneNumber >= 25 && sceneNumber <= 27) || (sceneNumber == 17))) {
					// Skip to title screen
					// It seems the German CD version uses scenes 25,26,27,17 for the intro,
					// instead of 13,14,15,11;  also, the title screen is 11 instead of 10
					SetNewScene((11 + sceneOffset) << SCNHANDLE_SHIFT, 1, TRANS_CUT);
				} else
#endif
				if ((sceneNumber >= 13) && (sceneNumber <= 15) || (sceneNumber == 11)) {
					// Skip to title screen
					SetNewScene((10 + sceneOffset) << SCNHANDLE_SHIFT, 1, TRANS_CUT);
				} else {
					// Not on an intro screen, so process the key normally
					ProcessKeyEvent(ESC_KEY);
				}
			}
			continue;

#ifdef SLOW_RINCE_DOWN
		case '>':
			AddInterlude(1);
			continue;
		case '<':
			AddInterlude(-1);
			continue;
#endif

		case Common::KEYCODE_F1:
			// Options dialog
			ProcessKeyEvent(OPTION_KEY);
			continue;
		case Common::KEYCODE_F5:
			// Save game
			ProcessKeyEvent(SAVE_KEY);
			continue;
		case Common::KEYCODE_F7:
			// Load game
			ProcessKeyEvent(LOAD_KEY);
			continue;
		case Common::KEYCODE_q:
			if ((evt.kbd.flags == Common::KBD_CTRL) || (evt.kbd.flags == Common::KBD_ALT))
				ProcessKeyEvent(QUIT_KEY);
			continue;
		case Common::KEYCODE_PAGEUP:
		case Common::KEYCODE_KP9:
			ProcessKeyEvent(PGUP_KEY);
			continue;
		case Common::KEYCODE_PAGEDOWN:
		case Common::KEYCODE_KP3:
			ProcessKeyEvent(PGDN_KEY);
			continue;
		case Common::KEYCODE_HOME:
		case Common::KEYCODE_KP7:
			ProcessKeyEvent(HOME_KEY);
			continue;
		case Common::KEYCODE_END:
		case Common::KEYCODE_KP1:
			ProcessKeyEvent(END_KEY);
			continue;
		default:
			ProcessKeyEvent(NOEVENT_KEY);
			break;
		}
	}
	CORO_END_CODE;
}

/**
 * Process to handle changes in the mouse buttons.
 */
void MouseProcess(CORO_PARAM, const void *) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
		bool lastLWasDouble;
		bool lastRWasDouble;
		uint32 lastLeftClick, lastRightClick;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->lastLWasDouble = false;
	_ctx->lastRWasDouble = false;
	_ctx->lastLeftClick = _ctx->lastRightClick = DwGetCurrentTime();

	while (true) {
		// FIXME: I'm still keeping the ctrl/Alt handling in the ProcessKeyEvent method.
		// Need to make sure that this works correctly
		//DragKeys();

		if (mouseButtons.empty()) {
			// allow scheduling
			CORO_SLEEP(1);
			continue;
		}

		// get next mouse button event
		Common::EventType type = *mouseButtons.begin();
		mouseButtons.erase(mouseButtons.begin());

		switch (type) {
		case Common::EVENT_LBUTTONDOWN:
			// left button press
			if (DwGetCurrentTime() - _ctx->lastLeftClick < (uint32)dclickSpeed) {
				// signal left drag start
				ProcessButEvent(BE_LDSTART);

				// signal left double click event
				ProcessButEvent(BE_DLEFT);

				_ctx->lastLWasDouble = true;
			} else {
				// signal left drag start
				ProcessButEvent(BE_LDSTART);

				// signal left single click event
				ProcessButEvent(BE_SLEFT);

				_ctx->lastLWasDouble = false;
			}
			break;

		case Common::EVENT_LBUTTONUP:
			// left button release

			// update click timer
			if (_ctx->lastLWasDouble == false)
				_ctx->lastLeftClick = DwGetCurrentTime();
			else
				_ctx->lastLeftClick -= dclickSpeed;

			// signal left drag end
			ProcessButEvent(BE_LDEND);
			break;

		case Common::EVENT_RBUTTONDOWN:
			// right button press

			if (DwGetCurrentTime() - _ctx->lastRightClick < (uint32)dclickSpeed) {
				// signal right drag start
				ProcessButEvent(BE_RDSTART);

				// signal right double click event
				ProcessButEvent(BE_DRIGHT);

				_ctx->lastRWasDouble = true;
			} else {
				// signal right drag start
				ProcessButEvent(BE_RDSTART);

				// signal right single click event
				ProcessButEvent(BE_SRIGHT);

				_ctx->lastRWasDouble = false;
			}
			break;

		case Common::EVENT_RBUTTONUP:
			// right button release

			// update click timer
			if (_ctx->lastRWasDouble == false)
				_ctx->lastRightClick = DwGetCurrentTime();
			else
				_ctx->lastRightClick -= dclickSpeed;

			// signal right drag end
			ProcessButEvent(BE_RDEND);
			break;

		default:
			break;
		}
	}
	CORO_END_CODE;
}

/**
 * Run the master script.
 * Continues between scenes, or until Interpret() returns.
 */
static void MasterScriptProcess(CORO_PARAM, const void *) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
		INT_CONTEXT *pic;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);
	_ctx->pic = InitInterpretContext(GS_MASTER, 0, NOEVENT, NOPOLY, 0, NULL);
	CORO_INVOKE_1(Interpret, _ctx->pic);
	CORO_END_CODE;
}

/**
 * Store the facts pertaining to a scene change.
 */

void SetNewScene(SCNHANDLE scene, int entrance, int transition) {
	if (HookScene.scene == 0 || bHookSuspend) {
		// This scene comes next
		NextScene.scene = scene;
		NextScene.entry = entrance;
		NextScene.trans = transition;
	} else {
		// This scene gets delayed
		DelayedScene.scene = scene;
		DelayedScene.entry = entrance;
		DelayedScene.trans = transition;

		// The hooked scene comes next
		NextScene.scene = HookScene.scene;
		NextScene.entry = HookScene.entry;
		NextScene.trans = HookScene.trans;

		HookScene.scene = 0;
	}
}

void SetHookScene(SCNHANDLE scene, int entrance, int transition) {
	assert(HookScene.scene == 0); // scene already hooked

	HookScene.scene = scene;
	HookScene.entry = entrance;
	HookScene.trans = transition;
}

void UnHookScene(void) {
	assert(DelayedScene.scene != 0); // no scene delayed

	// The delayed scene can go now
	NextScene.scene = DelayedScene.scene;
	NextScene.entry = DelayedScene.entry;
	NextScene.trans = DelayedScene.trans;

	DelayedScene.scene = 0;
}

void SuspendHook(void) {
	bHookSuspend = true;
}

void UnSuspendHook(void) {
	bHookSuspend = false;
}

void syncSCdata(Serializer &s) {
	s.syncAsUint32LE(HookScene.scene);
	s.syncAsSint32LE(HookScene.entry);
	s.syncAsSint32LE(HookScene.trans);

	s.syncAsUint32LE(DelayedScene.scene);
	s.syncAsSint32LE(DelayedScene.entry);
	s.syncAsSint32LE(DelayedScene.trans);
}


//-----------------------------------------------------------------------

static void RestoredProcess(CORO_PARAM, const void *param) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
		INT_CONTEXT *pic;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// get the stuff copied to process when it was created
	_ctx->pic = *((INT_CONTEXT **)param);

	_ctx->pic = RestoreInterpretContext(_ctx->pic);
	CORO_INVOKE_1(Interpret, _ctx->pic);

	CORO_END_CODE;
}

void RestoreProcess(INT_CONTEXT *pic) {
	g_scheduler->createProcess(PID_TCODE, RestoredProcess, &pic, sizeof(pic));
}

void RestoreMasterProcess(INT_CONTEXT *pic) {
	g_scheduler->createProcess(PID_MASTER_SCR, RestoredProcess, &pic, sizeof(pic));
}

// FIXME: CountOut is used by ChangeScene
static int CountOut = 1;	// == 1 for immediate start of first scene

/**
 * If a scene restore is going on, just return (we don't update the
 * screen during this time).
 * If a scene change is required, 'order' the data for the new scene and
 * start a fade out and a countdown.
 * When the count expires, the screen will have faded. Ensure the scene	|
 * is loaded, clear the screen, and start the new scene.
 */
void ChangeScene() {

	if (IsRestoringScene())
		return;

	if (NextScene.scene != 0) {
		if (!CountOut) {
			switch (NextScene.trans) {
			case TRANS_CUT:
				CountOut = 1;
				break;

			case TRANS_FADE:
			default:
				// Trigger pre-load and fade and start countdown
				CountOut = COUNTOUT_COUNT;
				FadeOutFast(NULL);
				break;
			}
		} else if (--CountOut == 0) {
			ClearScreen();
			
			NewScene(NextScene.scene, NextScene.entry);
			NextScene.scene = 0;

			switch (NextScene.trans) {
			case TRANS_CUT:
				SetDoFadeIn(false);
				break;

			case TRANS_FADE:
			default:
				SetDoFadeIn(true);
				break;
			}
		}
	}
}

/**
 * LoadBasicChunks
 */

void LoadBasicChunks(void) {
	byte *cptr;
	int numObjects;

	// Allocate RAM for savescene data
	InitialiseSs();

	// CHUNK_TOTAL_ACTORS seems to be missing in the released version, hard coding a value
	// TODO: Would be nice to just change 511 to MAX_SAVED_ALIVES
	cptr = FindChunk(MASTER_SCNHANDLE, CHUNK_TOTAL_ACTORS);
	RegisterActors((cptr != NULL) ? READ_LE_UINT32(cptr) : 511);

	// CHUNK_TOTAL_GLOBALS seems to be missing in some versions.
	// So if it is missing, set a reasonably high value for the number of globals.
	cptr = FindChunk(MASTER_SCNHANDLE, CHUNK_TOTAL_GLOBALS);
	RegisterGlobals((cptr != NULL) ? READ_LE_UINT32(cptr) : 512);

	cptr = FindChunk(INV_OBJ_SCNHANDLE, CHUNK_TOTAL_OBJECTS);
	numObjects = (cptr != NULL) ? READ_LE_UINT32(cptr) : 0;

	cptr = FindChunk(INV_OBJ_SCNHANDLE, CHUNK_OBJECTS);
	
#ifdef SCUMM_BIG_ENDIAN
	//convert to native endianness
	INV_OBJECT *io = (INV_OBJECT *)cptr;
	for (int i = 0; i < numObjects; i++, io++) {
		io->id        = FROM_LE_32(io->id);
		io->hFilm     = FROM_LE_32(io->hFilm);
		io->hScript   = FROM_LE_32(io->hScript);
		io->attribute = FROM_LE_32(io->attribute);
	}
#endif
	
	RegisterIcons(cptr, numObjects);

	cptr = FindChunk(MASTER_SCNHANDLE, CHUNK_TOTAL_POLY);
	if (cptr != NULL)
		MaxPolygons(*cptr);
}

//----------------- TinselEngine --------------------

// Global pointer to engine
TinselEngine *_vm;

struct GameSettings {
	const char *gameid;
	const char *description;
	byte id;
	uint32 features;
	const char *detectname;
};

static const GameSettings tinselSettings[] = {
	{"tinsel", "Tinsel game", 0, 0, 0},

	{NULL, NULL, 0, 0, NULL}
};

TinselEngine::TinselEngine(OSystem *syst, const TinselGameDescription *gameDesc) : 
		Engine(syst), _gameDescription(gameDesc) {
	_vm = this;

	// Setup mixer
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	const GameSettings *g;

	const char *gameid = ConfMan.get("gameid").c_str();
	for (g = tinselSettings; g->gameid; ++g)
		if (!scumm_stricmp(g->gameid, gameid))
			_gameId = g->id;

	int cd_num = ConfMan.getInt("cdrom");
	if (cd_num >= 0)
		_system->openCD(cd_num);
		
	int midiDriver = MidiDriver::detectMusicDriver(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MIDI);
	bool native_mt32 = ((midiDriver == MD_MT32) || ConfMan.getBool("native_mt32"));
	//bool adlib = (midiDriver == MD_ADLIB);

	_driver = MidiDriver::createMidi(midiDriver);
	if (native_mt32)
		_driver->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);

	_music = new MusicPlayer(_driver);
	//_music->setNativeMT32(native_mt32);
	//_music->setAdlib(adlib);

	_musicVolume = ConfMan.getInt("music_volume");
	
	_sound = new SoundManager(this);

	_mousePos.x = 0;
	_mousePos.y = 0;
	_keyHandler = NULL;
	_dosPlayerDir = 0;
}

TinselEngine::~TinselEngine() {
	delete _sound;
	delete _music;
	delete _console;
	delete _driver;
	_screenSurface.free();
	FreeSs();
	FreeTextBuffer();
	FreeHandleTable();
	FreeActors();
	FreeObjectList();
	FreeGlobals();
	delete _scheduler;
}

Common::Error TinselEngine::init() {
	// Initialize backend
	_system->beginGFXTransaction();
		initCommonGFX(false);
		_system->initSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	_system->endGFXTransaction();

	_screenSurface.create(SCREEN_WIDTH, SCREEN_HEIGHT, 1);

	g_system->getEventManager()->registerRandomSource(_random, "tinsel");

	_console = new Console();
	
	_scheduler = new Scheduler();

	// init memory manager
	MemoryInit();

	// load user configuration
	ReadConfig();

#if 1
	// FIXME: The following is taken from RestartGame().
	// It may have to be adjusted a bit
	CountOut = 1;

	RebootCursor();
	RebootDeadTags();
	RebootMovers();
	RebootTimers();
	RebootScalingReels();

	DelayedScene.scene = HookScene.scene = 0;
#endif

	// Init palette and object managers, scheduler, keyboard and mouse
	RestartDrivers();

	// TODO: More stuff from dos_main.c may have to be added here

	// load in text strings
	ChangeLanguage(g_language);

	// load in graphics info
	SetupHandleTable();

	// Actors, globals and inventory icons
	LoadBasicChunks();

	return Common::kNoError;
}

Common::String TinselEngine::getSavegameFilename(int16 saveNum) const {
	char filename[256];
	snprintf(filename, 256, "%s.%03d", getTargetName().c_str(), saveNum);
	return filename;
}

#define GAME_FRAME_DELAY (1000 / ONE_SECOND)

Common::Error TinselEngine::go() {
	uint32 timerVal = 0;

	// Continuous game processes
	CreateConstProcesses();

	// allow game to run in the background
	//RestartBackgroundProcess();	// FIXME: is this still needed?

	//dumpMusic();	// dumps all of the game's music in external XMIDI files

#if 0
	// Load game from specified slot, if any
	// FIXME: Not working correctly right now
	if (ConfMan.hasKey("save_slot")) {
		getList();
		RestoreGame(ConfMan.getInt("save_slot"));
	}
#endif

	// Foreground loop

	while (!shouldQuit()) {
		assert(_console);
		if (_console->isAttached())
			_console->onFrame();

		// Check for time to do next game cycle
		if ((g_system->getMillis() > timerVal + GAME_FRAME_DELAY)) {
			timerVal = g_system->getMillis();
			AudioCD.updateCD();
			NextGameCycle();
		}

		if (bRestart) {
			RestartGame();
			bRestart = false;
			bHasRestarted = true;	// Set restarted flag
		}

		// Save/Restore scene file transfers
		ProcessSRQueue();

#ifdef DEBUG
		if (bFast)
			continue;		// run flat-out
#endif
		// Loop processing events while there are any pending
		while (pollEvent());

		g_system->delayMillis(10);
	}

	// Write configuration
	WriteConfig();

	return Common::kNoError;
}


void TinselEngine::NextGameCycle(void) {
	//
	ChangeScene();

	// Allow a user event for this schedule
	ResetEcount();

	// schedule process
	_scheduler->schedule();

	// redraw background
	DrawBackgnd();

	// Why waste resources on yet another process?
	FettleTimers();
}


bool TinselEngine::pollEvent() {
	Common::Event event;
	
	if (!g_system->getEventManager()->pollEvent(event)) 
		return false;

	// Handle the various kind of events
	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_RBUTTONUP:
		// Add button to queue for the mouse process
		mouseButtons.push_back(event.type);
		break;

	case Common::EVENT_MOUSEMOVE:
		_mousePos = event.mouse;
		break;

	case Common::EVENT_KEYDOWN:
	case Common::EVENT_KEYUP:
		ProcessKeyEvent(event);
		break;
			
	default:
		break;
	}

	return true;
}

/**
 * Start the processes that continue between scenes.
 */

void TinselEngine::CreateConstProcesses(void) {
	// Process to run the master script
	_scheduler->createProcess(PID_MASTER_SCR, MasterScriptProcess, NULL, 0);

	// Processes to run the cursor and inventory,
	_scheduler->createProcess(PID_CURSOR, CursorProcess, NULL, 0);
	_scheduler->createProcess(PID_INVENTORY, InventoryProcess, NULL, 0);
}

/**
 * Restart the game
 */

void TinselEngine::RestartGame(void) {
	HoldItem(INV_NOICON);	// Holding nothing

	DropBackground();	// No background

	// Ditches existing infrastructure background
	PrimeBackground();

	// Next scene change won't need to fade out
	// -> reset the count used by ChangeScene
	CountOut = 1;

	RebootCursor();
	RebootDeadTags();
	RebootMovers();
	RebootTimers();
	RebootScalingReels();

	DelayedScene.scene = HookScene.scene = 0;

	// remove keyboard, mouse and joystick drivers
	ChopDrivers();

	// Init palette and object managers, scheduler, keyboard and mouse
	RestartDrivers();

	// Actors, globals and inventory icons
	LoadBasicChunks();

	// Continuous game processes
	CreateConstProcesses();
}

/**
 * Init palette and object managers, scheduler, keyboard and mouse.
 */

void TinselEngine::RestartDrivers(void) {
	// init the palette manager
	ResetPalAllocator();

	// init the object manager
	KillAllObjects();

	// init the process scheduler
	_scheduler->reset();

	// init the event handlers
	pMouseProcess = _scheduler->createProcess(PID_MOUSE, MouseProcess, NULL, 0);	
	pKeyboardProcess = _scheduler->createProcess(PID_KEYBOARD, KeyboardProcess, NULL, 0);	

	// open MIDI files
	OpenMidiFiles();

	// open sample files (only if mixer is ready)
	if (_mixer->isReady()) {
		_sound->openSampleFiles();
	}

	// Set midi volume
	SetMidiVolume(volMidi);
}

/**
 * Remove keyboard, mouse and joystick drivers.
 */

void TinselEngine::ChopDrivers(void) {
	// remove sound driver
	StopMidi();
	_sound->stopAllSamples();
	DeleteMidiBuffer();

	// remove event drivers
	_scheduler->killProcess(pMouseProcess);
	_scheduler->killProcess(pKeyboardProcess);
}

/**
 * Process a keyboard event
 */

void TinselEngine::ProcessKeyEvent(const Common::Event &event) {

	// Handle any special keys immediately
	switch (event.kbd.keycode) {
	case Common::KEYCODE_d:
		if ((event.kbd.flags == Common::KBD_CTRL) && (event.type == Common::EVENT_KEYDOWN)) {
			// Activate the debugger
			assert(_console);
			_console->attach();
			return;
		}
		break;
	default:
		break;
	}

	// Check for movement keys
	int idx = 0;
	switch (event.kbd.keycode) {
	case Common::KEYCODE_UP:
	case Common::KEYCODE_KP8:
		idx = MSK_UP;
		break;
	case Common::KEYCODE_DOWN:
	case Common::KEYCODE_KP2:
		idx = MSK_DOWN;
		break;
	case Common::KEYCODE_LEFT:
	case Common::KEYCODE_KP4:
		idx = MSK_LEFT;
		break;
	case Common::KEYCODE_RIGHT:
	case Common::KEYCODE_KP6:
		idx = MSK_RIGHT;
		break;
	default:
		break;
	}
	if (idx != 0) {
		if (event.type == Common::EVENT_KEYDOWN)
			_dosPlayerDir |= idx;
		else
			_dosPlayerDir &= ~idx;
		return;
	}

	// All other keypresses add to the queue for processing in KeyboardProcess 
	keypresses.push_back(event);
}

} // End of namespace Tinsel
