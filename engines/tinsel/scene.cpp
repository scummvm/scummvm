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
#include "tinsel/dw.h"
#include "tinsel/graphics.h"
#include "tinsel/handle.h"
#include "tinsel/inventory.h"
#include "tinsel/film.h"
#include "tinsel/move.h"
#include "tinsel/rince.h"
#include "tinsel/sched.h"
#include "tinsel/scn.h"
#include "tinsel/scroll.h"
#include "tinsel/sound.h"	// stopAllSamples()
#include "tinsel/object.h"
#include "tinsel/pcode.h"
#include "tinsel/pid.h"	// process IDs
#include "tinsel/polygons.h"
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
	int32 numEntrance;	//!< number of entrances in this scene
	int32 numPoly;		//!< number of various polygons in this scene
	int32 numActor;		//!< number of actors in this scene
	int32 defRefer;		//!< Default refer direction
	SCNHANDLE hSceneScript;	//!< handle to scene script
	SCNHANDLE hEntrance;	//!< handle to table of entrances
	SCNHANDLE hPoly;	//!< handle to table of polygons
	SCNHANDLE hActor;	//!< handle to table of actors
} PACKED_STRUCT;

/** entrance structure - one per entrance */
struct ENTRANCE_STRUC {
	int32 eNumber;		//!< entrance number
	SCNHANDLE hScript;	//!< handle to entrance script
} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING


//----------------- LOCAL GLOBAL DATA --------------------

#ifdef DEBUG
static bool ShowPosition = false;	// Set when showpos() has been called
#endif

static SCNHANDLE SceneHandle = 0;	// Current scene handle - stored in case of Save_Scene()


/**
 * Started up for scene script and entrance script.
 */
static void SceneTinselProcess(CORO_PARAM, const void *param) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
		INT_CONTEXT *pic;
	CORO_END_CONTEXT(_ctx);

	// get the stuff copied to process when it was created
	SCNHANDLE *ss = (SCNHANDLE *)param;
	assert(*ss);		// Must have some code to run

	CORO_BEGIN_CODE(_ctx);

	_ctx->pic = InitInterpretContext(GS_SCENE, READ_LE_UINT32(ss), NOEVENT, NOPOLY, 0, NULL);
	CORO_INVOKE_1(Interpret, _ctx->pic);

	CORO_END_CODE;
}

/**
 * Get the SCENE_STRUC
 * Initialise polygons for the scene
 * Initialise the actors for this scene
 * Run the appropriate entrance code (if any)
 * Get the default refer type
 */
static void LoadScene(SCNHANDLE scene, int entry) {
	const SCENE_STRUC	*ss;
	const ENTRANCE_STRUC	*es;
	uint	i;

	// Scene structure
	SceneHandle = scene;	// Save scene handle in case of Save_Scene()

	LockMem(SceneHandle);		// Make sure scene is loaded
	LockScene(SceneHandle);		// Prevent current scene from being discarded

	ss = (const SCENE_STRUC *)FindChunk(scene, CHUNK_SCENE);
	assert(ss != NULL);

	// Initialise all the polygons for this scene
	InitPolygons(FROM_LE_32(ss->hPoly), FROM_LE_32(ss->numPoly), (entry == NO_ENTRY_NUM));

	// Initialise the actors for this scene
	StartActors(FROM_LE_32(ss->hActor), FROM_LE_32(ss->numActor), (entry != NO_ENTRY_NUM));

	if (entry != NO_ENTRY_NUM) {

		// Run the appropriate entrance code (if any)
		es = (const ENTRANCE_STRUC *)LockMem(FROM_LE_32(ss->hEntrance));
		for (i = 0; i < FROM_LE_32(ss->numEntrance); i++, es++) {
			if (FROM_LE_32(es->eNumber) == (uint)entry) {
				if (es->hScript)
					g_scheduler->createProcess(PID_TCODE, SceneTinselProcess, &es->hScript, sizeof(es->hScript));
				break;
			}
		}

		if (i == FROM_LE_32(ss->numEntrance))
			error("Non-existant scene entry number");

		if (ss->hSceneScript)
			g_scheduler->createProcess(PID_TCODE, SceneTinselProcess, &ss->hSceneScript, sizeof(ss->hSceneScript));
	}

	// Default refer type
	SetDefaultRefer(FROM_LE_32(ss->defRefer));
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
	DropNoScrolls();	// No no-scrolls
	DropBackground();	// No background
	DropMActors();		// No moving actors
	DropCursor();		// No cursor
	DropActors();		// No actor reels running
	FreeAllTokens();	// No-one has tokens
	FreeMostInterpretContexts();	// Only master script still interpreting

	_vm->_sound->stopAllSamples();		// Kill off any still-running sample

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

	bNoBlocking = false;

	RestartCursor();	// Restart the cursor
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

void NewScene(SCNHANDLE scene, int entry) {
	EndScene();	// Wrap up the last scene.

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

} // end of namespace Tinsel
