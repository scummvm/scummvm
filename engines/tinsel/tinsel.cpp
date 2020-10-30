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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/debug-channels.h"
#include "common/endian.h"
#include "common/error.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/fs.h"
#include "common/config-manager.h"
#include "common/serializer.h"

#include "backends/audiocd/audiocd.h"

#include "engines/util.h"

#include "tinsel/actors.h"
#include "tinsel/background.h"
#include "tinsel/bmv.h"
#include "tinsel/config.h"
#include "tinsel/cursor.h"
#include "tinsel/drives.h"
#include "tinsel/dw.h"
#include "tinsel/events.h"
#include "tinsel/faders.h"
#include "tinsel/film.h"
#include "tinsel/font.h"
#include "tinsel/handle.h"
#include "tinsel/heapmem.h"			// MemoryInit
#include "tinsel/dialogs.h"
#include "tinsel/mareels.h"
#include "tinsel/music.h"
#include "tinsel/object.h"
#include "tinsel/pid.h"
#include "tinsel/polygons.h"
#include "tinsel/savescn.h"
#include "tinsel/scn.h"
#include "tinsel/sound.h"
#include "tinsel/strres.h"
#include "tinsel/sysvar.h"
#include "tinsel/timers.h"
#include "tinsel/tinsel.h"

namespace Tinsel {

//----------------- EXTERNAL FUNCTIONS ---------------------

// In CURSOR.CPP
extern void CursorProcess(CORO_PARAM, const void *);

// In INVENTORY.CPP
extern void InventoryProcess(CORO_PARAM, const void *);

// In SCENE.CPP
extern SCNHANDLE GetSceneHandle();

extern void ResetVarsDrives();
extern void ResetVarsEvents();
extern void RebootScalingReels();
extern void ResetVarsMove();
extern void ResetVarsPalette();
extern void ResetVarsPCode();
extern void ResetVarsPDisplay();
extern void ResetVarsPlay();
extern void ResetVarsPolygons();
extern void RebootMovers();
extern void ResetVarsSaveLoad();
extern void ResetVarsSaveScn();
extern void ResetVarsScene();
extern void ResetVarsSched();
extern void ResetVarsStrRes();
extern void FreeTextBuffer();
extern void ResetVarsSysVar();
extern void FreeAllTokens();
extern void RebootTimers();
extern void ResetVarsTinlib();

//----------------- FORWARD DECLARATIONS  ---------------------
void SetNewScene(SCNHANDLE scene, int entrance, int transition);

//----------------- GLOBAL GLOBAL DATA --------------------

// These vars are reset upon engine destruction

bool g_bRestart = false;
bool g_bHasRestarted = false;
bool g_loadingFromGMM = false;

static bool g_bCuttingScene = false;

static bool g_bChangingForRestore = false;

// FIXME: CountOut is used by ChangeScene
static int CountOut = 1; // == 1 for immediate start of first scene

#ifdef DEBUG
bool g_bFast;		// set to make it go ludicrously fast
#endif

//----------------- LOCAL GLOBAL DATA --------------------

struct Scene {
	SCNHANDLE scene;	// Memory handle for scene
	int	entry;		// Entrance number
	int	trans;		// Transition - not yet used
};

static Scene g_NextScene = { 0, 0, 0 };
static Scene g_HookScene = { 0, 0, 0 };
static Scene g_DelayedScene = { 0, 0, 0 };

static Common::PROCESS *g_pMouseProcess = nullptr;
static Common::PROCESS *g_pKeyboardProcess = nullptr;

static SCNHANDLE g_hCdChangeScene;

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
		if (_vm->_keypresses.empty()) {
			// allow scheduling
			CORO_SLEEP(1);
			continue;
		}

		// Get the next keyboard event off the stack
		Common::Event evt = _vm->_keypresses.front();
		_vm->_keypresses.pop_front();

		// Switch for special keys
		switch (evt.kbd.keycode) {
		// Drag action
		case Common::KEYCODE_LALT:
		case Common::KEYCODE_RALT:
			if (evt.type == Common::EVENT_KEYDOWN) {
				if (!_vm->_config->_swapButtons)
					ProcessButEvent(PLR_DRAG2_START);
				else
					ProcessButEvent(PLR_DRAG1_START);
			} else {
				if (!_vm->_config->_swapButtons)
					ProcessButEvent(PLR_DRAG1_END);
				else
					ProcessButEvent(PLR_DRAG2_END);
			}
			continue;

		case Common::KEYCODE_LCTRL:
		case Common::KEYCODE_RCTRL:
			if (evt.type == Common::EVENT_KEYDOWN) {
				ProcessKeyEvent(PLR_LOOK);
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
			ProcessKeyEvent(PLR_WALKTO);
			continue;

		/*** RETURN = ACTION ***/
		case Common::KEYCODE_RETURN:
		case Common::KEYCODE_KP_ENTER:
			ProcessKeyEvent(PLR_ACTION);
			continue;

		/*** l = LOOK ***/
		case Common::KEYCODE_l:		// LOOK
			ProcessKeyEvent(PLR_LOOK);
			continue;

		case Common::KEYCODE_ESCAPE:
			ProcessKeyEvent(PLR_ESCAPE);
			continue;

#ifdef SLOW_RINCE_DOWN
		case '>':
			AddInterlude(1);
			continue;
		case '<':
			AddInterlude(-1);
			continue;
#endif

		case Common::KEYCODE_1:
		case Common::KEYCODE_F1:
			// Options dialog
			ProcessKeyEvent(PLR_MENU);
			continue;
		case Common::KEYCODE_5:
		case Common::KEYCODE_F5:
			// Save game
			ProcessKeyEvent(PLR_SAVE);
			continue;
		case Common::KEYCODE_7:
		case Common::KEYCODE_F7:
			// Load game
			ProcessKeyEvent(PLR_LOAD);
			continue;
		case Common::KEYCODE_m:
			// Debug facility - scene hopper
			if (TinselV2 && (evt.kbd.hasFlags(Common::KBD_ALT)))
				ProcessKeyEvent(PLR_JUMP);
			break;
		case Common::KEYCODE_q:
			if ((evt.kbd.hasFlags(Common::KBD_CTRL)) || (evt.kbd.hasFlags(Common::KBD_ALT)))
				ProcessKeyEvent(PLR_QUIT);
			continue;
		case Common::KEYCODE_PAGEUP:
		case Common::KEYCODE_KP9:
			ProcessKeyEvent(PLR_PGUP);
			continue;
		case Common::KEYCODE_PAGEDOWN:
		case Common::KEYCODE_KP3:
			ProcessKeyEvent(PLR_PGDN);
			continue;
		case Common::KEYCODE_HOME:
		case Common::KEYCODE_KP7:
			ProcessKeyEvent(PLR_HOME);
			continue;
		case Common::KEYCODE_END:
		case Common::KEYCODE_KP1:
			ProcessKeyEvent(PLR_END);
			continue;
		default:
			ProcessKeyEvent(PLR_NOEVENT);
			break;
		}
	}
	CORO_END_CODE;
}

/**
 * Handles launching a single click action result if the timeout for a double-click
 * expires
 */
static void SingleLeftProcess(CORO_PARAM, const void *param) {
	CORO_BEGIN_CONTEXT;
		uint32 endTicks;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// Work out when to wait until
	_ctx->endTicks = DwGetCurrentTime() + (uint32)_vm->_config->_dclickSpeed;

	// Timeout a double click (may not work once every 49 days!)
	do {
		CORO_SLEEP(1);
	} while (DwGetCurrentTime() < _ctx->endTicks);

	if (GetProvNotProcessed()) {
		const Common::Point clickPos = *(const Common::Point *)param;
		PlayerEvent(PLR_WALKTO, clickPos);
	}

	CORO_KILL_SELF();
	CORO_END_CODE;
}

/**
 * Process to handle changes in the mouse buttons.
 */
static void MouseProcess(CORO_PARAM, const void *) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
		bool lastLWasDouble;
		bool lastRWasDouble;
		uint32 lastLeftClick, lastRightClick;
		Common::Point clickPos;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->lastLWasDouble = false;
	_ctx->lastRWasDouble = false;
	_ctx->lastLeftClick = _ctx->lastRightClick = DwGetCurrentTime();

	while (true) {

		if (_vm->_mouseButtons.empty()) {
			// allow scheduling
			CORO_SLEEP(1);
			continue;
		}

		// get next mouse button event
		Common::EventType type = _vm->_mouseButtons.front();
		_vm->_mouseButtons.pop_front();

		int xp, yp;
		_vm->_cursor->GetCursorXYNoWait(&xp, &yp, true);
		const Common::Point mousePos(xp, yp);

		switch (type) {
		case Common::EVENT_LBUTTONDOWN:
			// left button press
			if (DwGetCurrentTime() - _ctx->lastLeftClick < (uint32)_vm->_config->_dclickSpeed) {
				// Left button double-click

				if (TinselV2) {
					// Kill off the button process and fire off the action command
					CoroScheduler.killMatchingProcess(PID_BTN_CLICK, -1);
					PlayerEvent(PLR_ACTION, _ctx->clickPos);
				} else {
					// signal left drag start
					ProcessButEvent(PLR_DRAG1_START);

					// signal left double click event
					ProcessButEvent(PLR_DLEFT);
				}

				_ctx->lastLWasDouble = true;
			} else {
				// Initial mouse down - either for a single click, or potentially
				// the start of a double-click action

				if (TinselV2) {
					PlayerEvent(PLR_DRAG1_START, mousePos);

					ProvNotProcessed();
					PlayerEvent(PLR_PROV_WALKTO, mousePos);

				} else {
					// signal left drag start
					ProcessButEvent(PLR_DRAG1_START);

					// signal left single click event
					ProcessButEvent(PLR_SLEFT);
				}

				_ctx->lastLWasDouble = false;
			}
			break;

		case Common::EVENT_LBUTTONUP:
			// left button release

			// update click timer
			if (_ctx->lastLWasDouble == false) {
				_ctx->lastLeftClick = DwGetCurrentTime();

				// If player control is enabled, start a process which, if it times out,
				// will activate a single button click
				if (TinselV2 && ControlIsOn()) {
					_ctx->clickPos = mousePos;
					CoroScheduler.createProcess(PID_BTN_CLICK, SingleLeftProcess, &_ctx->clickPos, sizeof(Common::Point));
				}
			} else
				_ctx->lastLeftClick -= _vm->_config->_dclickSpeed;

			if (TinselV2)
				// Signal left drag end
				PlayerEvent(PLR_DRAG1_END, mousePos);
			else
				// signal left drag end
				ProcessButEvent(PLR_DRAG1_END);
			break;

		case Common::EVENT_RBUTTONDOWN:
			// right button press

			if (DwGetCurrentTime() - _ctx->lastRightClick < (uint32)_vm->_config->_dclickSpeed) {
				// Right button double-click
				if (TinselV2) {
					PlayerEvent(PLR_NOEVENT, _ctx->clickPos);
				} else {
					// signal right drag start
					ProcessButEvent(PLR_DRAG2_START);

					// signal right double click event
					ProcessButEvent(PLR_DRIGHT);
				}

				_ctx->lastRWasDouble = true;
			} else {
				if (TinselV2) {
					PlayerEvent(PLR_DRAG2_START, mousePos);
					PlayerEvent(PLR_LOOK, mousePos);
				} else {
					// signal right drag start
					ProcessButEvent(PLR_DRAG2_START);

					// signal right single click event
					ProcessButEvent(PLR_SRIGHT);
				}

				_ctx->lastRWasDouble = false;
			}
			break;

		case Common::EVENT_RBUTTONUP:
			// right button release

			// update click timer
			if (_ctx->lastRWasDouble == false)
				_ctx->lastRightClick = DwGetCurrentTime();
			else
				_ctx->lastRightClick -= _vm->_config->_dclickSpeed;

			if (TinselV2)
				// Signal left drag end
				PlayerEvent(PLR_DRAG2_END, mousePos);
			else
				// signal right drag end
				ProcessButEvent(PLR_DRAG2_END);
			break;

		case Common::EVENT_WHEELUP:
			PlayerEvent(PLR_WHEEL_UP, mousePos);
			break;

		case Common::EVENT_WHEELDOWN:
			PlayerEvent(PLR_WHEEL_DOWN, mousePos);
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
	if (!g_bCuttingScene && TinselV2)
		WrapScene();

	// If we're loading from the GMM, load the scene as a delayed one
	if (g_loadingFromGMM) {
		g_DelayedScene.scene = scene;
		g_DelayedScene.entry = entrance;
		g_DelayedScene.trans = transition;
		g_loadingFromGMM = false;
		return;
	}

	// If CD change will be required, stick in the scene change scene
	if (_vm->_handle->CdNumber(scene) != GetCurrentCD()) {
		// This scene gets delayed
		g_DelayedScene.scene = scene;
		g_DelayedScene.entry = entrance;
		g_DelayedScene.trans = transition;

		g_NextScene.scene = g_hCdChangeScene;
		g_NextScene.entry = _vm->_handle->CdNumber(scene) - '0';
		g_NextScene.trans = TRANS_FADE;

		return;
	}

	if (g_HookScene.scene == 0 || g_bCuttingScene) {
		// This scene comes next
		g_NextScene.scene = scene;
		g_NextScene.entry = entrance;
		g_NextScene.trans = transition;
	} else {
		// This scene gets delayed
		g_DelayedScene.scene = scene;
		g_DelayedScene.entry = entrance;
		g_DelayedScene.trans = transition;

		// The hooked scene comes next
		g_NextScene.scene = g_HookScene.scene;
		g_NextScene.entry = g_HookScene.entry;
		g_NextScene.trans = g_HookScene.trans;

		g_HookScene.scene = 0;
	}

	// Workaround for "Missing Red Dragon in square" bug in Discworld 1 PSX, act IV.
	// This happens with the original interpreter on PSX too: the red dragon in Act IV
	// doesn't show up inside the square at the right time. Original game required the
	// player to go in and out the square until the dragon appears (wasting hours).
	// I'm forcing the load of the right scene by checking that the player has (or has not) the
	// right items: player must have Mambo the swamp dragon, and mustn't have fireworks (used on
	// the swamp dragon previously to "load it up").
	if (TinselV1PSX && g_NextScene.scene == 0x1800000 && g_NextScene.entry == 2) {
		if ((_vm->_dialogs->IsInInventory(261, INV_1) || _vm->_dialogs->IsInInventory(261, INV_2)) &&
		    (!_vm->_dialogs->IsInInventory(232, INV_1) && !_vm->_dialogs->IsInInventory(232, INV_2)))
			g_NextScene.entry = 1;
	}
}

/**
 * Store a scene as hooked
 */
void SetHookScene(SCNHANDLE scene, int entrance, int transition) {
	g_HookScene.scene = scene;
	g_HookScene.entry = entrance;
	g_HookScene.trans = transition;
}

/**
 * Hooked scene is over, trigger a change to the delayed scene
 */
void UnHookScene() {
	assert(g_DelayedScene.scene != 0); // no scene delayed

	// The delayed scene can go now
	g_NextScene.scene = g_DelayedScene.scene;
	g_NextScene.entry = g_DelayedScene.entry;
	g_NextScene.trans = g_DelayedScene.trans;

	g_DelayedScene.scene = 0;
}

void SuspendHook() {
	g_bCuttingScene = true;
}

void CdHasChanged() {
	if (g_bChangingForRestore) {
		g_bChangingForRestore = false;
		RestoreGame(-2);
	} else {
		assert(g_DelayedScene.scene != 0);

		WrapScene();

		// The delayed scene can go now
		g_NextScene.scene = g_DelayedScene.scene;
		g_NextScene.entry = g_DelayedScene.entry;
		g_NextScene.trans = g_DelayedScene.trans;

		g_DelayedScene.scene = 0;
	}
}

void SetCdChangeScene(SCNHANDLE hScene) {
	g_hCdChangeScene = hScene;
}

void CDChangeForRestore(int cdNumber) {
	g_NextScene.scene = g_hCdChangeScene;
	g_NextScene.entry = cdNumber;
	g_NextScene.trans = TRANS_FADE;
	g_bChangingForRestore = true;
}

void UnSuspendHook() {
	g_bCuttingScene = false;
}

void syncSCdata(Common::Serializer &s) {
	s.syncAsUint32LE(g_HookScene.scene);
	s.syncAsSint32LE(g_HookScene.entry);
	s.syncAsSint32LE(g_HookScene.trans);

	s.syncAsUint32LE(g_DelayedScene.scene);
	s.syncAsSint32LE(g_DelayedScene.entry);
	s.syncAsSint32LE(g_DelayedScene.trans);
}


//-----------------------------------------------------------------------

static void RestoredProcess(CORO_PARAM, const void *param) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
		INT_CONTEXT *pic;
		bool bConverse;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// get the stuff copied to process when it was created
	_ctx->pic = *((INT_CONTEXT * const *)param);

	_ctx->pic = RestoreInterpretContext(_ctx->pic);
	_ctx->bConverse = TinselV2 && (_ctx->pic->event == CONVERSE);

	CORO_INVOKE_1(Interpret, _ctx->pic);

	// Restore control after CallScene() from a conversation icon
	if (_ctx->bConverse)
		ControlOn();

	CORO_END_CODE;
}

void RestoreProcess(INT_CONTEXT *pic) {
	CoroScheduler.createProcess(PID_TCODE, RestoredProcess, &pic, sizeof(pic));
}

void RestoreMasterProcess(INT_CONTEXT *pic) {
	CoroScheduler.createProcess(PID_MASTER_SCR, RestoredProcess, &pic, sizeof(pic));
}

/**
 * If a scene restore is going on, just return (we don't update the
 * screen during this time).
 * If a scene change is required, 'order' the data for the new scene and
 * start a fade out and a countdown.
 * When the count expires, the screen will have faded. Ensure the scene	|
 * is loaded, clear the screen, and start the new scene.
 */
bool ChangeScene(bool bReset) {

	// Prevent attempt to fade-out when restarting game
	if (bReset) {
		CountOut = 1;	// immediate start of first scene again
		g_DelayedScene.scene = g_HookScene.scene = 0;
		return false;
	}

	if (IsRestoringScene())
		return true;

	if (g_NextScene.scene != 0) {
		if (!CountOut) {
			switch (g_NextScene.trans) {
			case TRANS_CUT:
				CountOut = 1;
				break;

			case TRANS_FADE:
			default:
				// Trigger pre-load and fade and start countdown
				CountOut = COUNTOUT_COUNT;
				FadeOutFast();
				if (TinselV2)
					_vm->_pcmMusic->startFadeOut(COUNTOUT_COUNT);
				break;
			}
		} else if (--CountOut == 0) {
			if (!TinselV2)
				ClearScreen();

			StartNewScene(g_NextScene.scene, g_NextScene.entry);
			g_NextScene.scene = 0;

			switch (g_NextScene.trans) {
			case TRANS_CUT:
				_vm->_bg->SetDoFadeIn(false);
				break;

			case TRANS_FADE:
			default:
				_vm->_bg->SetDoFadeIn(true);
				break;
			}
		} else
			_vm->_pcmMusic->fadeOutIteration();
	}

	return false;
}

/**
 * CuttingScene
 */
void CuttingScene(bool bCutting) {
	g_bCuttingScene = bCutting;

	if (!bCutting)
		WrapScene();
}

/**
 * LoadBasicChunks
 */
void LoadBasicChunks() {
	byte *cptr;
	int numObjects;

	// Allocate RAM for savescene data
	InitializeSaveScenes();

	// CHUNK_TOTAL_ACTORS seems to be missing in the released version, hard coding a value
	// TODO: Would be nice to just change 511 to MAX_SAVED_ALIVES
	cptr = FindChunk(MASTER_SCNHANDLE, CHUNK_TOTAL_ACTORS);
	_vm->_actor->RegisterActors((cptr != NULL) ? READ_32(cptr) : 511);

	// CHUNK_TOTAL_GLOBALS seems to be missing in some versions.
	// So if it is missing, set a reasonably high value for the number of globals.
	cptr = FindChunk(MASTER_SCNHANDLE, CHUNK_TOTAL_GLOBALS);
	RegisterGlobals((cptr != NULL) ? READ_32(cptr) : 512);

	cptr = FindChunk(INV_OBJ_SCNHANDLE, CHUNK_TOTAL_OBJECTS);
	numObjects = (cptr != NULL) ? READ_32(cptr) : 0;

	cptr = FindChunk(INV_OBJ_SCNHANDLE, CHUNK_OBJECTS);

	// Convert to native endianness
	INV_OBJECT *io = (INV_OBJECT *)cptr;
	for (int i = 0; i < numObjects; i++, io++) {
		io->id        = FROM_32(io->id);
		io->hIconFilm = FROM_32(io->hIconFilm);
		io->hScript   = FROM_32(io->hScript);
		io->attribute = FROM_32(io->attribute);
	}

	_vm->_dialogs->RegisterIcons(cptr, numObjects);

	cptr = FindChunk(MASTER_SCNHANDLE, CHUNK_TOTAL_POLY);
	// Max polygons are 0 in DW1 Mac (both in the demo and the full version)
	if (cptr != NULL && *cptr != 0)
		MaxPolygons(*cptr);

	if (TinselV2) {
		// Global processes
		cptr = FindChunk(MASTER_SCNHANDLE, CHUNK_NUM_PROCESSES);
		assert(cptr && (*cptr < 100));
		int num = *cptr;
		cptr = FindChunk(MASTER_SCNHANDLE, CHUNK_PROCESSES);
		assert(!num || cptr);
		GlobalProcesses(num, cptr);

		// CdPlay() stuff
		cptr = FindChunk(MASTER_SCNHANDLE, CHUNK_CDPLAY_HANDLE);
		assert(cptr);
		uint32 playHandle = READ_32(cptr);
		assert(playHandle < 512);
		_vm->_handle->SetCdPlayHandle(playHandle);
	}
}

void ResetVarsTinsel() {
	g_bRestart = false;
	g_bHasRestarted = false;
	g_loadingFromGMM = false;

	g_bCuttingScene = false;

	g_bChangingForRestore = false;

	CountOut = 1;

	g_NextScene = {0, 0, 0};
	g_HookScene = {0, 0, 0};
	g_DelayedScene = {0, 0, 0};

	g_pMouseProcess = nullptr;
	g_pKeyboardProcess = nullptr;

	g_hCdChangeScene = 0;
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

// For the languages, refer to the LANGUAGE enum in dw.h

const char *const TinselEngine::_sampleIndices[][3] = {
	{ "english.idx", "english1.idx", "english2.idx" },	// English
	{ "french.idx", "french1.idx", "french2.idx" },		// French
	{ "german.idx", "german1.idx", "german2.idx" },		// German
	{ "english.idx", "english1.idx", "english2.idx" },	// Italian
	{ "english.idx", "english1.idx", "english2.idx" },	// Spanish
	{ "english.idx", "english1.idx", "english2.idx" },	// Hebrew (FIXME: not sure if this is correct)
	{ "english.idx", "english1.idx", "english2.idx" },	// Hungarian (FIXME: not sure if this is correct)
	{ "japanese.idx", "japanese1.idx", "japanese2.idx" },	// Japanese
	{ "us.idx", "us1.idx", "us2.idx" }					// US English
};
const char *const TinselEngine::_sampleFiles[][3] = {
	{ "english.smp", "english1.smp", "english2.smp" },	// English
	{ "french.smp", "french1.smp", "french2.smp" },		// French
	{ "german.smp", "german1.smp", "german2.smp" },		// German
	{ "english.smp", "english1.smp", "english2.smp" },	// Italian
	{ "english.smp", "english1.smp", "english2.smp" },	// Spanish
	{ "english.smp", "english1.smp", "english2.smp" },	// Hebrew (FIXME: not sure if this is correct)
	{ "english.smp", "english1.smp", "english2.smp" },	// Hungarian (FIXME: not sure if this is correct)
	{ "japanese.smp", "japanese1.smp", "japanese2.smp" },	// Japanese
	{ "us.smp", "us1.smp", "us2.smp" },					// US English
};
const char *const TinselEngine::_textFiles[][3] = {
	{ "english.txt", "english1.txt", "english2.txt" },	// English
	{ "french.txt", "french1.txt", "french2.txt" },		// French
	{ "german.txt", "german1.txt", "german2.txt" },		// German
	{ "italian.txt", "italian1.txt", "italian2.txt" },	// Italian
	{ "spanish.txt", "spanish1.txt", "spanish2.txt" },	// Spanish
	{ "english.txt", "english1.txt", "english2.txt" },	// Hebrew (FIXME: not sure if this is correct)
	{ "english.txt", "english1.txt", "english2.txt" },	// Hungarian (FIXME: not sure if this is correct)
	{ "japanese.txt", "japanese1.txt", "japanese2.txt" },	// Japanese
	{ "us.txt", "us1.txt", "us2.txt" }					// US English
};


TinselEngine::TinselEngine(OSystem *syst, const TinselGameDescription *gameDesc) :
		Engine(syst), _gameDescription(gameDesc), _random("tinsel"),
		_sound(0), _midiMusic(0), _pcmMusic(0), _bmv(0) {
	// Register debug flags
	DebugMan.addDebugChannel(kTinselDebugAnimations, "animations", "Animations debugging");
	DebugMan.addDebugChannel(kTinselDebugActions, "actions", "Actions debugging");
	DebugMan.addDebugChannel(kTinselDebugSound, "sound", "Sound debugging");
	DebugMan.addDebugChannel(kTinselDebugMusic, "music", "Music debugging");

	_vm = this;

	_gameId = 0;
	_driver = nullptr;

	_config = new Config(this);

	// Setup mixer
	syncSoundSettings();

	const GameSettings *g;

	const char *gameid = ConfMan.get("gameid").c_str();
	for (g = tinselSettings; g->gameid; ++g)
		if (!scumm_stricmp(g->gameid, gameid))
			_gameId = g->id;

	_system->getAudioCDManager()->open();

	_mousePos.x = 0;
	_mousePos.y = 0;
	_keyHandler = nullptr;
	_dosPlayerDir = 0;
}

TinselEngine::~TinselEngine() {
	_system->getAudioCDManager()->stop();
	delete _cursor;
	delete _bg;
	delete _font;
	delete _bmv;
	delete _sound;
	delete _music;
	delete _midiMusic;
	delete _pcmMusic;
	_screenSurface.free();
	FreeSaveScenes();
	FreeTextBuffer();
	FreeObjectList();
	FreeGlobalProcesses();
	FreeGlobals();

	delete _dialogs; 
	delete _scroll;
	delete _handle;
	delete _actor;
	delete _config;

	MemoryDeinit();

	// Reset global vars
	ResetVarsDrives();	// drives.cpp
	ResetVarsEvents();	// events.cpp
	RebootScalingReels(); // mareels.cpp
	ResetVarsMove();	// move.cpp
	ResetVarsPalette();	// palette.cpp
	ResetVarsPCode();	// pcode.cpp
	ResetVarsPDisplay();	// pdisplay.cpp
	ResetVarsPlay();	// play.cpp
	ResetVarsPolygons();	// polygons.cpp
	RebootMovers();       // rince.cpp
	ResetVarsSaveLoad();	// saveload.cpp
	ResetVarsSaveScn();	// savescn.cpp
	ResetVarsScene();	// scene.cpp
	ResetVarsSched();	// sched.cpp
	ResetVarsStrRes();	// strres.cpp
	FreeTextBuffer();     // strres.cpp
	ResetVarsSysVar();	// sysvar.cpp
	FreeAllTokens();	// token.cpp
	RebootTimers();       // timers.cpp
	ResetVarsTinlib();	// tinlib.cpp
	ResetVarsTinsel();	// tinsel.cpp
}

Common::String TinselEngine::getSavegameFilename(int16 saveNum) const {
	return Common::String::format("%s.%03d", getTargetName().c_str(), saveNum);
}

void TinselEngine::initializePath(const Common::FSNode &gamePath) {
	if (TinselV1PSX) {
		// Add subfolders needed for PSX versions of Discworld 1
		SearchMan.addDirectory(gamePath.getPath(), gamePath, 0, 3, true);
	} else {
		// Add DW2 subfolder to search path in case user is running directly from the CDs
		SearchMan.addSubDirectoryMatching(gamePath, "dw2");

		// Location of Miles audio files (sample.ad and sample.opl) in Discworld 1
		SearchMan.addSubDirectoryMatching(gamePath, "drivers");
		Engine::initializePath(gamePath);
	}
}

Common::Error TinselEngine::run() {
	_midiMusic = new MidiMusicPlayer(this);
	_pcmMusic = new PCMMusicPlayer();
	_music = new Music();
	_sound = new SoundManager(this);
	_bmv = new BMVPlayer();
	_font = new Font();
	_bg = new Background(_font);
	_cursor = new Cursor();
	_actor = new Actor();
	_handle = new Handle();
	_scroll = new Scroll();
	_dialogs = new Dialogs();

	// Initialize backend
	if (getGameID() == GID_NOIR) {
		error("Discworld Noir is not yet supported");
	} else if (getGameID() == GID_DW2) {
#ifndef DW2_EXACT_SIZE
		initGraphics(640, 480);
#else
		initGraphics(640, 432);
#endif
		_screenSurface.create(640, 432, Graphics::PixelFormat::createFormatCLUT8());
	} else {
		initGraphics(320, 200);
		_screenSurface.create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	}

	setDebugger(new Console());

	CoroScheduler.reset();

	InitSysVars();

	// init memory manager
	MemoryInit();

	// load user configuration
	_vm->_config->readFromDisk();

#if 1
	// FIXME: The following is taken from RestartGame().
	// It may have to be adjusted a bit
	CountOut = 1;

	_vm->_cursor->RebootCursor();
	RebootDeadTags();
	RebootMovers();
	resetUserEventTime();
	RebootTimers();
	RebootScalingReels();

	g_DelayedScene.scene = g_HookScene.scene = 0;
#endif

	// Load in text strings
	ChangeLanguage(_vm->_config->_language);

	// Init palette and object managers, scheduler, keyboard and mouse
	RestartDrivers();

	// load in graphics info
	_vm->_handle->SetupHandleTable();

	// Actors, globals and inventory icons
	LoadBasicChunks();

	// Continuous game processes
	CreateConstProcesses();

	// allow game to run in the background
	//RestartBackgroundProcess();	// FIXME: is this still needed?

	//dumpMusic();	// dumps all of the game's music in external XMIDI files

	// Load game from specified slot, if any
	//
	// TODO: We might want to think about properly taking care of possible
	// errors when loading the save state.

	if (ConfMan.hasKey("save_slot")) {
		if (loadGameState(ConfMan.getInt("save_slot")).getCode() == Common::kNoError)
			g_loadingFromGMM = true;
	}

	// Foreground loop
	uint32 timerVal = 0;
	while (!shouldQuit()) {
		// Check for time to do next game cycle
		if ((g_system->getMillis() > timerVal + GAME_FRAME_DELAY)) {
			timerVal = g_system->getMillis();
			_system->getAudioCDManager()->update();
			NextGameCycle();
		}

		if (g_bRestart) {
			RestartGame();
			g_bRestart = false;
			g_bHasRestarted = true;	// Set restarted flag
		}

		// Save/Restore scene file transfers
		ProcessSRQueue();

		// Handle any playing movie
		_bmv->FettleBMV();

#ifdef DEBUG
		if (g_bFast)
			continue;		// run flat-out
#endif
		// Loop processing events while there are any pending
		while (pollEvent())
			;

		DoCdChange();

		if (_bmv->MoviePlaying() && _bmv->NextMovieTime())
			g_system->delayMillis(MAX<int>(_bmv->NextMovieTime() - g_system->getMillis() + _bmv->MovieAudioLag(), 0));
		else
			g_system->delayMillis(10);
	}

	if (_bmv->MoviePlaying())
		_bmv->FinishBMV();

	// Write configuration
	_vm->_config->writeToDisk();

	EndScene();
	_bg->ResetBackground();

	return Common::kNoError;
}


void TinselEngine::NextGameCycle() {
	// Dim Music
	_pcmMusic->dimIteration();

	// Check for scene change
	ChangeScene(false);

	// Allow a user event for this schedule
	ResetEcount();

	// schedule process
	CoroScheduler.schedule();

	if (_bmv->MoviePlaying())
		_bmv->CopyMovieToScreen();
	else
		// redraw background
		_bg->DrawBackgnd();

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
	case Common::EVENT_WHEELUP:
	case Common::EVENT_WHEELDOWN:
		// Add button to queue for the mouse process
		_mouseButtons.push_back(event.type);
		break;

	case Common::EVENT_MOUSEMOVE:
		{
			// This fragment takes care of Tinsel 2 when it's been compiled with
			// blank areas at the top and bottom of the screen
			int ySkip = TinselV2 ? (g_system->getHeight() - _vm->screen().h) / 2 : 0;
			if ((event.mouse.y >= ySkip) && (event.mouse.y < (g_system->getHeight() - ySkip)))
				_mousePos = Common::Point(event.mouse.x, event.mouse.y - ySkip);
		}
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
void TinselEngine::CreateConstProcesses() {
	// Process to run the master script
	CoroScheduler.createProcess(PID_MASTER_SCR, MasterScriptProcess, NULL, 0);

	// Processes to run the cursor and inventory,
	CoroScheduler.createProcess(PID_CURSOR, CursorProcess, NULL, 0);
	CoroScheduler.createProcess(PID_INVENTORY, InventoryProcess, NULL, 0);
}

/**
 * Restart the game
 */
void TinselEngine::RestartGame() {
	_vm->_dialogs->HoldItem(INV_NOICON); // Holding nothing

	_bg->DropBackground();	// No background

	// Ditches existing infrastructure background
	_bg->InitBackground();

	// Next scene change won't need to fade out
	// -> reset the count used by ChangeScene
	CountOut = 1;

	_vm->_cursor->RebootCursor();
	RebootDeadTags();
	RebootMovers();
	RebootTimers();
	RebootScalingReels();

	g_DelayedScene.scene = g_HookScene.scene = 0;

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
void TinselEngine::RestartDrivers() {
	// init the palette manager
	ResetPalAllocator();

	// init the object manager
	KillAllObjects();

	// init the process scheduler
	CoroScheduler.reset();

	// init the event handlers
	g_pMouseProcess = CoroScheduler.createProcess(PID_MOUSE, MouseProcess, NULL, 0);
	g_pKeyboardProcess = CoroScheduler.createProcess(PID_KEYBOARD, KeyboardProcess, NULL, 0);

	// open MIDI files
	_vm->_music->OpenMidiFiles();

	// open sample files (only if mixer is ready)
	if (_mixer->isReady()) {
		_sound->openSampleFiles();
	}

	// Set midi volume
	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	_vm->_music->SetMidiVolume(mute ? 0 : _vm->_config->_musicVolume);
}

/**
 * Remove keyboard, mouse and joystick drivers.
 */
void TinselEngine::ChopDrivers() {
	// remove sound driver
	_vm->_music->StopMidi();
	_sound->stopAllSamples();
	_vm->_music->DeleteMidiBuffer();

	// remove event drivers
	CoroScheduler.killProcess(g_pMouseProcess);
	CoroScheduler.killProcess(g_pKeyboardProcess);
}

/**
 * Process a keyboard event
 */
void TinselEngine::ProcessKeyEvent(const Common::Event &event) {
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
	_keypresses.push_back(event);
}

const char *TinselEngine::getSampleIndex(LANGUAGE lang) {
	int cd;

	if (TinselV2) {
		cd = GetCurrentCD();
		assert((cd == 1) || (cd == 2));
		assert(((unsigned int) lang) < NUM_LANGUAGES);

		if (lang == TXT_ENGLISH)
			if (_vm->getLanguage() == Common::EN_USA)
				lang = TXT_US;

	} else {
		cd = 0;
		if (lang != TXT_JAPANESE)
			lang = TXT_ENGLISH;
	}

	return _sampleIndices[lang][cd];
}

const char *TinselEngine::getSampleFile(LANGUAGE lang) {
	int cd;

	if (TinselV2) {
		cd = GetCurrentCD();
		assert((cd == 1) || (cd == 2));
		assert(((unsigned int) lang) < NUM_LANGUAGES);

		if (lang == TXT_ENGLISH)
			if (_vm->getLanguage() == Common::EN_USA)
				lang = TXT_US;

	} else {
		cd = 0;
		if (lang != TXT_JAPANESE)
			lang = TXT_ENGLISH;
	}

	return _sampleFiles[lang][cd];
}

const char *TinselEngine::getTextFile(LANGUAGE lang) {
	assert(((unsigned int) lang) < NUM_LANGUAGES);

	int cd;

	if (TinselV2) {
		cd = GetCurrentCD();
		assert((cd == 1) || (cd == 2));

		if (lang == TXT_ENGLISH)
			if (_vm->getLanguage() == Common::EN_USA)
				lang = TXT_US;

	} else
		cd = 0;

	return _textFiles[lang][cd];
}

} // End of namespace Tinsel
