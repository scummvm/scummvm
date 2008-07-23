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
#include "tinsel/dw.h"
#include "tinsel/faders.h"		// FadeOutFast()
#include "tinsel/graphics.h"		// ClearScreen()
#include "tinsel/handle.h"
#include "tinsel/inventory.h"
#include "tinsel/music.h"
#include "tinsel/pid.h"
#include "tinsel/rince.h"
#include "tinsel/savescn.h"
#include "tinsel/sched.h"
#include "tinsel/scroll.h"
#include "tinsel/sound.h"
#include "tinsel/tinlib.h"
#include "tinsel/token.h"

namespace Tinsel {

//----------------- EXTERN FUNCTIONS --------------------

// in BG.C
extern void startupBackground(SCNHANDLE bfilm);
extern SCNHANDLE GetBgroundHandle(void);
extern void SetDoFadeIn(bool tf);

// In DOS_DW.C
void RestoreMasterProcess(PINT_CONTEXT pic);

// in EVENTS.C (declared here and not in events.h because of strange goings-on)
void RestoreProcess(PINT_CONTEXT pic);

// in PLAY.C
extern void playThisReel(SCNHANDLE film, short reelnum, short z, int x, int y);

// in SCENE.C
extern SCNHANDLE GetSceneHandle(void);
extern void NewScene(SCNHANDLE scene, int entry);




//----------------- LOCAL DEFINES --------------------

enum {
	RS_COUNT = 5,	// Restore scene count

	MAX_NEST = 4
};


//----------------- LOCAL GLOBAL DATA --------------------

static bool ASceneIsSaved = false;

static int savedSceneCount = 0;

//static SAVED_DATA s_ssData[MAX_NEST];
static SAVED_DATA *s_ssData = 0;
static SAVED_DATA sgData;

static SAVED_DATA *s_rsd = 0;

static int s_restoreSceneCount = 0;

static bool bNoFade = false;

//----------------- FORWARD REFERENCES --------------------



void InitialiseSs(void) {
	if (s_ssData == NULL) {
		s_ssData = (SAVED_DATA *)calloc(MAX_NEST, sizeof(SAVED_DATA));
		if (s_ssData == NULL) {
			error("Cannot allocate memory for scene changes");
		}
	} else
		savedSceneCount = 0;
}

void FreeSs(void) {
	if (s_ssData) {
		free(s_ssData);
		s_ssData = NULL;
	}
}

/**
 * Save current scene.
 * @param sd			Pointer to the scene data
 */
void SaveScene(SAVED_DATA *sd) {
	sd->SavedSceneHandle = GetSceneHandle();
	sd->SavedBgroundHandle = GetBgroundHandle();
	SaveMovers(sd->SavedMoverInfo);
	sd->NumSavedActors = SaveActors(sd->SavedActorInfo);
	PlayfieldGetPos(FIELD_WORLD, &sd->SavedLoffset, &sd->SavedToffset);
	SaveInterpretContexts(sd->SavedICInfo);
	SaveDeadPolys(sd->SavedDeadPolys);
	sd->SavedControl = TestToken(TOKEN_CONTROL);
	CurrentMidiFacts(&sd->SavedMidi, &sd->SavedLoop);
	sd->SavedNoBlocking = bNoBlocking;
	GetNoScrollData(&sd->SavedNoScrollData);

	ASceneIsSaved = true;
}

/**
 * Initiate restoration of the saved scene.
 * @param sd			Pointer to the scene data
 * @param bFadeOut		Flag to perform a fade out
 */
void RestoreScene(SAVED_DATA *sd, bool bFadeOut) {
	s_rsd = sd;

	if (bFadeOut)
		s_restoreSceneCount = RS_COUNT + COUNTOUT_COUNT;	// Set restore scene count
	else
		s_restoreSceneCount = RS_COUNT;	// Set restore scene count
}

/**
 * Checks that all non-moving actors are playing the same reel as when
 * the scene was saved.
 * Also 'stand' all the moving actors at their saved positions.
 */
void sortActors(SAVED_DATA *rsd) {
	for (int i = 0; i < rsd->NumSavedActors; i++) {
		ActorsLife(rsd->SavedActorInfo[i].actorID, rsd->SavedActorInfo[i].bAlive);

		// Should be playing the same reel.
		if (rsd->SavedActorInfo[i].presFilm != 0) {
			if (!actorAlive(rsd->SavedActorInfo[i].actorID))
				continue;

			playThisReel(rsd->SavedActorInfo[i].presFilm, rsd->SavedActorInfo[i].presRnum, rsd->SavedActorInfo[i].z,
					rsd->SavedActorInfo[i].presX, rsd->SavedActorInfo[i].presY);
		}
	}

	RestoreAuxScales(rsd->SavedMoverInfo);
	for (int i = 0; i < MAX_MOVERS; i++) {
		if (rsd->SavedMoverInfo[i].MActorState == NORM_MACTOR)
			stand(rsd->SavedMoverInfo[i].actorID, rsd->SavedMoverInfo[i].objx,
				rsd->SavedMoverInfo[i].objy, rsd->SavedMoverInfo[i].lastfilm);
	}
}


//---------------------------------------------------------------------------

void ResumeInterprets(SAVED_DATA *rsd) {
	// Master script only affected on restore game, not restore scene
	if (rsd == &sgData) {
		KillMatchingProcess(PID_MASTER_SCR, -1);
		FreeMasterInterpretContext();
	}

	for (int i = 0; i < MAX_INTERPRET; i++) {
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

		case GS_ACTOR:
			RestoreActorProcess(rsd->SavedICInfo[i].actorid, &rsd->SavedICInfo[i]);
			break;

		case GS_POLYGON:
		case GS_SCENE:
			RestoreProcess(&rsd->SavedICInfo[i]);
			break;
		}
	}
}

/**
 * Do restore scene
 * @param n			Id
 */
static int DoRestoreScene(SAVED_DATA *rsd, int n) {
	switch (n) {
	case RS_COUNT + COUNTOUT_COUNT:
		// Trigger pre-load and fade and start countdown
		FadeOutFast(NULL);
		break;

	case RS_COUNT:
		_vm->_sound->stopAllSamples();
		ClearScreen(0L);
		RestoreDeadPolys(rsd->SavedDeadPolys);
		NewScene(rsd->SavedSceneHandle, NO_ENTRY_NUM);
		SetDoFadeIn(!bNoFade);
		bNoFade = false;
		startupBackground(rsd->SavedBgroundHandle);
		KillScroll();
		PlayfieldSetPos(FIELD_WORLD, rsd->SavedLoffset, rsd->SavedToffset);
		bNoBlocking = rsd->SavedNoBlocking;
		RestoreNoScrollData(&rsd->SavedNoScrollData);
/*
		break;

	case RS_COUNT - 1:
*/
		sortActors(rsd);
		break;

	case 2:
		break;

	case 1:
		RestoreMidiFacts(rsd->SavedMidi, rsd->SavedLoop);
		if (rsd->SavedControl)
			control(CONTROL_ON);	// TOKEN_CONTROL was free
		ResumeInterprets(rsd);
	}

	return n - 1;
}

/**
 * Restore game
 * @param num			num
 */
void RestoreGame(int num) {
	KillInventory();

	RequestRestoreGame(num, &sgData, &savedSceneCount, s_ssData);
	
	// Actual restoring is performed by ProcessSRQueue
}

/**
 * Save game
 * @param name			Name of savegame
 * @param desc			Description of savegame
 */
void SaveGame(char *name, char *desc) {
	// Get current scene data
	SaveScene(&sgData);

	RequestSaveGame(name, desc, &sgData, &savedSceneCount, s_ssData);
	
	// Actual saving is performed by ProcessSRQueue
}


//---------------------------------------------------------------------------------

bool IsRestoringScene() {
	if (s_restoreSceneCount) {
		s_restoreSceneCount = DoRestoreScene(s_rsd, s_restoreSceneCount);
	}

	return s_restoreSceneCount ? true : false;
}

/**
 * Please Restore Scene
 */
void PleaseRestoreScene(bool bFade) {
	// only called by restore_scene PCODE
	if (s_restoreSceneCount == 0) {
		assert(savedSceneCount >= 1); // No saved scene to restore

		if (ASceneIsSaved)
			RestoreScene(&s_ssData[--savedSceneCount], bFade);
		if (!bFade)
			bNoFade = true;
	}
}

/**
 * Please Save Scene
 */
void PleaseSaveScene(CORO_PARAM) {
	// only called by save_scene PCODE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	assert(savedSceneCount < MAX_NEST); // nesting limit reached

	// Don't save the same thing multiple times!
	// FIXME/TODO: Maybe this can be changed to an assert?
	if (savedSceneCount && s_ssData[savedSceneCount-1].SavedSceneHandle == GetSceneHandle())
		CORO_KILL_SELF();

	SaveScene(&s_ssData[savedSceneCount++]);

	CORO_END_CODE;
}

} // end of namespace Tinsel
