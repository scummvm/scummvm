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
 * Glitter library functions.
 *
 * In the main called only from PCODE.C
 * Function names are the same as Glitter code function names.
 *
 * To ensure exclusive use of resources and exclusive control responsibilities.
 */

#define BODGE

#include "tinsel/actors.h"
#include "tinsel/background.h"
#include "tinsel/config.h"
#include "tinsel/coroutine.h"
#include "tinsel/cursor.h"
#include "tinsel/dw.h"
#include "tinsel/film.h"
#include "tinsel/font.h"
#include "tinsel/graphics.h"
#include "tinsel/handle.h"
#include "tinsel/inventory.h"
#include "tinsel/move.h"
#include "tinsel/multiobj.h"
#include "tinsel/music.h"
#include "tinsel/object.h"
#include "tinsel/palette.h"
#include "tinsel/pcode.h"
#include "tinsel/pid.h"
#include "tinsel/polygons.h"
#include "tinsel/rince.h"
#include "tinsel/savescn.h"
#include "tinsel/sched.h"
#include "tinsel/scn.h"
#include "tinsel/scroll.h"
#include "tinsel/sound.h"
#include "tinsel/strres.h"
#include "tinsel/text.h"
#include "tinsel/timers.h"		// For ONE_SECOND constant
#include "tinsel/tinlib.h"
#include "tinsel/tinsel.h"
#include "tinsel/token.h"


namespace Tinsel {

//----------------- EXTERNAL GLOBAL DATA --------------------

// In DOS_DW.C
extern bool bRestart;		// restart flag - set to restart the game
extern bool bHasRestarted;	// Set after a restart

// In DOS_MAIN.C
// TODO/FIXME: From dos_main.c: "Only used on PSX so far"
int clRunMode = 0;

//----------------- EXTERNAL FUNCTIONS ---------------------

// in BG.C
extern void startupBackground(SCNHANDLE bfilm);
extern void ChangePalette(SCNHANDLE hPal);
extern int BackgroundWidth(void);
extern int BackgroundHeight(void);

// in DOS_DW.C
extern void SetHookScene(SCNHANDLE scene, int entrance, int transition);
extern void SetNewScene(SCNHANDLE scene, int entrance, int transition);
extern void UnHookScene(void);
extern void SuspendHook(void);
extern void UnSuspendHook(void);

// in PDISPLAY.C
extern void EnableTags(void);
extern void DisableTags(void);
bool DisableTagsIfEnabled(void);
extern void setshowstring(void);

// in PLAY.C
extern void playFilm(SCNHANDLE film, int x, int y, int actorid, bool splay, int sfact, bool escOn, int myescEvent, bool bTop);
extern void playFilmc(CORO_PARAM, SCNHANDLE film, int x, int y, int actorid, bool splay, int sfact, bool escOn, int myescEvent, bool bTop);

// in SCENE.C
extern void setshowpos(void);

#ifdef BODGE
// In DOS_HAND.C
bool ValidHandle(SCNHANDLE offset);

// In SCENE.C
SCNHANDLE GetSceneHandle(void);
#endif

//----------------- GLOBAL GLOBAL DATA --------------------

bool bEnableF1;


//----------------- LOCAL DEFINES --------------------

#define JAP_TEXT_TIME	(2*ONE_SECOND)

/*----------------------------------------------------------------------*\
|*                      Library Procedure and Function codes            *|
\*----------------------------------------------------------------------*/

enum LIB_CODE {
	ACTORATTR = 0, ACTORDIRECTION, ACTORREF, ACTORSCALE, ACTORXPOS = 4,
	ACTORYPOS, ADDICON, ADDINV1, ADDINV2, ADDOPENINV, AUXSCALE = 10,
	BACKGROUND, CAMERA, CLOSEINVENTORY, CONTROL, CONVERSATION = 15,
	CONVICON, CURSORXPOS, CURSORYPOS, DEC_CONVW, DEC_CURSOR = 20,
	DEC_INV1, DEC_INV2, DEC_INVW, DEC_LEAD, DEC_TAGFONT = 25,
	DEC_TALKFONT, DELICON, DELINV, EFFECTACTOR, ESCAPE, EVENT = 31,
	GETINVLIMIT, HELDOBJECT, HIDE, ININVENTORY, INVDEPICT = 36,
	INVENTORY, KILLACTOR, KILLBLOCK, KILLEXIT, KILLTAG, LEFTOFFSET = 42,
	MOVECURSOR, NEWSCENE, NOSCROLL, OBJECTHELD, OFFSET, PAUSE = 48,
	PLAY, PLAYMIDI, PLAYSAMPLE, PREPARESCENE, PRINT, PRINTOBJ = 54,
	PRINTTAG, RANDOM, RESTORE_SCENE, SAVE_SCENE, SCALINGREELS = 59,
	SCANICON, SCROLL, SETACTOR, SETBLOCK, SETEXIT, SETINVLIMIT = 65,
	SETPALETTE, SETTAG, SETTIMER, SHOWPOS, SHOWSTRING, SPLAY = 71,
	STAND, STANDTAG, STOP, SWALK, TAGACTOR, TALK, TALKATTR, TIMER = 79,
	TOPOFFSET, TOPPLAY, TOPWINDOW, UNTAGACTOR, VIBRATE, WAITKEY = 85,
	WAITTIME, WALK, WALKED, WALKINGACTOR, WALKPOLY, WALKTAG = 91,
	WHICHINVENTORY = 92,
	ACTORSON, CUTSCENE, HOOKSCENE, IDLETIME, RESETIDLETIME = 97,
	TALKAT, UNHOOKSCENE, WAITFRAME,	DEC_CSTRINGS, STOPMIDI, STOPSAMPLE = 103,
	TALKATS = 104,
	DEC_FLAGS, FADEMIDI, CLEARHOOKSCENE, SETINVSIZE, INWHICHINV = 109,
	NOBLOCKING, SAMPLEPLAYING, TRYPLAYSAMPLE, ENABLEF1 = 113,
	RESTARTGAME, QUITGAME, FRAMEGRAB, PLAYRTF, CDPLAY, CDLOAD = 119,
	HASRESTARTED, RESTORE_CUT, RUNMODE, SUBTITLES, SETLANGUAGE = 124
};



//----------------- LOCAL GLOBAL DATA --------------------

// Saved cursor co-ordinates for control(on) to restore cursor position
// as it was at control(off).
// They are global so that movecursor(..) has a net effect if it
// precedes control(on).
static int controlX = 0, controlY = 0;

static int offtype = 0;			// used by control()
static uint32 lastValue = 0;	// used by dw_random()
static int scrollCount = 0;		// used by scroll()

static bool NotPointedRunning = false;	// Used in printobj and printobjPointed

static COLORREF s_talkfontColor = 0;

//----------------- FORWARD REFERENCES --------------------

void resetidletime(void);
void stopmidi(void);
void stopsample(void);
void walk(CORO_PARAM, int actor, int x, int y, SCNHANDLE film, int hold, bool igPath, bool escOn, int myescTime);


/**
 * NOT A LIBRARY FUNCTION
 *
 * Poke supplied colours into the DAC queue.
 */
static void setTextPal(COLORREF col) {
	s_talkfontColor = col;
	UpdateDACqueue(TALKFONT_COL, 1, &s_talkfontColor);
}


static int TextTime(char *pTstring) {
	if (isJapanMode())
		return JAP_TEXT_TIME;
	else if (!speedText)
		return strlen(pTstring) + ONE_SECOND;
	else
		return strlen(pTstring) + ONE_SECOND + (speedText * 5 * ONE_SECOND) / 100;
}

/*--------------------------------------------------------------------------*/


/**
 * Set actor's attributes.
 * - currently only the text colour.
 */
void actorattr(int actor, int r1, int g1, int b1) {
	storeActorAttr(actor, r1, g1, b1);
}

/**
 * Return the actor's direction.
 */
int actordirection(int actor) {
	PMACTOR pActor;

	pActor = GetMover(actor);
	assert(pActor != NULL); // not a moving actor

	return (int)GetMActorDirection(pActor);
}

/**
 * Return the actor's scale.
 */
int actorscale(int actor) {
	PMACTOR pActor;

	pActor = GetMover(actor);
	assert(pActor != NULL); // not a moving actor

	return (int)GetMActorScale(pActor);
}

/**
 * Returns the x or y position of an actor.
 */
int actorpos(int xory, int actor) {
	int x, y;

	GetActorPos(actor, &x, &y);
	return (xory == ACTORXPOS) ? x : y;
}

/**
 * Make all actors alive at the start of each scene.
 */
void actorson(void) {
	setactorson();
}

/**
 * Adds an icon to the conversation window.
 */
void addicon(int icon) {
	AddToInventory(INV_CONV, icon, false);
}

/**
 * Place the object in inventory 1 or 2.
 */
void addinv(int invno, int object) {
	assert(invno == INV_1 || invno == INV_2 || invno == INV_OPEN); // illegal inventory number

	AddToInventory(invno, object, false);
}

/**
 * Define an actor's walk and stand reels for an auxilliary scale.
 */
void auxscale(int actor, int scale, SCNHANDLE *rp) {
	PMACTOR pActor;

	pActor = GetMover(actor);
	assert(pActor); // Can't set aux scale for a non-moving actor

	int j;
	for (j = 0; j < 4; ++j)
		pActor->WalkReels[scale-1][j] = *rp++;
	for (j = 0; j < 4; ++j)
		pActor->StandReels[scale-1][j] = *rp++;
	for (j = 0; j < 4; ++j)
		pActor->TalkReels[scale-1][j] = *rp++;
}

/**
 * Defines the background image for a scene.
 */
void background(SCNHANDLE bfilm) {
	startupBackground(bfilm);
}

/**
 * Sets focus of the scroll process.
 */
void camera(int actor) {
	ScrollFocus(actor);
}

/**
 * A CDPLAY() is imminent.
 */
void cdload(SCNHANDLE start, SCNHANDLE next) {
	assert(start && next && start != next); // cdload() fault

// TODO/FIXME
//	LoadExtraGraphData(start, next);
}

/**
 * Clear the hooked scene (if any)
 */

void clearhookscene() {
	SetHookScene(0, 0, 0);
}

/**
 * Guess what.
 */

void closeinventory(void) {
	KillInventory();
}

/**
 * Turn off cursor and take control from player - and variations on the	 theme.
 *  OR Restore cursor and return control to the player.
 */

void control(int param) {
	bEnableF1 = false;

	switch (param) {
	case CONTROL_STARTOFF:
		GetControlToken();	// Take control
		DisableTags();			// Switch off tags
		DwHideCursor();			// Blank out cursor
		offtype = param;
		break;

	case CONTROL_OFF:
	case CONTROL_OFFV:
	case CONTROL_OFFV2:
		if (TestToken(TOKEN_CONTROL)) {
			GetControlToken();	// Take control

			DisableTags();			// Switch off tags
			GetCursorXYNoWait(&controlX, &controlY, true);	// Store cursor position

			// There may be a button timing out
			GetToken(TOKEN_LEFT_BUT);
			FreeToken(TOKEN_LEFT_BUT);
		}

		if (offtype == CONTROL_STARTOFF)
			GetCursorXYNoWait(&controlX, &controlY, true);	// Store cursor position

		offtype = param;

		if (param == CONTROL_OFF)
			DwHideCursor();		// Blank out cursor
		else if (param == CONTROL_OFFV) {
			UnHideCursor();
			FreezeCursor();
		} else if (param == CONTROL_OFFV2) {
			UnHideCursor();
		}
		break;

	case CONTROL_ON:
		if (offtype != CONTROL_OFFV2 && offtype != CONTROL_STARTOFF)
			SetCursorXY(controlX, controlY);// ... where it was

		FreeControlToken();	// Release control

		if (!InventoryActive())
			EnableTags();		// Tags back on

		RestoreMainCursor();		// Re-instate cursor...
	}
}

/**
 * Open or close the conversation window.
 */

void conversation(int fn, HPOLYGON hp, bool escOn, int myescEvent) {
	assert(hp != NOPOLY); // conversation() must (currently) be called from a polygon code block

	switch (fn) {
	case CONV_END:			// Close down conversation
		CloseDownConv();
		break;

	case CONV_DEF:			// Default (i.e. TOP of screen)
	case CONV_BOTTOM:		// BOTTOM of screen
		// Don't do it if it's not wanted
		if (escOn && myescEvent != GetEscEvents())
			break;

		if (IsConvWindow())
			break;

		KillInventory();
		convPos(fn);
		ConvPoly(hp);
		PopUpInventory(INV_CONV);	// Conversation window
		ConvAction(INV_OPENICON);	// CONVERSATION event
		break;
	}
}

/**
 * Add icon to conversation window's permanent default list.
 */

void convicon(int icon) {
	AddIconToPermanentDefaultList(icon);
}

/**
 * Returns the x or y position of the cursor.
 */

int cursorpos(int xory) {
	int x, y;

	GetCursorXY(&x, &y, true);
	return (xory == CURSORXPOS) ? x : y;
}

/**
 * Declare conversation window.
 */

void dec_convw(SCNHANDLE text, int MaxContents, int MinWidth, int MinHeight,
			int StartWidth, int StartHeight, int MaxWidth, int MaxHeight) {
	idec_convw(text, MaxContents, MinWidth, MinHeight,
			StartWidth, StartHeight, MaxWidth, MaxHeight);
}

/**
 * Declare config strings.
 */

void dec_cstrings(SCNHANDLE *tp) {
	setConfigStrings(tp);
}

/**
 * Declare cursor's reels.
 */

void dec_cursor(SCNHANDLE bfilm) {
	DwInitCursor(bfilm);
}

/**
 * Declare the language flags.
 */

void dec_flags(SCNHANDLE hf) {
	setFlagFilms(hf);
}

/**
 * Declare inventory 1's parameters.
 */

void dec_inv1(SCNHANDLE text, int MaxContents,
		int MinWidth, int MinHeight,
		int StartWidth, int StartHeight,
		int MaxWidth, int MaxHeight) {
	idec_inv1(text, MaxContents, MinWidth, MinHeight,
			StartWidth, StartHeight, MaxWidth, MaxHeight);
}

/**
 * Declare inventory 2's parameters.
 */

void dec_inv2(SCNHANDLE text, int MaxContents,
		int MinWidth, int MinHeight,
		int StartWidth, int StartHeight,
		int MaxWidth, int MaxHeight) {
	idec_inv2(text, MaxContents, MinWidth, MinHeight,
			StartWidth, StartHeight, MaxWidth, MaxHeight);
}

/**
 * Declare the bits that the inventory windows are constructed from.
 */

void dec_invw(SCNHANDLE hf) {
	setInvWinParts(hf);
}

/**
 * Declare lead actor.
 * - the actor's id, walk and stand reels for all the regular scales,
 * and the tag text.
 */

void dec_lead(uint32 id, SCNHANDLE *rp, SCNHANDLE text) {
	PMACTOR	pActor;		// Moving actor structure

	Tag_Actor(id, text, TAG_DEF);	// The lead actor is automatically tagged
	setleadid(id);			// Establish this as the lead
	SetMover(id);			// Establish as a moving actor

	pActor = GetMover(id);		// Get moving actor structure
	assert(pActor);

	// Store all those reels
	int i, j;
	for (i = 0; i < 5; ++i) {
		for (j = 0; j < 4; ++j)
			pActor->WalkReels[i][j] = *rp++;
		for (j = 0; j < 4; ++j)
			pActor->StandReels[i][j] = *rp++;
		for (j = 0; j < 4; ++j)
			pActor->TalkReels[i][j] = *rp++;
	}


	for (i = NUM_MAINSCALES; i < TOTAL_SCALES; i++) {
		for (j = 0; j < 4; ++j) {
			pActor->WalkReels[i][j] = pActor->WalkReels[4][j];
			pActor->StandReels[i][j] = pActor->StandReels[2][j];
			pActor->TalkReels[i][j] = pActor->TalkReels[4][j];
		}
	}
}

/**
 * Declare the text font.
 */

void dec_tagfont(SCNHANDLE hf) {
	TagFontHandle(hf);		// Store the font handle
}

/**
 * Declare the text font.
 */

void dec_talkfont(SCNHANDLE hf) {
	TalkFontHandle(hf);		// Store the font handle
}

/**
 * Remove an icon from the conversation window.
 */

void delicon(int icon) {
	RemFromInventory(INV_CONV, icon);
}

/**
 * Delete the object from inventory 1 or 2.
 */

void delinv(int object) {
	if (!RemFromInventory(INV_1, object))		// Remove from inventory 1...
		RemFromInventory(INV_2, object);		// ...or 2 (whichever)

	DropItem(object);			// Stop holding it
}

/**
 * enablef1
 */

void enablef1(void) {
	bEnableF1 = true;
}

/**
 * fademidi(in/out)
 */

void fademidi(CORO_PARAM, int inout) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);
	assert(inout == FM_IN || inout == FM_OUT);

	// To prevent compiler complaining
	if (inout == FM_IN || inout == FM_OUT)
		CORO_SLEEP(1);
	CORO_END_CODE;
}

/**
 * Guess what.
 */

int getinvlimit(int invno) {
	return InvGetLimit(invno);
}

/**
 * Returns TRUE if the game has been restarted, FALSE if not.
 */
bool hasrestarted(void) {
	return bHasRestarted;
}

/**
 * Returns which object is currently held.
 */

int heldobject(void) {
	return WhichItemHeld();
}

/**
 * Removes a player from the screen, probably when he's about to be
 * replaced by an animation.
 *
 * Not believed to work anymore! (hide() is not used).
 */

void hide(int actor) {
	HideActor(actor);
}

/**
 * hookscene(scene, entrance, transition)
 */

void hookscene(SCNHANDLE scene, int entrance, int transition) {
	SetHookScene(scene, entrance, transition);
}

/**
 * idletime
 */

int idletime(void) {
	uint32	x;

	x = getUserEventTime() / ONE_SECOND;
	
	if (!TestToken(TOKEN_CONTROL))
		resetidletime();

	return (int)x;
}

/**
 * invdepict
 */
void invdepict(int object, SCNHANDLE hFilm) {
	invObjectFilm(object, hFilm);
}

/**
 * See if an object is in the inventory.
 */
int ininventory(int object) {
	return (InventoryPos(object) != INV_NOICON);
}

/**
 * Open an inventory.
 */
void inventory(int invno, bool escOn, int myescEvent) {
	// Don't do it if it's not wanted
	if (escOn && myescEvent != GetEscEvents())
		return;

	assert((invno == INV_1 || invno == INV_2)); // Trying to open illegal inventory

	PopUpInventory(invno);
}

/**
 * See if an object is in the inventory.
 */
int inwhichinv(int object) {
	if (WhichItemHeld() == object)
		return 0;

	if (IsInInventory(object, INV_1))
		return 1;

	if (IsInInventory(object, INV_2))
		return 2;

	return -1;
}

/**
 * Kill an actor.
 */
void killactor(int actor) {
	DisableActor(actor);
}

/**
 * Turn a blocking polygon off.
 */
void killblock(int block) {
	DisableBlock(block);
}

/**
 * Turn an exit off.
 */
void killexit(int exit) {
	DisableExit(exit);
}

/**
 * Turn a tag off.
 */
void killtag(int tagno) {
	DisableTag(tagno);
}

/**
 * Returns the left or top offset of the screen.
 */
int ltoffset(int lort) {
	int Loffset, Toffset;

	PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);
	return (lort == LEFTOFFSET) ? Loffset : Toffset;
}

/**
 * Set new cursor position.
 */
void movecursor(int x, int y) {
	SetCursorXY(x, y);

	controlX = x;		// Save these values so that
	controlY = y;		// control(on) doesn't undo this
}

/**
 * Triggers change to a new scene.
 */
void newscene(CORO_PARAM, SCNHANDLE scene, int entrance, int transition) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

#ifdef BODGE
	if (!ValidHandle(scene)) {
		scene = GetSceneHandle();
		entrance = 1;
	}
	assert(scene); // Non-existant first scene!
#endif

	SetNewScene(scene, entrance, transition);

#if 1
	// Prevent tags and cursor re-appearing
	GetControl(CONTROL_STARTOFF);
#endif

	// Prevent code subsequent to this call running before scene changes
	if (g_scheduler->getCurrentPID() != PID_MASTER_SCR)
		CORO_KILL_SELF();
	CORO_END_CODE;
}

/**
 * Disable dynamic blocking for current scene.
 */
void noblocking(void) {
	bNoBlocking = true;
}

/**
 * Define a no-scroll boundary for the current scene.
 */
void noscroll(int x1, int y1, int x2, int y2) {
	SetNoScroll(x1, y1, x2, y2);
}

/**
 * Hold the specified object.
 */
void objectheld(int object) {
	HoldItem(object);
}

/**
 * Set the top left offset of the screen.
 */
void offset(int x, int y) {
	KillScroll();
	PlayfieldSetPos(FIELD_WORLD, x, y);
}

/**
 * Play a film.
 */
void play(CORO_PARAM, SCNHANDLE film, int x, int y, int compit, int actorid, bool splay, int sfact,
		  bool escOn, int myescEvent, bool bTop) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	assert(film != 0); // play(): Trying to play NULL film

	// Don't do it if it's not wanted
	if (escOn && myescEvent != GetEscEvents())
		return;

	// If this actor is dead, call a stop to the calling process
	if (actorid && !actorAlive(actorid))
		CORO_KILL_SELF();

	// 7/4/95
	if (!escOn)
		myescEvent = GetEscEvents();

	if (compit == 1) {
		// Play to completion before returning
		CORO_INVOKE_ARGS(playFilmc, (CORO_SUBCTX, film, x, y, actorid, splay, sfact, escOn, myescEvent, bTop));
	} else if (compit == 2) {
		error("play(): compit == 2 - please advise John");
	} else {
		// Kick off the play and return.
		playFilm(film, x, y, actorid, splay, sfact, escOn, myescEvent, bTop);
	}
	CORO_END_CODE;
}

/**
 * Play a midi file.
 */
void playmidi(CORO_PARAM, SCNHANDLE hMidi, int loop, bool complete) {
	// FIXME: This is a workaround for the FIXME below
	if (GetMidiVolume() == 0)
		return;

	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);
	assert(loop == MIDI_DEF || loop == MIDI_LOOP);

	PlayMidiSequence(hMidi, loop == MIDI_LOOP);

	// FIXME: The following check messes up the script arguments when
	// entering the secret door in the bookshelf in the library,
	// leading to a crash, when the music volume is set to 0 (MidiPlaying()
	// always false then).
	//
	// Why exactly this happens is unclear. An analysis of the involved
	// script(s) might reveal more.
	//
	// Note: This check&sleep was added in DW v2. It was most likely added
	// to ensure that the MIDI song started playing before the next opcode
	// is executed.
	if (!MidiPlaying())
		CORO_SLEEP(1);

	if (complete) {
		while (MidiPlaying())
			CORO_SLEEP(1);
	}
	CORO_END_CODE;
}

/**
 * Play a sample.
 */
void playsample(CORO_PARAM, int sample, bool complete, bool escOn, int myescEvent) {
	CORO_BEGIN_CONTEXT;
		Audio::SoundHandle handle;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);
	// Don't play SFX if voice is already playing
	if (_vm->_mixer->hasActiveChannelOfType(Audio::Mixer::kSpeechSoundType))
		return;

	// Don't do it if it's not wanted
	if (escOn && myescEvent != GetEscEvents()) {
		_vm->_sound->stopAllSamples();		// Stop any currently playing sample
		return;
	}

	if (volSound != 0 && _vm->_sound->sampleExists(sample)) {
		_vm->_sound->playSample(sample, Audio::Mixer::kSFXSoundType, &_ctx->handle);

		if (complete) {
			while (_vm->_mixer->isSoundHandleActive(_ctx->handle)) {
				// Abort if escapable and ESCAPE is pressed
				if (escOn && myescEvent != GetEscEvents()) {
					_vm->_mixer->stopHandle(_ctx->handle);
					break;
				}

				CORO_SLEEP(1);
			}
		}
	} else {
		// Prevent Glitter lock-up
		CORO_SLEEP(1);
	}
	CORO_END_CODE;
}

/**
 * Play a sample.
 */
void tryplaysample(CORO_PARAM, int sample, bool complete, bool escOn, int myescEvent) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);
	// Don't do it if it's not appropriate
	if (_vm->_sound->sampleIsPlaying()) {
		// return, but prevent Glitter lock-up
		CORO_SLEEP(1);
		return;
	}

	CORO_INVOKE_ARGS(playsample, (CORO_SUBCTX, sample, complete, escOn, myescEvent));
	CORO_END_CODE;
}

/**
 * Trigger pre-loading of a scene's data.
 */
void preparescene(SCNHANDLE scene) {
#ifdef BODGE
	if (!ValidHandle(scene))
		return;
#endif
}

/**
 * Print the given text at the given place for the given time.
 *
 * Print(....., h) -> hold = 1 (not used)
 * Print(....., s) -> hold = 2 (sustain)
 */
void print(CORO_PARAM, int x, int y, SCNHANDLE text, int time, int hold, bool escOn, int myescEvent) {
	CORO_BEGIN_CONTEXT;
		OBJECT *pText;			// text object pointer
		int	myleftEvent;
		bool bSample;			// Set if a sample is playing
		Audio::SoundHandle handle;
		int timeout;
		int time;
	CORO_END_CONTEXT(_ctx);

	bool	bJapDoPrintText;	// Bodge to get-around Japanese bodge

	CORO_BEGIN_CODE(_ctx);

	_ctx->pText = NULL;
	_ctx->bSample = false;

	// Don't do it if it's not wanted
	if (escOn && myescEvent != GetEscEvents())
		return;

	// Kick off the voice sample
	if (volVoice != 0 && _vm->_sound->sampleExists(text)) {
		_vm->_sound->playSample(text, Audio::Mixer::kSpeechSoundType, &_ctx->handle);
		_ctx->bSample = _vm->_mixer->isSoundHandleActive(_ctx->handle);
	}

	// Calculate display time
	LoadStringRes(text, tBufferAddr(), TBUFSZ);
	bJapDoPrintText = false;
	if (time == 0) {
		// This is a 'talky' print
		_ctx->time = TextTime(tBufferAddr());
		
		// Cut short-able if sustain was not set
		_ctx->myleftEvent = (hold == 2) ? 0 : GetLeftEvents();
	} else {
		_ctx->time = time * ONE_SECOND;
		_ctx->myleftEvent = 0;
		if (isJapanMode())
			bJapDoPrintText = true;
	}

	// Print the text
	if (bJapDoPrintText || (!isJapanMode() && (bSubtitles || !_ctx->bSample))) {
		int Loffset, Toffset;	// Screen position
		PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);
		_ctx->pText = ObjectTextOut(GetPlayfieldList(FIELD_STATUS), tBufferAddr(),
					0, x - Loffset, y - Toffset, hTalkFontHandle(), TXT_CENTRE);
		assert(_ctx->pText); // string produced NULL text
		if (IsTopWindow())
			MultiSetZPosition(_ctx->pText, Z_TOPW_TEXT);

		/*
		 * New feature: Don't go off the side of the background
		 */
		int	shift;
		shift = MultiRightmost(_ctx->pText) + 2;
		if (shift >= BackgroundWidth())			// Not off right
			MultiMoveRelXY(_ctx->pText, BackgroundWidth() - shift, 0);
		shift = MultiLeftmost(_ctx->pText) - 1;
		if (shift <= 0)					// Not off left
			MultiMoveRelXY(_ctx->pText, -shift, 0);
		shift = MultiLowest(_ctx->pText);
		if (shift > BackgroundHeight())			// Not off bottom
			MultiMoveRelXY(_ctx->pText, 0, BackgroundHeight() - shift);
	}

	// Give up if nothing printed and no sample
	if (_ctx->pText == NULL && !_ctx->bSample)
		return;

	// Leave it up until time runs out or whatever
	_ctx->timeout = SAMPLETIMEOUT;
	do {
		CORO_SLEEP(1);

		// Abort if escapable and ESCAPE is pressed
		// Abort if left click - hardwired feature for talky-print!
		// Will be ignored if myleftevent happens to be 0!
		// Abort if sample times out
		if ((escOn && myescEvent != GetEscEvents())
		|| (_ctx->myleftEvent && _ctx->myleftEvent != GetLeftEvents())
		|| (_ctx->bSample && --_ctx->timeout <= 0))
			break;

		if (_ctx->bSample) {
			// Wait for sample to end whether or not
			if (!_vm->_mixer->isSoundHandleActive(_ctx->handle)) {
				if (_ctx->pText == NULL || speedText == DEFTEXTSPEED) 				{
					// No text or speed modification - just depends on sample
					break;
				} else {
					// Must wait for time
					_ctx->bSample = false;
				}
			}
		} else {
			// No sample - just depends on time
			if (_ctx->time-- <= 0)
				break;
		}

	} while (1);

	// Delete the text
	if (_ctx->pText != NULL)
		MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), _ctx->pText);
	_vm->_mixer->stopHandle(_ctx->handle);

	CORO_END_CODE;
}


static void printobjPointed(CORO_PARAM, const SCNHANDLE text, const PINV_OBJECT pinvo, OBJECT *&pText, const int textx, const int texty, const int item);
static void printobjNonPointed(CORO_PARAM, const SCNHANDLE text, const OBJECT *pText);

/**
 * Print the given inventory object's name or whatever.
 */
void printobj(CORO_PARAM, const SCNHANDLE text, const PINV_OBJECT pinvo, const int event) {
	CORO_BEGIN_CONTEXT;
		OBJECT *pText;		// text object pointer
		int	textx, texty;
		int	item;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	assert(pinvo != 0); // printobj() may only be called from an object code block

	if (text == (SCNHANDLE)-1) {	// 'OFF'
		NotPointedRunning = true;
		return;
	}
	if (text == (SCNHANDLE)-2) {	// 'ON'
		NotPointedRunning = false;
		return;
	}

	GetCursorXY(&_ctx->textx, &_ctx->texty, false);	// Cursor position..
	_ctx->item = InvItem(&_ctx->textx, &_ctx->texty, true);	// ..to text position

	if (_ctx->item == INV_NOICON)
		return;

	if (event != POINTED) {
		NotPointedRunning = true;	// Get POINTED text to die
		CORO_SLEEP(1);		// Give it chance to
	} else
		NotPointedRunning = false;	// There may have been an OFF without an ON

	// Display the text and set it's Z position
	if (event == POINTED || (!isJapanMode() && (bSubtitles || !_vm->_sound->sampleExists(text)))) {
		int	xshift;

		LoadStringRes(text, tBufferAddr(), TBUFSZ);	// The text string
		_ctx->pText = ObjectTextOut(GetPlayfieldList(FIELD_STATUS), tBufferAddr(),
					0, _ctx->textx, _ctx->texty, hTagFontHandle(), TXT_CENTRE);
		assert(_ctx->pText); // printobj() string produced NULL text
		MultiSetZPosition(_ctx->pText, Z_INV_ITEXT);

		// Don't go off the side of the screen
		xshift = MultiLeftmost(_ctx->pText);
		if (xshift < 0) {
			MultiMoveRelXY(_ctx->pText, - xshift, 0);
			_ctx->textx -= xshift;
		}
		xshift = MultiRightmost(_ctx->pText);
		if (xshift > SCREEN_WIDTH) {
			MultiMoveRelXY(_ctx->pText, SCREEN_WIDTH - xshift, 0);
			_ctx->textx += SCREEN_WIDTH - xshift;
		}
	} else
		_ctx->pText = NULL;

	if (event == POINTED) {
		// FIXME: Is there ever an associated sound if in POINTED mode???
		assert(!_vm->_sound->sampleExists(text));
		CORO_INVOKE_ARGS(printobjPointed, (CORO_SUBCTX, text, pinvo, _ctx->pText, _ctx->textx, _ctx->texty, _ctx->item));
	} else {
		CORO_INVOKE_2(printobjNonPointed, text, _ctx->pText);
	}

	// Delete the text, if haven't already
	if (_ctx->pText)
		MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), _ctx->pText);

	CORO_END_CODE;
}

static void printobjPointed(CORO_PARAM, const SCNHANDLE text, const PINV_OBJECT pinvo, OBJECT *&pText, const int textx, const int texty, const int item) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);
		// Have to give way to non-POINTED-generated text
		// and go away if the item gets picked up
		int	x, y;
		do {
			// Give up if this item gets picked up
			if (WhichItemHeld() == pinvo->id)
				break;

			// Give way to non-POINTED-generated text
			if (NotPointedRunning) {
				// Delete the text, and wait for the all-clear
				MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), pText);
				pText = NULL;
				while (NotPointedRunning)
					CORO_SLEEP(1);

				GetCursorXY(&x, &y, false);
				if (InvItem(&x, &y, false) != item)
					break;

				// Re-display in the same place
				LoadStringRes(text, tBufferAddr(), TBUFSZ);
				pText = ObjectTextOut(GetPlayfieldList(FIELD_STATUS), tBufferAddr(),
							0, textx, texty, hTagFontHandle(), TXT_CENTRE);
				assert(pText); // printobj() string produced NULL text
				MultiSetZPosition(pText, Z_INV_ITEXT);
			}

			CORO_SLEEP(1);

			// Carry on until the cursor leaves this icon
			GetCursorXY(&x, &y, false);
		} while (InvItemId(x, y) == pinvo->id);

	CORO_END_CODE;
}

static void printobjNonPointed(CORO_PARAM, const SCNHANDLE text, const OBJECT *pText) {
	CORO_BEGIN_CONTEXT;
		bool bSample;		// Set if a sample is playing
		Audio::SoundHandle handle;

		int myleftEvent;
		bool took_control;
		int	ticks;
		int	timeout;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);
		// Kick off the voice sample
		if (volVoice != 0 && _vm->_sound->sampleExists(text)) {
			_vm->_sound->playSample(text, Audio::Mixer::kSpeechSoundType, &_ctx->handle);
			_ctx->bSample = _vm->_mixer->isSoundHandleActive(_ctx->handle);
		} else
			_ctx->bSample = false;

		_ctx->myleftEvent = GetLeftEvents();
		_ctx->took_control = GetControl(CONTROL_OFF);

		// Display for a time, but abort if conversation gets hidden
		if (isJapanMode())
			_ctx->ticks = JAP_TEXT_TIME;
		else if (pText)
			_ctx->ticks = TextTime(tBufferAddr());
		else
			_ctx->ticks = 0;

		_ctx->timeout = SAMPLETIMEOUT;
		do {
			CORO_SLEEP(1);
			--_ctx->timeout;

			// Abort if left click - hardwired feature for talky-print!
			// Abort if sample times out
			// Abort if conversation hidden
			if (_ctx->myleftEvent != GetLeftEvents() || _ctx->timeout <= 0 || convHid())
				break;

			if (_ctx->bSample) {
				// Wait for sample to end whether or not
				if (!_vm->_mixer->isSoundHandleActive(_ctx->handle)) {
					if (pText == NULL || speedText == DEFTEXTSPEED) {
						// No text or speed modification - just depends on sample
						break;
					} else {
						// Must wait for time
						_ctx->bSample = false;
					}
				}
			} else {
				// No sample - just depends on time
				if (_ctx->ticks-- <= 0)
					break;
			}
		} while (1);

		NotPointedRunning = false;	// Let POINTED text back in

		if (_ctx->took_control)
			control(CONTROL_ON);	// Free control if we took it

		_vm->_mixer->stopHandle(_ctx->handle);

	CORO_END_CODE;
}

/**
 * Register the fact that this poly would like its tag displayed.
 */
void printtag(HPOLYGON hp, SCNHANDLE text) {
	assert(hp != NOPOLY); // printtag() may only be called from a polygon code block

	if (PolyTagState(hp) == TAG_OFF) {
		SetPolyTagState(hp, TAG_ON);
		SetPolyTagHandle(hp, text);
	}
}

/**
 * quitgame
 */
void quitgame(void) {
	stopmidi();
	stopsample();
	_vm->quitFlag = true;
}

/**
 * Return a random number between optional limits.
 */
int dw_random(int n1, int n2, int norpt) {
	int i = 0;
	uint32 value;

	do {
		value = n1 + _vm->getRandomNumber(n2 - n1);
	} while ((lastValue == value) && (norpt == RAND_NORPT) && (++i <= 10));

	lastValue = value;
	return value;
}

/**
 * resetidletime
 */
void resetidletime(void) {
	resetUserEventTime();
}

/**
 * restartgame
 */
void restartgame(void) {
	stopmidi();
	stopsample();
	bRestart = true;
}

/**
 * Restore saved scene.
 */
void restore_scene(bool bFade) {
	UnSuspendHook();
	PleaseRestoreScene(bFade);
}

/**
 * runmode
 */
int runmode(void) {
	return clRunMode;
}

/**
 * sampleplaying
 */
bool sampleplaying(bool escOn, int myescEvent) {
	// escape effects introduced 14/12/95 to fix
	//	 while (sampleplaying()) pause;

	if (escOn && myescEvent != GetEscEvents())
		return false;

	return _vm->_sound->sampleIsPlaying();
}

/**
 * Save current scene.
 */
void save_scene(CORO_PARAM) {
	PleaseSaveScene(coroParam);
	SuspendHook();
}

/**
 * scalingreels
 */
void scalingreels(int actor, int scale, int direction,
		SCNHANDLE left, SCNHANDLE right, SCNHANDLE forward, SCNHANDLE away) {

	setscalingreels(actor, scale, direction, left, right, forward, away);
}

/**
 * Return the icon that caused the CONVERSE event.
 */

int scanicon(void) {
	return convIcon();
}

/**
 * Scroll the screen to target co-ordinates.
 */

void scroll(CORO_PARAM, int x, int y, int iter, bool comp, bool escOn, int myescEvent) {
	CORO_BEGIN_CONTEXT;
		int	mycount;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);
	if (escOn && myescEvent != GetEscEvents()) {
		// Instant completion!
		offset(x, y);
	} else {
		_ctx->mycount = ++scrollCount;

		ScrollTo(x, y, iter);

		if (comp) {
			int	Loffset, Toffset;
			do {
				CORO_SLEEP(1);

				// If escapable and ESCAPE is pressed...
				if (escOn && myescEvent != GetEscEvents()) {
					// Instant completion!
					offset(x, y);
					break;
				}

				// give up if have been superseded
				if (_ctx->mycount != scrollCount)
					CORO_KILL_SELF();

				PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);
			} while (Loffset != x || Toffset != y);
		}
	}
	CORO_END_CODE;
}

/**
 * Un-kill an actor.
 */
void setactor(int actor) {
	EnableActor(actor);
}

/**
 * Turn a blocking polygon on.
 */

void setblock(int blockno) {
	EnableBlock(blockno);
}

/**
 * Turn an exit on.
 */

void setexit(int exitno) {
	EnableExit(exitno);
}

/**
 * Guess what.
 */
void setinvlimit(int invno, int n) {
	InvSetLimit(invno, n);
}

/**
 * Guess what.
 */
void setinvsize(int invno, int MinWidth, int MinHeight,
		int StartWidth, int StartHeight, int MaxWidth, int MaxHeight) {
	InvSetSize(invno, MinWidth, MinHeight, StartWidth, StartHeight, MaxWidth, MaxHeight);
}

/**
 * Guess what.
 */
void setlanguage(LANGUAGE lang) {
	assert(lang == TXT_ENGLISH || lang == TXT_FRENCH
	     || lang == TXT_GERMAN  || lang == TXT_ITALIAN
	     || lang == TXT_SPANISH); // ensure language is valid

	ChangeLanguage(lang);
}

/**
 * Set palette
 */
void setpalette(SCNHANDLE hPal, bool escOn, int myescEvent) {
	// Don't do it if it's not wanted
	if (escOn && myescEvent != GetEscEvents())
		return;

	ChangePalette(hPal);
}

/**
 * Turn a tag on.
 */
void settag(int tagno) {
	EnableTag(tagno);
}

/**
 * Initialise a timer.
 */
void settimer(int timerno, int start, bool up, bool frame) {
	DwSetTimer(timerno, start, up != 0, frame != 0);
}

#ifdef DEBUG
/**
 * Enable display of diagnostic co-ordinates.
 */
void showpos(void) {
	setshowpos();
}

/**
 * Enable display of diagnostic co-ordinates.
 */
void showstring(void) {
	setshowstring();
}
#endif

/**
 * Special play - slow down associated actor's movement while the play
 * is running. After the play, position the actor where the play left
 * it and continue walking, if the actor still is.
 */

void splay(CORO_PARAM, int sf, SCNHANDLE film, int x, int y, bool complete, int actorid, bool escOn, int myescEvent) {
	// Don't do it if it's not wanted
	if (escOn && myescEvent != GetEscEvents())
		return;

	play(coroParam, film, x, y, complete, actorid, true, sf, escOn, myescEvent, false);
}

/**
 * (Re)Position an actor.
 * If moving actor is not around yet in this scene, start it up.
 */

void stand(int actor, int x, int y, SCNHANDLE film) {
	PMACTOR pActor;		// Moving actor structure

	pActor = GetMover(actor);
	if (pActor) {
		if (pActor->MActorState == NO_MACTOR) {
			// create a moving actor process
			MActorProcessCreate(x, y, (actor == LEAD_ACTOR) ? LeadId() : actor, pActor);

			if (film == TF_NONE) {
				SetMActorStanding(pActor);
			} else {
				switch (film) {
				case TF_NONE:
					break;
	
				case TF_UP:
					SetMActorDirection(pActor, AWAY);
					SetMActorStanding(pActor);
					break;
				case TF_DOWN:
					SetMActorDirection(pActor, FORWARD);
					SetMActorStanding(pActor);
					break;
				case TF_LEFT:
					SetMActorDirection(pActor, LEFTREEL);
					SetMActorStanding(pActor);
					break;
				case TF_RIGHT:
					SetMActorDirection(pActor, RIGHTREEL);
					SetMActorStanding(pActor);
					break;
	
				default:
					AlterMActor(pActor, film, AR_NORMAL);
					break;
				}
			}
		} else {
			switch (film) {
			case TF_NONE:
				if (x != -1 && y != -1)
					MoveMActor(pActor, x, y);
				break;

			case TF_UP:
				SetMActorDirection(pActor, AWAY);
				if (x != -1 && y != -1)
					MoveMActor(pActor, x, y);
				SetMActorStanding(pActor);
				break;
			case TF_DOWN:
				SetMActorDirection(pActor, FORWARD);
				if (x != -1 && y != -1)
					MoveMActor(pActor, x, y);
				SetMActorStanding(pActor);
				break;
			case TF_LEFT:
				SetMActorDirection(pActor, LEFTREEL);
				if (x != -1 && y != -1)
					MoveMActor(pActor, x, y);
				SetMActorStanding(pActor);
				break;
			case TF_RIGHT:
				SetMActorDirection(pActor, RIGHTREEL);
				if (x != -1 && y != -1)
					MoveMActor(pActor, x, y);
				SetMActorStanding(pActor);
				break;

			default:
				if (x != -1 && y != -1)
					MoveMActor(pActor, x, y);
				AlterMActor(pActor, film, AR_NORMAL);
				break;
			}
		}
	} else if (actor == NULL_ACTOR) {
		//
	} else {
		assert(film != 0); // Trying to play NULL film

		// Kick off the play and return.
		playFilm(film, x, y, actor, false, 0, false, 0, false);
	}
}

/**
 * Position the actor at the polygon's tag node.
 */
void standtag(int actor, HPOLYGON hp) {
	SCNHANDLE film;
	int	pnodex, pnodey;

	assert(hp != NOPOLY); // standtag() may only be called from a polygon code block

	// Lead actor uses tag node film
	film = getPolyFilm(hp);
	getPolyNode(hp, &pnodex, &pnodey);
	if (film && (actor == LEAD_ACTOR || actor == LeadId()))
		stand(actor, pnodex, pnodey, film);
	else
		stand(actor, pnodex, pnodey, 0);
}

/**
 * Kill a moving actor's walk.
 */
void stop(int actor) {
	PMACTOR pActor;

	pActor = GetMover(actor);
	assert(pActor); // Trying to stop a null actor

	GetToken(pActor->actorToken);	// Kill the walk process
	pActor->stop = true;			// Cause the actor to stop
	FreeToken(pActor->actorToken);
}

void stopmidi(void) {
	StopMidi();		// Stop any currently playing midi
}

void stopsample(void) {
	_vm->_sound->stopAllSamples();		// Stop any currently playing sample
}

void subtitles(int onoff) {
	assert (onoff == ST_ON || onoff == ST_OFF);

	if (isJapanMode())
		return;	// Subtitles are always off in JAPAN version (?)

	if (onoff == ST_ON)
		bSubtitles = true;
	else
		bSubtitles = false;
}

/**
 * Special walk.
 * Walk into or out of a legal path.
 */
void swalk(CORO_PARAM, int actor, int x1, int y1, int x2, int y2, SCNHANDLE film, bool escOn, int myescEvent) {
	CORO_BEGIN_CONTEXT;
		bool	took_control;			// Set if this function takes control
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// Don't do it if it's not wanted
	if (escOn && myescEvent != GetEscEvents())
		return;

	// For lead actor, lock out the user (if not already locked out)
	if (actor == LeadId() || actor == LEAD_ACTOR)
		_ctx->took_control = GetControl(CONTROL_OFFV2);
	else
		_ctx->took_control = false;

	HPOLYGON hPath;

	hPath = InPolygon(x1, y1, PATH);
	if (hPath != NOPOLY) {
		// Walking out of a path
		stand(actor, x1, y1, 0);
	} else {
		hPath = InPolygon(x2, y2, PATH);
		// One of them has to be in a path
		assert(hPath != NOPOLY); //one co-ordinate must be in a legal path

		// Walking into a path
		stand(actor, x2, y2, 0);	// Get path's characteristics
		stand(actor, x1, y1, 0);
	}

	CORO_INVOKE_ARGS(walk, (CORO_SUBCTX, actor, x2, y2, film, 0, true, escOn, myescEvent));

	// Free control if we took it
	if (_ctx->took_control)
		control(CONTROL_ON);

	CORO_END_CODE;
}

/**
 * Define a tagged actor.
 */

void tagactor(int actor, SCNHANDLE text, int tp) {
	Tag_Actor(actor, text, tp);
}

/**
 * Text goes over actor's head while actor plays the talk reel.
 */

void FinishTalkingReel(PMACTOR pActor, int actor) {
	if (pActor) {
		SetMActorStanding(pActor);
		AlterMActor(pActor, 0, AR_POPREEL);
	} else {
		setActorTalking(actor, false);
		playFilm(getActorPlayFilm(actor), -1, -1, 0, false, 0, false, 0, false);
	}
}

void talk(CORO_PARAM, SCNHANDLE film, const SCNHANDLE text, int actorid, bool escOn, int myescEvent) {
	CORO_BEGIN_CONTEXT;
		int		Loffset, Toffset;	// Top left of display
		int		actor;			// The speaking actor
		PMACTOR	pActor;			// For moving actors
		int		myleftEvent;
		int		ticks;
		bool	bTookControl;	// Set if this function takes control
		bool	bTookTags;		// Set if this function disables tags
		OBJECT	*pText;			// text object pointer
		bool	bSample;		// Set if a sample is playing
		bool	bTalkReel;		// Set while talk reel is playing
		Audio::SoundHandle handle;
		int	timeout;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->Loffset = 0;
	_ctx->Toffset = 0;
	_ctx->ticks = 0;

	// Don't do it if it's not wanted
	if (escOn && myescEvent != GetEscEvents())
		return;

	_ctx->myleftEvent = GetLeftEvents();

	// If this actor is dead, call a stop to the calling process
	if (actorid && !actorAlive(actorid))
		CORO_KILL_SELF();

	/*
	 * Find out which actor is talking
	 * and with which direction if no film supplied
	 */
	TFTYPE	direction;
	switch (film) {
	case TF_NONE:
	case TF_UP:
	case TF_DOWN:
	case TF_LEFT:
	case TF_RIGHT:
		_ctx->actor = LeadId();	// If no film, actor is lead actor
		direction = (TFTYPE)film;
		break;

	default:
		_ctx->actor = extractActor(film);
		assert(_ctx->actor); // talk() - no actor ID in the reel
		direction = TF_BOGUS;
		break;
	}

	/*
	 * Lock out the user (for lead actor, if not already locked out)
	 * May need to disable tags for other actors
	 */
	if (_ctx->actor == LeadId())
		_ctx->bTookControl = GetControl(CONTROL_OFF);
	else
		_ctx->bTookControl = false;
	_ctx->bTookTags = DisableTagsIfEnabled();

	/*
	 * Kick off the voice sample
	 */
	if (volVoice != 0 && _vm->_sound->sampleExists(text)) {
		_vm->_sound->playSample(text, Audio::Mixer::kSpeechSoundType, &_ctx->handle);
		_ctx->bSample = _vm->_mixer->isSoundHandleActive(_ctx->handle);
	} else
		_ctx->bSample = false;

	/*
	 * Replace actor with the talk reel, saving the current one
	 */
	_ctx->pActor = GetMover(_ctx->actor);
	if (_ctx->pActor) {
		if (direction != TF_BOGUS)
			film = GetMactorTalkReel(_ctx->pActor, direction);
		AlterMActor(_ctx->pActor, film, AR_PUSHREEL);
	} else {
		setActorTalking(_ctx->actor, true);
		setActorTalkFilm(_ctx->actor, film);
		playFilm(film, -1, -1, 0, false, 0, escOn, myescEvent, false);
	}
	_ctx->bTalkReel = true;
	CORO_SLEEP(1);		// Allow the play to come in

	/*
	 * Display the text.
	 */
	_ctx->pText = NULL;
	if (isJapanMode()) {
		_ctx->ticks = JAP_TEXT_TIME;
	} else if (bSubtitles || !_ctx->bSample) {
		int	aniX, aniY;		// actor position
		int	xshift, yshift;
		/*
		 * Work out where to display the text
		 */
		PlayfieldGetPos(FIELD_WORLD, &_ctx->Loffset, &_ctx->Toffset);
		GetActorMidTop(_ctx->actor, &aniX, &aniY);
		aniY -= _ctx->Toffset;

		setTextPal(getActorTcol(_ctx->actor));
		LoadStringRes(text, tBufferAddr(), TBUFSZ);
		_ctx->pText = ObjectTextOut(GetPlayfieldList(FIELD_STATUS), tBufferAddr(),
					0, aniX - _ctx->Loffset, aniY, hTalkFontHandle(), TXT_CENTRE);
		assert(_ctx->pText); // talk() string produced NULL text;
		if (IsTopWindow())
			MultiSetZPosition(_ctx->pText, Z_TOPW_TEXT);

		/*
		 * Set bottom of text just above the speaker's head
		 * But don't go off the top of the screen
		 */
		yshift = aniY - MultiLowest(_ctx->pText) - 2;		// Just above head
		MultiMoveRelXY(_ctx->pText, 0, yshift);		//
		yshift = MultiHighest(_ctx->pText);
		if (yshift < 4)
			MultiMoveRelXY(_ctx->pText, 0, 4 - yshift);	// Not off top

		/*
		 * Don't go off the side of the screen
		 */
		xshift = MultiRightmost(_ctx->pText) + 2;
		if (xshift >= SCREEN_WIDTH)			// Not off right
			MultiMoveRelXY(_ctx->pText, SCREEN_WIDTH - xshift, 0);
		xshift = MultiLeftmost(_ctx->pText) - 1;
		if (xshift <= 0)					// Not off left
			MultiMoveRelXY(_ctx->pText, -xshift, 0);
		/*
		 * Work out how long to talk.
		 * During this time, reposition the text if the screen scrolls.
		 */
		_ctx->ticks = TextTime(tBufferAddr());
	}

	_ctx->timeout = SAMPLETIMEOUT;
	do {
		// Keep text in place if scrolling
		if (_ctx->pText != NULL) {
			int	nLoff, nToff;

			PlayfieldGetPos(FIELD_WORLD, &nLoff, &nToff);
			if (nLoff != _ctx->Loffset || nToff != _ctx->Toffset) {
				MultiMoveRelXY(_ctx->pText, _ctx->Loffset - nLoff, _ctx->Toffset - nToff);
				_ctx->Loffset = nLoff;
				_ctx->Toffset = nToff;
			}
		}

		CORO_SLEEP(1);
		--_ctx->timeout;

		// Abort if escapable and ESCAPE is pressed
		// Abort if left click - hardwired feature for talk!
		// Abort if sample times out
		if ((escOn && myescEvent != GetEscEvents())
				|| (_ctx->myleftEvent != GetLeftEvents())
				|| (_ctx->timeout <= 0))
			break;

		if (_ctx->bSample) {
			// Wait for sample to end whether or not
			if (!_vm->_mixer->isSoundHandleActive(_ctx->handle)) {
				if (_ctx->pText == NULL || speedText == DEFTEXTSPEED) {
					// No text or speed modification - just depends on sample
					break;
				} else {
					// Talk reel stops at end of speech
					FinishTalkingReel(_ctx->pActor, _ctx->actor);
					_ctx->bTalkReel = false;
					_ctx->bSample = false;
				}
			}
		} else {
			// No sample - just depends on time
			if (_ctx->ticks-- <= 0)
				break;
		}
	} while (1);

	/*
	 * The talk is over now - dump the text
	 * Stop the sample
	 * Restore the actor's film or standing reel
	 */
	if (_ctx->pText != NULL)
		MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), _ctx->pText);
	_vm->_mixer->stopHandle(_ctx->handle);
	if (_ctx->bTalkReel)
		FinishTalkingReel(_ctx->pActor, _ctx->actor);

	/*
	 * Restore user control and tags, as appropriate
	 * And, finally, release the talk token.
	 */
	if (_ctx->bTookControl)
		control(CONTROL_ON);
	if (_ctx->bTookTags)
		EnableTags();

	CORO_END_CODE;
}

/**
 * talkat(actor, x, y, text)
 */
void talkat(CORO_PARAM, int actor, int x, int y, SCNHANDLE text, bool escOn, int myescEvent) {
	if (!coroParam) {
		// Don't do it if it's not wanted
		if (escOn && myescEvent != GetEscEvents())
			return;

		if (!isJapanMode() && (bSubtitles || !_vm->_sound->sampleExists(text)))
			setTextPal(getActorTcol(actor));
	}

	print(coroParam, x, y, text, 0, 0, escOn, myescEvent);
}

/**
 * talkats(actor, x, y, text, sustain)
 */
void talkats(CORO_PARAM, int actor, int x, int y, SCNHANDLE text, int sustain, bool escOn, int myescEvent) {
	if (!coroParam) {
		assert(sustain == 2);

		// Don't do it if it's not wanted
		if (escOn && myescEvent != GetEscEvents())
			return;

		if (!isJapanMode())
			setTextPal(getActorTcol(actor));
	}

	print(coroParam, x, y, text, 0, sustain, escOn, myescEvent);
}

/**
 * Set talk font's palette entry.
 */
void talkattr(int r1, int g1, int b1, bool escOn, int myescEvent) {
	if (isJapanMode())
		return;

	// Don't do it if it's not wanted
	if (escOn && myescEvent != GetEscEvents())
		return;

	if (r1 > MAX_INTENSITY)	r1 = MAX_INTENSITY;	// } Ensure
	if (g1 > MAX_INTENSITY)	g1 = MAX_INTENSITY;	// } within limits
	if (b1 > MAX_INTENSITY)	b1 = MAX_INTENSITY;	// }

	setTextPal(RGB(r1, g1, b1));
}

/**
 * Get a timer's current count.
 */
int timer(int timerno) {
	return Timer(timerno);
}

/**
 * topplay(film, x, y, actor, hold, complete)
 */
void topplay(CORO_PARAM, SCNHANDLE film, int x, int y, int complete, int actorid, bool splay, int sfact, bool escOn, int myescTime) {
	play(coroParam, film, x, y, complete, actorid, splay, sfact, escOn, myescTime, true);
}

/**
 * Open or close the 'top window'
 */

void topwindow(int bpos) {
	assert(bpos == TW_START || bpos == TW_END);

	switch (bpos) {
	case TW_END:
		KillInventory();
		break;

	case TW_START:
		KillInventory();
		PopUpConf(TOPWIN);
		break;
	}
}

/**
 * unhookscene
 */

void unhookscene(void) {
	UnHookScene();
}

/**
 * Un-define an actor as tagged.
 */

void untagactor(int actor) {
	UnTagActor(actor);
}

/**
 * vibrate
 */

void vibrate(void) {
}

/**
 * waitframe(int actor, int frameNumber)
 */

void waitframe(CORO_PARAM, int actor, int frameNumber, bool escOn, int myescEvent) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);
	// Don't do it if it's not wanted
	if (escOn && myescEvent != GetEscEvents())
		return;

	while (getActorSteps(actor) < frameNumber) {
		CORO_SLEEP(1);

		// Abort if escapable and ESCAPE is pressed
		if (escOn && myescEvent != GetEscEvents())
			break;
	}
	CORO_END_CODE;
}

/**
 * Return when a key pressed or button pushed.
 */

void waitkey(CORO_PARAM, bool escOn, int myescEvent) {
	CORO_BEGIN_CONTEXT;
		int	startEvent;
		int startX, startY;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// Don't do it if it's not wanted
	if (escOn && myescEvent != GetEscEvents())
		return;

	while (1) {
		_ctx->startEvent = getUserEvents();
		// Store cursor position
		while (!GetCursorXYNoWait(&_ctx->startX, &_ctx->startY, false))
			CORO_SLEEP(1);

		while (_ctx->startEvent == getUserEvents()) {
			CORO_SLEEP(1);

			// Not necessary to monitor escape as it's an event anyway

			int curX, curY;
			GetCursorXY(&curX, &curY, false);	// Store cursor position
			if (curX != _ctx->startX || curY != _ctx->startY)
				break;

			if (IsConfWindow())
				break;
		}

		if (!IsConfWindow())
			return;

		do {
			CORO_SLEEP(1);
		} while (IsConfWindow());

		CORO_SLEEP(ONE_SECOND / 2);		// Let it die down
	}
	CORO_END_CODE;
}

/**
 * Pause for requested time.
 */

void waittime(CORO_PARAM, int time, bool frame, bool escOn, int myescEvent) {
	CORO_BEGIN_CONTEXT;
		int time;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);
	// Don't do it if it's not wanted
	if (escOn && myescEvent != GetEscEvents())
		return;

	if (!frame)
		time *= ONE_SECOND;

	_ctx->time = time;
	do {
		CORO_SLEEP(1);

		// Abort if escapable and ESCAPE is pressed
		if (escOn && myescEvent != GetEscEvents())
			break;
	} while (_ctx->time--);
	CORO_END_CODE;
}

/**
 * Set a moving actor off on a walk.
 */
void walk(CORO_PARAM, int actor, int x, int y, SCNHANDLE film, int hold, bool igPath, bool escOn, int myescEvent) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	PMACTOR pActor = GetMover(actor);
	assert(pActor); // Can't walk a non-moving actor

	CORO_BEGIN_CODE(_ctx);

	// Straight there if escaped
	if (escOn && myescEvent != GetEscEvents()) {
		stand(actor, x, y, 0);
		return;
	}

	assert(pActor->hCpath != NOPOLY); // moving actor not in path

	GetToken(pActor->actorToken);
	SetActorDest(pActor, x, y, igPath, film);
	DontScrollCursor();

	if (hold == 2) {
		;
	} else {
		while (MAmoving(pActor)) {
			CORO_SLEEP(1);

			// Straight there if escaped
			if (escOn && myescEvent != GetEscEvents()) {
				stand(actor, x, y, 0);
				FreeToken(pActor->actorToken);
				return;
			}
		}
	}
	FreeToken(pActor->actorToken);
	CORO_END_CODE;
}

/**
 * Set a moving actor off on a walk.
 * Wait to see if its aborted or completed.
 */
void walked(CORO_PARAM, int actor, int x, int y, SCNHANDLE film, bool escOn, int myescEvent, bool &retVal) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
		int	ticket;
	CORO_END_CONTEXT(_ctx);

	PMACTOR pActor = GetMover(actor);
	assert(pActor); // Can't walk a non-moving actor

	CORO_BEGIN_CODE(_ctx);

	// Straight there if escaped
	if (escOn && myescEvent != GetEscEvents()) {
		stand(actor, x, y, 0);
		retVal = true;
		return;
	}

	CORO_SLEEP(ONE_SECOND);

	assert(pActor->hCpath != NOPOLY); // moving actor not in path

	// Briefly aquire token to kill off any other normal walk
	GetToken(pActor->actorToken);
	FreeToken(pActor->actorToken);

	SetActorDest(pActor, x, y, false, film);
	DontScrollCursor();

	_ctx->ticket = GetActorTicket(pActor);

	while (MAmoving(pActor)) {
		CORO_SLEEP(1);

		if (_ctx->ticket != GetActorTicket(pActor)) {
			retVal = false;
			return;
		}

		// Straight there if escaped
		if (escOn && myescEvent != GetEscEvents()) {
			stand(actor, x, y, 0);
			retVal = true;
			return;
		}
	}

	int	endx, endy;
	GetMActorPosition(pActor, &endx, &endy);
	retVal = (_ctx->ticket == GetActorTicket(pActor) && endx == x && endy == y);

	CORO_END_CODE;
}

/**
 * Declare a moving actor.
 */
void walkingactor(uint32 id, SCNHANDLE *rp) {
	PMACTOR	pActor;		// Moving actor structure

	SetMover(id);		// Establish as a moving actor
	pActor = GetMover(id);
	assert(pActor);

	// Store all those reels
	int i, j;
	for (i = 0; i < 5; ++i) {
		for (j = 0; j < 4; ++j)
			pActor->WalkReels[i][j] = *rp++;
		for (j = 0; j < 4; ++j)
			pActor->StandReels[i][j] = *rp++;
	}


	for (i = NUM_MAINSCALES; i < TOTAL_SCALES; i++) {
		for (j = 0; j < 4; ++j) {
			pActor->WalkReels[i][j] = pActor->WalkReels[4][j];
			pActor->StandReels[i][j] = pActor->StandReels[2][j];
		}
	}
}

/**
 * Walk a moving actor towards the polygon's tag, but return when the
 * actor enters the polygon.
 */

void walkpoly(CORO_PARAM, int actor, SCNHANDLE film, HPOLYGON hp, bool escOn, int myescEvent) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	PMACTOR pActor = GetMover(actor);
	assert(pActor); // Can't walk a non-moving actor

	CORO_BEGIN_CODE(_ctx);

	int	aniX, aniY;		// cursor/actor position
	int	pnodex, pnodey;

	assert(hp != NOPOLY); // walkpoly() may only be called from a polygon code block

	// Straight there if escaped
	if (escOn && myescEvent != GetEscEvents()) {
		standtag(actor, hp);
		return;
	}

	GetToken(pActor->actorToken);
	getPolyNode(hp, &pnodex, &pnodey);
	SetActorDest(pActor, pnodex, pnodey, false, film);
	DoScrollCursor();

	do {
		CORO_SLEEP(1);

		if (escOn && myescEvent != GetEscEvents()) {
			// Straight there if escaped
			standtag(actor, hp);
			FreeToken(pActor->actorToken);
			return;
		}

		GetMActorPosition(pActor, &aniX, &aniY);
	} while (!MActorIsInPolygon(pActor, hp) && MAmoving(pActor));

	FreeToken(pActor->actorToken);

	CORO_END_CODE;
}

/**
 * walktag(actor, reel, hold)
 */

void walktag(CORO_PARAM, int actor, SCNHANDLE film, HPOLYGON hp, bool escOn, int myescEvent) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	PMACTOR pActor = GetMover(actor);
	assert(pActor); // Can't walk a non-moving actor

	CORO_BEGIN_CODE(_ctx);

	int	pnodex, pnodey;

	assert(hp != NOPOLY); // walkpoly() may only be called from a polygon code block

	// Straight there if escaped
	if (escOn && myescEvent != GetEscEvents()) {
		standtag(actor, hp);
		return;
	}

	GetToken(pActor->actorToken);
	getPolyNode(hp, &pnodex, &pnodey);
	SetActorDest(pActor, pnodex, pnodey, false, film);
	DoScrollCursor();

	while (MAmoving(pActor)) {
		CORO_SLEEP(1);

		if (escOn && myescEvent != GetEscEvents()) {
			// Straight there if escaped
			standtag(actor, hp);
			FreeToken(pActor->actorToken);
			return;
		}
	}

	// Adopt the tag-related reel
	SCNHANDLE pfilm = getPolyFilm(hp);

	switch (pfilm) {
	case TF_NONE:
		break;

	case TF_UP:
		SetMActorDirection(pActor, AWAY);
		SetMActorStanding(pActor);
		break;
	case TF_DOWN:
		SetMActorDirection(pActor, FORWARD);
		SetMActorStanding(pActor);
		break;
	case TF_LEFT:
		SetMActorDirection(pActor, LEFTREEL);
		SetMActorStanding(pActor);
		break;
	case TF_RIGHT:
		SetMActorDirection(pActor, RIGHTREEL);
		SetMActorStanding(pActor);
		break;

	default:
		if (actor == LEAD_ACTOR || actor == LeadId())
			AlterMActor(pActor, pfilm, AR_NORMAL);
		else
			SetMActorStanding(pActor);
		break;
	}

	FreeToken(pActor->actorToken);
	CORO_END_CODE;
}

/**
 * whichinventory
 */

int whichinventory(void) {
	return WhichInventoryOpen();
}


/**
 * Subtract one less that the number of parameters from pp
 * pp then points to the first parameter.
 *
 * If the library function has no return value:
 * return -(the number of parameters) to pop them from the stack
 *
 * If the library function has a return value:
 * return -(the number of parameters - 1) to pop most of them from
 * the stack, and stick the return value in pp[0]
 * @param operand			Library function
 * @param pp				Top of parameter stack
 */
int CallLibraryRoutine(CORO_PARAM, int operand, int32 *pp, const PINT_CONTEXT pic, RESUME_STATE *pResumeState) {
	debug(7, "CallLibraryRoutine op %d (escOn %d, myescEvent %d)", operand, pic->escOn, pic->myescEvent);
	switch (operand) {
	case ACTORATTR:
		pp -= 3;			// 4 parameters
		actorattr(pp[0], pp[1], pp[2], pp[3]);
		return -4;

	case ACTORDIRECTION:
		pp[0] = actordirection(pp[0]);
		return 0;

	case ACTORREF:
		error("actorref isn't a real function!");

	case ACTORSCALE:
		pp[0] = actorscale(pp[0]);
		return 0;

	case ACTORSON:
		actorson();
		return 0;

	case ACTORXPOS:
		pp[0] = actorpos(ACTORXPOS, pp[0]);
		return 0;

	case ACTORYPOS:
		pp[0] = actorpos(ACTORYPOS, pp[0]);
		return 0;

	case ADDICON:
		addicon(pp[0]);
		return -1;

	case ADDINV1:
		addinv(INV_1, pp[0]);
		return -1;

	case ADDINV2:
		addinv(INV_2, pp[0]);
		return -1;

	case ADDOPENINV:
		addinv(INV_OPEN, pp[0]);
		return -1;

	case AUXSCALE:
		pp -= 13;			// 14 parameters
		auxscale(pp[0], pp[1], (SCNHANDLE *)(pp+2));
		return -14;

	case BACKGROUND:
		background(pp[0]);
		return -1;

	case CAMERA:
		camera(pp[0]);
		return -1;

	case CDLOAD:
		pp -= 1;                // 2 parameters
		cdload(pp[0], pp[1]);
		return -2;

	case CDPLAY:
		error("cdplay isn't a real function!");

	case CLEARHOOKSCENE:
		clearhookscene();
		return 0;

	case CLOSEINVENTORY:
		closeinventory();
		return 0;

	case CONTROL:
		control(pp[0]);
		return -1;

	case CONVERSATION:
		conversation(pp[0], pic->hpoly, pic->escOn, pic->myescEvent);
		return -1;

	case CONVICON:
		convicon(pp[0]);
		return -1;

	case CURSORXPOS:
		pp[0] = cursorpos(CURSORXPOS);
		return 0;

	case CURSORYPOS:
		pp[0] = cursorpos(CURSORYPOS);
		return 0;

	case CUTSCENE:
		error("cutscene isn't a real function!");

	case DEC_CONVW:
		pp -= 7;			// 8 parameters
		dec_convw(pp[0], pp[1], pp[2], pp[3],
			 pp[4], pp[5], pp[6], pp[7]);
		return -8;

	case DEC_CSTRINGS:
		pp -= 19;			// 20 parameters
		dec_cstrings((SCNHANDLE *)pp);
		return -20;

	case DEC_CURSOR:
		dec_cursor(pp[0]);
		return -1;

	case DEC_FLAGS:
		dec_flags(pp[0]);
		return -1;

	case DEC_INV1:
		pp -= 7;			// 8 parameters
		dec_inv1(pp[0], pp[1], pp[2], pp[3],
			 pp[4], pp[5], pp[6], pp[7]);
		return -8;

	case DEC_INV2:
		pp -= 7;			// 8 parameters
		dec_inv2(pp[0], pp[1], pp[2], pp[3],
			 pp[4], pp[5], pp[6], pp[7]);
		return -8;

	case DEC_INVW:
		dec_invw(pp[0]);
		return -1;

	case DEC_LEAD:
		pp -= 61;			// 62 parameters
		dec_lead(pp[0], (SCNHANDLE *)&pp[1], pp[61]);
		return -62;

	case DEC_TAGFONT:
		dec_tagfont(pp[0]);
		return -1;

	case DEC_TALKFONT:
		dec_talkfont(pp[0]);
		return -1;

	case DELICON:
		delicon(pp[0]);
		return -1;

	case DELINV:
		delinv(pp[0]);
		return -1;

	case EFFECTACTOR:
		assert(pic->event == ENTER || pic->event == LEAVE); // effectactor() must be from effect poly code

		pp[0] = pic->actorid;
		return 0;

	case ENABLEF1:
		enablef1();
		return 0;

	case EVENT:
		pp[0] = pic->event;
		return 0;

	case FADEMIDI:
		fademidi(coroParam, pp[0]);
		return -1;

	case FRAMEGRAB:
		return -1;

	case GETINVLIMIT:
		pp[0] = getinvlimit(pp[0]);
		return 0;

	case HASRESTARTED:
		pp[0] = hasrestarted();
		return 0;

	case HELDOBJECT:
		pp[0] = heldobject();
		return 0;

	case HIDE:
		hide(pp[0]);
		return -1;

	case HOOKSCENE:
		pp -= 2;			// 3 parameters
		hookscene(pp[0], pp[1], pp[2]);
		return -3;

	case IDLETIME:
		pp[0] = idletime();
		return 0;

	case ININVENTORY:
		pp[0] = ininventory(pp[0]);
		return 0;			// using return value

	case INVDEPICT:
		pp -= 1;			// 2 parameters
		invdepict(pp[0], pp[1]);
		return -2;

	case INVENTORY:
		inventory(pp[0], pic->escOn, pic->myescEvent);
		return -1;

	case INWHICHINV:
		pp[0] = inwhichinv(pp[0]);
		return 0;			// using return value

	case KILLACTOR:
		killactor(pp[0]);
		return -1;

	case KILLBLOCK:
		killblock(pp[0]);
		return -1;

	case KILLEXIT:
		killexit(pp[0]);
		return -1;

	case KILLTAG:
		killtag(pp[0]);
		return -1;

	case LEFTOFFSET:
		pp[0] = ltoffset(LEFTOFFSET);
		return 0;

	case MOVECURSOR:
		pp -= 1;			// 2 parameters
		movecursor(pp[0], pp[1]);
		return -2;

	case NEWSCENE:
		pp -= 2;			// 3 parameters
		if (*pResumeState == RES_2)
			*pResumeState = RES_NOT;
		else
			newscene(coroParam, pp[0], pp[1], pp[2]);
		return -3;

	case NOBLOCKING:
		noblocking();
		return 0;

	case NOSCROLL:
		pp -= 3;			// 4 parameters
		noscroll(pp[0], pp[1], pp[2], pp[3]);
		return -4;

	case OBJECTHELD:
		objectheld(pp[0]);
		return -1;

	case OFFSET:
		pp -= 1;			// 2 parameters
		offset(pp[0], pp[1]);
		return -2;

	case PLAY:
		pp -= 5;			// 6 parameters

		if (pic->event == ENTER || pic->event == LEAVE)
			play(coroParam, pp[0], pp[1], pp[2], pp[5], 0, false, 0, pic->escOn, pic->myescEvent, false);
		else
			play(coroParam, pp[0], pp[1], pp[2], pp[5], pic->actorid, false, 0, pic->escOn, pic->myescEvent, false);
		return -6;

	case PLAYMIDI:
		pp -= 2;			// 3 parameters
		playmidi(coroParam, pp[0], pp[1], pp[2]);
		return -3;

	case PLAYRTF:
		error("playrtf only applies to cdi!");

	case PLAYSAMPLE:
		pp -= 1;			// 2 parameters
		playsample(coroParam, pp[0], pp[1], pic->escOn, pic->myescEvent);
		return -2;

	case PREPARESCENE:
		preparescene(pp[0]);
		return -1;

	case PRINT:
		pp -= 5;			// 6 parameters
		/* pp[2] was intended to be attribute */
		print(coroParam, pp[0], pp[1], pp[3], pp[4], pp[5], pic->escOn, pic->myescEvent);
		return -6;

	case PRINTOBJ:
		printobj(coroParam, pp[0], pic->pinvo, pic->event);
		return -1;

	case PRINTTAG:
		printtag(pic->hpoly, pp[0]);
		return -1;

	case QUITGAME:
		quitgame();
		return 0;

	case RANDOM:
		pp -= 2;			// 3 parameters
		pp[0] = dw_random(pp[0], pp[1], pp[2]);
		return -2;		// One holds return value

	case RESETIDLETIME:
		resetidletime();
		return 0;

	case RESTARTGAME:
		restartgame();
		return 0;

	case RESTORE_CUT:
		restore_scene(false);
		return 0;

	case RESTORE_SCENE:
		restore_scene(true);
		return 0;

	case RUNMODE:
		pp[0] = runmode();
		return 0;

	case SAMPLEPLAYING:
		pp[0] = sampleplaying(pic->escOn, pic->myescEvent);
		return 0;

	case SAVE_SCENE:
		if (*pResumeState == RES_1)
			*pResumeState = RES_2;
		else
			save_scene(coroParam);
		return 0;

	case SCALINGREELS:
		pp -= 6;			// 7 parameters
		scalingreels(pp[0], pp[1], pp[2], pp[3], pp[4], pp[5], pp[6]);
		return -7;

	case SCANICON:
		pp[0] = scanicon();
		return 0;

	case SCROLL:
		pp -= 3;			// 4 parameters
		scroll(coroParam, pp[0], pp[1], pp[2], pp[3], pic->escOn, pic->myescEvent);
		return -4;

	case SETACTOR:
		setactor(pp[0]);
		return -1;

	case SETBLOCK:
		setblock(pp[0]);
		return -1;

	case SETEXIT:
		setexit(pp[0]);
		return -1;

	case SETINVLIMIT:
		pp -= 1;			// 2 parameters
		setinvlimit(pp[0], pp[1]);
		return -2;

	case SETINVSIZE:
		pp -= 6;			// 7 parameters
		setinvsize(pp[0], pp[1], pp[2], pp[3], pp[4], pp[5], pp[6]);
		return -7;

	case SETLANGUAGE:
		setlanguage((LANGUAGE)pp[0]);
		return -1;

	case SETPALETTE:
		setpalette(pp[0], pic->escOn, pic->myescEvent);
		return -1;

	case SETTAG:
		settag(pp[0]);
		return -1;

	case SETTIMER:
		pp -= 3;			// 4 parameters
		settimer(pp[0], pp[1], pp[2], pp[3]);
		return -4;

	case SHOWPOS:
#ifdef DEBUG
		showpos();
#endif
		return 0;

	case SHOWSTRING:
#ifdef DEBUG
		showstring();
#endif
		return 0;

	case SPLAY:
		pp -= 6;			// 7 parameters

		if (pic->event == ENTER || pic->event == LEAVE)
			splay(coroParam, pp[0], pp[1], pp[2], pp[3], pp[6], 0, pic->escOn, pic->myescEvent);
		else
			splay(coroParam, pp[0], pp[1], pp[2], pp[3], pp[6], pic->actorid, pic->escOn, pic->myescEvent);
		return -7;

	case STAND:
		pp -= 3;			// 4 parameters
		stand(pp[0], pp[1], pp[2], pp[3]);
		return -4;

	case STANDTAG:
		standtag(pp[0], pic->hpoly);
		return -1;

	case STOP:
		stop(pp[0]);
		return -1;

	case STOPMIDI:
		stopmidi();
		return 0;

	case STOPSAMPLE:
		stopsample();
		return 0;

	case SUBTITLES:
		subtitles(pp[0]);
		return -1;

	case SWALK:
		pp -= 5;			// 6 parameters
		swalk(coroParam, pp[0], pp[1], pp[2], pp[3], pp[4], pp[5], pic->escOn, pic->myescEvent);
		return -6;

	case TAGACTOR:
		pp -= 2;			// 3 parameters
		tagactor(pp[0], pp[1], pp[2]);
		return -3;

	case TALK:
		pp -= 1;			// 2 parameters

		if (pic->event == ENTER || pic->event == LEAVE)
			talk(coroParam, pp[0], pp[1], 0, pic->escOn, pic->myescEvent);
		else
			talk(coroParam, pp[0], pp[1], pic->actorid, pic->escOn, pic->myescEvent);
		return -2;

	case TALKAT:
		pp -= 3;			// 4 parameters
		talkat(coroParam, pp[0], pp[1], pp[2], pp[3], pic->escOn, pic->myescEvent);
		return -4;

	case TALKATS:
		pp -= 4;			// 5 parameters
		talkats(coroParam, pp[0], pp[1], pp[2], pp[3], pp[4], pic->escOn, pic->myescEvent);
		return -5;

	case TALKATTR:
		pp -= 2;			// 3 parameters
		talkattr(pp[0], pp[1], pp[2], pic->escOn, pic->myescEvent);
		return -3;

	case TIMER:
		pp[0] = timer(pp[0]);
		return 0;

	case TOPOFFSET:
		pp[0] = ltoffset(TOPOFFSET);
		return 0;

	case TOPPLAY:
		pp -= 5;			// 6 parameters
		topplay(coroParam, pp[0], pp[1], pp[2], pp[5], pic->actorid, false, 0, pic->escOn, pic->myescEvent);
		return -6;

	case TOPWINDOW:
		topwindow(pp[0]);
		return -1;

	case TRYPLAYSAMPLE:
		pp -= 1;			// 2 parameters
		tryplaysample(coroParam, pp[0], pp[1], pic->escOn, pic->myescEvent);
		return -2;

	case UNHOOKSCENE:
		unhookscene();
		return 0;

	case UNTAGACTOR:
		untagactor(pp[0]);
		return -1;

	case VIBRATE:
		vibrate();
		return 0;

	case WAITKEY:
		waitkey(coroParam, pic->escOn, pic->myescEvent);
		return 0;

	case WAITFRAME:
		pp -= 1;			// 2 parameters
		waitframe(coroParam, pp[0], pp[1], pic->escOn, pic->myescEvent);
		return -2;

	case WAITTIME:
		pp -= 1;			// 2 parameters
		waittime(coroParam, pp[0], pp[1], pic->escOn, pic->myescEvent);
		return -2;

	case WALK:
		pp -= 4;			// 5 parameters
		walk(coroParam, pp[0], pp[1], pp[2], pp[3], pp[4], false, pic->escOn, pic->myescEvent);
		return -5;

	case WALKED: {
		pp -= 3;			// 4 parameters
		bool tmp;
		walked(coroParam, pp[0], pp[1], pp[2], pp[3], pic->escOn, pic->myescEvent, tmp);
		if (!coroParam) {
			// Only write the result to the stack if walked actually completed running.
			pp[0] = tmp;
		}
		}
		return -3;

	case WALKINGACTOR:
		pp -= 40;			// 41 parameters
		walkingactor(pp[0], (SCNHANDLE *)&pp[1]);
		return -41;

	case WALKPOLY:
		pp -= 2;			// 3 parameters
		walkpoly(coroParam, pp[0], pp[1], pic->hpoly, pic->escOn, pic->myescEvent);
		return -3;

	case WALKTAG:
		pp -= 2;			// 3 parameters
		walktag(coroParam, pp[0], pp[1], pic->hpoly, pic->escOn, pic->myescEvent);
		return -3;

	case WHICHINVENTORY:
		pp[0] = whichinventory();
		return 0;

	default:
		error("Unsupported library function");
	}

	error("Can't possibly get here");
}


} // end of namespace Tinsel
