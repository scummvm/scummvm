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

// max number of fx in queue at once [DO NOT EXCEED 255]
#define FXQ_LENGTH 32

namespace Sword2 {

// fx types

enum {
	FX_SPOT		= 0,
	FX_LOOP		= 1,
	FX_RANDOM	= 2,
	FX_SPOT2	= 3
};

} // End of namespace Sword2

#endif
