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

#include "saga2/std.h"
#include "saga2/fta.h"
#include "saga2/audio.h"

#include "saga2/queues.h"
#include "saga2/idtypes.h"
#include "saga2/audiosmp.h"
#include "saga2/audqueue.h"
#include "saga2/audiosys.h"

#include "saga2/savefile.h"
namespace Saga2 {

/* ====================================================================== *
   Exports
 * ====================================================================== */

volatile int32      gameTime;

//#ifndef WINKLUDGE
extern audioInterface *audio;
//#endif

/* ====================================================================== *
   Locals
 * ====================================================================== */

//static HTIMER     gameTimer;
//static int16          suspendTimerLevel;

/* ====================================================================== *
   Timer Interrupt Hook
 * ====================================================================== */



void initTimer(void) {
	//  Our game master clock is based off of the AIL
	//  (Audio Interface Library) timing services

	//gameTimer = AIL_register_timer( timerHookFunc );
	//AIL_set_timer_frequency( gameTimer, ticksPerSecond );
	//AIL_start_timer( gameTimer );

	gameTime = 0;
}

void saveTimer(SaveFileConstructor &saveGame) {
	int32   time = gameTime;

	saveGame.writeChunk(
	    MakeID('T', 'I', 'M', 'E'),
	    &time,
	    sizeof(time));
}

void loadTimer(SaveFileReader &saveGame) {
	int32   time;

	saveGame.read(&time, sizeof(time));
	gameTime = time;
}

/*
void cleanupTimer( void )
{
    //  Nothing to do, actually... AIL_Shutdown takes care of it...
}
*/

/* ====================================================================== *
   Timer Control
 * ====================================================================== */

void pauseTimer(void) {
	if (audio)
		audio->suspendGameClock();
}

void resumeTimer(void) {
	if (audio)
		audio->resumeGameClock();
}

/* ====================================================================== *
   Alarms
 * ====================================================================== */

void Alarm::set(uint32 dur) {
	basetime = gameTime;
	duration = dur;
}

bool Alarm::check(void) {
	return ((uint32)(gameTime - basetime) > duration);
}

// time elapsed since alarm set

uint32 Alarm::elapsed(void) {
	return (uint32)(gameTime - basetime);
}

} // end of namespace Saga2
