/* Copyright (C) 1994-2003 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

/*****************************************************************************
 *		SOUND.H		Sound engine
 *
 *		SOUND.CPP Contains the sound engine, fx & music functions
 *		Some very 'sound' code in here ;)
 *
 *		(16Dec96 JEL)
 *
 ****************************************************************************/

#ifndef SOUND_H
#define SOUND_H

#include "common/scummsys.h"

// fx types

#define FX_SPOT		0
#define FX_LOOP		1
#define FX_RANDOM	2
#define FX_SPOT2	3

// to be called during system initialisation
void Init_fx_queue(void);

// to be called from the main loop, once per cycle
void Process_fx_queue(void);

// stops all fx & clears the queue - eg. when leaving a location
void Clear_fx_queue(void);

void PauseAllSound(void);
void UnpauseAllSound(void);

void Kill_music(void);

int32 FN_play_music(int32 *params);		// for save_Rest.cpp
int32 FN_stop_music(int32 *params);

// used to store id of tunes that loop, for save & restore
extern uint32 looping_music_id;

#endif
