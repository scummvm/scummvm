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

#ifndef	_SPEECH
#define	_SPEECH

#include "header.h"

namespace Sword2 {

#define	MAX_SUBJECT_LIST 30	// is that enough?

// array of these for subject menu build up
typedef	struct {
	uint32 res;
	uint32 ref;
} _subject_unit;

// so speech text cleared when running a new start-script
extern uint32 speech_text_bloc_no;

extern int16 officialTextNumber;

extern int32 speechScriptWaiting;

//could alternately use logic->looping of course
extern	int choosing;

extern	uint32 unpause_zone;

} // End of namespace Sword2

#endif
