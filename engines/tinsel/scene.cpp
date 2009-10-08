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
 * Starts up new scenes.
 */

#include "tinsel/actors.h"
#include "tinsel/anim.h"
#include "tinsel/background.h"
#include "tinsel/config.h"
#include "tinsel/cursor.h"
#include "tinsel/dialogs.h"
#include "tinsel/dw.h"
#include "tinsel/graphics.h"
#include "tinsel/handle.h"
#include "tinsel/film.h"
#include "tinsel/font.h"
#include "tinsel/mareels.h"
#include "tinsel/move.h"
#include "tinsel/music.h"
#include "tinsel/object.h"
#include "tinsel/pcode.h"
#include "tinsel/pid.h"	// process IDs
#include "tinsel/play.h"
#include "tinsel/polygons.h"
#include "tinsel/rince.h"
#include "tinsel/sched.h"
#include "tinsel/scn.h"
#include "tinsel/scroll.h"
#include "tinsel/sound.h"	// stopAllSamples()
#include "tinsel/sysvar.h"
#include "tinsel/token.h"


namespace Tinsel {

//----------------- EXTERNAL FUNCTIONS ---------------------

// in BG.C
extern void DropBackground(void);

// in EFFECT.C
extern void EffectPolyProcess(CORO_PARAM, const void *);

// in PDISPLAY.C
#ifdef DEBUG
extern void CursorPositionProcess(CORO_PARAM, const void *);
#endif
extern void TagProcess(CORO_PARAM, const void *);
extern void PointProcess(CORO_PARAM, const void *);
extern void EnableTags(void);


//----------------- LOCAL DEFINES --------------------

#include "common/pack-start.h"	// START STRUCT PACKING

/** scene structure - one per scene */
struct SCENE_STRUC {
	int32 defRefer;		// Default refer direction (REFTYPE)
	SCNHANDLE hSceneScript;	// handle to scene script
	SCNHANDLE hSceneDesc;	// handle to scene description
	int32 numEntrance;	// number of entrances in this scene
	SCNHANDLE hEntrance;	// handle to table of entrances
	int32 numPoly;		// number of various polygons in this scene
	SCNHANDLE hPoly;	// handle to table of polygons
	int32 numTaggedActor;	// number of tagged actors in this scene
	SCNHANDLE hTaggedActor;	// handle to table of tagged actors
	int32 numProcess;	// number of processes in this scene
	SCNHANDLE hProcess;	// handle to table of processes
	SCNHANDLE hMusicScript;	// handle to music script data
	SCNHANDLE hMusicSegment;// handle to music segments
} PACKED_STRUCT;

/** entrance structure - one per entrance */
struct ENTRANCE_STRUC {
	int32 eNumber;		///< entrance number
	SCNHANDLE hScript;	///< handle to entrance script
	// Tinsel 2 fields
	SCNHANDLE hEntDesc;	// handle to entrance description
	uint32 flags;
} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING


//----------------- LOCAL GLOBAL DATA --------------------

#ifdef DEBUG
static bool ShowPosition = false;	// Set when showpos() has been called
#endif

SCNHANDLE newestScene = 0;

int sceneCtr = 0;
static int initialMyEscape;

static SCNHANDLE SceneHandle = 0;	// Current scene handle - stored in case of Save_Scene()

static bool bWatchingOut = false;

SCENE_STRUC tempStruc;

struct TP_INIT {
	SCNHANDLE hTinselCode;		// Code
	TINSEL_EVENT event;			// Triggering event
};

const SCENE_STRUC *GetSceneStruc(const byte *pStruc) {
	if (TinselVersion == TINSEL_V2)
		return (const SCENE_STRUC *)pStruc;

	// Copy appropriate fields into tempStruc, and return a pointer to it
	const byte *p = pStruc;
	memset(&tempStruc, 0, sizeof(SCENE_STRUC));

	tempStruc.numEntrance = READ_UINT32(p); p += sizeof(uint32);
	tempStruc.numPoly = READ_UINT32(p); p += sizeof(uint32);
	tempStruc.numTaggedActor = READ_UINT32(p); p += sizeof(uint32);
	tempStruc.defRefer = READ_UINT32(p); p += sizeof(uint32);
	tempStruc.hSceneScript = READ_UINT32(p); p += sizeof(uint32);
	tempStruc.hEntrance = READ_UINT32(p); p += sizeof(uint32);
	tempStruc.hPoly = READ_UINT32(p); p += sizeof(uint32);
	tempStruc.hTaggedActor = READ_UINT32(p); p += sizeof(uint32);

	return &tempStruc;
}


/**
 * Started up for scene script and entrance script.
 */
static void SceneTinselProcess(CORO_PARAM, const void *param) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
		INT_CONTEXT *pic;
		const TP_INIT *pInit;
		int myEscape;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// The following myEscape value setting is used for enabling title screen skipping in DW1
	if (TinselV1 && (sceneCtr == 1)) initialMyEscape = GetEscEvents();
	_ctx->myEscape = (TinselV1 && (sceneCtr < 4)) ? initialMyEscape : 0;

	// get the stuff copied to process when it was created
	_ctx->pInit = (const TP_INIT *)param;
	assert(_ctx->pInit);
	assert(_ctx->pInit->hTinselCode);		// Must have some code to run

	_ctx->pic = InitInterpretContext(GS_SCENE,
		READ_LE_UINT32(&_ctx->pInit->hTinselCode),
		TinselV2 ? _ctx->pInit->event : NOEVENT,
		NOPOLY,			// No polygon
		0,				// No actor
		NULL,			// No object
		_ctx->myEscape);
	CORO_INVOKE_1(Interpret, _ctx->pic);

	if (_ctx->pInit->event == CLOSEDOWN || _ctx->pInit->event == LEAVE_T2)
		bWatchingOut = false;

	CORO_END_CODE;
}

/**
 * Start up a SceneTinselProcess() running the scene
 * script for various events.
 */
void SendSceneTinselProcess(TINSEL_EVENT event) {
	SCENE_STRUC	*ss;

	if (event == CLOSEDOWN || event == LEAVE_T2)
		bWatchingOut = true;

	if (SceneHandle != (SCNHANDLE)NULL) {
		ss = (SCENE_STRUC *) FindChunk(SceneHandle, CHUNK_SCENE);

		if (ss->hSceneScript) {
			TP_INIT	init;

			init.event = event;
			init.hTinselCode = ss->hSceneScript;

			g_scheduler->createProcess(PID_TCODE, SceneTinselProcess, &init, sizeof(init));
		} else if (event == CLOSEDOWN)
			bWatchingOut = false;
	}
	else if (event == CLOSEDOWN)
		bWatchingOut = false;
}


/**
 * Get the SCENE_STRUC
 * Initialise polygons for the scene
 * Initialise the actors for this scene
 * Run the appropriate entrance code (if any)
 * Get the default refer type
 */

static void LoadScene(SCNHANDLE scene, int entry) {
	uint	i;
	TP_INIT init;
	const SCENE_STRUC	*ss;
	const ENTRANCE_STRUC	*es;

	// Scene handle
	SceneHandle = scene;		// Save scene handle in case of Save_Scene()
	LockMem(SceneHandle);		// Make sure scene is loaded
	LockScene(SceneHandle);		// Prevent current scene from being discarded

	if (TinselV2) {
		// CdPlay() stuff
		byte *cptr = FindChunk(scene, CHUNK_CDPLAY_FILENUM);
		assert(cptr);
		i = READ_LE_UINT32(cptr);
		assert(i < 512);
		cptr = FindChunk(scene, CHUNK_CDPLAY_FILENAME);
		assert(cptr);
		SetCdPlaySceneDetails(i, (const char *)cptr);
	}

	// Find scene structure
	ss = GetSceneStruc(FindChunk(scene, CHUNK_SCENE));
	assert(ss != NULL);

	if (TinselV2) {
		// Handle to scene description
		newestScene = FROM_LE_32(ss->hSceneDesc);

		// Music stuff
		char *cptr = (char *)FindChunk(scene, CHUNK_MUSIC_FILENAME);
		assert(cptr);
		_vm->_pcmMusic->setMusicSceneDetails(FROM_LE_32(ss->hMusicScript),
			FROM_LE_32(ss->hMusicSegment), cptr);
	}

	if (entry == NO_ENTRY_NUM) {
		// Restoring scene

		// Initialise all the polygons for this scene
		InitPolygons(FROM_LE_32(ss->hPoly), FROM_LE_32(ss->numPoly), true);

		// Initialise the actors for this scene
		StartTaggedActors(FROM_LE_32(ss->hTaggedActor), FROM_LE_32(ss->numTaggedActor), false);

		if (TinselV2)
			// Returning from cutscene
			SendSceneTinselProcess(RESTORE);

	} else {
		// Genuine new scene

		// Initialise all the polygons for this scene
		InitPolygons(FROM_LE_32(ss->hPoly), FROM_LE_32(ss->numPoly), false);

		// Initialise the actors for this scene
		StartTaggedActors(FROM_LE_32(ss->hTaggedActor), FROM_LE_32(ss->numTaggedActor), true);

		// Run the appropriate entrance code (if any)
		es = (const ENTRANCE_STRUC *)LockMem(FROM_LE_32(ss->hEntrance));
		for (i = 0; i < FROM_LE_32(ss->numEntrance); i++) {
			if (FROM_LE_32(es->eNumber) == (uint)entry) {
				if (es->hScript) {
					init.event = STARTUP;
					init.hTinselCode = es->hScript;

					g_scheduler->createProcess(PID_TCODE, SceneTinselProcess, &init, sizeof(init));
				}
				break;
			}

			// Move to next entrance
			if (TinselV2)
				++es;
			else
				es = (const ENTRANCE_STRUC *)((const byte *)es + 8);

		}

		if (i == FROM_LE_32(ss->numEntrance))
			error("Non-existant scene entry number");

		if (ss->hSceneScript) {
			init.event = STARTUP;
			init.hTinselCode = ss->hSceneScript;

			g_scheduler->createProcess(PID_TCODE, SceneTinselProcess, &init, sizeof(init));
		}
	}

	// Default refer type
	SetDefaultRefer(FROM_LE_32(ss->defRefer));

	// Scene's processes
	SceneProcesses(FROM_LE_32(ss->numProcess), FROM_LE_32(ss->hProcess));
}


/**
 * Wrap up the last scene.
 */
void EndScene(void) {
	if (SceneHandle != 0) {
		UnlockScene(SceneHandle);
		SceneHandle = 0;
	}

	KillInventory();	// Close down any open inventory

	DropPolygons();		// No polygons
	DropScroll();	// No no-scrolls
	DropBackground();	// No background
	DropMovers();		// No moving actors
	DropCursor();		// No cursor
	DropActors();		// No actor reels running
	FreeAllTokens();	// No-one has tokens
	FreeMostInterpretContexts();	// Only master script still interpreting

	if (TinselV2) {
		SetSysVar(ISV_DIVERT_ACTOR, 0);
		SetSysVar(ISV_GHOST_ACTOR, 0);
		SetSysVar(SV_MinimumXoffset, 0);
		SetSysVar(SV_MaximumXoffset, 0);
		SetSysVar(SV_MinimumYoffset, 0);
		SetSysVar(SV_MaximumYoffset, 0);
		ResetFontHandles();
		NoSoundReels();
	}

	_vm->_sound->stopAllSamples();		// Kill off any still-running sample
	//_vm->_mixer->stopAll();

	// init the palette manager
	ResetPalAllocator();

	// init the object manager
	KillAllObjects();

	// kill all destructable process
	g_scheduler->killMatchingProcess(PID_DESTROY, PID_DESTROY);
}

/**
 *
 */
void PrimeBackground(void) {
	// structure for playfields
	static PLAYFIELD playfield[] = {
		{	// FIELD WORLD
			NULL,		// display list
			0,			// init field x
			0,			// init field y
			0,			// x vel
			0,			// y vel
			Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT),	// clip rect
			false		// moved flag
		},
		{	// FIELD STATUS
			NULL,		// display list
			0,			// init field x
			0,			// init field y
			0,			// x vel
			0,			// y vel
			Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT),	// clip rect
			false		// moved flag
		}
	};

	// structure for background
	static BACKGND backgnd = {
		BLACK,			// sky colour
		Common::Point(0, 0),	// initial world pos
		Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT),	// scroll limits
		0,				// no background update process
		NULL,			// no x scroll table
		NULL,			// no y scroll table
		2,				// 2 playfields
		playfield,		// playfield pointer
		false			// no auto-erase
	};

	InitBackground(&backgnd);
}

/**
 * Start up the standard stuff for the next scene.
 */

void PrimeScene(void) {
	SetNoBlocking(false);
	SetSysVar(SYS_SceneFxDimFactor, SysVar(SYS_DefaultFxDimFactor));

	RestartCursor();	// Restart the cursor
	if (!TinselV2)
		EnableTags();		// Next scene with tags enabled

	g_scheduler->createProcess(PID_SCROLL, ScrollProcess, NULL, 0);
	g_scheduler->createProcess(PID_SCROLL, EffectPolyProcess, NULL, 0);

#ifdef DEBUG
	if (ShowPosition)
		g_scheduler->createProcess(PID_POSITION, CursorPositionProcess, NULL, 0);
#endif

	g_scheduler->createProcess(PID_TAG, TagProcess, NULL, 0);
	g_scheduler->createProcess(PID_TAG, PointProcess, NULL, 0);

	// init the current background
	PrimeBackground();
}

/**
 * Wrap up the last scene and start up the next scene.
 */

void StartNewScene(SCNHANDLE scene, int entry) {
	EndScene();	// Wrap up the last scene.

	if (TinselV2) {
		TouchMoverReels();

		LockMem(scene);	// Do CD change before PrimeScene
	}

	PrimeScene();	// Start up the standard stuff for the next scene.

	LoadScene(scene, entry);
}

#ifdef DEBUG
/**
 * Sets the ShowPosition flag, causing the cursor position process to be
 * created in each scene.
 */

void setshowpos(void) {
	ShowPosition = true;
}
#endif

/**
 * Return the current scene handle.
 */

SCNHANDLE GetSceneHandle(void) {
	return SceneHandle;
}

/**
 * DoHailScene
 */

void DoHailScene(SCNHANDLE scene) {
	// Find scene structure
	const SCENE_STRUC *ss = GetSceneStruc(FindChunk(scene, CHUNK_SCENE));

	if (ss != NULL && ss->hSceneScript) {
		TP_INIT	init;

		init.event = NOEVENT;
		init.hTinselCode = ss->hSceneScript;

		g_scheduler->createProcess(PID_TCODE, SceneTinselProcess, &init, sizeof(init));
	}
}

/**
 * WrapScene
 */
void WrapScene(void) {
	SendSceneTinselProcess(CLOSEDOWN);
}

} // End of namespace Tinsel
