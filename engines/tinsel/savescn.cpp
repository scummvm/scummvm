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
 * Save and restore scene and game.
 */


#include "tinsel/actors.h"
#include "tinsel/background.h"
#include "tinsel/config.h"
#include "tinsel/drives.h"
#include "tinsel/dw.h"
#include "tinsel/faders.h"		// FadeOutFast()
#include "tinsel/graphics.h"		// ClearScreen()
#include "tinsel/handle.h"
#include "tinsel/heapmem.h"
#include "tinsel/dialogs.h"
#include "tinsel/music.h"
#include "tinsel/pid.h"
#include "tinsel/play.h"
#include "tinsel/polygons.h"
#include "tinsel/rince.h"
#include "tinsel/savescn.h"
#include "tinsel/scene.h"
#include "tinsel/sched.h"
#include "tinsel/scroll.h"
#include "tinsel/sound.h"
#include "tinsel/sysvar.h"
#include "tinsel/tinlib.h"
#include "tinsel/token.h"

namespace Tinsel {

//----------------- EXTERN FUNCTIONS --------------------

// in BG.C
extern SCNHANDLE GetBgroundHandle(void);
extern void SetDoFadeIn(bool tf);

// In DOS_DW.C
void RestoreMasterProcess(INT_CONTEXT *pic);

// in EVENTS.C (declared here and not in events.h because of strange goings-on)
void RestoreProcess(INT_CONTEXT *pic);

// in SCENE.C
extern SCNHANDLE GetSceneHandle(void);


//----------------- LOCAL DEFINES --------------------

enum {
	RS_COUNT = 5,	// Restore scene count

	MAX_NEST = 4
};

//----------------- EXTERNAL GLOBAL DATA --------------------

extern int	thingHeld;
extern int	restoreCD;
extern SRSTATE SRstate;

//----------------- LOCAL GLOBAL DATA --------------------

static bool ASceneIsSaved = false;

static int savedSceneCount = 0;

static bool bNotDoneYet = false;

//static SAVED_DATA ssData[MAX_NEST];
static SAVED_DATA *ssData = NULL;
static SAVED_DATA sgData;

static SAVED_DATA *rsd = 0;

static int RestoreSceneCount = 0;

static bool bNoFade = false;

//----------------- FORWARD REFERENCES --------------------

/**
 * Save current scene.
 * @param sd			Pointer to the scene data
 */
void DoSaveScene(SAVED_DATA *sd) {
	sd->SavedSceneHandle = GetSceneHandle();
	sd->SavedBgroundHandle = GetBgroundHandle();
	SaveMovers(sd->SavedMoverInfo);
	sd->NumSavedActors = SaveActors(sd->SavedActorInfo);
	PlayfieldGetPos(FIELD_WORLD, &sd->SavedLoffset, &sd->SavedToffset);
	SaveInterpretContexts(sd->SavedICInfo);
	sd->SavedControl = ControlIsOn();
	sd->SavedNoBlocking = GetNoBlocking();
	GetNoScrollData(&sd->SavedNoScrollData);

	if (TinselV2) {
		// Tinsel 2 specific data save
		SaveActorZ(sd->savedActorZ);
		SaveZpositions(sd->zPositions);
		SavePolygonStuff(sd->SavedPolygonStuff);
		_vm->_pcmMusic->getTunePlaying(sd->SavedTune, sizeof(sd->SavedTune));
		sd->bTinselDim = _vm->_pcmMusic->getMusicTinselDimmed();
		sd->SavedScrollFocus = GetScrollFocus();
		SaveSysVars(sd->SavedSystemVars);
		SaveSoundReels(sd->SavedSoundReels);

	} else {
		// Tinsel 1 specific data save
		SaveDeadPolys(sd->SavedDeadPolys);
		CurrentMidiFacts(&sd->SavedMidi, &sd->SavedLoop);
	}

	ASceneIsSaved = true;
}

/**
 * Initiate restoration of the saved scene.
 * @param sd			Pointer to the scene data
 * @param bFadeOut		Flag to perform a fade out
 */
void DoRestoreScene(SAVED_DATA *sd, bool bFadeOut) {
	rsd = sd;

	if (bFadeOut)
		RestoreSceneCount = RS_COUNT + COUNTOUT_COUNT;	// Set restore scene count
	else
		RestoreSceneCount = RS_COUNT;	// Set restore scene count
}

void InitialiseSaveScenes(void) {
	if (ssData == NULL) {
		ssData = (SAVED_DATA *)calloc(MAX_NEST, sizeof(SAVED_DATA));
		if (ssData == NULL) {
			error("Cannot allocate memory for scene changes");
		}
	} else {
		// Re-initialise - no scenes saved
		savedSceneCount = 0;
	}
}

void FreeSaveScenes(void) {
	if (ssData) {
		free(ssData);
		ssData = NULL;
	}
}

/**
 * Checks that all non-moving actors are playing the same reel as when
 * the scene was saved.
 * Also 'stand' all the moving actors at their saved positions.
 */
void sortActors(SAVED_DATA *sd) {
	assert(!TinselV2);
	for (int i = 0; i < sd->NumSavedActors; i++) {
		ActorsLife(sd->SavedActorInfo[i].actorID, sd->SavedActorInfo[i].bAlive);

		// Should be playing the same reel.
		if (sd->SavedActorInfo[i].presFilm != 0) {
			if (!actorAlive(sd->SavedActorInfo[i].actorID))
				continue;

			RestoreActorReels(sd->SavedActorInfo[i].presFilm, sd->SavedActorInfo[i].presRnum, sd->SavedActorInfo[i].zFactor,
					sd->SavedActorInfo[i].presPlayX, sd->SavedActorInfo[i].presPlayY);
		}
	}

	RestoreAuxScales(sd->SavedMoverInfo);
	for (int i = 0; i < MAX_MOVERS; i++) {
		if (sd->SavedMoverInfo[i].bActive)
			Stand(nullContext, sd->SavedMoverInfo[i].actorID, sd->SavedMoverInfo[i].objX,
				sd->SavedMoverInfo[i].objY, sd->SavedMoverInfo[i].hLastfilm);
	}
}

/**
 * Stand all the moving actors at their saved positions.
 * Not called from the foreground.
 */
static void SortMAProcess(CORO_PARAM, const void *) {
	CORO_BEGIN_CONTEXT;
		int i;
		int viaActor;
	CORO_END_CONTEXT(_ctx);


	CORO_BEGIN_CODE(_ctx);

	// Disable via actor for the stands
	_ctx->viaActor = SysVar(ISV_DIVERT_ACTOR);
	SetSysVar(ISV_DIVERT_ACTOR, 0);

	RestoreAuxScales(rsd->SavedMoverInfo);

	for (_ctx->i = 0; _ctx->i < MAX_MOVERS; _ctx->i++) {
		if (rsd->SavedMoverInfo[_ctx->i].bActive) {
			CORO_INVOKE_ARGS(Stand, (CORO_SUBCTX, rsd->SavedMoverInfo[_ctx->i].actorID,
				rsd->SavedMoverInfo[_ctx->i].objX, rsd->SavedMoverInfo[_ctx->i].objY,
				rsd->SavedMoverInfo[_ctx->i].hLastfilm));

			if (rsd->SavedMoverInfo[_ctx->i].bHidden)
				HideMover(GetMover(rsd->SavedMoverInfo[_ctx->i].actorID));
		}

		ActorPalette(rsd->SavedMoverInfo[_ctx->i].actorID,
			rsd->SavedMoverInfo[_ctx->i].startColour, rsd->SavedMoverInfo[_ctx->i].paletteLength);

		if (rsd->SavedMoverInfo[_ctx->i].brightness != BOGUS_BRIGHTNESS)
			ActorBrightness(rsd->SavedMoverInfo[_ctx->i].actorID, rsd->SavedMoverInfo[_ctx->i].brightness);
	}

	// Restore via actor
	SetSysVar(ISV_DIVERT_ACTOR, _ctx->viaActor);

	bNotDoneYet = false;

	CORO_END_CODE;
}


//---------------------------------------------------------------------------

void ResumeInterprets(void) {
	// Master script only affected on restore game, not restore scene
	if (!TinselV2 && (rsd == &sgData)) {
		g_scheduler->killMatchingProcess(PID_MASTER_SCR, -1);
		FreeMasterInterpretContext();
	}

	for (int i = 0; i < NUM_INTERPRET; i++) {
		switch (rsd->SavedICInfo[i].GSort) {
		case GS_NONE:
			break;

		case GS_INVENTORY:
			if (rsd->SavedICInfo[i].event != POINTED) {
				RestoreProcess(&rsd->SavedICInfo[i]);
			}
			break;

		case GS_MASTER:
			// Master script only affected on restore game, not restore scene
			if (rsd == &sgData)
				RestoreMasterProcess(&rsd->SavedICInfo[i]);
			break;

		case GS_PROCESS:
			// Tinsel 2 process
			RestoreSceneProcess(&rsd->SavedICInfo[i]);
			break;

		case GS_GPROCESS:
			// Tinsel 2 Global processes only affected on restore game, not restore scene
			if (rsd == &sgData)
				RestoreGlobalProcess(&rsd->SavedICInfo[i]);
			break;

		case GS_ACTOR:
			if (TinselV2)
				RestoreProcess(&rsd->SavedICInfo[i]);
			else
				RestoreActorProcess(rsd->SavedICInfo[i].idActor, &rsd->SavedICInfo[i]);
			break;

		case GS_POLYGON:
		case GS_SCENE:
			RestoreProcess(&rsd->SavedICInfo[i]);
			break;

		default:
			warning("Unhandled GSort in ResumeInterprets");
		}
	}
}

/**
 * Do restore scene
 * @param n			Id
 */
static int DoRestoreSceneFrame(SAVED_DATA *sd, int n) {
	switch (n) {
	case RS_COUNT + COUNTOUT_COUNT:
		// Trigger pre-load and fade and start countdown
		FadeOutFast(NULL);
		break;

	case RS_COUNT:
		_vm->_sound->stopAllSamples();
		ClearScreen();

		// Master script only affected on restore game, not restore scene
		if (TinselV2 && (sd == &sgData)) {
			g_scheduler->killMatchingProcess(PID_MASTER_SCR);
			KillGlobalProcesses();
			FreeMasterInterpretContext();
		}

		if (TinselV2) {
			RestorePolygonStuff(sd->SavedPolygonStuff);

			// Abandon temporarily if different CD
			if (sd == &sgData && restoreCD != GetCurrentCD()) {
				SRstate = SR_IDLE;

				EndScene();
				SetNextCD(restoreCD);
				CDChangeForRestore(restoreCD);

				return 0;
			}
		} else {
			RestoreDeadPolys(sd->SavedDeadPolys);
		}

		// Start up the scene
		StartNewScene(sd->SavedSceneHandle, NO_ENTRY_NUM);

		SetDoFadeIn(!bNoFade);
		bNoFade = false;
		StartupBackground(nullContext, sd->SavedBgroundHandle);

		if (TinselV2) {
			Offset(EX_USEXY, sd->SavedLoffset, sd->SavedToffset);
		} else {
			KillScroll();
			PlayfieldSetPos(FIELD_WORLD, sd->SavedLoffset, sd->SavedToffset);
			SetNoBlocking(sd->SavedNoBlocking);
		}

		RestoreNoScrollData(&sd->SavedNoScrollData);

		if (TinselV2) {
			// create process to sort out the moving actors
			g_scheduler->createProcess(PID_MOVER, SortMAProcess, NULL, 0);
			bNotDoneYet = true;

			RestoreActorZ(sd->savedActorZ);
			RestoreZpositions(sd->zPositions);
			RestoreSysVars(sd->SavedSystemVars);
			CreateGhostPalette(BgPal());
			RestoreActors(sd->NumSavedActors, sd->SavedActorInfo);
			RestoreSoundReels(sd->SavedSoundReels);
			return 1;
		}

		sortActors(sd);
		break;

	case 2:
		break;

	case 1:
		if (TinselV2) {
			if (bNotDoneYet)
				return n;

			if (sd == &sgData)
				HoldItem(thingHeld, true);
			if (sd->bTinselDim)
				_vm->_pcmMusic->dim(true);
			_vm->_pcmMusic->restoreThatTune(sd->SavedTune);
			ScrollFocus(sd->SavedScrollFocus);
		} else {
			RestoreMidiFacts(sd->SavedMidi, sd->SavedLoop);
		}

		if (sd->SavedControl)
			ControlOn();	// Control was on
		ResumeInterprets();
		break;

	default:
		break;
	}

	return n - 1;
}

/**
 * Restore game
 * @param num			num
 */
void RestoreGame(int num) {
	KillInventory();

	RequestRestoreGame(num, &sgData, &savedSceneCount, ssData);

	// Actual restoring is performed by ProcessSRQueue
}

/**
 * Save game
 * @param name			Name of savegame
 * @param desc			Description of savegame
 */
void SaveGame(char *name, char *desc) {
	// Get current scene data
	DoSaveScene(&sgData);

	RequestSaveGame(name, desc, &sgData, &savedSceneCount, ssData);

	// Actual saving is performed by ProcessSRQueue
}


//---------------------------------------------------------------------------------

bool IsRestoringScene() {
	if (RestoreSceneCount) {
		RestoreSceneCount = DoRestoreSceneFrame(rsd, RestoreSceneCount);
	}

	return RestoreSceneCount ? true : false;
}

/**
 * Restores Scene
 */
void TinselRestoreScene(bool bFade) {
	// only called by restore_scene PCODE
	if (RestoreSceneCount == 0) {
		assert(savedSceneCount >= 1); // No saved scene to restore

		if (ASceneIsSaved)
			DoRestoreScene(&ssData[--savedSceneCount], bFade);
		if (!bFade)
			bNoFade = true;
	}
}

/**
 * Please Save Scene
 */
void TinselSaveScene(CORO_PARAM) {
	// only called by save_scene PCODE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	assert(savedSceneCount < MAX_NEST); // nesting limit reached

	// Don't save the same thing multiple times!
	// FIXME/TODO: Maybe this can be changed to an assert?
	if (savedSceneCount && ssData[savedSceneCount-1].SavedSceneHandle == GetSceneHandle())
		CORO_KILL_SELF();

	DoSaveScene(&ssData[savedSceneCount++]);

	CORO_END_CODE;
}

} // End of namespace Tinsel
